/*
 * ExtendedFSMComponent.cpp
 *
 * Created on: 2025.2
 * Authors: André Rocco, Samantha Costa
 */

#include "ExtendedFSMComponent.h"
#include "FSMTransition.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <regex>

ExtendedFSMComponent::ExtendedFSMComponent(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<ExtendedFSMComponent>(), name) {
}

ExtendedFSMComponent::~ExtendedFSMComponent() {
}

std::string ExtendedFSMComponent::show() {
    return ModelComponent::show() + ",initialState=" + (_initialState ? _initialState->getName() : "") + ",currentState=" + (_currentState ? _currentState->getName() : "");
}

ModelDataDefinition* ExtendedFSMComponent::NewInstance(Model* model, std::string name) {
    return new ExtendedFSMComponent(model, name);
}

ModelDataDefinition* ExtendedFSMComponent::LoadInstance(Model* model, PersistenceRecord *fields) {
    ExtendedFSMComponent* newElement = new ExtendedFSMComponent(model);
    try {
        newElement->_loadInstance(fields);
    } catch (const std::exception& e) {

    }
    return newElement;
}

PluginInformation* ExtendedFSMComponent::GetPluginInformation() {
    PluginInformation* info = new PluginInformation(Util::TypeOf<ExtendedFSMComponent>(), &ExtendedFSMComponent::LoadInstance, &ExtendedFSMComponent::NewInstance);
    info->setDescriptionHelp("Extended Finite State Machine (EFSM) component.");
    return info;
}

bool ExtendedFSMComponent::_loadInstance(PersistenceRecord *fields) {
    bool res = ModelComponent::_loadInstance(fields);
    if (res) {
        std::string initialStateName = fields->loadField("initialState", "");
        if (!initialStateName.empty()) {
            _initialState = dynamic_cast<FSMState*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), initialStateName));
            _currentState = _initialState;
        }
    }
    return res;
}

void ExtendedFSMComponent::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
    ModelComponent::_saveInstance(fields, saveDefaultValues);
    if (_initialState) {
        fields->saveField("initialState", _initialState->getName(), "", saveDefaultValues);
    }
}

bool ExtendedFSMComponent::_check(std::string* errorMessage) {
    if (!_initialState) {
        *errorMessage = "Initial state not defined";
        return false;
    }
    return true;
}

void ExtendedFSMComponent::_createInternalAndAttachedData() {
    // Initialize current state if not set
    if (!_currentState && _initialState) {
        _currentState = _initialState;
    }
    // Add states to internal data to prevent orphan removal
    for (auto const& pair : _states) {
        _internalDataInsert(pair.first, pair.second);
        // Add transitions of this state
        for (FSMTransition* trans : *pair.second->getTransitions()) {
            _internalDataInsert(trans->getName(), trans);
        }
    }
}

void ExtendedFSMComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
    if (!_currentState) {
        _currentState = _initialState;
    }

    traceSimulation(this, "Entity " + entity->getName() + " arrived at state " + _currentState->getName());

    // Fire Phase: Evaluate guards and choose transition
    FSMTransition* selectedTransition = nullptr;
    std::list<FSMTransition*>* transitions = _currentState->getTransitions();
    
    if (transitions) {
        // Sort transitions by priority if needed (omitted for now)
        for (FSMTransition* trans : *transitions) {
            if (evaluateGuard(trans->getGuard(), entity)) {
                selectedTransition = trans;
                break;
            }
        }
    
        if (!selectedTransition) {
            for (FSMTransition* trans : *transitions) {
                if (trans->isDefault()) {
                    selectedTransition = trans;
                    break;
                }
            }
        }
    }

    if (selectedTransition) {
        traceSimulation(this, "Transition selected: " + selectedTransition->getName());
        
        // Execute Output Actions
        executeOutputActions(entity, selectedTransition->getOutputActions());
        
        // Postfire Phase: Update State and Variables
        executeSetActions(selectedTransition->getSetActions());
        
        _currentState = selectedTransition->getDestination();
        traceSimulation(this, "Moved to state " + _currentState->getName());
        
        // Safety check before sending
        if (this->getConnectionManager()->size() > 0) {
            _parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection(), 0.0);
        } else {
            traceSimulation(this, "Warning: Transition done but no output connection. Entity remains in component.");
        }

    } else {
        traceSimulation(this, "No transition enabled. Entity stuck in state " + _currentState->getName());
        
        // Safety check before sending (or keep stuck)
        if (this->getConnectionManager()->size() > 0) {
            _parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection(), 0.0);
        }
    }
}

FSMState* ExtendedFSMComponent::createState(std::string name, bool isFinal) {
    FSMState* state = new FSMState(_parentModel, name);
    state->setFinal(isFinal);
    _states[name] = state;
    return state;
}

void ExtendedFSMComponent::createVariable(std::string name, double initialValue) {
    _variables[name] = initialValue;
}

void ExtendedFSMComponent::setInitialState(FSMState* state) {
    _initialState = state;
    _currentState = state;
}

FSMState* ExtendedFSMComponent::getInitialState() const {
    return _initialState;
}

FSMState* ExtendedFSMComponent::getCurrentState() const {
    return _currentState;
}

bool ExtendedFSMComponent::evaluateGuard(std::string guard, Entity* entity) {
    if (guard.empty()) return true; 
    
    std::string safeGuard = guard;
    std::map<std::string, double> entityVars;
    
    // Handle Entity Attributes: Replace Entity.Attr with Entity_Attr and fetch values
    std::regex attrRegex("Entity\\.([a-zA-Z0-9_]+)");
    auto words_begin = std::sregex_iterator(guard.begin(), guard.end(), attrRegex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string suffix = match[1].str(); 
        std::string safeName = "Entity_" + suffix;
        entityVars[safeName] = entity->getAttributeValue(suffix);
    }
    
    // Replace in string
    safeGuard = std::regex_replace(guard, attrRegex, "Entity_$1");

    // Prepare variables for tinyexpr
    std::vector<te_variable> vars;
    for (auto const& pair : _variables) {
        vars.push_back({pair.first.c_str(), &_variables.at(pair.first), TE_VARIABLE, 0});
    }
    
    // Add Entity variables
    for (auto& pair : entityVars) {
        vars.push_back({pair.first.c_str(), &pair.second, TE_VARIABLE, 0});
    }
    
    int err;
    te_expr* expr = te_compile(safeGuard.c_str(), vars.data(), vars.size(), &err);
    
    if (expr) {
        double result = te_eval(expr);
        te_free(expr);
        return result != 0.0;
    } else {
        traceSimulation(this, "Error parsing guard: " + guard + " (safe: " + safeGuard + ") Error at: " + std::to_string(err));
        return false;
    }
}

void ExtendedFSMComponent::executeSetActions(std::string actions) {
    if (actions.empty()) return;
    
    std::map<std::string, std::string> actionMap = parseActions(actions);
    
    for (auto const& actionPair : actionMap) {
        std::string varName = actionPair.first;
        std::string exprStr = actionPair.second;
        
        if (_variables.find(varName) != _variables.end()) {
             // Prepare variables for tinyexpr
            std::vector<te_variable> vars;
            for (auto const& pair : _variables) {
                vars.push_back({pair.first.c_str(), &(_variables.at(pair.first)), TE_VARIABLE, 0});
            }
            
            int err;
            te_expr* expr = te_compile(exprStr.c_str(), vars.data(), vars.size(), &err);
            if (expr) {
                double result = te_eval(expr);
                _variables[varName] = result;
                te_free(expr);
                traceSimulation(this, "Updated variable " + varName + " to " + std::to_string(result));
            } else {
                traceSimulation(this, "Error parsing set action expression: " + exprStr);
            }
        } else {
             traceSimulation(this, "Variable not found: " + varName);
        }
    }
}

void ExtendedFSMComponent::executeOutputActions(Entity* entity, std::string actions) {
    // Similar to set actions but for entity attributes or other outputs
    // For now, let's implement simple attribute setting: "attr=val"
    if (actions.empty()) return;
    
    std::map<std::string, std::string> actionMap = parseActions(actions);
    
     for (auto const& actionPair : actionMap) {
         std::string attrName = actionPair.first;
         std::string exprStr = actionPair.second;
         
         std::vector<te_variable> vars;
        for (auto const& pair : _variables) {
            vars.push_back({pair.first.c_str(), &(_variables.at(pair.first)), TE_VARIABLE, 0});
        }
        
        int err;
        te_expr* expr = te_compile(exprStr.c_str(), vars.data(), vars.size(), &err);
        if (expr) {
            double result = te_eval(expr);
            entity->setAttributeValue(attrName, result);
            te_free(expr);
            traceSimulation(this, "Set entity attribute " + attrName + " to " + std::to_string(result));
        }
     }
}

std::map<std::string, std::string> ExtendedFSMComponent::parseActions(std::string actions) {
    std::map<std::string, std::string> result;
    std::stringstream ss(actions);
    std::string segment;
    while (std::getline(ss, segment, ',')) {
        size_t eqPos = segment.find('=');
        if (eqPos != std::string::npos) {
            std::string key = segment.substr(0, eqPos);
            std::string val = segment.substr(eqPos + 1);
            
            size_t first = key.find_first_not_of(" \t");
            if (first != std::string::npos) key.erase(0, first);
            
            size_t last = key.find_last_not_of(" \t");
            if (last != std::string::npos) key.erase(last + 1);
            
            first = val.find_first_not_of(" \t");
            if (first != std::string::npos) val.erase(0, first);
            
            last = val.find_last_not_of(" \t");
            if (last != std::string::npos) val.erase(last + 1);
            
            result[key] = val;
        }
    }
    return result;
}
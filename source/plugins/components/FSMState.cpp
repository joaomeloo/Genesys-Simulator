/*
 * FSMState.cpp
 *
 * Created on: 2025.2
 * Authors: André Rocco, Samantha Costa
 */

#include "FSMState.h"
#include "../../kernel/simulator/Model.h"

FSMState::FSMState(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<FSMState>(), name) {
}

FSMState::~FSMState() {
}

std::string FSMState::show() {
    return ModelDataDefinition::show() + ",isFinal=" + std::to_string(_isFinal);
}

ModelDataDefinition* FSMState::NewInstance(Model* model, std::string name) {
    return new FSMState(model, name);
}

ModelDataDefinition* FSMState::LoadInstance(Model* model, PersistenceRecord *fields) {
    FSMState* newElement = new FSMState(model);
    try {
        newElement->_loadInstance(fields);
    } catch (const std::exception& e) {

    }
    return newElement;
}

PluginInformation* FSMState::GetPluginInformation() {
    PluginInformation* info = new PluginInformation(Util::TypeOf<FSMState>(), &FSMState::LoadInstance, &FSMState::NewInstance);
    return info;
}

bool FSMState::_loadInstance(PersistenceRecord *fields) {
    bool res = ModelDataDefinition::_loadInstance(fields);
    if (res) {
        _isFinal = fields->loadField("isFinal", false);
    }
    return res;
}

void FSMState::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
    ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
    fields->saveField("isFinal", _isFinal, false, saveDefaultValues);
}

bool FSMState::_check(std::string* errorMessage) {
    return true;
}

void FSMState::enter() {
    // Logic for entering state (if any)
}

void FSMState::exit() {
    // Logic for exiting state (if any)
}

std::list<FSMTransition*>* FSMState::getTransitions() const {
    return _transitions;
}

void FSMState::setFinal(bool isFinal) {
    _isFinal = isFinal;
}

bool FSMState::isFinal() const {
    return _isFinal;
}

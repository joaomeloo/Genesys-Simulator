/*
 * FSMTransition.cpp
 *
 * Created on: 2025.2
 * Authors: André Rocco, Samantha Costa
 */

#include "FSMTransition.h"
#include "../../kernel/simulator/Model.h"

FSMTransition::FSMTransition(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<FSMTransition>(), name) {
}

FSMTransition::~FSMTransition() {
}

std::string FSMTransition::show() {
    return ModelDataDefinition::show() + ",guard=" + _guard + ",source=" + (_source ? _source->getName() : "") + ",destination=" + (_destination ? _destination->getName() : "");
}

ModelDataDefinition* FSMTransition::NewInstance(Model* model, std::string name) {
    return new FSMTransition(model, name);
}

ModelDataDefinition* FSMTransition::LoadInstance(Model* model, PersistenceRecord *fields) {
    FSMTransition* newElement = new FSMTransition(model);
    try {
        newElement->_loadInstance(fields);
    } catch (const std::exception& e) {

    }
    return newElement;
}

PluginInformation* FSMTransition::GetPluginInformation() {
    PluginInformation* info = new PluginInformation(Util::TypeOf<FSMTransition>(), &FSMTransition::LoadInstance, &FSMTransition::NewInstance);
    return info;
}

bool FSMTransition::_loadInstance(PersistenceRecord *fields) {
    bool res = ModelDataDefinition::_loadInstance(fields);
    if (res) {
        _guard = fields->loadField("guard", "");
        _outputActions = fields->loadField("outputActions", "");
        _setActions = fields->loadField("setActions", "");
        _priority = fields->loadField("priority", 0);
        _isDefault = fields->loadField("isDefault", false);
        
        std::string sourceName = fields->loadField("source", "");
        if (!sourceName.empty()) {
            _source = dynamic_cast<FSMState*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), sourceName));
        }
        
        std::string destName = fields->loadField("destination", "");
        if (!destName.empty()) {
            _destination = dynamic_cast<FSMState*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), destName));
        }
    }
    return res;
}

void FSMTransition::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
    ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
    fields->saveField("guard", _guard, "", saveDefaultValues);
    fields->saveField("outputActions", _outputActions, "", saveDefaultValues);
    fields->saveField("setActions", _setActions, "", saveDefaultValues);
    fields->saveField("priority", _priority, 0, saveDefaultValues);
    fields->saveField("isDefault", _isDefault, false, saveDefaultValues);
    
    if (_source) {
        fields->saveField("source", _source->getName(), "", saveDefaultValues);
    }
    if (_destination) {
        fields->saveField("destination", _destination->getName(), "", saveDefaultValues);
    }
}

bool FSMTransition::_check(std::string* errorMessage) {
    if (!_source) {
        *errorMessage = "Source state not defined";
        return false;
    }
    if (!_destination) {
        *errorMessage = "Destination state not defined";
        return false;
    }
    return true;
}

void FSMTransition::setGuard(std::string guard) {
    _guard = guard;
}

std::string FSMTransition::getGuard() const {
    return _guard;
}

void FSMTransition::setSource(FSMState* source) {
    _source = source;
    if (_source) {
        _source->getTransitions()->push_back(this);
    }
}

FSMState* FSMTransition::getSource() const {
    return _source;
}

void FSMTransition::setDestination(FSMState* destination) {
    _destination = destination;
}

FSMState* FSMTransition::getDestination() const {
    return _destination;
}

void FSMTransition::setOutputActions(std::string outputActions) {
    _outputActions = outputActions;
}

std::string FSMTransition::getOutputActions() const {
    return _outputActions;
}

void FSMTransition::setSetActions(std::string setActions) {
    _setActions = setActions;
}

std::string FSMTransition::getSetActions() const {
    return _setActions;
}

void FSMTransition::setPriority(int priority) {
    _priority = priority;
}

int FSMTransition::getPriority() const {
    return _priority;
}

bool FSMTransition::isDefault() const {
    return _isDefault;
}

void FSMTransition::setDefault(bool isDefault) {
    _isDefault = isDefault;
}

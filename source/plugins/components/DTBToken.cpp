#include "DTBToken.h"
#include <iostream>

DTBToken::DTBToken(Model* model, std::string name) 
    : ModelComponent(model, Util::TypeOf<DTBToken>(), name), _value("default"), _lifeTime(0.0) {}

void DTBToken::setValue(const std::string& value) {
    _value = value;
}

std::string DTBToken::getValue() const {
    return _value;
}

void DTBToken::setLifeTime(double time) {
    _lifeTime = time;
}

double DTBToken::getLifeTime() const {
    return _lifeTime;
}

std::string DTBToken::show() {
    return "DTBToken - Value: " + _value + ", LifeTime: " + std::to_string(_lifeTime);
}

void DTBToken::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
    if (_lifeTime > 0.0) {
        _lifeTime -= 1.0;
    }
    std::cout << "DTBToken dispatched! Entity: " << entity->getName() 
              << ", LifeTime: " << _lifeTime << std::endl;
}

PluginInformation* DTBToken::GetPluginInformation() {
    PluginInformation* info = new PluginInformation(Util::TypeOf<DTBToken>(), &DTBToken::LoadInstance, &DTBToken::NewInstance);
    info->setDescriptionHelp("DTBToken represents a token in a colored Petri Net. Tokens carry data, which can be used in transitions.");
    return info;
}

ModelComponent* DTBToken::LoadInstance(Model* model, PersistenceRecord *fields) {
    DTBToken* newComponent = new DTBToken(model);
    newComponent->_loadInstance(fields);
    return newComponent;
}

ModelDataDefinition* DTBToken::NewInstance(Model* model, std::string name) {
    return new DTBToken(model, name);
}

#include "Place.h"
#include "DTBToken.h"

Place::Place(Model* model, std::string name) 
    : ModelComponent(model, Util::TypeOf<Place>(), name) {}

void Place::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
    if (this->getTokenCount() < Place::MAX_CAPACITY) {
        DTBToken* token = dynamic_cast<DTBToken*>(entity); 

        if (token) {
            this->addToken(token);
        } 

    } else {
        
    }
}

void Place::addToken(DTBToken* token) {
    _tokens.push_back(token);
}

void Place::removeToken(DTBToken* token) {
    _tokens.remove(token);
}

size_t Place::getTokenCount() const {
    return _tokens.size();
}

std::string Place::show() {
    return ModelComponent::show() + ", tokensCount=" + std::to_string(_tokens.size()) 
           + ", maxCapacity=" + std::to_string(Place::MAX_CAPACITY);
}

PluginInformation* Place::GetPluginInformation() {
    PluginInformation* info = new PluginInformation(Util::TypeOf<Place>(), &Place::LoadInstance, &Place::NewInstance);
    info->setDescriptionHelp("Place represents a place in a Petri Net, storing tokens up to a max capacity of 30.");
    return info;
}

ModelComponent* Place::LoadInstance(Model* model, PersistenceRecord *fields) {
    Place* newComponent = new Place(model);
    newComponent->_loadInstance(fields);
    return newComponent;
}

ModelDataDefinition* Place::NewInstance(Model* model, std::string name) {
    return new Place(model, name);
}
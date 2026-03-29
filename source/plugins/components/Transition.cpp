#include "Transition.h"

Transition::Transition(Model* model, std::string name) 
    : ModelComponent(model, Util::TypeOf<Transition>(), name) {}

void Transition::addInputPlace(Place* place) {
    _inputPlaces.push_back(place);
}

void Transition::addOutputPlace(Place* place) {
    _outputPlaces.push_back(place);
}

void Transition::fire() {
    bool canFire = true;
    for (Place* p : _inputPlaces) {
        if (p->getTokenCount() == 0) {
            canFire = false;
            break;
        }
    }

    if (canFire) {
        for (Place* p : _inputPlaces) {
            DTBToken* token = p->getTokenCount() > 0 ? p->getToken() : nullptr;
            if (token) p->removeToken(token);
        }

        for (Place* p : _outputPlaces) {
            DTBToken* newToken = new DTBToken(_parentModel, "GeneratedToken"); 
            p->addToken(newToken);
			
			this->getConnectionManager()->insert(newToken);
			newToken->getConnectionManager()->insert(p);
        }
    }
}

std::string Transition::show() {
    return ModelComponent::show() + ", inputPlaces=" + std::to_string(_inputPlaces.size()) + ", outputPlaces=" + std::to_string(_outputPlaces.size());
}

PluginInformation* Transition::GetPluginInformation() {
    PluginInformation* info = new PluginInformation(Util::TypeOf<Transition>(), &Transition::LoadInstance, &Transition::NewInstance);
    info->setDescriptionHelp("Transition represents a transition in a Petri Net, consuming tokens from input places and generating tokens in output places.");
    return info;
}

ModelComponent* Transition::LoadInstance(Model* model, PersistenceRecord *fields) {
    Transition* newComponent = new Transition(model);
    newComponent->_loadInstance(fields);
    return newComponent;
}

ModelDataDefinition* Transition::NewInstance(Model* model, std::string name) {
    return new Transition(model, name);
}

void Transition::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
    this->fire(); 
}
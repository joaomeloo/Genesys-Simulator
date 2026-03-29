#ifndef TRANSITION_H
#define TRANSITION_H

#include <vector>
#include "Place.h"
#include "DTBToken.h"
#include "../../kernel/simulator/ModelComponent.h"

class Transition : public ModelComponent {
public:
    Transition(Model* model, std::string name = "");
    virtual ~Transition() = default;

    void addInputPlace(Place* place);
    void addOutputPlace(Place* place);
    
    void fire();
    virtual std::string show() override;
    
    static PluginInformation* GetPluginInformation();
    static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
    virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;

private:
    std::vector<Place*> _inputPlaces;
    std::vector<Place*> _outputPlaces;
};

#endif

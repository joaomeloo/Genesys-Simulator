/*
 * Smart_ExtendedFSM.cpp
 *
 * Created on: 2025.2
 * Authors: André Rocco, Samantha Costa
 */

#include "Smart_Old_ExtendedFSM.h"


// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/ExtendedFSMComponent.h"
#include "../../../../plugins/components/FSMState.h"
#include "../../../../plugins/components/FSMTransition.h"
#include <iostream>

Smart_Old_ExtendedFSM::Smart_Old_ExtendedFSM() {
}

int Smart_Old_ExtendedFSM::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    this->setDefaultTraceHandlers(genesys->getTraceManager());
    genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L9_mostDetailed);

    Model* model = genesys->getModelManager()->newModel();
    PluginManager* plugins = genesys->getPluginManager();
    
    // Register Plugins
    plugins->insert(new Plugin(&ExtendedFSMComponent::GetPluginInformation));
    plugins->insert(new Plugin(&FSMState::GetPluginInformation));
    plugins->insert(new Plugin(&FSMTransition::GetPluginInformation));
    plugins->insert(new Plugin(&Create::GetPluginInformation));
    plugins->insert(new Plugin(&Dispose::GetPluginInformation));

    // Create Components
    Create* create1 = new Create(model, "Create1");
    create1->setEntityType(new EntityType(model, "Entity1"));
    create1->setTimeBetweenCreationsExpression("10"); // Create every 10 time units
    create1->setEntitiesPerCreation(1);
    
    ExtendedFSMComponent* efsm1 = new ExtendedFSMComponent(model, "EFSM1");
    
    Dispose* dispose1 = new Dispose(model, "Dispose1");

    // Connect Components
    create1->getConnectionManager()->insert(efsm1);
    efsm1->getConnectionManager()->insert(dispose1);

    // Configure FSM
    // Create States
    FSMState* stateA = efsm1->createState("StateA");
    FSMState* stateB = efsm1->createState("StateB");
    FSMState* stateC = efsm1->createState("StateC", true); // Final state

    efsm1->setInitialState(stateA);
    efsm1->createVariable("count", 0.0);

    // Create Transitions
    // A -> B (always)
    FSMTransition* trans1 = new FSMTransition(model, "Trans_A_to_B");
    trans1->setSource(stateA);
    trans1->setDestination(stateB);
    trans1->setGuard(""); // Always true
    trans1->setSetActions("count = count + 1");
    trans1->setOutputActions("Entity.Attribute1 = 1");

    // B -> A (if count < 3)
    FSMTransition* trans2 = new FSMTransition(model, "Trans_B_to_A");
    trans2->setSource(stateB);
    trans2->setDestination(stateA);
    trans2->setGuard("count < 3");
    trans2->setPriority(1);

    // B -> C (if count >= 3)
    FSMTransition* trans3 = new FSMTransition(model, "Trans_B_to_C");
    trans3->setSource(stateB);
    trans3->setDestination(stateC);
    trans3->setGuard("count >= 3");
    trans3->setPriority(2);

    // Run Simulation
    model->getSimulation()->setReplicationLength(100);
    model->save("./models/Smart_ExtendedFSM.gen");
    model->getSimulation()->start();

    delete genesys;
    return 0;
}

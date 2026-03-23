/*
 * Smart_EFSM_TrafficLight.cpp
 *
 * Created on: 2025.2
 * Authors: André Rocco, Samantha Costa
 */

#include "Smart_EFSM_TrafficLight.h"
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/ExtendedFSMComponent.h"
#include "../../../../plugins/components/FSMState.h"
#include "../../../../plugins/components/FSMTransition.h"
#include "../../../../plugins/components/Assign.h"

Smart_EFSM_TrafficLight::Smart_EFSM_TrafficLight() {
}

int Smart_EFSM_TrafficLight::main(int argc, char** argv) {
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
    plugins->insert(new Plugin(&Assign::GetPluginInformation));

    // --- Components ---

    // 1. Clock Source (Type 1)
    Create* createClock = new Create(model, "CreateClock");
    createClock->setEntityType(new EntityType(model, "ClockTick"));
    createClock->setTimeBetweenCreationsExpression("1"); 
    createClock->setEntitiesPerCreation(1);
    
    Assign* assignClock = new Assign(model, "AssignClockType");
    Assignment* assignClockType = new Assignment(model, "Entity.Type", "1");
    assignClock->getAssignments()->insert(assignClockType);

    // 2. Emergency Source (Type 2) - Arrives at t=25
    Create* createEmergency = new Create(model, "CreateEmergency");
    createEmergency->setEntityType(new EntityType(model, "EmergencyVehicle"));
    createEmergency->setTimeBetweenCreationsExpression("25");
    createEmergency->setEntitiesPerCreation(1);
    createEmergency->setMaxCreations(1); // Only one
    
    Assign* assignEmergency = new Assign(model, "AssignEmergencyType");
    Assignment* assignEmergencyType = new Assignment(model, "Entity.Type", "2");
    assignEmergency->getAssignments()->insert(assignEmergencyType);

    // 3. Clear Source (Type 3) - Arrives at t=40
    Create* createClear = new Create(model, "CreateClear");
    createClear->setEntityType(new EntityType(model, "ClearSignal"));
    createClear->setTimeBetweenCreationsExpression("40");
    createClear->setEntitiesPerCreation(1);
    createClear->setMaxCreations(1); // Only one
    
    Assign* assignClear = new Assign(model, "AssignClearType");
    Assignment* assignClearType = new Assignment(model, "Entity.Type", "3");
    assignClear->getAssignments()->insert(assignClearType);

    // 4. Traffic Light Controller (EFSM)
    ExtendedFSMComponent* trafficLight = new ExtendedFSMComponent(model, "TrafficLightController");
    
    // 5. Dispose
    Dispose* dispose = new Dispose(model, "Dispose");

    // --- Connections ---
    createClock->getConnectionManager()->insert(assignClock);
    assignClock->getConnectionManager()->insert(trafficLight);
    
    createEmergency->getConnectionManager()->insert(assignEmergency);
    assignEmergency->getConnectionManager()->insert(trafficLight);
    
    createClear->getConnectionManager()->insert(assignClear);
    assignClear->getConnectionManager()->insert(trafficLight);
    
    trafficLight->getConnectionManager()->insert(dispose);

    // --- EFSM Configuration ---
    
    // Variables
    trafficLight->createVariable("timer", 0.0);

    // States
    FSMState* stGreen = trafficLight->createState("Green");
    FSMState* stYellow = trafficLight->createState("Yellow");
    FSMState* stRed = trafficLight->createState("Red");
    FSMState* stEmergency = trafficLight->createState("Emergency");
    
    trafficLight->setInitialState(stGreen);

    // Transitions

    // -- Normal Operation (Type 1) --
    
    // Green -> Green (Loop): timer < 5
    FSMTransition* trGreenLoop = new FSMTransition(model, "Green_Loop");
    trGreenLoop->setSource(stGreen);
    trGreenLoop->setDestination(stGreen);
    trGreenLoop->setGuard("Entity.Type == 1 && timer < 5");
    trGreenLoop->setSetActions("timer = timer + 1");
    trGreenLoop->setPriority(1);

    // Green -> Yellow: timer >= 5
    FSMTransition* trGreenToYellow = new FSMTransition(model, "Green_to_Yellow");
    trGreenToYellow->setSource(stGreen);
    trGreenToYellow->setDestination(stYellow);
    trGreenToYellow->setGuard("Entity.Type == 1 && timer >= 5");
    trGreenToYellow->setSetActions("timer = 0");
    trGreenToYellow->setPriority(2);

    // Yellow -> Yellow (Loop): timer < 2
    FSMTransition* trYellowLoop = new FSMTransition(model, "Yellow_Loop");
    trYellowLoop->setSource(stYellow);
    trYellowLoop->setDestination(stYellow);
    trYellowLoop->setGuard("Entity.Type == 1 && timer < 2");
    trYellowLoop->setSetActions("timer = timer + 1");
    trYellowLoop->setPriority(1);

    // Yellow -> Red: timer >= 2
    FSMTransition* trYellowToRed = new FSMTransition(model, "Yellow_to_Red");
    trYellowToRed->setSource(stYellow);
    trYellowToRed->setDestination(stRed);
    trYellowToRed->setGuard("Entity.Type == 1 && timer >= 2");
    trYellowToRed->setSetActions("timer = 0");
    trYellowToRed->setPriority(2);

    // Red -> Red (Loop): timer < 5
    FSMTransition* trRedLoop = new FSMTransition(model, "Red_Loop");
    trRedLoop->setSource(stRed);
    trRedLoop->setDestination(stRed);
    trRedLoop->setGuard("Entity.Type == 1 && timer < 5");
    trRedLoop->setSetActions("timer = timer + 1");
    trRedLoop->setPriority(1);

    // Red -> Green: timer >= 5
    FSMTransition* trRedToGreen = new FSMTransition(model, "Red_to_Green");
    trRedToGreen->setSource(stRed);
    trRedToGreen->setDestination(stGreen);
    trRedToGreen->setGuard("Entity.Type == 1 && timer >= 5");
    trRedToGreen->setSetActions("timer = 0");
    trRedToGreen->setPriority(2);

    // -- Emergency Logic (Type 2) --
    
    // From Green
    FSMTransition* trGreenToEm = new FSMTransition(model, "Green_to_Emergency");
    trGreenToEm->setSource(stGreen);
    trGreenToEm->setDestination(stEmergency);
    trGreenToEm->setGuard("Entity.Type == 2");
    trGreenToEm->setSetActions("timer = 0");
    trGreenToEm->setPriority(10); // High priority

    // From Yellow
    FSMTransition* trYellowToEm = new FSMTransition(model, "Yellow_to_Emergency");
    trYellowToEm->setSource(stYellow);
    trYellowToEm->setDestination(stEmergency);
    trYellowToEm->setGuard("Entity.Type == 2");
    trYellowToEm->setSetActions("timer = 0");
    trYellowToEm->setPriority(10);

    // From Red
    FSMTransition* trRedToEm = new FSMTransition(model, "Red_to_Emergency");
    trRedToEm->setSource(stRed);
    trRedToEm->setDestination(stEmergency);
    trRedToEm->setGuard("Entity.Type == 2");
    trRedToEm->setSetActions("timer = 0");
    trRedToEm->setPriority(10);

    // -- Clear Logic (Type 3) --
    
    // Emergency -> Green
    FSMTransition* trEmToGreen = new FSMTransition(model, "Emergency_to_Green");
    trEmToGreen->setSource(stEmergency);
    trEmToGreen->setDestination(stGreen);
    trEmToGreen->setGuard("Entity.Type == 3");
    trEmToGreen->setSetActions("timer = 0");
    trEmToGreen->setPriority(10);

    // Emergency Loop (ignore clock ticks)
    FSMTransition* trEmLoop = new FSMTransition(model, "Emergency_Loop");
    trEmLoop->setSource(stEmergency);
    trEmLoop->setDestination(stEmergency);
    trEmLoop->setGuard("Entity.Type == 1"); // Clock tick
    trEmLoop->setPriority(1);


    // Run Simulation
    model->getSimulation()->setReplicationLength(60); // Run for 60 seconds
    model->save("./models/Smart_EFSM_TrafficLight.gen");
    model->getSimulation()->start();

    delete genesys;
    return 0;
}

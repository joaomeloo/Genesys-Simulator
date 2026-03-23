/*
 * testExtendedFSMComponent.cpp
 * Unit tests for the ExtendedFSMComponent class.
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include "../../source/gtest/gtest/gtest.h"

// --- WHITE-BOX TESTING HACK ---
// Enables access to protected methods (like _onDispatchEvent) for testing logic 
// without running the full simulation engine.
#define protected public 
#include "../plugins/components/ExtendedFSMComponent.h"
#undef protected
// ------------------------------

#include "../plugins/components/FSMState.h"
#include "../plugins/components/FSMTransition.h"
#include "../kernel/simulator/Attribute.h"
#include "../kernel/simulator/Simulator.h"
#include "../kernel/simulator/Model.h"
#include "../kernel/simulator/Entity.h"

// ------------------ Unit tests ------------------

/**
 * Test: StructureAndInitialization
 * Description: Verifies the structural integrity of the EFSM, including 
 * state creation, variable definition, and initial state assignment.
 */
TEST(ExtendedFSMTests, StructureAndInitialization) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    
    ExtendedFSMComponent* efsm = new ExtendedFSMComponent(model, "EFSM_Test");
    
    FSMState* s1 = efsm->createState("Idle", false);
    efsm->createState("Busy", false);
    efsm->createVariable("counter", 0.0);
    efsm->setInitialState(s1);
    
    EXPECT_EQ(efsm->getInitialState(), s1);
    EXPECT_EQ(efsm->getCurrentState(), s1); 
    
    delete model;
    delete sim;
}

/**
 * Test: TransitionByGuard
 * Description: Validates the transition logic based on internal variable guards.
 * Ensures the state changes only when the guard condition (e.g., "x > 10") is met.
 */
TEST(ExtendedFSMTests, TransitionByGuard) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    ExtendedFSMComponent* efsm = new ExtendedFSMComponent(model, "EFSM_Guard");

    efsm->createVariable("x", 0.0);
    FSMState* s1 = efsm->createState("S1", false);
    FSMState* s2 = efsm->createState("S2", false);
    efsm->setInitialState(s1);

    FSMTransition* t1 = new FSMTransition(model, "T1");
    t1->setSource(s1);
    t1->setDestination(s2);
    t1->setGuard("x > 10");

    Entity* entity = model->createEntity("Ent");

    // Step 1: x is 0.0. Transition should fail.
    efsm->_onDispatchEvent(entity, 0); 
    EXPECT_EQ(efsm->getCurrentState(), s1); 

    // Cleanup to recreate scenario with x = 15
    delete model; delete sim;
    
    // New Model with X = 15
    sim = new Simulator();
    model = new Model(sim);
    efsm = new ExtendedFSMComponent(model, "EFSM_Guard_True");
    s1 = efsm->createState("S1", false);
    s2 = efsm->createState("S2", false);
    efsm->setInitialState(s1);
    efsm->createVariable("x", 15.0); 
    
    t1 = new FSMTransition(model, "T1_True");
    t1->setSource(s1);
    t1->setDestination(s2);
    t1->setGuard("x > 10");
    
    entity = model->createEntity("Ent2");
    
    // Step 2: Execute logic. Transition should succeed.
    efsm->_onDispatchEvent(entity, 0);
    EXPECT_EQ(efsm->getCurrentState(), s2);

    delete model;
    delete sim;
}

/**
 * Test: EntityAttributeGuard
 * Description: Verifies that guards can access and evaluate specific attributes 
 * of the processing Entity (e.g., "Entity.MyVal").
 * Note: Requires an Attribute definition in the Model.
 */
TEST(ExtendedFSMTests, EntityAttributeGuard) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    ExtendedFSMComponent* efsm = new ExtendedFSMComponent(model, "EFSM_Attr");

    // CRITICAL: Create the Attribute in the model so the Entity can store the value.
    new Attribute(model, "MyVal");

    FSMState* s1 = efsm->createState("Start", false);
    FSMState* s2 = efsm->createState("End", true);
    efsm->setInitialState(s1);

    FSMTransition* t1 = new FSMTransition(model, "T_Attr");
    t1->setSource(s1);
    t1->setDestination(s2);
    t1->setGuard("Entity.MyVal == 99"); 

    Entity* entity = model->createEntity("TestEntity");
    entity->setAttributeValue("MyVal", 50.0); 

    // Execute logic (Expected Failure)
    efsm->_onDispatchEvent(entity, 0);
    EXPECT_EQ(efsm->getCurrentState(), s1); 

    // Update attribute and execute logic again (Expected Success)
    entity->setAttributeValue("MyVal", 99.0); 
    efsm->_onDispatchEvent(entity, 0);
    EXPECT_EQ(efsm->getCurrentState(), s2);

    delete model;
    delete sim;
}

/**
 * Test: SetAndOutputActions
 * Description: Validates the execution of Set Actions (updating internal variables)
 * and Output Actions (updating Entity attributes) upon transition firing.
 */
TEST(ExtendedFSMTests, SetAndOutputActions) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    ExtendedFSMComponent* efsm = new ExtendedFSMComponent(model, "EFSM_Actions");

    // CRITICAL: Create the output Attribute in the model
    new Attribute(model, "Result");

    efsm->createVariable("varZ", 0.0);
    FSMState* s1 = efsm->createState("S1", false);
    FSMState* s2 = efsm->createState("S2", false);
    FSMState* s3 = efsm->createState("S3", true);
    
    efsm->setInitialState(s1);

    // T1: S1 -> S2. Sets varZ = 5 and Entity.Result = 1
    FSMTransition* t1 = new FSMTransition(model, "T1");
    t1->setSource(s1);
    t1->setDestination(s2);
    t1->setGuard("1"); 
    t1->setSetActions("varZ = 5");
    t1->setOutputActions("Result = 1"); 

    // T2: S2 -> S3. Only fires if varZ == 5.
    FSMTransition* t2 = new FSMTransition(model, "T2");
    t2->setSource(s2);
    t2->setDestination(s3);
    t2->setGuard("varZ == 5");

    Entity* entity = model->createEntity("ActionEntity");
    entity->setAttributeValue("Result", 0.0);

    // 1st Trigger: S1 -> S2. Executes actions.
    efsm->_onDispatchEvent(entity, 0);
    
    EXPECT_EQ(efsm->getCurrentState(), s2);
    // Verify that the entity attribute was updated to 1
    EXPECT_EQ(entity->getAttributeValue("Result"), 1.0); 

    // 2nd Trigger: S2 -> S3. Verifies if varZ was internally updated to 5.
    efsm->_onDispatchEvent(entity, 0);
    EXPECT_EQ(efsm->getCurrentState(), s3); 

    delete model;
    delete sim;
}
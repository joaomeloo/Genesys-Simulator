/*
 * testFSMTransition.cpp
 * Unit tests for the FSMTransition class.
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include "../../source/gtest/gtest/gtest.h"

#include "../plugins/components/FSMTransition.h"
#include "../plugins/components/FSMState.h"
#include "../kernel/simulator/Simulator.h"
#include "../kernel/simulator/Model.h"

// ------------------ Unit tests ------------------

/**
 * Test: NewInstanceAndBasicProperties
 * Description: Verifies correct instantiation via the factory method and 
 * checks default values for priority, guards, actions, and topological pointers.
 */
TEST(FSMTransitionTests, NewInstanceAndBasicProperties) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    
    // Test factory method
    ModelDataDefinition* def = FSMTransition::NewInstance(model, "TransTest");
    FSMTransition* t = dynamic_cast<FSMTransition*>(def);
    
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->getName(), "TransTest");
    
    // Check default values
    EXPECT_EQ(t->getPriority(), 0);
    EXPECT_FALSE(t->isDefault());
    EXPECT_EQ(t->getGuard(), "");
    EXPECT_EQ(t->getOutputActions(), "");
    EXPECT_EQ(t->getSetActions(), "");
    
    EXPECT_EQ(t->getSource(), nullptr);
    EXPECT_EQ(t->getDestination(), nullptr);

    delete model;
    delete sim;
}

/**
 * Test: SettersAndGetters
 * Description: Validates the accessor and mutator methods for transition 
 * properties including Guard, Priority, Actions, and the Default flag.
 */
TEST(FSMTransitionTests, SettersAndGetters) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    FSMTransition* t = new FSMTransition(model, "T1");

    // Guard
    t->setGuard("x > 10");
    EXPECT_EQ(t->getGuard(), "x > 10");

    // Priority
    t->setPriority(5);
    EXPECT_EQ(t->getPriority(), 5);

    // IsDefault
    t->setDefault(true);
    EXPECT_TRUE(t->isDefault());
    t->setDefault(false);
    EXPECT_FALSE(t->isDefault());

    // Actions
    t->setOutputActions("write(1)");
    EXPECT_EQ(t->getOutputActions(), "write(1)");

    t->setSetActions("x = 1");
    EXPECT_EQ(t->getSetActions(), "x = 1");

    delete model;
    delete sim;
}

/**
 * Test: TopologyConnection
 * Description: Verifies topological connectivity. Ensures that source and 
 * destination pointers are correctly stored and reflected in the show() output.
 * Note: The logic of adding the transition to the Source state's list is 
 * tested in FSMStateTests.
 */
TEST(FSMTransitionTests, TopologyConnection) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    
    FSMState* source = new FSMState(model, "S_Origin");
    FSMState* dest = new FSMState(model, "S_Target");
    FSMTransition* t = new FSMTransition(model, "T_Link");

    t->setSource(source);
    t->setDestination(dest);

    EXPECT_EQ(t->getSource(), source);
    EXPECT_EQ(t->getDestination(), dest);
    
    // Check representation string
    std::string showRes = t->show();
    EXPECT_NE(showRes.find("source=S_Origin"), std::string::npos);
    EXPECT_NE(showRes.find("destination=S_Target"), std::string::npos);

    delete model;
    delete sim;
}

/**
 * Test: CheckMethodLogic
 * Description: Validates the structural requirements. Ensures that source 
 * and destination pointers are manageable, simulating the validation logic 
 * required for model consistency (similar to the protected _check method).
 */
TEST(FSMTransitionTests, CheckMethodLogic) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    FSMTransition* t = new FSMTransition(model, "T_Check");

    // Case 1: No Source or Destination -> Invalid state
    EXPECT_EQ(t->getSource(), nullptr);
    EXPECT_EQ(t->getDestination(), nullptr);

    // Case 2: Configuring Source and Destination (Loopback)
    FSMState* s = new FSMState(model, "S1");
    t->setSource(s);
    t->setDestination(s); 

    EXPECT_NE(t->getSource(), nullptr);
    EXPECT_NE(t->getDestination(), nullptr);

    delete model;
    delete sim;
}
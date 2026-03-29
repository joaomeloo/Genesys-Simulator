/*
 * testFSMState.cpp
 * Unit tests for the FSMState class.
 */

#include <cstdlib>
#include <iostream>
#include <list>
#include <algorithm>
#include "../../source/gtest/gtest/gtest.h"

#include "../plugins/components/FSMState.h"
#include "../plugins/components/FSMTransition.h" 
#include "../kernel/simulator/Simulator.h"
#include "../kernel/simulator/Model.h"

// ------------------ Unit tests ------------------

/**
 * Test: NewInstanceAndBasicProperties
 * Description: Verifies correct instantiation via the factory method and 
 * validates the 'isFinal' property behavior.
 */
TEST(FSMStateTests, NewInstanceAndBasicProperties) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    
    // Test factory method instantiation
    ModelDataDefinition* def = FSMState::NewInstance(model, "StateTest");
    FSMState* s = dynamic_cast<FSMState*>(def);
    
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->getName(), "StateTest");
    
    // Check default values
    EXPECT_FALSE(s->isFinal());

    // Verify setter functionality
    s->setFinal(true);
    EXPECT_TRUE(s->isFinal());

    // Verify string representation
    std::string showRes = s->show();
    EXPECT_NE(showRes.find("isFinal=1"), std::string::npos);

    delete model;
    delete sim;
}

/**
 * Test: PluginInformationMatches
 * Description: Validates that the PluginInformation metadata is generated correctly.
 */
TEST(FSMStateTests, PluginInformationMatches) {
    PluginInformation* info = FSMState::GetPluginInformation();
    ASSERT_NE(info, nullptr);
    delete info;
}

/**
 * Test: TransitionsIntegration
 * Description: Ensures topological consistency. Verifies that setting a Transition's 
 * source automatically registers that transition in the State's internal list.
 */
TEST(FSMStateTests, TransitionsIntegration) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    
    FSMState* stateSource = new FSMState(model, "SourceState");
    FSMState* stateDest = new FSMState(model, "DestState");
    FSMTransition* transition = new FSMTransition(model, "Transition1");

    // Pre-condition: Transitions list must be initialized and empty
    ASSERT_NE(stateSource->getTransitions(), nullptr);
    EXPECT_EQ(stateSource->getTransitions()->size(), 0);

    // Action: Configure transition topology
    transition->setSource(stateSource);
    transition->setDestination(stateDest);

    // Check: Transition should be automatically added to the source state's list
    std::list<FSMTransition*>* transList = stateSource->getTransitions();
    EXPECT_EQ(transList->size(), 1);
    
    if (transList->size() > 0) {
        EXPECT_EQ(transList->front(), transition);
    }

    // Check: Bi-directional consistency
    EXPECT_EQ(transition->getSource(), stateSource);
    EXPECT_EQ(transition->getDestination(), stateDest);

    delete model;
    delete sim;
}
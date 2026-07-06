// Unit tests for the continuous ODE plugin slice (ODESolver + ContinuousSystemComponent).
//
// These exercises the real Model/Simulator kernel with the new plugin classes, so
// the tests validate the ported data definition and the component bridge instead of
// only the header-only solver utilities.
//
// Build target: genesys_test_plugins_continuous_ode.

#include <gtest/gtest.h>

#include <cmath>
#include <string>

#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/components/Continuous/ContinuousSystemComponent.h"
#include "plugins/data/Continuous/ODESolver.h"
#include "plugins/data/Logic/Variable.h"

namespace {

class FakeModelPersistenceRuntime : public Persistence_if {
public:
	bool save(std::string) override { return false; }
	bool load(std::string) override { return false; }
	bool hasChanged() override { return false; }
	void setHasChanged(bool) override {}
	bool getOption(Persistence_if::Options) override { return false; }
	void setOption(Persistence_if::Options, bool) override {}
	std::string getFormatedField(PersistenceRecord*) override { return ""; }
};

class HarmonicOscillator : public OdeSystem_if {
public:
	unsigned int dimension() const override { return 2; }
	void evaluate(double, const double* y, double* dydt) const override {
		dydt[0] = y[1];
		dydt[1] = -y[0];
	}
};

} // namespace

TEST(ContinuousODESolverPluginTest, IntegratesHarmonicOscillatorAndUpdatesVariables) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	auto* time = new Variable(model, "t");
	auto* x = new Variable(model, "x");
	auto* v = new Variable(model, "v");
	ASSERT_NE(time, nullptr);
	ASSERT_NE(x, nullptr);
	ASSERT_NE(v, nullptr);

	ODESolver solver(model, "Solver");
	solver.setTimeVariableName("t");
	solver.setStateVariableNames({"x", "v"});
	solver.setEquationExpressions({"v", "-x"});
	solver.setStep(0.01);
	solver.setPrecision(1e-9);
	solver.setMaxSteps(10000);
	solver.setInitialStateValues({1.0, 0.0});
	solver.resetState();

	solver.integrate(1.0);

	EXPECT_NEAR(x->getValue(), std::cos(1.0), 1e-4);
	EXPECT_NEAR(v->getValue(), -std::sin(1.0), 1e-4);
	EXPECT_NEAR(time->getValue(), 1.0, 1e-12);
	EXPECT_NEAR(solver.getStateValue(0), x->getValue(), 1e-12);
	EXPECT_NEAR(solver.getStateValue(1), v->getValue(), 1e-12);
}

TEST(ContinuousODESolverPluginTest, SaveAndLoadRoundTripPreservesStateConfiguration) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	auto* time = new Variable(model, "t");
	auto* x = new Variable(model, "x");
	auto* v = new Variable(model, "v");
	ASSERT_NE(time, nullptr);
	ASSERT_NE(x, nullptr);
	ASSERT_NE(v, nullptr);

	ODESolver solver(model, "Solver");
	solver.setTimeVariableName("t");
	solver.setStateVariableNames({"x", "v"});
	solver.setEquationExpressions({"v", "-x"});
	solver.setStep(0.02);
	solver.setPrecision(0.01);
	solver.setMaxSteps(2000);
	solver.setCurrentTime(0.25);
	solver.setAutoAdvance(false);
	solver.setInitialStateValues({0.5, -0.25});
	solver.setStateValues({0.5, -0.25});

	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	ODESolver::SaveInstance(&fields, &solver);

	ModelDataDefinition* loadedDef = ODESolver::LoadInstance(model, &fields);
	ASSERT_NE(loadedDef, nullptr);
	auto* loaded = dynamic_cast<ODESolver*>(loadedDef);
	ASSERT_NE(loaded, nullptr);

	EXPECT_EQ(loaded->getTimeVariableName(), "t");
	EXPECT_EQ(loaded->getStateVariableNames().size(), 2u);
	EXPECT_EQ(loaded->getStateVariableNames()[0], "x");
	EXPECT_EQ(loaded->getStateVariableNames()[1], "v");
	EXPECT_EQ(loaded->getEquationExpressions()[0], "v");
	EXPECT_EQ(loaded->getEquationExpressions()[1], "-x");
	EXPECT_DOUBLE_EQ(loaded->getStep(), 0.02);
	EXPECT_DOUBLE_EQ(loaded->getPrecision(), 0.01);
	EXPECT_EQ(loaded->getMaxSteps(), 2000);
	EXPECT_DOUBLE_EQ(loaded->getCurrentTime(), 0.25);
	EXPECT_FALSE(loaded->isAutoAdvance());
	EXPECT_DOUBLE_EQ(loaded->getStateValue(0), 0.5);
	EXPECT_DOUBLE_EQ(loaded->getStateValue(1), -0.25);
}

TEST(ContinuousSystemComponentPluginTest, SaveAndLoadRoundTripResolvesOdeSolverByName) {
	Simulator simulator;
	Model* model = simulator.getModelManager()->newModel();
	ASSERT_NE(model, nullptr);

	auto* time = new Variable(model, "t");
	auto* x = new Variable(model, "x");
	auto* v = new Variable(model, "v");
	ASSERT_NE(time, nullptr);
	ASSERT_NE(x, nullptr);
	ASSERT_NE(v, nullptr);

	auto* solver = new ODESolver(model, "Solver");
	solver->setTimeVariableName("t");
	solver->setStateVariableNames({"x", "v"});
	solver->setEquationExpressions({"v", "-x"});
	ASSERT_NE(solver, nullptr);

	ContinuousSystemComponent component(model, "Bridge");
	component.setOdeSolver(solver);

	FakeModelPersistenceRuntime persistence;
	PersistenceRecord fields(persistence);
	ContinuousSystemComponent::SaveInstance(&fields, &component);

	ModelComponent* loadedDef = ContinuousSystemComponent::LoadInstance(model, &fields);
	ASSERT_NE(loadedDef, nullptr);
	auto* loaded = dynamic_cast<ContinuousSystemComponent*>(loadedDef);
	ASSERT_NE(loaded, nullptr);

	EXPECT_EQ(loaded->getOdeSolverName(), "Solver");
	EXPECT_NE(loaded->getOdeSolver(), nullptr);
	EXPECT_EQ(loaded->getOdeSolver(), solver);
	EXPECT_NE(loaded->show().find("odeSolver=\"Solver\""), std::string::npos);
}

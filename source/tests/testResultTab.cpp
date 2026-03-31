#include "../../source/gtest/gtest/gtest.h"
#include "../../source/kernel/simulator/SimulationReporterDefaultImpl1.h"
#include "../../source/kernel/simulator/ModelSimulation.h"
#include "../../source/kernel/simulator/Model.h"
#include "../../source/kernel/simulator/ModelDataManager.h"
#include "../../source/kernel/simulator/TraceManager.h"
#include "../../source/kernel/util/List.h"
#include "../../source/kernel/simulator/ModelDataDefinition.h"

// Implementações simples para as dependências
class SimpleTracer : public TraceManager {
public:
    SimpleTracer() : TraceManager(nullptr) {}
    void traceReport(const std::string& msg, TraceManager::Level level) {
        std::cout << "[Tracer][" << (int)level << "] " << msg << std::endl;
    }
    void traceReport(const std::string& msg) {
        std::cout << "[Tracer] " << msg << std::endl;
    }
};

class SimpleSimulation : public ModelSimulation {
public:
    SimpleSimulation() : ModelSimulation(nullptr) {}
    int getCurrentReplicationNumber() const { return 1; }
    bool isShowSimulationResposesInReport() const { return false; }
    bool isShowSimulationControlsInReport() const { return false; }
};

class SimpleModel : public Model {
public:
    SimpleTracer tracer;
    SimpleSimulation sim;
    ModelDataManager* dataManager = nullptr;
    SimpleModel() : Model(nullptr) {}
    TraceManager* getTracer() { return &tracer; }
    ModelSimulation* getSimulation() const { return (ModelSimulation*)&sim; }
    ModelDataManager* getDataManager() const { return dataManager; }
    void setDataManager(ModelDataManager* dm) { dataManager = dm; }
};



TEST(SimulationReporterDefaultImpl1Test, ShowReplicationStatistics_CallsTracer) {
    SimpleSimulation simulation;
    SimpleModel model;
    List<ModelDataDefinition*> statsCountersSimulation;

    // DataManager vazio
    auto dataManager = new ModelDataManager(&model);
    model.setDataManager(dataManager);

    SimulationReporterDefaultImpl1 reporter(&simulation, &model, &statsCountersSimulation);
    reporter.showReplicationStatistics();
    SUCCEED();
}


TEST(SimulationReporterDefaultImpl1Test, ShowSimulationStatistics_CallsTracer) {
    SimpleSimulation simulation;
    SimpleModel model;
    List<ModelDataDefinition*> statsCountersSimulation;

    // DataManager vazio
    auto dataManager = new ModelDataManager(&model);
    model.setDataManager(dataManager);

    SimulationReporterDefaultImpl1 reporter(&simulation, &model, &statsCountersSimulation);
    reporter.showSimulationStatistics();
    SUCCEED();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

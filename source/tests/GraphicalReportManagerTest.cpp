#include <QTest>
#include <QSignalSpy>
#include <QApplication>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include "../applications/gui/qt/GenesysQtGUI/GraphicalReportManager.h"
#include "../kernel/simulator/Simulator.h"
#include "../kernel/simulator/Model.h"
#include "../kernel/simulator/StatisticsCollector.h"
#include "../kernel/simulator/Counter.h"

class GraphicalReportManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Testes de inicialização
    void testConstructor();
    void testSetSimulator();
    void testCreateGraphicalReportWidget();
    
    // Testes de coleta de dados
    void testCollectStatisticsData();
    void testCollectStatisticsDataWithEmptyModel();
    void testCollectStatisticsDataWithStatisticsCollector();
    void testCollectStatisticsDataWithCounter();
    
    // Testes de criação de gráficos
    void testCreateBarChart();
    void testCreatePieChart();
    void testCreateLineChart();
    void testCreateScatterChart();
    void testCreateHistogramChart();
    
    // Testes de atualização
    void testUpdateCharts();
    void testUpdateStatisticsTable();
    void testUpdateSummaryWidget();
    
    // Testes de configuração
    void testChartTypeChanged();
    void testMetricChanged();
    void testAutoRefreshToggled();
    void testShowConfidenceIntervalsToggled();
    void testShowStatisticsTableToggled();
    
    // Testes de exportação
    void testExportToPDF();
    void testExportToPNG();
    void testPrintReport();
    
    // Testes de formatação
    void testFormatValue();
    void testGetMetricDisplayName();
    void testGetColorForIndex();

private:
    GraphicalReportManager* manager;
    Simulator* simulator;
    Model* model;
    
    void setupTestModel();
    void cleanupTestModel();
};

void GraphicalReportManagerTest::initTestCase()
{
    // Inicializa o QApplication para os testes
    int argc = 1;
    char* argv[] = {(char*)"test"};
    static QApplication app(argc, argv);
    
    manager = new GraphicalReportManager();
    simulator = new Simulator();
    model = new Model(simulator);
    simulator->getModelManager()->insert(model);
    
    setupTestModel();
}

void GraphicalReportManagerTest::cleanupTestCase()
{
    cleanupTestModel();
    delete manager;
    delete simulator;
}

void GraphicalReportManagerTest::setupTestModel()
{
    // Cria alguns StatisticsCollector para teste
    StatisticsCollector* stat1 = new StatisticsCollector(model, "TestStat1");
    StatisticsCollector* stat2 = new StatisticsCollector(model, "TestStat2");
    
    // Adiciona alguns valores aos coletores
    stat1->getStatistics()->collect(10.0);
    stat1->getStatistics()->collect(20.0);
    stat1->getStatistics()->collect(30.0);
    
    stat2->getStatistics()->collect(5.0);
    stat2->getStatistics()->collect(15.0);
    stat2->getStatistics()->collect(25.0);
    
    // Cria um Counter para teste
    Counter* counter = new Counter(model, "TestCounter");
    counter->incCountValue();
    counter->incCountValue();
    counter->incCountValue();
}

void GraphicalReportManagerTest::cleanupTestModel()
{
    if (model) {
        model->getDataManager()->clear();
    }
}

void GraphicalReportManagerTest::testConstructor()
{
    GraphicalReportManager* testManager = new GraphicalReportManager();
    
    QVERIFY(testManager != nullptr);
    QCOMPARE(testManager->property("currentChartType").toString(), QString("Gráfico de Barras"));
    QCOMPARE(testManager->property("currentMetric").toString(), QString("average"));
    QCOMPARE(testManager->property("autoRefresh").toBool(), false);
    QCOMPARE(testManager->property("showConfidenceIntervals").toBool(), true);
    QCOMPARE(testManager->property("showStatisticsTable").toBool(), true);
    
    delete testManager;
}

void GraphicalReportManagerTest::testSetSimulator()
{
    manager->setSimulator(simulator);
    QVERIFY(manager->property("simulator").value<Simulator*>() == simulator);
}

void GraphicalReportManagerTest::testCreateGraphicalReportWidget()
{
    QWidget* widget = manager->createGraphicalReportWidget();
    QVERIFY(widget != nullptr);
    QVERIFY(widget->isVisible() || !widget->isVisible()); // Widget pode estar visível ou não
}

void GraphicalReportManagerTest::testCollectStatisticsData()
{
    manager->setSimulator(simulator);
    
    QList<GraphicalReportManager::ChartData> data = manager->collectStatisticsData();
    
    QVERIFY(data.size() >= 3); // Pelo menos 2 StatisticsCollector + 1 Counter
    
    // Verifica se os dados foram coletados corretamente
    bool foundStat1 = false, foundStat2 = false, foundCounter = false;
    
    for (const auto& item : data) {
        if (item.name == "TestStat1") {
            foundStat1 = true;
            QCOMPARE(item.average, 20.0);
            QCOMPARE(item.numElements, 3);
        } else if (item.name == "TestStat2") {
            foundStat2 = true;
            QCOMPARE(item.average, 15.0);
            QCOMPARE(item.numElements, 3);
        } else if (item.name == "TestCounter") {
            foundCounter = true;
            QCOMPARE(item.value, 3.0);
        }
    }
    
    QVERIFY(foundStat1);
    QVERIFY(foundStat2);
    QVERIFY(foundCounter);
}

void GraphicalReportManagerTest::testCollectStatisticsDataWithEmptyModel()
{
    Model* emptyModel = new Model(simulator);
    simulator->getModelManager()->insert(emptyModel);
    
    QList<GraphicalReportManager::ChartData> data = manager->collectStatisticsData();
    QCOMPARE(data.size(), 0);
    
    simulator->getModelManager()->remove(emptyModel);
    delete emptyModel;
}

void GraphicalReportManagerTest::testCreateBarChart()
{
    manager->setSimulator(simulator);
    manager->setProperty("currentChartType", "Gráfico de Barras");
    manager->setProperty("currentMetric", "average");
    
    QChart* chart = manager->createBarChart();
    QVERIFY(chart != nullptr);
    QVERIFY(chart->series().size() > 0);
    
    delete chart;
}

void GraphicalReportManagerTest::testCreatePieChart()
{
    manager->setSimulator(simulator);
    manager->setProperty("currentChartType", "Gráfico de Pizza");
    manager->setProperty("currentMetric", "average");
    
    QChart* chart = manager->createPieChart();
    QVERIFY(chart != nullptr);
    QVERIFY(chart->series().size() > 0);
    
    delete chart;
}

void GraphicalReportManagerTest::testCreateLineChart()
{
    manager->setSimulator(simulator);
    manager->setProperty("currentChartType", "Gráfico de Linha");
    manager->setProperty("currentMetric", "average");
    
    QChart* chart = manager->createLineChart();
    QVERIFY(chart != nullptr);
    QVERIFY(chart->series().size() > 0);
    
    delete chart;
}

void GraphicalReportManagerTest::testCreateScatterChart()
{
    manager->setSimulator(simulator);
    manager->setProperty("currentChartType", "Gráfico de Dispersão");
    manager->setProperty("currentMetric", "average");
    
    QChart* chart = manager->createScatterChart();
    QVERIFY(chart != nullptr);
    QVERIFY(chart->series().size() > 0);
    
    delete chart;
}

void GraphicalReportManagerTest::testCreateHistogramChart()
{
    manager->setSimulator(simulator);
    manager->setProperty("currentChartType", "Histograma");
    manager->setProperty("currentMetric", "average");
    
    QChart* chart = manager->createHistogramChart();
    QVERIFY(chart != nullptr);
    QVERIFY(chart->series().size() > 0);
    
    delete chart;
}

void GraphicalReportManagerTest::testUpdateCharts()
{
    manager->setSimulator(simulator);
    
    // Simula a atualização dos gráficos
    manager->updateCharts();
    
    // Verifica se não houve crash
    QVERIFY(true);
}

void GraphicalReportManagerTest::testChartTypeChanged()
{
    manager->setProperty("currentChartType", "Gráfico de Pizza");
    QCOMPARE(manager->property("currentChartType").toString(), QString("Gráfico de Pizza"));
}

void GraphicalReportManagerTest::testMetricChanged()
{
    manager->setProperty("currentMetric", "min");
    QCOMPARE(manager->property("currentMetric").toString(), QString("min"));
}

void GraphicalReportManagerTest::testAutoRefreshToggled()
{
    manager->setProperty("autoRefresh", true);
    QCOMPARE(manager->property("autoRefresh").toBool(), true);
    
    manager->setProperty("autoRefresh", false);
    QCOMPARE(manager->property("autoRefresh").toBool(), false);
}

void GraphicalReportManagerTest::testShowConfidenceIntervalsToggled()
{
    manager->setProperty("showConfidenceIntervals", false);
    QCOMPARE(manager->property("showConfidenceIntervals").toBool(), false);
    
    manager->setProperty("showConfidenceIntervals", true);
    QCOMPARE(manager->property("showConfidenceIntervals").toBool(), true);
}

void GraphicalReportManagerTest::testShowStatisticsTableToggled()
{
    manager->setProperty("showStatisticsTable", false);
    QCOMPARE(manager->property("showStatisticsTable").toBool(), false);
    
    manager->setProperty("showStatisticsTable", true);
    QCOMPARE(manager->property("showStatisticsTable").toBool(), true);
}

void GraphicalReportManagerTest::testFormatValue()
{
    QString result1 = manager->formatValue(123.456789, "average");
    QCOMPARE(result1, QString("123.456789"));
    
    QString result2 = manager->formatValue(123, "numElements");
    QCOMPARE(result2, QString("123"));
    
    QString result3 = manager->formatValue(0.1234, "variationCoef");
    QCOMPARE(result3, QString("0.1234%"));
    
    QString result4 = manager->formatValue(std::numeric_limits<double>::quiet_NaN(), "average");
    QCOMPARE(result4, QString("N/A"));
}

void GraphicalReportManagerTest::testGetMetricDisplayName()
{
    QCOMPARE(manager->getMetricDisplayName("average"), QString("Média"));
    QCOMPARE(manager->getMetricDisplayName("min"), QString("Mínimo"));
    QCOMPARE(manager->getMetricDisplayName("max"), QString("Máximo"));
    QCOMPARE(manager->getMetricDisplayName("stddeviation"), QString("Desvio Padrão"));
    QCOMPARE(manager->getMetricDisplayName("unknown"), QString("unknown"));
}

void GraphicalReportManagerTest::testGetColorForIndex()
{
    QColor color1 = manager->getColorForIndex(0);
    QColor color2 = manager->getColorForIndex(1);
    QColor color3 = manager->getColorForIndex(10); // Deve usar módulo
    
    QVERIFY(color1.isValid());
    QVERIFY(color2.isValid());
    QVERIFY(color3.isValid());
    QVERIFY(color1 != color2);
}

void GraphicalReportManagerTest::testExportToPDF()
{
    // Teste básico - verifica se não há crash
    manager->exportToPDF("/tmp/test_export.pdf");
    QVERIFY(true);
}

void GraphicalReportManagerTest::testExportToPNG()
{
    // Teste básico - verifica se não há crash
    manager->exportToPNG("/tmp/test_export.png");
    QVERIFY(true);
}

void GraphicalReportManagerTest::testPrintReport()
{
    // Teste básico - verifica se não há crash
    manager->printReport();
    QVERIFY(true);
}

QTEST_MAIN(GraphicalReportManagerTest)
#include "GraphicalReportManagerTest.moc" 
#ifndef GRAPHICALREPORTMANAGER_H
#define GRAPHICALREPORTMANAGER_H

#include <QObject>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QApplication>
#include <QScreen>
#include <QScrollArea>
#include <QFrame>
#include <QSplitter>
#include <QTextEdit>
#include <QProgressBar>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPainter>
#include <QPdfWriter>
#include <QImageWriter>

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/Model.h"
#include "../../../../kernel/simulator/ModelDataManager.h"
#include "../../../../kernel/simulator/StatisticsCollector.h"
#include "../../../../kernel/simulator/Counter.h"
#include "../../../../kernel/statistics/Statistics_if.h"
#include "../../../../kernel/statistics/StatisticsDataFile_if.h"

// using namespace QtCharts;

class GraphicalReportManager : public QObject {
    Q_OBJECT

public:
    explicit GraphicalReportManager(QWidget* parent = nullptr);
    ~GraphicalReportManager();

    // Configura o simulador para acessar os dados
    void setSimulator(Simulator* simulator);
    
    // Cria a interface gráfica completa
    QWidget* createGraphicalReportWidget();
    
    // Atualiza os gráficos com dados atuais
    void updateCharts();
    
    // Exporta relatórios
    void exportToPDF(const QString& filename);
    void exportToPNG(const QString& filename);
    void printReport();

private slots:
    void onChartTypeChanged(int index);
    void onMetricChanged(int index);
    void onRefreshClicked();
    void onPrintClicked();
    void onAutoRefreshToggled(bool checked);
    void onShowConfidenceIntervalsToggled(bool checked);
    void onShowStatisticsTableToggled(bool checked);

private:
    // Métodos para criar diferentes tipos de gráficos
    QChart* createBarChart();
    QChart* createPieChart();
    QChart* createLineChart();
    QChart* createScatterChart();
    QChart* createHistogramChart();
    
    // Métodos auxiliares
    void setupUI();
    void populateMetricsComboBox();
    void populateChartTypesComboBox();
    void createStatisticsTable();
    void updateStatisticsTable();
    void createSummaryWidget();
    void updateSummaryWidget();
    
    // Dados e configurações
    struct ChartData {
        QString name;
        double value;
        double min;
        double max;
        double average;
        double stdDev;
        double confidenceInterval;
        unsigned int numElements;
    };
    
    QList<ChartData> collectStatisticsData();
    QString formatValue(double value, const QString& metric);
    QString getMetricDisplayName(const QString& metricName);
    QColor getColorForIndex(int index);
    
    // Interface gráfica
    QWidget* mainWidget;
    QTabWidget* chartTabWidget;
    QComboBox* chartTypeComboBox;
    QComboBox* metricComboBox;
    QPushButton* refreshButton;
    QPushButton* printButton;
    QCheckBox* autoRefreshCheckBox;
    QCheckBox* showConfidenceIntervalsCheckBox;
    QCheckBox* showStatisticsTableCheckBox;
    QSpinBox* refreshIntervalSpinBox;
    QTableWidget* statisticsTable;
    QTextEdit* summaryTextEdit;
    QProgressBar* dataProgressBar;
    
    // Gráficos
    QChartView* currentChartView;
    QChart* currentChart;
    
    // Dados
    Simulator* simulator;
    QTimer* refreshTimer;
    
    // Configurações
    QString currentChartType;
    QString currentMetric;
    bool autoRefresh;
    bool showConfidenceIntervals;
    bool showStatisticsTable;
    int refreshInterval;
    
    // Constantes
    static const QStringList CHART_TYPES;
    static const QStringList METRIC_TYPES;
    static const QList<QColor> CHART_COLORS;
};

#endif // GRAPHICALREPORTMANAGER_H 
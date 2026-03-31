#include "GraphicalReportManager.h"
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextList>
#include <QTextListFormat>
#include <cmath>

// Constantes estáticas
const QStringList GraphicalReportManager::CHART_TYPES = {
    "Gráfico de Barras",
    "Gráfico de Pizza",
    "Gráfico de Linha",
    "Gráfico de Dispersão",
    "Histograma"
};

const QStringList GraphicalReportManager::METRIC_TYPES = {
    "average",
    "min",
    "max",
    "stddeviation",
    "variance",
    "variationCoef",
    "halfWidthConfidenceInterval",
    "numElements"
};

const QList<QColor> GraphicalReportManager::CHART_COLORS = {
    QColor(52, 152, 219),   // Azul
    QColor(231, 76, 60),    // Vermelho
    QColor(46, 204, 113),   // Verde
    QColor(155, 89, 182),   // Roxo
    QColor(241, 196, 15),   // Amarelo
    QColor(230, 126, 34),   // Laranja
    QColor(26, 188, 156),   // Turquesa
    QColor(52, 73, 94),     // Cinza escuro
    QColor(142, 68, 173),   // Violeta
    QColor(39, 174, 96)     // Verde escuro
};

GraphicalReportManager::GraphicalReportManager(QWidget* parent) : QObject(parent) {
    simulator = nullptr;
    mainWidget = nullptr;
    currentChartView = nullptr;
    currentChart = nullptr;
    
    // Configurações padrão
    currentChartType = "Gráfico de Barras";
    currentMetric = "average";
    autoRefresh = false;
    showConfidenceIntervals = true;
    showStatisticsTable = true;
    refreshInterval = 5000; // 5 segundos
    
    // Timer para atualização automática
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &GraphicalReportManager::updateCharts);
}

GraphicalReportManager::~GraphicalReportManager() {
    if (refreshTimer) {
        refreshTimer->stop();
    }
}

void GraphicalReportManager::setSimulator(Simulator* sim) {
    simulator = sim;
}

QWidget* GraphicalReportManager::createGraphicalReportWidget() {
    if (mainWidget) {
        delete mainWidget;
    }
    
    mainWidget = new QWidget();
    setupUI();
    return mainWidget;
}

void GraphicalReportManager::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
    
    // Barra de ferramentas superior
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    
    // Controles de gráfico
    QGroupBox* chartControlsGroup = new QGroupBox("Configurações do Gráfico");
    QHBoxLayout* chartControlsLayout = new QHBoxLayout(chartControlsGroup);
    
    QLabel* chartTypeLabel = new QLabel("Tipo:");
    chartTypeComboBox = new QComboBox();
    populateChartTypesComboBox();
    
    QLabel* metricLabel = new QLabel("Métrica:");
    metricComboBox = new QComboBox();
    populateMetricsComboBox();
    
    chartControlsLayout->addWidget(chartTypeLabel);
    chartControlsLayout->addWidget(chartTypeComboBox);
    chartControlsLayout->addWidget(metricLabel);
    chartControlsLayout->addWidget(metricComboBox);
    chartControlsLayout->addStretch();
    
    // Controles de ação
    QGroupBox* actionControlsGroup = new QGroupBox("Ações");
    QHBoxLayout* actionControlsLayout = new QHBoxLayout(actionControlsGroup);
    
    refreshButton = new QPushButton("Atualizar");
    printButton = new QPushButton("Imprimir");
    
    actionControlsLayout->addWidget(refreshButton);
    actionControlsLayout->addWidget(printButton);
    actionControlsLayout->addStretch();
    
    // Controles de configuração
    QGroupBox* configControlsGroup = new QGroupBox("Configurações");
    QHBoxLayout* configControlsLayout = new QHBoxLayout(configControlsGroup);
    
    autoRefreshCheckBox = new QCheckBox("Atualização Automática");
    showConfidenceIntervalsCheckBox = new QCheckBox("Mostrar Intervalos de Confiança");
    showStatisticsTableCheckBox = new QCheckBox("Mostrar Tabela de Estatísticas");
    
    QLabel* intervalLabel = new QLabel("Intervalo (ms):");
    refreshIntervalSpinBox = new QSpinBox();
    refreshIntervalSpinBox->setRange(1000, 60000);
    refreshIntervalSpinBox->setValue(refreshInterval);
    refreshIntervalSpinBox->setSuffix(" ms");
    
    configControlsLayout->addWidget(autoRefreshCheckBox);
    configControlsLayout->addWidget(showConfidenceIntervalsCheckBox);
    configControlsLayout->addWidget(showStatisticsTableCheckBox);
    configControlsLayout->addWidget(intervalLabel);
    configControlsLayout->addWidget(refreshIntervalSpinBox);
    configControlsLayout->addStretch();
    
    // Adiciona grupos à barra de ferramentas
    toolbarLayout->addWidget(chartControlsGroup);
    toolbarLayout->addWidget(actionControlsGroup);
    toolbarLayout->addWidget(configControlsGroup);
    
    mainLayout->addLayout(toolbarLayout);
    
    // Barra de progresso
    dataProgressBar = new QProgressBar();
    dataProgressBar->setVisible(false);
    mainLayout->addWidget(dataProgressBar);
    
    // Área principal com abas
    chartTabWidget = new QTabWidget();
    
    // Aba do gráfico
    QWidget* chartTab = new QWidget();
    QVBoxLayout* chartLayout = new QVBoxLayout(chartTab);
    
    currentChartView = new QChartView();
    currentChartView->setRenderHint(QPainter::Antialiasing);
    currentChartView->setMinimumHeight(400);
    chartLayout->addWidget(currentChartView);
    
    chartTabWidget->addTab(chartTab, "Gráfico");
    
    // Aba da tabela de estatísticas
    if (showStatisticsTable) {
        createStatisticsTable();
        chartTabWidget->addTab(statisticsTable, "Tabela de Estatísticas");
    }
    
    // Aba do resumo
    createSummaryWidget();
    chartTabWidget->addTab(summaryTextEdit, "Resumo");
    
    mainLayout->addWidget(chartTabWidget);
    
    // Conecta sinais
    connect(chartTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GraphicalReportManager::onChartTypeChanged);
    connect(metricComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GraphicalReportManager::onMetricChanged);
    connect(refreshButton, &QPushButton::clicked, this, &GraphicalReportManager::onRefreshClicked);
    connect(printButton, &QPushButton::clicked, this, &GraphicalReportManager::onPrintClicked);
    connect(autoRefreshCheckBox, &QCheckBox::toggled, this, &GraphicalReportManager::onAutoRefreshToggled);
    connect(showConfidenceIntervalsCheckBox, &QCheckBox::toggled, this, &GraphicalReportManager::onShowConfidenceIntervalsToggled);
    connect(showStatisticsTableCheckBox, &QCheckBox::toggled, this, &GraphicalReportManager::onShowStatisticsTableToggled);
    
    // Configurações iniciais
    showConfidenceIntervalsCheckBox->setChecked(showConfidenceIntervals);
    showStatisticsTableCheckBox->setChecked(showStatisticsTable);
    autoRefreshCheckBox->setChecked(autoRefresh);
    
    // Cria o primeiro gráfico
    updateCharts();
}

void GraphicalReportManager::populateChartTypesComboBox() {
    chartTypeComboBox->clear();
    chartTypeComboBox->addItems(CHART_TYPES);
}

void GraphicalReportManager::populateMetricsComboBox() {
    metricComboBox->clear();
    for (const QString& metric : METRIC_TYPES) {
        metricComboBox->addItem(getMetricDisplayName(metric), metric);
    }
}

void GraphicalReportManager::createStatisticsTable() {
    statisticsTable = new QTableWidget();
    statisticsTable->setAlternatingRowColors(true);
    statisticsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    statisticsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    QStringList headers = {
        "Nome", "Tipo", "Elementos", "Mínimo", "Máximo", "Média", 
        "Desvio Padrão", "Variância", "Coef. Variação", "Int. Confiança"
    };
    
    statisticsTable->setColumnCount(headers.size());
    statisticsTable->setHorizontalHeaderLabels(headers);
    statisticsTable->horizontalHeader()->setStretchLastSection(true);
    statisticsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void GraphicalReportManager::createSummaryWidget() {
    summaryTextEdit = new QTextEdit();
    summaryTextEdit->setReadOnly(true);
    summaryTextEdit->setFont(QFont("Courier", 10));
}

QList<GraphicalReportManager::ChartData> GraphicalReportManager::collectStatisticsData() {
    QList<ChartData> data;
    
    if (!simulator || !simulator->getModelManager()->current()) {
        return data;
    }
    
    Model* model = simulator->getModelManager()->current();
    ModelDataManager* dataManager = model->getDataManager();
    
    // Coleta dados de StatisticsCollector
    const std::string statisticsType = Util::TypeOf<StatisticsCollector>();
    std::list<ModelDataDefinition*>* statistics = dataManager->getDataDefinitionList(statisticsType)->list();
    
    for (ModelDataDefinition* item : *statistics) {
        if (StatisticsCollector* statCollector = dynamic_cast<StatisticsCollector*>(item)) {
            Statistics_if* stats = statCollector->getStatistics();
            
            ChartData chartData;
            chartData.name = QString::fromStdString(statCollector->getName());
            chartData.numElements = stats->numElements();
            chartData.min = stats->min();
            chartData.max = stats->max();
            chartData.average = stats->average();
            chartData.stdDev = stats->stddeviation();
            chartData.confidenceInterval = stats->halfWidthConfidenceInterval();
            
            // Define o valor baseado na métrica selecionada
            if (currentMetric == "average") chartData.value = stats->average();
            else if (currentMetric == "min") chartData.value = stats->min();
            else if (currentMetric == "max") chartData.value = stats->max();
            else if (currentMetric == "stddeviation") chartData.value = stats->stddeviation();
            else if (currentMetric == "variance") chartData.value = stats->variance();
            else if (currentMetric == "variationCoef") chartData.value = stats->variationCoef();
            else if (currentMetric == "halfWidthConfidenceInterval") chartData.value = stats->halfWidthConfidenceInterval();
            else if (currentMetric == "numElements") chartData.value = stats->numElements();
            else chartData.value = stats->average();
            
            data.append(chartData);
        }
    }
    
    // Coleta dados de Counter
    const std::string counterType = Util::TypeOf<Counter>();
    std::list<ModelDataDefinition*>* counters = dataManager->getDataDefinitionList(counterType)->list();
    
    for (ModelDataDefinition* item : *counters) {
        if (Counter* counter = dynamic_cast<Counter*>(item)) {
            ChartData chartData;
            chartData.name = QString::fromStdString(counter->getName());
            chartData.numElements = counter->getCountValue();
            chartData.min = counter->getCountValue();
            chartData.max = counter->getCountValue();
            chartData.average = counter->getCountValue();
            chartData.stdDev = 0.0;
            chartData.confidenceInterval = 0.0;
            chartData.value = counter->getCountValue();
            
            data.append(chartData);
        }
    }
    
    return data;
}

void GraphicalReportManager::updateCharts() {
    if (!simulator || !simulator->getModelManager()->current()) {
        return;
    }
    
    dataProgressBar->setVisible(true);
    dataProgressBar->setRange(0, 100);
    dataProgressBar->setValue(0);
    
    QApplication::processEvents();
    
    // Coleta dados
    QList<ChartData> chartData = collectStatisticsData();
    
    dataProgressBar->setValue(50);
    QApplication::processEvents();
    
    if (chartData.isEmpty()) {
        dataProgressBar->setVisible(false);
        return;
    }
    
    // Cria o gráfico baseado no tipo selecionado
    QChart* newChart = nullptr;
    
    if (currentChartType == "Gráfico de Barras") {
        newChart = createBarChart();
    } else if (currentChartType == "Gráfico de Pizza") {
        newChart = createPieChart();
    } else if (currentChartType == "Gráfico de Linha") {
        newChart = createLineChart();
    } else if (currentChartType == "Gráfico de Dispersão") {
        newChart = createScatterChart();
    } else if (currentChartType == "Histograma") {
        newChart = createHistogramChart();
    }
    
    dataProgressBar->setValue(75);
    QApplication::processEvents();
    
    if (newChart) {
        if (currentChart) {
            currentChartView->chart()->deleteLater();
        }
        currentChart = newChart;
        currentChartView->setChart(currentChart);
    }
    
    // Atualiza tabela de estatísticas
    if (showStatisticsTable) {
        updateStatisticsTable();
    }
    
    // Atualiza resumo
    updateSummaryWidget();
    
    dataProgressBar->setValue(100);
    QTimer::singleShot(1000, dataProgressBar, &QProgressBar::hide);
}

QChart* GraphicalReportManager::createBarChart() {
    QList<ChartData> data = collectStatisticsData();
    
    QBarSeries* series = new QBarSeries();
    QBarSet* barSet = new QBarSet(getMetricDisplayName(currentMetric));
    
    QStringList categories;
    
    for (int i = 0; i < data.size(); ++i) {
        const ChartData& item = data[i];
        barSet->append(item.value);
        categories << item.name;
        
        // Define cor para a barra
        barSet->setColor(getColorForIndex(i));
    }
    
    series->append(barSet);
    
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Relatório Gráfico - " + getMetricDisplayName(currentMetric));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    // Adiciona intervalos de confiança se habilitado
    if (showConfidenceIntervals) {
        QBarSet* confidenceSet = new QBarSet("Intervalo de Confiança");
        for (const ChartData& item : data) {
            confidenceSet->append(item.confidenceInterval);
        }
        confidenceSet->setColor(QColor(255, 0, 0, 100));
        series->append(confidenceSet);
    }
    
    return chart;
}

QChart* GraphicalReportManager::createPieChart() {
    QList<ChartData> data = collectStatisticsData();
    
    QPieSeries* series = new QPieSeries();
    
    for (int i = 0; i < data.size(); ++i) {
        const ChartData& item = data[i];
        QPieSlice* slice = series->append(item.name, item.value);
        slice->setColor(getColorForIndex(i));
        slice->setLabelVisible(true);
        slice->setLabelPosition(QPieSlice::LabelOutside);
    }
    
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Relatório Gráfico - " + getMetricDisplayName(currentMetric));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    return chart;
}

QChart* GraphicalReportManager::createLineChart() {
    QList<ChartData> data = collectStatisticsData();
    
    QLineSeries* series = new QLineSeries();
    series->setName(getMetricDisplayName(currentMetric));
    
    for (int i = 0; i < data.size(); ++i) {
        const ChartData& item = data[i];
        series->append(i, item.value);
    }
    
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Relatório Gráfico - " + getMetricDisplayName(currentMetric));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    QValueAxis* axisX = new QValueAxis();
    axisX->setRange(0, data.size() - 1);
    axisX->setLabelFormat("%d");
    axisX->setTitleText("Índice");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    return chart;
}

QChart* GraphicalReportManager::createScatterChart() {
    QList<ChartData> data = collectStatisticsData();
    
    QScatterSeries* series = new QScatterSeries();
    series->setName(getMetricDisplayName(currentMetric));
    series->setMarkerSize(10);
    
    for (int i = 0; i < data.size(); ++i) {
        const ChartData& item = data[i];
        series->append(i, item.value);
    }
    
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Relatório Gráfico - " + getMetricDisplayName(currentMetric));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    QValueAxis* axisX = new QValueAxis();
    axisX->setRange(0, data.size() - 1);
    axisX->setLabelFormat("%d");
    axisX->setTitleText("Índice");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    return chart;
}

QChart* GraphicalReportManager::createHistogramChart() {
    QList<ChartData> data = collectStatisticsData();
    
    QBarSeries* series = new QBarSeries();
    QBarSet* barSet = new QBarSet("Frequência");
    
    QStringList categories;
    
    // Agrupa dados em faixas para histograma
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();
    
    for (const ChartData& item : data) {
        minVal = qMin(minVal, item.value);
        maxVal = qMax(maxVal, item.value);
    }
    
    const int numBins = qMin(10, data.size());
    const double binWidth = (maxVal - minVal) / numBins;
    
    QVector<int> bins(numBins, 0);
    
    for (const ChartData& item : data) {
        int binIndex = qMin(static_cast<int>((item.value - minVal) / binWidth), numBins - 1);
        bins[binIndex]++;
    }
    
    for (int i = 0; i < numBins; ++i) {
        barSet->append(bins[i]);
        double binStart = minVal + i * binWidth;
        double binEnd = minVal + (i + 1) * binWidth;
        categories << QString("[%1-%2]").arg(binStart, 0, 'f', 2).arg(binEnd, 0, 'f', 2);
    }
    
    series->append(barSet);
    
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Histograma - " + getMetricDisplayName(currentMetric));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    
    return chart;
}

void GraphicalReportManager::updateStatisticsTable() {
    if (!statisticsTable) return;
    
    QList<ChartData> data = collectStatisticsData();
    
    statisticsTable->setRowCount(data.size());
    
    for (int row = 0; row < data.size(); ++row) {
        const ChartData& item = data[row];
        
        statisticsTable->setItem(row, 0, new QTableWidgetItem(item.name));
        statisticsTable->setItem(row, 1, new QTableWidgetItem("Statistics"));
        statisticsTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.numElements)));
        statisticsTable->setItem(row, 3, new QTableWidgetItem(formatValue(item.min, "min")));
        statisticsTable->setItem(row, 4, new QTableWidgetItem(formatValue(item.max, "max")));
        statisticsTable->setItem(row, 5, new QTableWidgetItem(formatValue(item.average, "average")));
        statisticsTable->setItem(row, 6, new QTableWidgetItem(formatValue(item.stdDev, "stddeviation")));
        statisticsTable->setItem(row, 7, new QTableWidgetItem(formatValue(item.stdDev * item.stdDev, "variance")));
        statisticsTable->setItem(row, 8, new QTableWidgetItem(formatValue(item.stdDev / item.average * 100, "variationCoef")));
        statisticsTable->setItem(row, 9, new QTableWidgetItem(formatValue(item.confidenceInterval, "halfWidthConfidenceInterval")));
    }
}

void GraphicalReportManager::updateSummaryWidget() {
    if (!summaryTextEdit) return;
    
    QList<ChartData> data = collectStatisticsData();
    
    QString summary;
    summary += "<h2>Resumo do Relatório Gráfico</h2>";
    summary += "<p><b>Data/Hora:</b> " + QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss") + "</p>";
    summary += "<p><b>Tipo de Gráfico:</b> " + currentChartType + "</p>";
    summary += "<p><b>Métrica:</b> " + getMetricDisplayName(currentMetric) + "</p>";
    summary += "<p><b>Total de Estatísticas:</b> " + QString::number(data.size()) + "</p>";
    
    if (!data.isEmpty()) {
        summary += "<h3>Estatísticas Gerais</h3>";
        
        double totalValue = 0;
        double minValue = std::numeric_limits<double>::max();
        double maxValue = std::numeric_limits<double>::lowest();
        
        for (const ChartData& item : data) {
            totalValue += item.value;
            minValue = qMin(minValue, item.value);
            maxValue = qMax(maxValue, item.value);
        }
        
        double avgValue = totalValue / data.size();
        
        summary += "<table border='1' cellpadding='5'>";
        summary += "<tr><td><b>Valor Total:</b></td><td>" + formatValue(totalValue, currentMetric) + "</td></tr>";
        summary += "<tr><td><b>Valor Mínimo:</b></td><td>" + formatValue(minValue, currentMetric) + "</td></tr>";
        summary += "<tr><td><b>Valor Máximo:</b></td><td>" + formatValue(maxValue, currentMetric) + "</td></tr>";
        summary += "<tr><td><b>Valor Médio:</b></td><td>" + formatValue(avgValue, currentMetric) + "</td></tr>";
        summary += "</table>";
        
        summary += "<h3>Detalhes por Estatística</h3>";
        summary += "<table border='1' cellpadding='5'>";
        summary += "<tr><th>Nome</th><th>Valor</th><th>Elementos</th><th>Média</th><th>Desvio Padrão</th></tr>";
        
        for (const ChartData& item : data) {
            summary += "<tr>";
            summary += "<td>" + item.name + "</td>";
            summary += "<td>" + formatValue(item.value, currentMetric) + "</td>";
            summary += "<td>" + QString::number(item.numElements) + "</td>";
            summary += "<td>" + formatValue(item.average, "average") + "</td>";
            summary += "<td>" + formatValue(item.stdDev, "stddeviation") + "</td>";
            summary += "</tr>";
        }
        summary += "</table>";
    }
    
    summaryTextEdit->setHtml(summary);
}

QString GraphicalReportManager::formatValue(double value, const QString& metric) {
    if (std::isnan(value) || std::isinf(value)) {
        return "N/A";
    }
    
    if (metric == "numElements") {
        return QString::number(static_cast<unsigned int>(value));
    } else if (metric == "variationCoef") {
        return QString::number(value, 'f', 4) + "%";
    } else {
        return QString::number(value, 'f', 6);
    }
}

QString GraphicalReportManager::getMetricDisplayName(const QString& metricName) {
    static QMap<QString, QString> displayNames = {
        {"average", "Média"},
        {"min", "Mínimo"},
        {"max", "Máximo"},
        {"stddeviation", "Desvio Padrão"},
        {"variance", "Variância"},
        {"variationCoef", "Coeficiente de Variação"},
        {"halfWidthConfidenceInterval", "Intervalo de Confiança"},
        {"numElements", "Número de Elementos"}
    };
    
    return displayNames.value(metricName, metricName);
}

QColor GraphicalReportManager::getColorForIndex(int index) {
    return CHART_COLORS[index % CHART_COLORS.size()];
}

void GraphicalReportManager::exportToPDF(const QString& filename) {
    if (!currentChartView) return;
    
    QPdfWriter writer(filename);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);
    
    QPainter painter(&writer);
    
    // Desenha o gráfico
    currentChartView->render(&painter);
    
    painter.end();
}

void GraphicalReportManager::exportToPNG(const QString& filename) {
    if (!currentChartView) return;
    
    QPixmap pixmap = currentChartView->grab();
    pixmap.save(filename, "PNG");
}

void GraphicalReportManager::printReport() {
    if (!currentChartView) return;
    
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, mainWidget);
    
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        currentChartView->render(&painter);
        painter.end();
    }
}

// Slots
void GraphicalReportManager::onChartTypeChanged(int index) {
    currentChartType = CHART_TYPES[index];
    updateCharts();
}

void GraphicalReportManager::onMetricChanged(int index) {
    currentMetric = METRIC_TYPES[index];
    updateCharts();
}

void GraphicalReportManager::onRefreshClicked() {
    updateCharts();
}

void GraphicalReportManager::onPrintClicked() {
    printReport();
}

void GraphicalReportManager::onAutoRefreshToggled(bool checked) {
    autoRefresh = checked;
    if (checked) {
        refreshTimer->start(refreshInterval);
    } else {
        refreshTimer->stop();
    }
}

void GraphicalReportManager::onShowConfidenceIntervalsToggled(bool checked) {
    showConfidenceIntervals = checked;
    updateCharts();
}

void GraphicalReportManager::onShowStatisticsTableToggled(bool checked) {
    showStatisticsTable = checked;
    
    if (checked && !statisticsTable) {
        createStatisticsTable();
        chartTabWidget->addTab(statisticsTable, "Tabela de Estatísticas");
    } else if (!checked && statisticsTable) {
        chartTabWidget->removeTab(chartTabWidget->indexOf(statisticsTable));
        statisticsTable = nullptr;
    }
} 
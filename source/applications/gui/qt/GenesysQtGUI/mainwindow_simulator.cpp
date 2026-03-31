#include "mainwindow.h"
#include "ui_mainwindow.h"


//-------------------------
// Simulator Trace Handlers
//-------------------------

void MainWindow::_simulatorTraceHandler(TraceEvent e) {
    std::cout << e.getText() << std::endl;
    // set color based on Trace Level
    if (e.getTracelevel() == TraceManager::Level::L1_errorFatal)
        ui->textEdit_Console->setTextColor(QColor::fromRgb(255, 0, 0));
    else if (e.getTracelevel() == TraceManager::Level::L2_results)
        ui->textEdit_Console->setTextColor(QColor::fromRgb(0, 0, 255));
    else if (e.getTracelevel() == TraceManager::Level::L3_errorRecover)
        ui->textEdit_Console->setTextColor(QColor::fromRgb(223, 0, 0));
    else if (e.getTracelevel() == TraceManager::Level::L4_warning)
        ui->textEdit_Console->setTextColor(QColor::fromRgb(128, 0, 0));
    else {
        unsigned short grayVal = 20 * (static_cast<unsigned int> (e.getTracelevel()));// - 5);
        ui->textEdit_Console->setTextColor(QColor::fromRgb(grayVal, grayVal, grayVal));
    }
    ui->textEdit_Console->append(QString::fromStdString(e.getText()));
    ui->textEdit_Console->moveCursor(QTextCursor::MoveOperation::End, QTextCursor::MoveMode::MoveAnchor);
    QCoreApplication::processEvents();
}

void MainWindow::_simulatorTraceErrorHandler(TraceErrorEvent e) {

    std::cout << e.getText() << std::endl;
    ui->textEdit_Console->setTextColor(QColor::fromRgb(255, 0, 0));
    ui->textEdit_Console->append(QString::fromStdString(e.getText()));
    QCoreApplication::processEvents();
}

void MainWindow::_simulatorTraceSimulationHandler(TraceSimulationEvent e) {
    std::cout << e.getText() << std::endl;
    if (e.getText().find("Event {time=") != std::string::npos) {
        ui->textEdit_Simulation->setTextColor(QColor::fromRgb(0, 0, 128));
    } else {
        unsigned short grayVal = 20 * (static_cast<unsigned int> (e.getTracelevel()));// - 5);
        ui->textEdit_Simulation->setTextColor(QColor::fromRgb(grayVal, grayVal, grayVal));
    }
    ui->textEdit_Simulation->append(QString::fromStdString(e.getText()));
    QCoreApplication::processEvents();
}

void MainWindow::_simulatorTraceReportsHandler(TraceEvent e) {

    // Filtro: ignora mensagens HTML (usadas só na aba Results)
    const std::string& msg = e.getText();
    if (
        (msg.rfind("<html", 0) == 0) ||
        (msg.rfind("<!DOCTYPE html", 0) == 0)
    ) {
        // Não exibe HTML na aba Reports
        return;
    }
    std::cout << msg << std::endl;
    ui->textEdit_Reports->append(QString::fromStdString(msg));
    QCoreApplication::processEvents();
}


// Handler para resultados (L2_results)
void MainWindow::_simulatorTraceResultsHandler(TraceEvent e) {
    // Exibe HTML se detectar, senão texto puro. Se HTML já estiver exibido, ignora texto puro subsequente.
    const std::string& msg = e.getText();
    static bool htmlMostrado = false;
    if ((msg.rfind("<html", 0) == 0) || (msg.rfind("<!DOCTYPE html", 0) == 0)) {
        std::cout << "[HTML] " << msg.substr(0, 80) << (msg.size() > 80 ? "..." : "") << std::endl;
        ui->textEdit_Results->clear();
        ui->textEdit_Results->setHtml(QString::fromStdString(msg));
        htmlMostrado = true;
    } else {
        if (htmlMostrado) {
            // Se já exibiu HTML, ignora texto puro subsequente
            return;
        }
        std::cout << msg << std::endl;
        ui->textEdit_Results->append(QString::fromStdString(msg));
    }
    QCoreApplication::processEvents();
}

//
// simulator event handlers
//

void MainWindow::_onModelCheckSuccessHandler(ModelEvent* re) {
    // create (and positione and draw) or remove GraphicalModelDataDefinitions based on what actually exists on the model
    Model* model = simulator->getModelManager()->current();
    if (simulator->getModelManager()->current() == re->getModel()) { // the current model is the one changed
        ModelDataManager* dm = model->getDataManager();
        ModelGraphicsView* modelGraphView = ((ModelGraphicsView*)(ui->graphicsView));
        for(auto elemclassname: *dm->getDataDefinitionClassnames()) {
            for (ModelDataDefinition* elem: *dm->getDataDefinitionList(elemclassname)->list()) {
                Util::identification id = elem->getId();
                //modelGraphView->;
            }
        }
    }
}

void MainWindow::_onReplicationStartHandler(SimulationEvent * re) {

    ModelSimulation* sim = simulator->getModelManager()->current()->getSimulation();
    QString text = QString::fromStdString(std::to_string(sim->getCurrentReplicationNumber())) + "/" + QString::fromStdString(std::to_string(sim->getNumberOfReplications()));
    ui->label_ReplicationNum->setText(text);
    int row = ui->tableWidget_Simulation_Event->rowCount();
    ui->tableWidget_Simulation_Event->setRowCount(row + 1);
    QTableWidgetItem* newItem;
    newItem = new QTableWidgetItem(QString::fromStdString("Replication " + std::to_string(re->getCurrentReplicationNumber())));
    ui->tableWidget_Simulation_Event->setItem(row, 2, newItem);

    QCoreApplication::processEvents();
}

void MainWindow::_onSimulationStartHandler(SimulationEvent * re) {
    _actualizeActions();
    ui->progressBarSimulation->setMaximum(simulator->getModelManager()->current()->getSimulation()->getReplicationLength());
    ui->tableWidget_Simulation_Event->setRowCount(0);
    ui->tableWidget_Entities->setRowCount(0);
    ui->tableWidget_Variables->setRowCount(0);
    ui->textEdit_Simulation->clear();
    ui->textEdit_Reports->clear();

    // Fator de conversão para segundos
    Util::TimeUnit replicationBaseTimeUnit = simulator->getModelManager()->current()->getSimulation()->getReplicationBaseTimeUnit();
    double conversionFactorToSeconds = Util::TimeUnitConvert(replicationBaseTimeUnit, Util::TimeUnit(5));
    AnimationTimer::setConversionFactorToSeconds(conversionFactorToSeconds);

    QCoreApplication::processEvents();
}

void MainWindow::_onSimulationPausedHandler(SimulationEvent * re) {
    _actualizeActions();
    QCoreApplication::processEvents();
}

void MainWindow::_onSimulationResumeHandler(SimulationEvent * re) {
    _actualizeActions();

    if (myScene()->getAnimationPaused()) {
        if (!myScene()->getAnimationPaused()->empty()) {
            QList<AnimationTransition *> *animationPaused = myScene()->getAnimationPaused()->value(re->getCurrentEvent());

            if (animationPaused) {
                if (!animationPaused->empty()) {
                    for (AnimationTransition *animation : *animationPaused) {
                        myScene()->runAnimateTransition(animation, re->getCurrentEvent(), true);
                    }
                    animationPaused->clear();
                }
            }
            myScene()->getAnimationPaused()->clear();
        }
    }

    QCoreApplication::processEvents();
}

void MainWindow::_onSimulationEndHandler(SimulationEvent * re) {
    myScene()->getAnimationPaused()->clear();
    _actualizeActions();
    ui->tabWidgetCentral->setCurrentIndex(CONST.TabCentralReportsIndex);
    for (unsigned int i = 0; i < 50; i++) {
        QCoreApplication::processEvents();
    }

    myScene()->clearAnimationsQueue();

    _modelCheked = false;
}

void MainWindow::_onProcessEventHandler(SimulationEvent * re) {
    ui->progressBarSimulation->setValue(simulator->getModelManager()->current()->getSimulation()->getSimulatedTime());
    _actualizeSimulationEvents(re);
    _actualizeDebugEntities(false);
    _actualizeDebugVariables(false);
    _actualizeGraphicalModel(re);
    QCoreApplication::processEvents();
}

void MainWindow::_onEntityCreateHandler(SimulationEvent* re) {

}

void MainWindow::_onEntityRemoveHandler(SimulationEvent* re) {

}

void MainWindow::_onMoveEntityEvent(SimulationEvent *re) {
    // Cria as animações de contadores, variáveis e tempo
    myScene()->animateCounter();
    myScene()->animateVariable();

    // Cria a animação de transição
    if (re) {
        if (re->getCurrentEvent()) {
            if (re->getCurrentEvent()->getComponent()) {
                ModelComponent *source = re->getCurrentEvent()->getComponent();
                ModelComponent *destination = re->getDestinationComponent();

                // Remove animação de fila se for o caso
                myScene()->animateQueueRemove(source);

                myScene()->animateTransition(source, destination, ui->actionActivateGraphicalSimulation->isChecked(), re->getCurrentEvent());
            }
        }
    }
}

void MainWindow::_onAfterProcessEvent(SimulationEvent *re) {
    // Cria as animações de contadores, variáveis e tempo (atualiza assim que termina)
    myScene()->animateCounter();
    myScene()->animateVariable();
    myScene()->animateTimer(simulator->getModelManager()->current()->getSimulation()->getSimulatedTime());
}


void MainWindow::_setOnEventHandlers() {
    OnEventManager* eventManager = simulator->getModelManager()->current()->getOnEventManager();
    eventManager->addOnAfterProcessEventHandler(this, &MainWindow::_onAfterProcessEvent);
    eventManager->addOnEntityCreateHandler(this, &MainWindow::_onEntityCreateHandler);
    eventManager->addOnEntityRemoveHandler(this, &MainWindow::_onEntityRemoveHandler);
    eventManager->addOnEntityMoveHandler(this, &MainWindow::_onMoveEntityEvent);
    eventManager->addOnProcessEventHandler(this, &MainWindow::_onProcessEventHandler);
    eventManager->addOnReplicationStartHandler(this, &MainWindow::_onReplicationStartHandler);
    eventManager->addOnSimulationStartHandler(this, &MainWindow::_onSimulationStartHandler);
    eventManager->addOnSimulationPausedHandler(this, &MainWindow::_onSimulationPausedHandler);
    eventManager->addOnSimulationResumeHandler(this, &MainWindow::_onSimulationResumeHandler);
    eventManager->addOnSimulationEndHandler(this, &MainWindow::_onSimulationEndHandler);
    //@Todo: Check for new events that were created later
}


//-------------------------
// Simulator Fake Plugins
//-------------------------

void MainWindow::_insertPluginUI(Plugin * plugin) {
    if (plugin != nullptr) {
        if (plugin->isIsValidPlugin()) {
            QTreeWidgetItem *treeItemChild = new QTreeWidgetItem();
            //QTreeWidgetItem *treeItem = new QTreeWidgetItem; //(ui->treeWidget_Plugins);
            std::string plugtextAdds = "[" + plugin->getPluginInfo()->getCategory() + "]: ";
            QBrush brush;
            if (plugin->getPluginInfo()->isComponent()) {
                plugtextAdds += " Component";
                //brush.setColor(Qt::white);
                //treeItemChild->setBackground(brush);
                //treeItemChild->setBackgroundColor(Qt::white);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/component.ico"));
            } else {
                plugtextAdds += " DataDefinition";
                //brush.setColor(Qt::lightGray);
                //treeItemChild->setBackground(brush);
                //treeItemChild->setBackgroundColor(Qt::lightGray);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/calendarred.ico"));
                //treeItemChild->setFont(QFont::Style::StyleItalic);
            }
            if (plugin->getPluginInfo()->isSink()) {
                plugtextAdds += ", Sink";
                treeItemChild->setForeground(0, Qt::blue); //setTextColor(0, Qt::blue);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/loadinv.ico"));
            }
            if (plugin->getPluginInfo()->isSource()) {
                plugtextAdds += ", Source";
                treeItemChild->setForeground(0, Qt::blue); //setTextColor(0, Qt::blue);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/load.ico"));
            }
            if (plugin->getPluginInfo()->isReceiveTransfer()) {
                plugtextAdds += ", ReceiveTransfer";
                treeItemChild->setForeground(0, Qt::blue); //setTextColor(0, Qt::blue);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/load.ico"));
            }
            if (plugin->getPluginInfo()->isSendTransfer()) {
                plugtextAdds += ", SendTransfer";
                treeItemChild->setForeground(0, Qt::blue); //setTextColor(0, Qt::blue);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/loadinv.ico"));
            }
            //treeItem->setText(0,QString::fromStdString(plugtextAdds));
            plugtextAdds += "\n\nDescrption: " + plugin->getPluginInfo()->getDescriptionHelp();
            plugtextAdds += "\n\nTemplate: " + plugin->getPluginInfo()->getLanguageTemplate() + " (double click to add to model)";

            QTreeWidgetItem *treeRootItem;
            QString category;
            if (plugin->getPluginInfo()->isComponent())
                category = QString::fromStdString(plugin->getPluginInfo()->getCategory());
            else
                category = "Data Definition";
            QList<QTreeWidgetItem*> founds = ui->treeWidget_Plugins->findItems(category, Qt::MatchContains);
            if (founds.size() == 0) {
                QFont font("Nimbus Sans", 12, QFont::Bold);
                treeRootItem = new QTreeWidgetItem(ui->treeWidget_Plugins);
                treeRootItem->setText(0, category);
                QBrush bforeground(Qt::white);
                treeRootItem->setForeground(0, bforeground);
                QBrush bbackground(Qt::black);
                if (category == "Data Definition") {
                    bbackground.setColor(Qt::darkRed);
                } else if (category == "Discrete Processing") {
                    bbackground.setColor(Qt::darkGreen);
                } else if (category == "Decisions") {
                    bbackground.setColor(Qt::darkYellow);
                } else if (category == "Grouping") {
                    bbackground.setColor(Qt::magenta);
                } else if (category == "Input Output") {
                    bbackground.setColor(Qt::darkCyan);
                } else if (category == "Material Handling") {
                    bbackground.setColor(Qt::darkBlue);
                }
                treeRootItem->setBackground(0, bbackground);
                treeRootItem->setFont(0, font);
                treeRootItem->setExpanded(false); //(true);
                //treeRootItem->sortChildren(0, Qt::AscendingOrder);
                if (plugin->getPluginInfo()->getCategory() == category.toStdString()) {
                    _pluginCategoryColor->insert({plugin->getPluginInfo()->getCategory(), bbackground.color()});
                }
            } else {
                treeRootItem = *founds.begin();
            }
            if (plugin->getPluginInfo()->isComponent() && !plugin->getPluginInfo()->isSendTransfer() && !plugin->getPluginInfo()->isReceiveTransfer() && !plugin->getPluginInfo()->isSink() && !plugin->getPluginInfo()->isSource()) {
                if (treeRootItem->background(0).color().blue() < 32 && treeRootItem->background(0).color().green() < 32 && treeRootItem->background(0).color().red() < 32) {
                    treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentblack.ico"));
                } else if (treeRootItem->background(0).color().red() >= treeRootItem->background(0).color().blue() &&
                           treeRootItem->background(0).color().red() > treeRootItem->background(0).color().green()) {
                    treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentred.ico"));
                } else if (treeRootItem->background(0).color().blue() > treeRootItem->background(0).color().red() &&
                           treeRootItem->background(0).color().blue() > treeRootItem->background(0).color().green()) {
                    treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentblue.ico"));
                } else if (treeRootItem->background(0).color().red() > treeRootItem->background(0).color().blue() &&
                           treeRootItem->background(0).color().green() > treeRootItem->background(0).color().blue()) {
                    treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentyellow.ico"));
                }
            }
            treeItemChild->setWhatsThis(0, QString::fromStdString(plugin->getPluginInfo()->getPluginTypename()));
            /* TODO: Qt6 has no more setTextColor */
            //treeItemChild->setTextColor(0, treeRootItem->background(0).color());
            //treeItemChild->setTextColor(0, treeRootItem->backgroundColor(0));
            treeItemChild->setText(0, QString::fromStdString(plugin->getPluginInfo()->getPluginTypename()));
            treeItemChild->setToolTip(0, QString::fromStdString(plugtextAdds));
            treeItemChild->setStatusTip(0, QString::fromStdString(plugin->getPluginInfo()->getLanguageTemplate()));
            //treeItemChild->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
            treeRootItem->addChild(treeItemChild);
        }
    }
}

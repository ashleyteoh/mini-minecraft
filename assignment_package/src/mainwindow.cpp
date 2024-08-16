#include "mainwindow.h"
#include <ui_mainwindow.h>
#include "cameracontrolshelp.h"
#include "crosshair.h"
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), cHelp()
{
    ui->setupUi(this);
    ui->mygl->setFocus();
    this->playerInfoWindow.show();
    playerInfoWindow.move(QGuiApplication::primaryScreen()->availableGeometry().center() - this->rect().center() + QPoint(this->width() * 0.75, 0));

    connect(ui->mygl, SIGNAL(sig_sendPlayerPos(QString)), &playerInfoWindow, SLOT(slot_setPosText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerVel(QString)), &playerInfoWindow, SLOT(slot_setVelText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerAcc(QString)), &playerInfoWindow, SLOT(slot_setAccText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerLook(QString)), &playerInfoWindow, SLOT(slot_setLookText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerChunk(QString)), &playerInfoWindow, SLOT(slot_setChunkText(QString)));
    connect(ui->mygl, SIGNAL(sig_sendPlayerTerrainZone(QString)), &playerInfoWindow, SLOT(slot_setZoneText(QString)));

    connect(ui->mygl, SIGNAL(sig_inventoryOpenClose(bool)), this, SLOT(slot_inventoryOpenClose(bool)));
    connect(ui->mygl, SIGNAL(sig_sendNumGrass(int)), &inventoryWindow, SLOT(slot_setNumGrass(int)));
    connect(ui->mygl, SIGNAL(sig_sendNumDirt(int)), &inventoryWindow, SLOT(slot_setNumDirt(int)));
    connect(ui->mygl, SIGNAL(sig_sendNumStone(int)), &inventoryWindow, SLOT(slot_setNumStone(int)));
    connect(ui->mygl, SIGNAL(sig_sendNumSand(int)), &inventoryWindow, SLOT(slot_setNumSand(int)));
    connect(ui->mygl, SIGNAL(sig_sendNumWood(int)), &inventoryWindow, SLOT(slot_setNumWood(int)));
    connect(ui->mygl, SIGNAL(sig_sendNumLeaf(int)), &inventoryWindow, SLOT(slot_setNumLeaf(int)));
    connect(ui->mygl, SIGNAL(sig_sendNumIce(int)), &inventoryWindow, SLOT(slot_setNumIce(int)));
    connect(ui->mygl, SIGNAL(sig_sendNumSnow(int)), &inventoryWindow, SLOT(slot_setNumSnow(int)));

    connect(ui->mygl, SIGNAL(sig_sendNumCactus(int)), &inventoryWindow, SLOT(slot_setNumCactus(int)));
    connect(ui->mygl, SIGNAL(sig_sendNumMushStem(int)), &inventoryWindow, SLOT(slot_setNumMushStem(int)));
    connect(ui->mygl, SIGNAL(sig_sendNumMushHat(int)), &inventoryWindow, SLOT(slot_setNumMushHat(int)));
    connect(ui->mygl, SIGNAL(sig_sendNumSandstone(int)), &inventoryWindow, SLOT(slot_setNumSandStone(int)));

    // setupUI();
    setupCrosshairOverlay();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    cHelp.show();
}

void MainWindow::slot_inventoryOpenClose(bool openInventory) {
    if (openInventory) {
        this->inventoryWindow.ui_main = ui;
        this->inventoryWindow.show();
        // open in center
        inventoryWindow.move(QPoint(this->width() * 0.5, this->height() * 0.5) -
                             QPoint(inventoryWindow.width() * 0.5, inventoryWindow.height() * 0.5));
    } else {
        this->inventoryWindow.close();
    }
}

void MainWindow::setupUI() {
    // Inventory* inventory = new Inventory(this);


    QDockWidget* dockWidget = new QDockWidget(tr("Inventory"), this);
    dockWidget->setWidget(&this->inventoryWindow);
    dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

    // Optionally, you can make the dock widget toggleable from a menu
    // QAction *toggleAction = dockWidget->toggleViewAction();
    // menuBar()->addAction(toggleAction);

    // connect(this, &MainWindow::toggleInventoryVisibility, dockWidget, &QDockWidget::setVisible);
}


void MainWindow::setupCrosshairOverlay() {
    Crosshair *overlay = new Crosshair(this);
    overlay->resize(size());  // Cover the whole window
    overlay->show();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    findChild<Crosshair*>()->resize(size());  // Resize overlay when the window size changes
}

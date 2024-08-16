#ifndef INVENTORY_H
#define INVENTORY_H

#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include "ui_mainwindow.h"


namespace Ui {
class Inventory;
}

class Inventory: public QWidget
{
    Q_OBJECT

public:
    explicit Inventory(QWidget *parent = nullptr);
    ~Inventory();

    void keyPressEvent(QKeyEvent *e);
    Ui::MainWindow *ui_main;

public slots:
    void slot_setNumGrass(int);
    void slot_setNumDirt(int);
    void slot_setNumStone(int);
    void slot_setNumSand(int);
    void slot_setNumWood(int);
    void slot_setNumLeaf(int);
    void slot_setNumIce(int);
    void slot_setNumSnow(int);
    void slot_setNumCactus(int);
    void slot_setNumMushHat(int);
    void slot_setNumMushStem(int);
    void slot_setNumSandStone(int);


    void slot_setCurrBlockToGrass();
    void slot_setCurrBlockToDirt();
    void slot_setCurrBlockToStone();
    void slot_setCurrBlockToSand();
    void slot_setCurrBlockToWood();
    void slot_setCurrBlockToLeaf();
    void slot_setCurrBlockToIce();
    void slot_setCurrBlockToSnow();
    void slot_setCurrBlockToCactus();
    void slot_setCurrBlockToMushHat();
    void slot_setCurrBlockToMushStem();
    void slot_setCurrBlockToSandStone();

private slots:
    void on_radioButtonMushHat_clicked();

private:
    Ui::Inventory *ui;
};

#endif // INVENTORY_H

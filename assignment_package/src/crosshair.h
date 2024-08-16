#ifndef CROSSHAIR_H
#define CROSSHAIR_H

#include <QWidget>

class Crosshair : public QWidget {
    Q_OBJECT
public:
    explicit Crosshair(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // CROSSHAIR_H

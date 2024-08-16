#include "crosshair.h"
#include <QPainter>

Crosshair::Crosshair(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);  // Ensure mouse events pass through the widget
}

void Crosshair::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(QPen(Qt::white, 2));  // Set crosshair color and thickness

    int centerX = width() / 2;
    int centerY = height() / 2;
    int lineLength = 10;  // Length of each line of the crosshair

    // Draw vertical line
    painter.drawLine(centerX, centerY - lineLength, centerX, centerY + lineLength);
    // Draw horizontal line
    painter.drawLine(centerX - lineLength, centerY, centerX + lineLength, centerY);
}

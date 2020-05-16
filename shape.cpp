#include "shape.h"

Shape::Shape() {}

Shape::Shape(const QColor &color, const QVector<QPoint> &points, const QString& colorName, const QString& shapeName,
             int shiftX, int shiftY, int size):
        color(color), points(points), colorName(colorName), shapeName(shapeName), shiftX(shiftX), shiftY(shiftY), size(size) {}

void Shape::draw(QPainter &p) const {
    p.setPen(QPen(color));
    p.setBrush(QBrush(color));
    for(int i = 0;i < points.size();i++) {
        p.drawPoint(points[i]);
    }
}

void Shape::drawAsIcon(QPainter &p, int x, int y, int w, int h) const {
    p.setPen(Qt::white);
    p.setBrush(QBrush(Qt::white));
    for(int i = 0;i < points.size();i++) {
        QPointF cur = points[i];
        cur.setX(cur.x() - shiftX);
        cur.setY(cur.y() - shiftY);
        cur.setX(cur.x() / size * w);
        cur.setY(cur.y() / size * h);
        cur.setX(cur.x() + x);
        cur.setY(cur.y() + y);
        p.drawPoint(cur);
    }
}

QString Shape::getColorName() const {
    return colorName;
}

QString Shape::getShapeName() const {
    return shapeName;
}

QColor Shape::getColor() const {
    return color;
}

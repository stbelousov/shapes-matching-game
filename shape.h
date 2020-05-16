#ifndef SHAPE_H
#define SHAPE_H
#include <QPainter>

class Shape {
    QColor color;
    QVector < QPoint > points;
    QString colorName, shapeName;

    int shiftX, shiftY, size;
public:
    Shape();
    Shape(const QColor& color, const QVector < QPoint > &points, const QString& colorName, const QString& shapeName,
          int shiftX, int shiftY, int size);
    void draw(QPainter& p) const;
    void drawAsIcon(QPainter &p, int x, int y, int w, int h) const;
    QString getColorName() const;
    QString getShapeName() const;
    QColor getColor() const;
};

#endif // SHAPE_H

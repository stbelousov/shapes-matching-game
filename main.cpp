#include <QApplication>
#include "widget.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setApplicationName("Shapes Matching Game");
    Widget w;
    w.show();
    return a.exec();
}

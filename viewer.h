#ifndef VIEWER_H
#define VIEWER_H

#include <QWidget>
#include <QVector>
#include <QPainter>
#include <QSoundEffect>
#include <QTimer>
#include <map>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>

#include "shape.h"
#include "widget.h"

namespace Ui {
class Viewer;
}

class Viewer : public QWidget {
    Q_OBJECT

public:
    explicit Viewer(QWidget *parent);
    ~Viewer();

private:
    Ui::Viewer *ui;
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *);

    void drawMenu();
    void drawPlay();
    void drawEndGame();
    void drawHelp();
    void drawScores();

    void keyMenu(QKeyEvent *);
    void keyPlay(QKeyEvent *);
    void keyEndGame(QKeyEvent *);
    void keyHelp(QKeyEvent *);
    void keyScores(QKeyEvent *);

    void play(bool newGame);

    enum FrameType { PLAY, RESUME, SCORES, HELP, EXIT, MENU, ENDGAME };

    QVector < Shape > shapes;
    QTimer timer;
    QString text;
    int curShape, score, totalRounds, round, timerInterval;
    bool ready, shapeTextEquals, clicked, correct, okShown, firstPlay;
    double probGood, probSameShape, probSameColor, probBad;
    QImage leftKey, rightKey, okIcon, failIcon;
    boost::mt19937 rnd;
    FrameType curFrame, curMenuPos;
    QVector < QString > buttons;
    QSoundEffect okSound, failSound;
};

#endif // VIEWER_H

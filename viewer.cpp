#include "viewer.h"
#include "ui_viewer.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QFile>
#include <QUrl>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <set>

Viewer::Viewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Viewer),
    timer(this),
    rnd(time(nullptr)) {
    ui->setupUi(this);

    QPalette pal = this->palette();
    pal.setBrush(QPalette::Background, QBrush(QPixmap(":/img/wallpaper.jpg")));
    this->setPalette(pal);
    this->setAutoFillBackground(true);

    this->setFocus();

    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
    timerInterval = 1000;

    assert(leftKey.load(":/img/left_key.png"));
    assert(rightKey.load(":/img/right_key.png"));
    assert(okIcon.load(":/img/ok.png"));
    assert(failIcon.load(":/img/fail.png"));

    okSound.setSource(QUrl("qrc:/sound/ok.wav"));
    failSound.setSource(QUrl("qrc:/sound/fail.wav"));

    totalRounds = 50;
    ready = false;
    firstPlay = true;
    probGood = 0.5;
    probSameShape = probSameColor = probBad = 0.5 / 3;

    buttons.push_back("New game");
    buttons.push_back("Resume game");
    buttons.push_back("Scoreboard");
    buttons.push_back("Help");
    buttons.push_back("Quit");

    curFrame = MENU;
    curMenuPos = PLAY;
}

Viewer::~Viewer() {
    delete ui;
}

void Viewer::play(bool newGame) {
    if(newGame) {
        firstPlay = false;
        curFrame = PLAY;
        timer.start(timerInterval);
        curShape = score = round = 0;
        clicked = correct = okShown = false;
    } else if(!firstPlay) {
        curFrame = PLAY;
        timer.start(timerInterval);
    }
}

void Viewer::paintEvent(QPaintEvent *) {
    switch(curFrame) {
    case MENU:
        drawMenu();
        break;
    case PLAY:
        drawPlay();
        break;
    case ENDGAME:
        drawEndGame();
        break;
    case HELP:
        drawHelp();
        break;
    case SCORES:
        drawScores();
        break;
    default:
        break;
    }
}

void Viewer::drawMenu() {
    QPainter p(this);

    // Draw header
    QFont font;
    font.setPointSize(20);
    p.setFont(font);
    p.setPen(Qt::white);
    const int textBoxSize = 400;
    p.drawText(this->width() / 2 - textBoxSize / 2, 50 - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "Menu");

    // Draw buttons
    const int buttonWidth = 200;
    const int buttonHeight = 30;
    font.setPointSize(15);
    p.setFont(font);
    for (int i = 0;i < buttons.size();i++) {
        if(i == curMenuPos) {
            p.setPen(QPen(Qt::red));
            p.setBrush(QBrush(Qt::red));
        } else {
            p.setPen(QPen(Qt::gray));
            p.setBrush(QBrush(Qt::gray));
        }
        p.drawRect(this->width() / 2 - buttonWidth / 2, (2 * i + 4) * buttonHeight, buttonWidth, buttonHeight);
        p.setPen(QPen(Qt::white));
        p.drawText(this->width() / 2 - textBoxSize / 2, (2 * i + 4 + 0.5) * buttonHeight - textBoxSize / 2,
                   textBoxSize, textBoxSize, Qt::AlignCenter, buttons[i]);
    }
}

void Viewer::drawPlay() {
    if(okShown) {
        okShown = false;
        clicked = false;
    }
    if(!clicked) {
        round++;
    }
    if(round > totalRounds) {
        curFrame = ENDGAME;
        firstPlay = true;

        QString playerName = QInputDialog::getText(this, "Game over", "Your name (for scoreboard):");
        if (!playerName.isEmpty()) {
            FILE* scoreboard = fopen("scoreboard.txt", "a");
            assert(scoreboard);
            fprintf(scoreboard, "%s\t%d\n", playerName.toStdString().c_str(), score);
            fclose(scoreboard);
        }

        update();
        return;
    }

    QPainter p(this);

    // Draw frame
    p.setPen(QPen(Qt::white));
    p.setBrush(QBrush(Qt::white));
    p.drawRect(this->width() / 2 - this->height() / 4, this->height() / 4, this->height() / 2, this->height() / 2);

    // Draw shape
    if(!clicked) {
        int prevShape = curShape;
        while(curShape == prevShape) {
            curShape = boost::uniform_int<>(0, shapes.size() - 1)(rnd);
        }
    }
    shapes[curShape].draw(p);

    // Print text
    if(!clicked) {
        double dice = 1.0 * boost::uniform_int<>(0, 1000000)(rnd) / 1000000;
        if(dice < probGood) {
            text = shapes[curShape].getColorName() + " " + shapes[curShape].getShapeName();
        } else if(dice < probGood + probSameShape) {
            text = shapes[boost::uniform_int<>(0, shapes.size() - 1)(rnd)].getColorName() + " " + shapes[curShape].getShapeName();
        } else if(dice < probGood + probSameShape + probSameColor) {
            text = shapes[curShape].getColorName() + " " + shapes[boost::uniform_int<>(0, shapes.size() - 1)(rnd)].getShapeName();
        } else {
            text = shapes[boost::uniform_int<>(0, shapes.size() - 1)(rnd)].getColorName()
                   + " " + shapes[boost::uniform_int<>(0, shapes.size() - 1)(rnd)].getShapeName();
        }
    }
    QFont font;
    font.setPointSize(15);
    p.setFont(font);
    p.setPen(Qt::white);
    const int textBoxSize = 400;
    p.drawText(this->width() / 2 - textBoxSize / 2, this->height() * 5 / 6 - 20 - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, text + " ?");
    shapeTextEquals = (text == shapes[curShape].getColorName() + " " + shapes[curShape].getShapeName());

    QRect rect = leftKey.rect();
    rect.moveCenter(QPoint(this->width() / 3, this->height() - 35));
    p.drawImage(rect.topLeft(), leftKey);
    rect = rightKey.rect();
    rect.moveCenter(QPoint(this->width() * 2 / 3, this->height() - 35));
    p.drawImage(rect.topLeft(), rightKey);
    p.drawText(this->width() / 3 - textBoxSize / 2, this->height() - 70 - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "No");
    p.drawText(this->width() * 2 / 3 - textBoxSize / 2, this->height() - 70 - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "Yes");

    p.drawText(this->width() - 60 - textBoxSize / 2, 30 - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "Score: " + QString::number(score));
    p.drawText(90 - textBoxSize / 2, 30 - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "Round: " + QString::number(round) + " / " + QString::number(totalRounds));

    if(clicked) {
        if(correct) {
            rect = okIcon.rect();
            rect.moveCenter(QPoint(this->width() / 2, this->height() - 45));
            p.drawImage(rect.topLeft(), okIcon);
            okSound.play();
        } else {
            rect = failIcon.rect();
            rect.moveCenter(QPoint(this->width() / 2, this->height() - 45));
            p.drawImage(rect.topLeft(), failIcon);
            failSound.play();
        }
        okShown = true;
        timer.start(timerInterval);
    }
}

void Viewer::drawEndGame() {
    QPainter p(this);

    // Draw header
    QFont font;
    font.setPointSize(20);
    p.setFont(font);
    p.setPen(Qt::white);
    const int textBoxSize = 400;
    p.drawText(this->width() / 2 - textBoxSize / 2, 50 - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "Game over");
    p.drawText(this->width() / 2 - textBoxSize / 2, 100 - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "Your score: " + QString::number(score));

    // Draw buttons
    const int buttonWidth = 200;
    const int buttonHeight = 30;
    font.setPointSize(15);
    p.setFont(font);
    p.setPen(QPen(Qt::red));
    p.setBrush(QBrush(Qt::red));
    p.drawRect(this->width() / 2 - buttonWidth / 2, 150, buttonWidth, buttonHeight);
    p.setPen(QPen(Qt::white));
    p.drawText(this->width() / 2 - textBoxSize / 2, 150 + 0.5 * buttonHeight - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "To Menu");
}

void Viewer::drawHelp() {
    QPainter p(this);

    // Draw header
    QFont font;
    font.setPointSize(20);
    p.setFont(font);
    p.setPen(Qt::white);
    const int textBoxSize = 400;
    p.drawText(this->width() / 2 - textBoxSize / 2, 50 - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "Game rules");

    // Draw rules
    font.setPointSize(15);
    p.setFont(font);
    const int helpTextBoxWidth = 600;
    const int helpTextBoxHeight = 700;
    p.drawText(this->width() / 2 - helpTextBoxWidth / 2, 170 - helpTextBoxHeight / 2,
               helpTextBoxWidth, helpTextBoxHeight, Qt::AlignCenter | Qt::TextWordWrap,
               "The game consists of " + QString::number(totalRounds) + " rounds. "
               + "Each round lasts one second. In each round, you have to answer whether the shown image corresponds "
               + "to the provided description or not. All images are coloured geometric shapes. All variants of shapes and colours used in this "
               + "game are shown below. Each correct answer gives you +1 to your score, an incorrect one gives -1 and if you skip the question, "
               + "your score wouldn't change. You can pause the game and open the menu by pressing ESC at any time.");

    // Draw shapes
    std::map < QString, Shape > shapeNames;
    for(int i = 0;i < shapes.size();i++) {
        QString name = shapes[i].getShapeName();
        name[0] = name[0].toUpper();
        shapeNames.insert(std::make_pair(name, shapes[i]));
    }
    font.setPointSize(12);
    p.setFont(font);
    p.setBrush(QBrush(Qt::white));
    int gap = this->width() / shapeNames.size();
    int cnt = 0;
    for(std::map < QString, Shape > :: iterator it = shapeNames.begin();it != shapeNames.end();++it) {
        p.drawText(cnt * gap + 40 - textBoxSize / 2, this->height() - 200 - textBoxSize / 2,
                   textBoxSize, textBoxSize, Qt::AlignCenter, it->first);
        it->second.drawAsIcon(p, cnt * gap + 40 - 20 / 2, this->height() - 170 - 20 / 2, 20, 20);
        cnt++;
    }

    // Draw colors
    std::map < QString, QColor > colorNames;
    for(int i = 0;i < shapes.size();i++) {
        colorNames.insert(std::make_pair(shapes[i].getColorName(), shapes[i].getColor()));
    }
    gap = this->width() / colorNames.size();
    cnt = 0;
    for(std::map < QString, QColor > :: iterator it = colorNames.begin();it != colorNames.end();++it) {
        p.setPen(Qt::white);
        p.drawText(cnt * gap + 47 - textBoxSize / 2, this->height() - 120 - textBoxSize / 2,
                   textBoxSize, textBoxSize, Qt::AlignCenter, it->first);
        p.setPen(it->second);
        p.setBrush(QBrush(it->second));
        p.drawEllipse(cnt * gap + 40, this->height() - 100, 15, 15);
        cnt++;
    }

    // Draw buttons
    const int buttonWidth = 200;
    const int buttonHeight = 30;
    font.setPointSize(15);
    p.setFont(font);
    p.setPen(QPen(Qt::red));
    p.setBrush(QBrush(Qt::red));
    p.drawRect(this->width() / 2 - buttonWidth / 2, this->height() - 50, buttonWidth, buttonHeight);
    p.setPen(QPen(Qt::white));
    p.drawText(this->width() / 2 - textBoxSize / 2, this->height() - 50 + 0.5 * buttonHeight - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "To Menu");
}

void Viewer::drawScores() {
    QPainter p(this);

    // Draw header
    QFont font;
    font.setPointSize(20);
    p.setFont(font);
    p.setPen(Qt::white);
    const int textBoxSize = 235;
    p.drawText(this->width() / 2 - textBoxSize / 2, 50 - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "Scoreboard");

    // Get scores
    QVector < std::pair < int, QString > > data;
    FILE* scoreboard = fopen("scoreboard.txt", "r");
    if(scoreboard) {
        char line[1001], name[1001];
        int cnt;
        while(fgets(line, 1001, scoreboard)) {
            sscanf(line, "%1000[^\t]%d", name, &cnt);
            data.push_back(std::make_pair(cnt, name));
        }
        fclose(scoreboard);
    }
    std::sort(data.begin(), data.end(), std::greater < std::pair < int, QString > > ());

    // Draw scoreboard
    for(int i = 0;i < std::min(8, data.size());i++) {
        if(i != 0) {
            p.drawLine(60, (i + 2.5) * 40, this->width() - 60, (i + 2.5) * 40);
        }
        p.drawText(this->width() / 4 - textBoxSize / 2, (i + 3) * 40 - textBoxSize / 2,
                   textBoxSize, textBoxSize, Qt::AlignCenter, data[i].second);
        p.drawText(this->width() * 3 / 4 - textBoxSize / 2, (i + 3) * 40 - textBoxSize / 2,
                   textBoxSize, textBoxSize, Qt::AlignCenter, QString::number(data[i].first));
    }

    // Draw buttons
    const int buttonWidth = 200;
    const int buttonHeight = 30;
    font.setPointSize(15);
    p.setFont(font);
    p.setPen(QPen(Qt::red));
    p.setBrush(QBrush(Qt::red));
    p.drawRect(this->width() / 2 - buttonWidth / 2, this->height() - 50, buttonWidth, buttonHeight);
    p.setPen(QPen(Qt::white));
    p.drawText(this->width() / 2 - textBoxSize / 2, this->height() - 50 + 0.5 * buttonHeight - textBoxSize / 2,
               textBoxSize, textBoxSize, Qt::AlignCenter, "To Menu");
}

void Viewer::keyPressEvent(QKeyEvent *event) {
    switch(curFrame) {
    case MENU:
        keyMenu(event);
        break;
    case PLAY:
        keyPlay(event);
        break;
    case ENDGAME:
        keyEndGame(event);
        break;
    case HELP:
        keyHelp(event);
        break;
    case SCORES:
        keyScores(event);
        break;
    default:
        break;
    }
}

void Viewer::keyPlay(QKeyEvent *event) {
    if(event->key() == Qt::Key_Escape) {
        curFrame = MENU;
        update();
        return;
    }
    if(clicked) {
        return;
    }
    if(event->key() == Qt::Key_Left) {
        correct = !shapeTextEquals;
    } else if(event->key() == Qt::Key_Right) {
        correct = shapeTextEquals;
    } else {
        return;
    }
    score = std::max(0, score + correct * 2 - 1);
    clicked = true;
    update();
}

void Viewer::keyMenu(QKeyEvent *event) {
    if(event->key() == Qt::Key_Up) {
        curMenuPos = (FrameType)std::max(0, curMenuPos - 1);
    } else if(event->key() == Qt::Key_Down) {
        curMenuPos = (FrameType)std::min(buttons.size() - 1, curMenuPos + 1);
    } else if(event->key() == Qt::Key_Return) {
        switch(curMenuPos) {
        case PLAY:
            play(true);
            break;
        case RESUME:
            play(false);
            break;
        case HELP:
            curFrame = HELP;
            break;
        case SCORES:
            curFrame = SCORES;
            break;
        case EXIT:
            this->parentWidget()->close();
            break;
        default:
            break;
        }
    } else if(event->key() == Qt::Key_Escape) {
        play(false);
    }
    update();
}

void Viewer::keyEndGame(QKeyEvent *event) {
    if(event->key() == Qt::Key_Return) {
        curFrame = MENU;
    }
    update();
}

void Viewer::keyHelp(QKeyEvent *event) {
    if(event->key() == Qt::Key_Return) {
        curFrame = MENU;
    }
    update();
}

void Viewer::keyScores(QKeyEvent *event) {
    if(event->key() == Qt::Key_Return) {
        curFrame = MENU;
    }
    update();
}

void Viewer::resizeEvent(QResizeEvent *) {
    if(!ready) {
        ready = true;

        QVector < QPoint > triangle, circle, square, cross, plus, circumference, rhombus;
        QVector < QColor > colors;
        QVector < QString > colorNames;
        int size = 120;
        int center = size / 2;
        int shiftX = this->width() / 2 - center;
        int shiftY = this->height() / 2 - center;
        for(int i = 0;i <= size;i++) {
            for(int j = 0;j <= size;j++) {
                QPoint point(shiftX + i, shiftY + j);
                square.push_back(point);
                if(abs(i - center) <= 0.7 * (size / 2 - abs(j - center))) {
                    rhombus.push_back(point);
                }
                if((i - center) * (i - center) + (j - center) * (j - center) < center * center) {
                    circle.push_back(point);
                    if((i - center) * (i - center) + (j - center) * (j - center) > center * center - 30 * 30) {
                        circumference.push_back(point);
                    }
                }
                double delta = double(j) * center / size;
                if(center - delta <= i && i <= center + delta) {
                    triangle.push_back(point);
                }
                if((j - 7 <= i && i <= j + 7) || (j - 7 <= size - i && size - i <= j + 7)) {
                    cross.push_back(point);
                }
                if((i - 7 <= center && center <= i + 7) || (j - 7 <= center && center <= j + 7)) {
                    plus.push_back(point);
                }
            }
        }
        colors.push_back(Qt::black);
        colors.push_back(Qt::red);
        colors.push_back(Qt::darkGreen);
        colors.push_back(Qt::blue);
        colors.push_back(Qt::yellow);
        colors.push_back(Qt::magenta);
        colors.push_back(Qt::cyan);
        colors.push_back(QColor(0xFF, 0xA5, 0x00));
        colorNames.push_back("Black");
        colorNames.push_back("Red");
        colorNames.push_back("Green");
        colorNames.push_back("Blue");
        colorNames.push_back("Yellow");
        colorNames.push_back("Magenta");
        colorNames.push_back("Cyan");
        colorNames.push_back("Orange");
        for(int i = 0;i < colors.size();i++) {
            shapes.push_back(Shape(colors[i], triangle, colorNames[i], "triangle", shiftX, shiftY, size));
            shapes.push_back(Shape(colors[i], square, colorNames[i], "square", shiftX, shiftY, size));
            shapes.push_back(Shape(colors[i], circle, colorNames[i], "circle", shiftX, shiftY, size));
            shapes.push_back(Shape(colors[i], cross, colorNames[i], "cross", shiftX, shiftY, size));
            shapes.push_back(Shape(colors[i], plus, colorNames[i], "plus", shiftX, shiftY, size));
            shapes.push_back(Shape(colors[i], circumference, colorNames[i], "circumference", shiftX, shiftY, size));
            shapes.push_back(Shape(colors[i], rhombus, colorNames[i], "rhombus", shiftX, shiftY, size));
        }
    }
}

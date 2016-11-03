#include "myscene.h"
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "player.h"
#include "coin.h"
#include "backgrounditem.h"
#include "tube.h"

MyScene::MyScene(QObject *parent) :
    QGraphicsScene(parent)
  , m_velocity(7)
  , m_skippedMoving(0)
  , m_groundLevel(0)
  , m_minX(0)//minimum x coordinate of world
  , m_maxX(0)//maximum x coordinate of world
  , m_jumpAnimation(new QPropertyAnimation(this))
  , m_jumpHeight(180)
  , m_fieldWidth(8000)//width of the virtual world
  , m_player()
  , m_sky(0)
  , m_Scene(0)
  , m_tube(0)

{
    initPlayField();

    //timer emits a timeout signal every 20 milliseconds
    //Then we connect that signal to the scene's slot called movePlayer()
    //Pressing the arrow keys starts the timer
    m_timer.setInterval(20);
    connect(&m_timer, &QTimer::timeout, this, &MyScene::movePlayer);

    jumpSound = new QMediaPlayer();
    jumpSound->setMedia(QUrl("qrc:/music/jump.mp3"));

    m_jumpAnimation->setTargetObject(this);
    m_jumpAnimation->setPropertyName("jumpFactor");
    m_jumpAnimation->setStartValue(0);
    m_jumpAnimation->setKeyValueAt(0.5, 1);
    m_jumpAnimation->setEndValue(0);
    m_jumpAnimation->setDuration(800);
    m_jumpAnimation->setEasingCurve(QEasingCurve::OutInQuad);
    connect(this, &MyScene::jumpFactorChanged, this, &MyScene::jumpPlayer);

}

void MyScene::keyPressEvent(QKeyEvent *event)
{

    if (event->isAutoRepeat())  //First check if the key event was triggered because of an auto repeat
            return;             // Meaning that you are holding down the key. Qt will continuously deliver the key press event

    switch (event->key()) {
    case Qt::Key_Right:
        m_player->addDirection(1);
        checkTimer();
        break;
    case Qt::Key_Left:
        m_player->addDirection(-1);
        checkTimer();
        break;
    case Qt::Key_Space:
        if (QAbstractAnimation::Stopped == m_jumpAnimation->state()) {
            m_jumpAnimation->start();

            if (jumpSound->state() == QMediaPlayer::PlayingState){
               jumpSound->setPosition(0);
           }
           else if (jumpSound->state() == QMediaPlayer::StoppedState){
               jumpSound->play();
           }

            //m_timer.start();
        }
        break;
    default:
        break;
    }
}

void MyScene::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    switch (event->key()) {
    case Qt::Key_Right:
        m_player->addDirection(-1);
        checkTimer();
        break;
    case Qt::Key_Left:
        m_player->addDirection(1);
        checkTimer();
        break;
        //    case Qt::Key_Space:
        //        return;
        //        break;
    default:
        break;
    }
}

void MyScene::movePlayer()
{
    m_player->nextFrame();

    int direction = m_player->direction();     //Cache the player's current direction in a local variable to avoid multiple calls of direction()
    if (0 == direction)                       //check whether the player is moving at all. If they aren't, we exit the function
        return;


    const int dx = direction * m_velocity; //we calculate the shift the player item should get and store it in dx.
                                          //The distance the player should move every 20 milliseconds is defined by the member variable m_velocity,expressed in pixels
                                         //Multiplied by the direction (which could only be 1 or -1 at this point), we get a shift of the player by 4 pixels to the right or to the left.

    qreal newPos = m_realPos + dx;     //Based on this shift, we calculate the new x position of the player and store it in newPos.
    if (newPos < m_minX)              //check whether that new position is inside the range of m_minX and m_maxX, two member variables that are already calculated and set up properly at this point
        newPos = m_minX;
    else if (newPos > m_maxX)       //if the new position is not equal to the actual position, which is stored in m_realPos, we proceed by assigning the new position as the current one
        newPos = m_maxX;
    if (newPos == m_realPos)      //Otherwise, we exit the function since there is nothing to move.
        return;
    m_realPos = newPos;

    const int rightBorder = 970 - m_player->boundingRect().width(); //How far the character can move before view moves
    const int leftBorder = 970;

    if (direction > 0) {

        if (m_realPos > m_fieldWidth - (width() - rightBorder)) {
            m_player->moveBy(dx, 0);
        } else {
            if (m_realPos - m_skippedMoving < rightBorder) {   //If the position of the character in "view coordinates", calculated by m_realPos – m_skippedMoving,
                m_player->moveBy(dx, 0);                      //is smaller than rightBorder, then move the character by calling moveBy()
            } else {
                m_skippedMoving += dx;                       //The m_skippedMoving element is the difference between the view's x coordinate and the virtual world's x coordinate
            }
        }
    } else {
        if (m_realPos < leftBorder && m_realPos >= m_minX) {

            m_player->moveBy(dx, 0);
        } else {
            if (m_realPos - m_skippedMoving > leftBorder) {
                m_player->moveBy(dx, 0);
            } else {
                m_skippedMoving = qMax(0, m_skippedMoving + dx);
            }
        }

    }

    /*
    MOVE THE BACKGROUND
    ff is the minimum shift (0 * shift, which equals 0) and the maximum shift (1 * shift, which equals shift).
    The calculation reads: If we subtract the width of the view (width()) from the virtual field's width m_fieldWidth, we have the area
    where the player isn't moved by (m_player->moveBy()) because in that range only thebackground should move.
    How often the moving of the player was skipped is saved in m_skippedMoving. So by
    dividing m_skippedMoving through m_fieldWidth – width(), we get the needed factor.
    */
    qreal ff = qMin(1.0, m_skippedMoving/(m_fieldWidth - width()));
    m_sky->setPos(-(m_sky->boundingRect().width() - width()) * ff, 0);

    foreach(Coin *coin, m_coins){
        coin->setPos (coin->pos().x() + (coin->boundingRect().width() - width()) * (ff * direction)/50, coin->y());
    }

    m_Scene->setPos(-(m_Scene->boundingRect().width() - width()) * ff, m_Scene->y());
    flag->setPos(6500+(flag->boundingRect().width() - width()) * ff*5, flag->y());
    m_tube->setPos(+(m_tube->boundingRect().width() - width()) * ff*5, m_tube->y());
    m_ground->setPos(-(m_ground->boundingRect().width() - width()) * ff, m_ground->y());

   // checkColliding();
}

void MyScene::jumpPlayer(){

    if (QAbstractAnimation::Stopped == m_jumpAnimation->state())
        return;

    const qreal y = (m_groundLevel - m_player->boundingRect().height()) - m_jumpAnimation->currentValue().toReal() * m_jumpHeight;
    m_player->setPos(m_player->pos().x(), y);

    //checkColliding();
}

// All animations inside the playing field are done by moving items, not the scene.
void MyScene::initPlayField(){

    setSceneRect(0,0,1280,720);
    m_groundLevel = 660;

    m_sky = new BackgroundItem(QPixmap(":images/sky"));
    addItem(m_sky);

    m_ground = new BackgroundItem(QPixmap(":images/ground"));
    addItem(m_ground);
    m_ground->setPos(0, m_groundLevel );

    m_Scene = new BackgroundItem(QPixmap(":images/Scene"));
    m_Scene->setPos(0, m_groundLevel - m_Scene->boundingRect().height());
    addItem(m_Scene);

    flag = new BackgroundItem(QPixmap(":images/flag"));
    flag->setPos(6500, m_groundLevel - flag->boundingRect().height());
    addItem(flag);

    m_player = new Player();
    m_minX = m_player->boundingRect().width()/2 ;
    m_maxX = m_fieldWidth - m_player->boundingRect().width() * 1.5;
    m_player->setPos(m_minX, m_groundLevel - m_player->boundingRect().height() );
    m_realPos = m_minX;
    addItem(m_player);

    int xrang = (m_maxX - m_minX) * 0.94;
    m_tube = new Tube();
    for (int i = 0; i < 10; ++i) {
        Tube *T = new Tube(m_tube);
        T->setPos(m_minX + qrand()%xrang, 0);
    }
    addItem(m_tube);
    m_tube->setPos(0, m_groundLevel - m_tube->boundingRect().height());

      //ADD COINS
      int xrange = (m_maxX - m_minX) * 0.94;
      int hrange = m_jumpHeight;

      for (int i = 0; i < 55; ++i) {
        Coin *C = new Coin();
        C->setPos(m_minX + qrand()%xrange, qrand()%hrange+400);
        addItem(C);
        m_coins.append(C);
      }

      startTimer( 100 );
}

//TIMER EVENT ADDED FOR COINS
void MyScene::timerEvent(QTimerEvent *){
    foreach (Coin *coin, m_coins) {
        coin->nextFrame2();
    }
}

qreal MyScene::jumpFactor() const
{
    return m_jumpFactor;
}

void MyScene::setJumpFactor(const qreal &jumpFactor)
{
    if (m_jumpFactor == jumpFactor)
        return;

    m_jumpFactor = jumpFactor;
    emit jumpFactorChanged(m_jumpFactor);
}

void MyScene::checkTimer()
{

    //If player is not moving, stop the timer
    if (0 == m_player->direction())
        m_timer.stop();
    // check if timer is already active, it will only start if not already active.
    else if (!m_timer.isActive())
        m_timer.start();
}



/*
void MyScene::checkColliding()
{
    QList<QGraphicsItem*> items =  collidingItems(m_player);
   for (int i = 0, total = items.count(); i < total; ++i) {
        if (Coin *c = qgraphicsitem_cast<Coin*>(items.at(i)))
           c->explode();
    }

}

*/

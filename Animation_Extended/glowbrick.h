#ifndef GLOWBRICK_H
#define GLOWBRICK_H
#include <QGraphicsItem>
#include <QPixmap>


class GlowBrick : public QGraphicsItem
{
public:
    GlowBrick(QGraphicsItem *parent = 0);
    void nextFrame5();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


private:

    int mCurrentFrame5;
    QPixmap mPixmap5;

};
#endif // GLOWBRICK_H

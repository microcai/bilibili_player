

#pragma once

#include <QGraphicsItem>
#include <QGraphicsObject>

class QGraphicsBusybufferingItem : public QGraphicsObject
{
	Q_OBJECT

	Q_PROPERTY(qreal Rotate MEMBER  rotate NOTIFY RotateChanged)


#ifndef Q_MOC_RUN
	Q_INTERFACES(QGraphicsItem)
#endif


public:

	QGraphicsBusybufferingItem(QGraphicsItem* parent = nullptr);

Q_SIGNALS:
	void RotateChanged(qreal);

protected Q_SLOTS:
	void redraw();

protected:
	virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

private:
	qreal rotate = 0;

};


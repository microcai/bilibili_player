
#include "qgraphicsbusybufferingitem.hpp"
#include <QPainter>
#include <QRectF>

#include <QPropertyAnimation>
#include <QGraphicsScene>
#include <QGraphicsView>

QGraphicsBusybufferingItem::QGraphicsBusybufferingItem(QGraphicsItem* parent)
	: QGraphicsObject(parent)
{

	connect(this, SIGNAL(RotateChanged(qreal)), this, SLOT(redraw()));


	auto animation = new QPropertyAnimation(this, "Rotate");

	animation->setDuration(2500);
	animation->setStartValue(0);
	animation->setEndValue(360);
	animation->start();

	connect(animation, SIGNAL(finished()), animation, SLOT(start()));

}

void QGraphicsBusybufferingItem::redraw()
{
	update();
}

QRectF QGraphicsBusybufferingItem::boundingRect() const
{
	// 考虑dip
	qreal dpiX = 72;
	qreal dpiY = 72;

	if(!scene()->views().empty())
	{
		dpiX = scene()->views().at(0)->logicalDpiX();
		dpiY = scene()->views().at(0)->logicalDpiY();
	}

	// 然后从 DPI 计算应该有的大小.

	// 大小是 24point
	// 因此计算是  piontsize/72*dpi

	return QRect(-24.0/72*dpiX, -24.0/72*dpiY, 48.0/72*dpiX*2, 48.0/72*dpiY*2);
}

void QGraphicsBusybufferingItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	// 考虑dip
	qreal dpiX = 72;
	qreal dpiY = 72;

	if(!scene()->views().empty())
	{
		dpiX = scene()->views().at(0)->logicalDpiX();
		dpiY = scene()->views().at(0)->logicalDpiY();
	}

	// 在这里绘制 等待的圈圈
	// QPainter 实际上是 QOpenGLPainter 哦！ 硬件加速的

	QBrush my_brush(QColor(255,0,0, 160), Qt::SolidPattern);

	QPen my_pen(my_brush, 5, Qt::SolidLine);
// 	my_brush.setStyle(Qt::RadialGradientPattern);

	painter->setBrush(my_brush);

	painter->setPen(my_pen);

	painter->setRenderHint(QPainter::HighQualityAntialiasing);

	const int num_of_juhuaban = 8;

	for (int i = 0; i< num_of_juhuaban; i++)
	{
		QTransform model;

		model.scale(dpiX/72, dpiY/72);

		model.rotate(rotate);

		model.rotate(360/num_of_juhuaban * i);

		model.translate(15,0);

		// 有了transform 了，设定
		QTransform old_transform = old_transform = painter->transform();

		painter->setTransform(model, true);

		painter->drawArc(QRectF(-3.0, -3.0, 6.0, 6.0), 0, 5760);

		painter->setTransform(old_transform);

	}
}

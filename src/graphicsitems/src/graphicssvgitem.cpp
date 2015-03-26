

#include "graphicssvgitem.hpp"
#include <QPainter>

GraphicsSvgItem::GraphicsSvgItem(QGraphicsItem* parentItem)
	: QGraphicsSvgItem(parentItem)
{

}

GraphicsSvgItem::GraphicsSvgItem(const QString& fileName, QGraphicsItem* parentItem)
	: QGraphicsSvgItem(fileName, parentItem)
{

}


QRectF GraphicsSvgItem::boundingRect() const
{
	auto br = QGraphicsSvgItem::boundingRect();

	br.moveCenter(QPointF());

	return br;
}

void GraphicsSvgItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->save();

	// 移动后 paint !
	auto br = QGraphicsSvgItem::boundingRect();

	QTransform move_centor;

	move_centor.translate(-br.width()/2, -br.height()/2);

	painter->setTransform(move_centor, true);

	QGraphicsSvgItem::paint(painter, option, widget);

	painter->restore();
}


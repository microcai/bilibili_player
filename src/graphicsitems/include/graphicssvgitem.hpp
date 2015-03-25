
#pragma once

#include <QGraphicsSvgItem>

class GraphicsSvgItem : public QGraphicsSvgItem
{
    Q_OBJECT
public:
	public:
    GraphicsSvgItem(QGraphicsItem *parentItem=0);
    GraphicsSvgItem(const QString &fileName, QGraphicsItem *parentItem=0);

public:
	virtual QRectF boundingRect() const;
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
};


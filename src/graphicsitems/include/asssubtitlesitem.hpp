
#pragma once

#include <QImage>
#include <QGraphicsObject>
class AssSubtitlesItemPrivate;

class AssSubtitlesItem : public QGraphicsObject
{
	Q_OBJECT
public:
	AssSubtitlesItem(QGraphicsItem* parent = nullptr);
	AssSubtitlesItem(QString filename, QGraphicsItem* parent = nullptr);

	virtual QPainterPath shape() const;
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
	virtual QRectF boundingRect() const;

	virtual int type() const{return 65538;}
	void update_play_position(qulonglong pos);

	void update_video_size(QSizeF);

private:
	QRectF unified_rect;
	QImage m_texture;

	class AssSubtitlesItemPrivate* const d_ptr;
	Q_DECLARE_PRIVATE(AssSubtitlesItem)
};

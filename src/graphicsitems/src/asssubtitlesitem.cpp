extern "C" {
#include <ass/ass.h>
}

#include "asssubtitlesitem.hpp"
#include <QPainter>
#include <QRectF>
#include <QBitmap>


class AssSubtitlesItemPrivate
{
public:
    AssSubtitlesItemPrivate(QString filename, AssSubtitlesItem* q);
    AssSubtitlesItemPrivate(AssSubtitlesItem* q);
    virtual ~AssSubtitlesItemPrivate();

protected:

	void ass_setup();

private:
    AssSubtitlesItem* const q_ptr;
    Q_DECLARE_PUBLIC(AssSubtitlesItem)

	qlonglong _current_pos;
	qlonglong _next_pos;

	ASS_Library* _ass_library;
	ASS_Track* _ass_track;
    ASS_Renderer* _ass_render;
    ASS_Image* _ass_frame = nullptr;
};

void AssSubtitlesItemPrivate::ass_setup()
{
	_ass_library = ass_library_init();
	_ass_render = ass_renderer_init(_ass_library);

    ass_set_frame_size(_ass_render, 1280, 720);
    ass_set_fonts(_ass_render, NULL, "Sans", 1, NULL, 1);
}

AssSubtitlesItemPrivate::AssSubtitlesItemPrivate(AssSubtitlesItem* q)
	: q_ptr(q)
{
	ass_setup();
	_ass_track = ass_new_track(_ass_library);
	_next_pos = ass_step_sub(_ass_track, _current_pos, 0);
}

AssSubtitlesItemPrivate::AssSubtitlesItemPrivate(QString filename, AssSubtitlesItem* q)
	: q_ptr(q)
{
	ass_setup();
	_ass_track = ass_read_file(_ass_library, &(filename.toStdString()[0]), (char*) "UTF-8");

	_next_pos = ass_step_sub(_ass_track, _current_pos, 0);

}

AssSubtitlesItemPrivate::~AssSubtitlesItemPrivate()
{
	ass_renderer_done(_ass_render);
	ass_free_track(_ass_track);
	ass_library_done(_ass_library);
}

AssSubtitlesItem::AssSubtitlesItem(QGraphicsItem* parent)
	: QGraphicsObject(parent)
	, d_ptr(new AssSubtitlesItemPrivate(this))
{

}

AssSubtitlesItem::AssSubtitlesItem(QString filename, QGraphicsItem* parent)
	: QGraphicsObject(parent)
	, d_ptr(new AssSubtitlesItemPrivate(filename, this))
{
}

void AssSubtitlesItem::update_video_size(QSizeF s)
{
	Q_D(AssSubtitlesItem);

	ass_set_frame_size(d->_ass_render, s.width(), s.height());
}

void AssSubtitlesItem::update_play_position(qulonglong pos)
{
	Q_D(AssSubtitlesItem);

	int detect_change = 0;

	d->_ass_frame = ass_render_frame(d->_ass_render, d->_ass_track, pos, &detect_change);

	if (!(d->_ass_frame))
		return;
	if (detect_change == 0)
		return;

	// 计算哦！

	QRectF unified;

	ASS_Image* _frame;
	_frame = d->_ass_frame;

	do{

		QRectF thisbmp(_frame->dst_x, _frame->dst_y, _frame->w, _frame->h);
		unified = unified.united(thisbmp);

	}while(_frame = _frame->next);
	unified_rect = unified;

	m_texture = QImage(unified_rect.size().toSize(), QImage::Format_ARGB32_Premultiplied);

	switch(detect_change)
	{
		case 2:
		case 3:
			update();
		case 1:
			prepareGeometryChange();
	}
}

#include <QDebug>
#include <QTimer>

QPainterPath AssSubtitlesItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void AssSubtitlesItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_D(AssSubtitlesItem);
	if (!d->_ass_frame)
		return;

	ASS_Image* _frame = d->_ass_frame;

	_frame = d->_ass_frame;

	do{
		// 开始渲染到 QImage 里
		QSize s(_frame->w, _frame->w);

		QImage img(_frame->bitmap, _frame->w, _frame->h, _frame->stride, QImage::Format_Indexed8);

		QVector<QRgb> ctable(256);

		ctable.reserve(256);

		float A = (255 - (_frame->color & 0xff)) / 255.0;
		float B =  ( (_frame->color & 0xFF00) >> 8);
		float G =  ( (_frame->color & 0xFF0000) >> 16);
		float R =  ( (_frame->color & 0xFF000000) >> 24);

		for(int i=0; i<256; i++)
		{
 			ctable[i] = qRgba(R, G, B, i*A );
		}

		img.setColorTable(ctable);

		painter->drawImage(QPointF(_frame->dst_x, _frame->dst_y), img);

	}while(_frame = _frame->next);
}

QRectF AssSubtitlesItem::boundingRect() const
{
	return unified_rect;
}

/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "videoitem.hpp"

#include <QWidget>
#include <QPainter>
#include <QTransform>
#include <QVideoSurfaceFormat>
#include <QOpenGLFunctions>
#include <QOpenGLPixelTransferOptions>
#include <QOpenGLFunctions_2_0>

#include <QDebug>
#include <QGraphicsWidget>

class VideoPainter : public QOpenGLFunctions
{
	virtual void initializeGL()
	{
		initializeOpenGLFunctions();
	}

public:
	virtual void paintGL(QSizeF viewport_size, const QVideoFrame& framePaintee)
	{
		auto video_size = framePaintee.size();
		glEnable(GL_TEXTURE_2D);
		m_program.bind();

		m_texture_Y->bind(0);
		m_texture_U->bind(1);
		m_texture_V->bind(2);

		float brightness = 1.0;
		float contrast = 1.0;
		float saturation = 1.0;

		m_program.setUniformValue(m_program.uniformLocation("brightness"), brightness);
		m_program.setUniformValue(m_program.uniformLocation("contrast"), contrast);
		m_program.setUniformValue(m_program.uniformLocation("saturation"), saturation);

		m_program.setUniformValue(m_program.uniformLocation("tex0"), 0);
		m_program.setUniformValue(m_program.uniformLocation("tex1"), 1);
		m_program.setUniformValue(m_program.uniformLocation("tex2"), 2);

		const float txLeft = 1.0 / video_size.width() * 2;
		const float txRight = (video_size.width() * 2.0 - 1.0) / (video_size.width() * 2.0);

		const float txTop = 1.0 / (video_size.height() * 2.0);
		const float txBottom = (video_size.height() * 2.0 - 1.0) / (video_size.height() * 2.0);


		const GLdouble tx_array[] =
		{
			txLeft, txTop,
			txRight, txTop,
			txRight, txBottom,
			txLeft, txBottom
		};

		const GLdouble v_array[] =
		{
			0.0     , 0.0,
			viewport_size.width(), 0.0,
			viewport_size.width(), viewport_size.height(),

			0.0, viewport_size.height(),
		};

		// using this can avoid linking to OpenGL libraries
		QOpenGLFunctions_2_0 glfunc;

		glfunc.initializeOpenGLFunctions();
		glfunc.glVertexPointer(2, GL_DOUBLE, 0, v_array);
		glfunc.glTexCoordPointer(2, GL_DOUBLE, 0, tx_array);

		glfunc.glEnableClientState(GL_VERTEX_ARRAY);
		glfunc.glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glDrawArrays(GL_POLYGON, 0, 4);

		glfunc.glDisableClientState(GL_VERTEX_ARRAY);
		glfunc.glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		m_texture_Y->release();
		m_texture_U->release();
		m_texture_V->release();
		m_program.release();

	}

public:

	VideoPainter(VideoItem* parent)
	{
		initializeGL();

		m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/yuv.frag");

		m_program.link();

		m_texture_Y.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
		m_texture_U.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
		m_texture_V.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
	}

	~VideoPainter()
	{
		m_texture_Y->destroy();
		m_texture_U->destroy();
		m_texture_V->destroy();
	}

	void update_texture(const QVideoFrame& newframe)
	{
		auto vsize = newframe.size();
		// update texture fome newframe
		if (m_texture_Y->isCreated())
			m_texture_Y->destroy();

		m_texture_Y->setSize(newframe.bytesPerLine(0), vsize.height(), 0);
		m_texture_Y->setFormat(QOpenGLTexture::R8_UNorm);
		m_texture_Y->setMinificationFilter(QOpenGLTexture::Nearest);
		m_texture_Y->setMagnificationFilter(QOpenGLTexture::Nearest);
		m_texture_Y->allocateStorage();
		m_texture_Y->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, newframe.bits(0));

		if(m_texture_U->isCreated())
			m_texture_U->destroy();

		m_texture_U->setSize(newframe.bytesPerLine(1), vsize.height()/2, 0);
		m_texture_U->setFormat( QOpenGLTexture::R8_UNorm);
		m_texture_U->setMinificationFilter(QOpenGLTexture::Nearest);
		m_texture_U->setMagnificationFilter(QOpenGLTexture::Nearest);
		m_texture_U->allocateStorage();
		m_texture_U->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, newframe.bits(1));

		if(m_texture_V->isCreated())
			m_texture_V->destroy();

		m_texture_V->setSize(newframe.bytesPerLine(2), vsize.height()/2, 0);
		m_texture_V->setFormat(QOpenGLTexture::R8_UNorm);
		m_texture_V->setMinificationFilter(QOpenGLTexture::Nearest);
		m_texture_V->setMagnificationFilter(QOpenGLTexture::Nearest);
		m_texture_V->allocateStorage();
		m_texture_V->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, newframe.bits(2));
	}

	QScopedPointer<QOpenGLTexture> m_texture_Y;
	QScopedPointer<QOpenGLTexture> m_texture_U;
	QScopedPointer<QOpenGLTexture> m_texture_V;
	QOpenGLShaderProgram m_program;
};

VideoItem::VideoItem(QGraphicsItem *parent)
	: QGraphicsItem(parent)
	, imageFormat(QImage::Format_Invalid)
	, framePainted(false)
{
	m_painter = nullptr;
	updatePaintDevice = true;
}

VideoItem::~VideoItem()
{
}

QRectF VideoItem::boundingRect() const
{
	if( my_size.isValid())
		return QRectF(QPointF(0,0), my_size);
    return QRectF(QPointF(0,0), surfaceFormat().sizeHint());
}

void VideoItem::viewportDestroyed()
{
	delete m_painter;

	m_painter = nullptr;

	updatePaintDevice = true;
}

void VideoItem::resize(QSizeF newsize)
{
	my_size = newsize;
}

void VideoItem::paintImage(QPainter* painter)
{
	painter->drawImage(boundingRect(), QImage(
			currentFrame.bits(),
			imageSize.width(),
			imageSize.height(),
			imageFormat));
}

void VideoItem::paintGL(QPainter* painter)
{
	Q_ASSERT( QOpenGLContext::currentContext() );

	painter->beginNativePainting();

	auto vsize = currentFrame.size();

	if ( need_update_gltexture && currentFrame.map(QAbstractVideoBuffer::ReadOnly))
	{
		m_painter->update_texture(currentFrame);
		currentFrame.unmap();

		need_update_gltexture = false;
	}

	m_painter->paintGL(my_size, currentFrame);
	painter->endNativePainting();

}

void VideoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	QMutexLocker l(&m_render_lock);
	if(!currentFrame.isValid())
		return;

	if (updatePaintDevice && !m_painter)
	{
		updatePaintDevice = false;

		m_painter = new VideoPainter(this);
        if (widget)
             connect(widget, SIGNAL(destroyed()), this, SLOT(viewportDestroyed()));
	}

	const QTransform oldTransform = painter->transform();

	if (currentFrame.pixelFormat() != QVideoFrame::Format_YUV420P)
	{
		if (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop)
		{
			painter->scale(1, -1);
			painter->translate(0, -boundingRect().height());
		}

		if (currentFrame.map(QAbstractVideoBuffer::ReadOnly))
		{
			paintImage(painter);
			currentFrame.unmap();
		}
	}
	else
	{
		paintGL(painter);
	}
	painter->setTransform(oldTransform);

	framePainted = true;
}

QList<QVideoFrame::PixelFormat> VideoItem::supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType) const
{
	if (handleType == QAbstractVideoBuffer::NoHandle)
	{
		return QList<QVideoFrame::PixelFormat>()
				<< QVideoFrame::Format_RGB32
				<< QVideoFrame::Format_ARGB32
				<< QVideoFrame::Format_ARGB32_Premultiplied
				<< QVideoFrame::Format_RGB565
				<< QVideoFrame::Format_RGB555
				<< QVideoFrame::Format_YUV420P
				<< QVideoFrame::Format_NV12;
	}
	else
	{
		return QList<QVideoFrame::PixelFormat>();
	}
}

bool VideoItem::start(const QVideoSurfaceFormat &format)
{
	if (isFormatSupported(format))
	{
		imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
		imageSize = format.frameSize();
		framePainted = true;

		qDebug() << format.pixelFormat();

		QAbstractVideoSurface::start(format);

		prepareGeometryChange();

		return true;
	} else {
		return false;
	}
}

void VideoItem::stop()
{
	currentFrame = QVideoFrame();
	framePainted = false;

	QAbstractVideoSurface::stop();
}

bool VideoItem::present(const QVideoFrame &frame)
{
	QMutexLocker l(&m_render_lock);

	if (frame.isValid())
		currentFrame = frame;

	need_update_gltexture = true;

	if (!framePainted) {
		if (!QAbstractVideoSurface::isActive())
			setError(QAbstractVideoSurface::StoppedError);
		return false;
	} else {
		framePainted = false;

		update();

		return true;
	}
}


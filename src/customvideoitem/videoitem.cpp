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

#include <QDebug>
#include <QGraphicsWidget>

// 用来保持图形纹理数据
class GLTextureVideoBuffer : QAbstractVideoBuffer
{
public:
	GLTextureVideoBuffer()
		: QAbstractVideoBuffer(QAbstractVideoBuffer::GLTextureHandle)
	{}

	virtual uchar* map(MapMode mode, int* numBytes, int* bytesPerLine)
	{
		return nullptr;
	}

	virtual void unmap(){}

	QVariant handle() const {
		return 0;
	}

private:


};


class VideoPainter : public QOpenGLFunctions
{
	virtual void initializeGL()
	{
		initializeOpenGLFunctions();
	}

	virtual void paintGL(){

	}

public:

	VideoPainter(VideoItem* parent)
	{
		initializeGL();
	}
};

VideoItem::VideoItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , imageFormat(QImage::Format_Invalid)
    , framePainted(false)
	, m_Y(QOpenGLTexture::Target2D)
	, m_U(QOpenGLTexture::Target2D)
	, m_V(QOpenGLTexture::Target2D)
{
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
	glEnable(GL_TEXTURE_2D);

	m_program.bind();

	auto vsize = currentFrame.size();

	if ( need_update_gltexture && currentFrame.map(QAbstractVideoBuffer::ReadOnly))
	{
		static uint8_t all_zero [1920*1080*4] = { 0 };

		int bytesPerLine = (vsize.width() + 3) & ~3;
		int bytesPerLine2 = (vsize.width() / 2 + 3) & ~3;

		if (m_Y.isCreated())
			m_Y.destroy();

		m_Y.setSize(bytesPerLine, vsize.height(), 0);
		m_Y.setFormat(QOpenGLTexture::R8_UNorm);
		m_Y.setMinificationFilter(QOpenGLTexture::Nearest);
		m_Y.setMagnificationFilter(QOpenGLTexture::Nearest);
		m_Y.allocateStorage();
  		m_Y.setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, currentFrame.bits(0));
// 		m_Y.setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, all_zero);

		if(m_U.isCreated())
			m_U.destroy();

		m_U.setSize(currentFrame.bytesPerLine(1), vsize.height()/2, 0);
		m_U.setFormat( QOpenGLTexture::R8_UNorm);
		m_U.setMinificationFilter(QOpenGLTexture::Nearest);
		m_U.setMagnificationFilter(QOpenGLTexture::Nearest);
		m_U.allocateStorage();
		m_U.setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, currentFrame.bits(1));
// 		m_U.setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, all_zero);

		auto bits = currentFrame.bits(1);

// 		qDebug() << QByteArray::fromRawData((const char*)bits, currentFrame.bytesPerLine(1)).toBase64();

		if(m_V.isCreated())
			m_V.destroy();

		m_V.setSize(currentFrame.bytesPerLine(2), vsize.height()/2, 0);
		m_V.setFormat(QOpenGLTexture::R8_UNorm);
		m_V.setMinificationFilter(QOpenGLTexture::Nearest);
		m_V.setMagnificationFilter(QOpenGLTexture::Nearest);
		m_V.allocateStorage();
		m_V.setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, currentFrame.bits(2));
// 		m_V.setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, all_zero);

		bits = currentFrame.bits(2);

// 		qDebug() << QByteArray::fromRawData((const char*)bits, currentFrame.bytesPerLine(2)).toBase64();

		need_update_gltexture = false;

		currentFrame.unmap();
	}

	m_Y.bind(0, QOpenGLTexture::ResetTextureUnit);
	m_U.bind(1, QOpenGLTexture::ResetTextureUnit);
	m_V.bind(2, QOpenGLTexture::ResetTextureUnit);

	m_program.setUniformValue(m_program.uniformLocation("tex0"), 0);
	m_program.setUniformValue(m_program.uniformLocation("tex1"), 1);
	m_program.setUniformValue(m_program.uniformLocation("tex2"), 2);

// 	qDebug() << "U is bound ? : " << m_U.isBound(1);
// 	qDebug() << "V is bound ? : " << m_V.isBound(2);

	qDebug() << "Y is bound to : "	<< m_Y.boundTextureId(QOpenGLTexture::BindingTarget2D);
	qDebug() << "U is bound to : "	<< m_U.boundTextureId(QOpenGLTexture::BindingTarget2D);
	qDebug() << "V is bound to : " <<	m_V.boundTextureId(QOpenGLTexture::BindingTarget2D);

// 	m_program.uniformLocation();

	glBegin(GL_POLYGON);
	glTexCoord2d(0,0);
	glVertex2d(0,0);

	glTexCoord2d(1,0);
	glVertex2d(my_size.width(), 0);

	glTexCoord2d(1,1);
	glVertex2d(my_size.width(), my_size.height());

	glTexCoord2d(0,1);
	glVertex2d(0, my_size.height());
	glEnd();

	m_Y.release();
	m_U.release();
	m_V.release();
	m_program.release();
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
		if (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop) {
			painter->scale(1, -1);
			painter->translate(0, -boundingRect().height());
		}

		if (currentFrame.map(QAbstractVideoBuffer::ReadOnly)) {
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
	if (handleType == QAbstractVideoBuffer::NoHandle) {
		return QList<QVideoFrame::PixelFormat>()
				<< QVideoFrame::Format_RGB32
				<< QVideoFrame::Format_ARGB32
				<< QVideoFrame::Format_ARGB32_Premultiplied
				<< QVideoFrame::Format_RGB565
				<< QVideoFrame::Format_RGB555
				<< QVideoFrame::Format_YUV420P;
	} else {
		return QList<QVideoFrame::PixelFormat>();
	}
}

bool VideoItem::start(const QVideoSurfaceFormat &format)
{
	if (isFormatSupported(format)) {
		imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
		imageSize = format.frameSize();
		framePainted = true;

		qDebug() << format.pixelFormat();

		m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/yuv.frag");

		m_program.link();

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

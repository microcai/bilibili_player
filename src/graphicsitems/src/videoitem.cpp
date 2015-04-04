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
#include <memory>

#include <QWidget>
#include <QPainter>
#include <QTransform>
#include <QVideoSurfaceFormat>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QPaintEngine>

#include <QApplication>
#include <QDebug>
#include <QGraphicsWidget>

#include "videoitem.hpp"
class VideoPainter : public QOpenGLFunctions
{
	virtual void initializeGL()
	{
		initializeOpenGLFunctions();

		QOpenGLVersionProfile glversion( QOpenGLContext::currentContext()->format() );

		QString glVendor((const char*)glGetString(GL_VENDOR));
		QString glRender((const char*)glGetString(GL_RENDERER));
		QString glslVersion((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

		char openglversion[30]={0};
		sprintf(openglversion, "%d.%d", glversion.version().first ,glversion.version().second);

		qDebug() << "OpenGL version is " << openglversion;
		qDebug() << "GLSL version is " << glslVersion;
		qDebug() << "using OpenGL render" << glRender << "from" << glVendor;;
		qDebug() << "is legecy opengl? " << glversion.isLegacyVersion();

		if (glversion.isLegacyVersion())
		{
			qDebug() << "	you are running very old opengl stack, please upgrade your computer!";
			qDebug() << "	if your graphics card is new, but you still see the message, upgrade your driver";
			qDebug() << "	if your driver is latest, then your driver really sucks";
		}


	}

public:

	void set_viewport_size(QSizeF _viewport_size)
	{
		viewport_size = _viewport_size;
		const GLdouble v_array[] =
		{
			0.0     , 0.0,
			viewport_size.width(), 0.0,
			viewport_size.width(), viewport_size.height(),

			0.0, viewport_size.height(),
		};

		if(!m_drawing_vexteres.isCreated())
			m_drawing_vexteres.create();

		m_drawing_vexteres.bind();

		m_drawing_vexteres.allocate(v_array,  sizeof(v_array));
	}

	virtual void paintGL(const QMatrix4x4& project_matrix, const QMatrix4x4& model_matrix, const QSizeF& video_size)
	{
		float brightness = 1.0;
		float contrast = 1.0;
		float saturation = 1.0;

		m_current_program->bind();

		m_current_program->setUniformValue("ProjectionMatrix", project_matrix);
		m_current_program->setUniformValue("ModelViewMatrix", model_matrix);
		m_current_program->setUniformValue("ModelViewProjectionMatrix", project_matrix * model_matrix);

		texture_setuper();

		m_current_program->setUniformValue("video_window_size", QVector2D(viewport_size.width(), viewport_size.height()));
		m_current_program->setUniformValue("texture_size", QVector2D(video_size.width(), video_size.height()));

 		m_drawing_vexteres.bind();

		m_current_program->enableAttributeArray(1);
		glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 0, 0);

		glDrawArrays(GL_POLYGON, 0, 4);

		m_drawing_vexteres.release();
		texture_cleanuper();
		m_current_program->release();
	}

public:
	~VideoPainter()
	{
	}

	VideoPainter(VideoItem* parent)
		: m_drawing_vexteres(QOpenGLBuffer::VertexBuffer)
		, m_video_windows_shader(QOpenGLShader::Vertex)
		, m_common_shader_lib(QOpenGLShader::Fragment)
	{
		initializeGL();
		m_drawing_vexteres.setUsagePattern(QOpenGLBuffer::DynamicDraw);

		m_common_shader_lib.compileSourceFile(":/glsl/yuv2rgb.glsl");
		Q_ASSERT(m_common_shader_lib.isCompiled());
		m_video_windows_shader.compileSourceFile(":/glsl/videowindow.vert");

		m_program_yuv420p_shader.addShader(&m_video_windows_shader);
		m_program_yuv420p_shader.addShader(&m_common_shader_lib);
		m_program_yuv420p_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/yuv420.frag");

		m_program_yuv420p_shader.bindAttributeLocation("attrVertex", 1);
		m_program_yuv420p_shader.link();

		m_program_yuv444_shader.addShader(&m_video_windows_shader);
		m_program_yuv444_shader.addShader(&m_common_shader_lib);
		m_program_yuv444_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/yuv444.frag");
		m_program_yuv444_shader.bindAttributeLocation("attrVertex", 1);
		m_program_yuv444_shader.link();


		m_program_normalrgb_shader.addShader(&m_video_windows_shader);
		m_program_normalrgb_shader.addShader(&m_common_shader_lib);
		m_program_normalrgb_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/passthru.frag");
		m_program_normalrgb_shader.bindAttributeLocation("attrVertex", 1);
		m_program_normalrgb_shader.link();

		m_program_nv12_shader.addShader(&m_video_windows_shader);
		m_program_nv12_shader.addShader(&m_common_shader_lib);
		if (!m_program_nv12_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/nv12.frag"))
			qApp->quit();
		m_program_nv12_shader.bindAttributeLocation("attrVertex", 1);
		if (!m_program_nv12_shader.link())
			exit(1);
	}



	void update_nv12_texture(const QVideoFrame& newframe)
	{
		if (!m_texture_Y)
			m_texture_Y.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
		if (!m_texture_UV)
			m_texture_UV.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));


		auto vsize = newframe.size();
		// update texture fome newframe
		if (m_texture_Y->isCreated())
			m_texture_Y->destroy();

		m_texture_Y->setSize(newframe.bytesPerLine(0), vsize.height(), 0);
		m_texture_Y->setFormat(QOpenGLTexture::R8_UNorm);
		m_texture_Y->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		m_texture_Y->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
		m_texture_Y->allocateStorage();
		m_texture_Y->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, newframe.bits(0));

		if(m_texture_UV->isCreated())
			m_texture_UV->destroy();

		m_texture_UV->setSize(newframe.bytesPerLine(1), vsize.height()/2, 0);
		m_texture_UV->setFormat( QOpenGLTexture::R8_UNorm);
		m_texture_UV->setMinificationFilter(QOpenGLTexture::Nearest);
		m_texture_UV->setMagnificationFilter(QOpenGLTexture::Nearest);
		m_texture_UV->setAutoMipMapGenerationEnabled(false);
		m_texture_UV->setWrapMode(QOpenGLTexture::ClampToEdge);
		m_texture_UV->allocateStorage();
		m_texture_UV->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, newframe.bits(1));

		texture_cleanuper = [this]()
		{
			m_texture_Y->release();
			m_texture_UV->release();
		};

		bool is_nv12 = newframe.pixelFormat() == QVideoFrame::Format_NV12;
		texture_setuper = [this, is_nv12]()
		{
			// 把 设定传递给 shader 里的对应变量.
			m_program_nv12_shader.setUniformValue("brightness", 1.0f);
			m_program_nv12_shader.setUniformValue("contrast", 1.0f);
			m_program_nv12_shader.setUniformValue("saturation", 1.0f);

			// 把 shader 里的 tex0 tex1 tex2变量 和 0号 1号 2号 三个纹理缓存绑定.
			m_program_nv12_shader.setUniformValue("texY", 0u);
			m_program_nv12_shader.setUniformValue("texUV", 1u);

			m_program_nv12_shader.setUniformValue("type_nv21", (GLuint) !is_nv12);

			m_texture_Y->bind(0);
			m_texture_UV->bind(1);
		};
	}

	void update_yuv420p_texture(const QVideoFrame& newframe)
	{
		if (!m_texture_Y)
			m_texture_Y.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
		if (!m_texture_U)
			m_texture_U.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
		if (!m_texture_V)
			m_texture_V.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));

		auto vsize = newframe.size();
		// update texture fome newframe
		if (m_texture_Y->isCreated())
			m_texture_Y->destroy();

		m_texture_Y->setSize(newframe.bytesPerLine(0), vsize.height(), 0);
		m_texture_Y->setFormat(QOpenGLTexture::R8_UNorm);
		m_texture_Y->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		m_texture_Y->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
		m_texture_Y->allocateStorage();
		m_texture_Y->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, newframe.bits(0));

		if(m_texture_U->isCreated())
			m_texture_U->destroy();

		m_texture_U->setSize(newframe.bytesPerLine(1), vsize.height()/2, 0);
		m_texture_U->setFormat( QOpenGLTexture::R8_UNorm);
		m_texture_U->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		m_texture_U->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
		m_texture_U->allocateStorage();
		m_texture_U->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, newframe.bits(1));

		if(m_texture_V->isCreated())
			m_texture_V->destroy();

		m_texture_V->setSize(newframe.bytesPerLine(2), vsize.height()/2, 0);
		m_texture_V->setFormat(QOpenGLTexture::R8_UNorm);
		m_texture_V->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		m_texture_V->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
		m_texture_V->allocateStorage();
		m_texture_V->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, newframe.bits(2));


		texture_cleanuper = [this]()
		{
			m_texture_Y->release();
			m_texture_U->release();
			m_texture_V->release();
		};

		texture_setuper = [this]()
		{
			// 把 设定传递给 shader 里的对应变量.
			m_program_yuv420p_shader.setUniformValue("brightness", 1.0f);
			m_program_yuv420p_shader.setUniformValue("contrast", 1.0f);
			m_program_yuv420p_shader.setUniformValue("saturation", 1.0f);

			// 把 shader 里的 tex0 tex1 tex2变量 和 0号 1号 2号 三个纹理缓存绑定.
			m_program_yuv420p_shader.setUniformValue("texY", 0u);
			m_program_yuv420p_shader.setUniformValue("texU", 1u);
			m_program_yuv420p_shader.setUniformValue("texV", 2u);

			m_texture_Y->bind(0);
			m_texture_U->bind(1);
			m_texture_V->bind(2);
		};
	}

	void update_direct_texture(GLuint textureid)
	{
		texture_setuper = [this, textureid]()
		{
			glBindTexture(GL_TEXTURE_2D,  textureid);
		};
	}

	void update_YUV_texture(const QVideoFrame& newframe)
	{
		auto vsize = newframe.size();
		// newframe 里是 YUV444 格式的数据。
		if (!m_texture_Y)
			m_texture_Y.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
		if (m_texture_Y->isCreated())
			m_texture_Y->destroy();

		int yuv444_type = 0;

		m_texture_Y->setSize(newframe.bytesPerLine(0)/4, vsize.height(), 0);
		m_texture_Y->setFormat(QOpenGLTexture::RGBA8_UNorm);
		m_texture_Y->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
		m_texture_Y->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
		m_texture_Y->allocateStorage();

		if (newframe.pixelFormat() == QVideoFrame::Format_YUV444)
		{
			yuv444_type = 2;
			m_texture_Y->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, newframe.bits(0));
		}else
		{
			m_texture_Y->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, newframe.bits(0));
		}

		texture_setuper = [this, yuv444_type]()
		{
			// 把 设定传递给 shader 里的对应变量.
			m_program_yuv444_shader.setUniformValue("brightness", 1.0f);
			m_program_yuv444_shader.setUniformValue("contrast", 1.0f);
			m_program_yuv444_shader.setUniformValue("saturation", 1.0f);

			m_program_yuv444_shader.setUniformValue("yuv444_type", yuv444_type);

			// 把 shader 里的 tex0 tex1 tex2变量 和 0号 1号 2号 三个纹理缓存绑定.
			m_program_yuv444_shader.setUniformValue("texYUV", 0u);

			m_texture_Y->bind(0);
		};

		texture_cleanuper = [this]()
		{
			m_texture_Y->release();
		};
	}

	void update_texture(QVideoFrame& newframe)
	{
		if(!m_drawing_vexteres.isCreated())
			m_drawing_vexteres.create();


		if (newframe.handleType() == QAbstractVideoBuffer::NoHandle)
		{
			newframe.map(QAbstractVideoBuffer::ReadOnly);
			switch(newframe.pixelFormat())
			{
				case QVideoFrame::Format_YUV444:
				case QVideoFrame::Format_AYUV444:
				case QVideoFrame::Format_ARGB32_Premultiplied:
					m_current_program = & m_program_yuv444_shader;
					update_YUV_texture(newframe);
					break;
				case QVideoFrame::Format_YUV420P:
					m_current_program = & m_program_yuv420p_shader;
					update_yuv420p_texture(newframe);
					break;
				case QVideoFrame::Format_NV12:
				case QVideoFrame::Format_NV21:
					m_current_program = & m_program_nv12_shader;
					update_nv12_texture(newframe);
				default:
					break;
			}
			newframe.unmap();
		}
		else if (newframe.handleType() == QAbstractVideoBuffer::GLTextureHandle)
		{
			// 已经是 texture 啦？
			update_direct_texture(newframe.handle().toUInt());

			m_current_program = & m_program_normalrgb_shader;
		}
	}

	bool can_paint(const QVideoFrame& newframe)
	{
		if (newframe.handleType() == QAbstractVideoBuffer::NoHandle)
		{
			switch(newframe.pixelFormat())
			{
				case QVideoFrame::Format_YUV444:
				case QVideoFrame::Format_AYUV444:
				case QVideoFrame::Format_ARGB32_Premultiplied:
				case QVideoFrame::Format_YUV420P:
				case QVideoFrame::Format_NV12:
				case QVideoFrame::Format_NV21:
					return true;
			}
		}
		else if (newframe.handleType() == QAbstractVideoBuffer::GLTextureHandle)
		{
			return true;
		}
		return false;
	}

private:
	QSizeF viewport_size;
	QScopedPointer<QOpenGLTexture> m_texture_rgb;

	QScopedPointer<QOpenGLTexture> m_texture_Y;
	QScopedPointer<QOpenGLTexture> m_texture_U;
	QScopedPointer<QOpenGLTexture> m_texture_V;

	// unsigned int texture !
	QScopedPointer<QOpenGLTexture> m_texture_UV;

	QOpenGLShader m_common_shader_lib;
	QOpenGLShader m_video_windows_shader;

	QOpenGLShaderProgram m_program_normalrgb_shader;
	QOpenGLShaderProgram m_program_yuv420p_shader;
	QOpenGLShaderProgram m_program_yuv444_shader;
	QOpenGLShaderProgram m_program_nv12_shader;

	std::function<void()> texture_setuper;
	std::function<void()> texture_cleanuper;

	QOpenGLShaderProgram* m_current_program;

	QOpenGLBuffer m_drawing_vexteres;
};

VideoItem::VideoItem(QGraphicsItem *parent)
	: QGraphicsItem(parent)
	, imageFormat(QImage::Format_Invalid)
{
	m_painter = nullptr;
	updatePaintDevice = true;
}

VideoItem::~VideoItem()
{
}

QRectF VideoItem::boundingRect() const
{
	if( viewport_size.isValid())
		return QRectF(QPointF(0,0), viewport_size);
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
	viewport_size = newsize;
	if (m_painter)
		m_painter->set_viewport_size(viewport_size);
}

void VideoItem::paintImage(QPainter* painter)
{
	painter->drawImage(boundingRect(), QImage(
			currentFrame.bits(),
			imageSize.width(),
			imageSize.height(),
			imageFormat));
}

void VideoItem::paintGL(QPainter* painter, QWidget* widget)
{
	Q_ASSERT( QOpenGLContext::currentContext() );


	painter->beginNativePainting();

	auto vsize = currentFrame.size();

	QMatrix4x4 gl_Projection;
	QMatrix4x4 gl_Model = painter->combinedTransform();

	if (! painter->paintEngine()->hasFeature(QPaintEngine::PixmapTransform))
	{
		gl_Model = painter->combinedTransform() * painter->deviceTransform();
	}

	gl_Projection.ortho(0, painter->device()->width(), painter->device()->height(), 0, -1, 1);
	m_painter->paintGL(gl_Projection, gl_Model, vsize);
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

		m_painter->set_viewport_size(viewport_size);
	}

	const QTransform oldTransform = painter->transform();

	bool use_gl_painter = m_painter->can_paint(currentFrame);

	if (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop)
	{
		painter->scale(1, -1);
		painter->translate(0, - viewport_size.height());
	}

	if (use_gl_painter)
	{
		if (need_update_gltexture)
		{
			need_update_gltexture = false;
			m_painter->update_texture(currentFrame);
		}
		paintGL(painter, widget);
	}
	else
	{
		if (currentFrame.map(QAbstractVideoBuffer::ReadOnly))
		{
			paintImage(painter);
			currentFrame.unmap();
		}
	}
	painter->setTransform(oldTransform);
}

bool VideoItem::present(const QVideoFrame &frame)
{
	QMutexLocker l(&m_render_lock);

	if (frame.isValid())
		currentFrame = frame;

	need_update_gltexture = true;

	update();

	if (!QAbstractVideoSurface::isActive())
	{
		setError(QAbstractVideoSurface::StoppedError);
		return false;
	}
	return true;
}

QList<QVideoFrame::PixelFormat> VideoItem::supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType) const
{
// 	return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_AYUV444;
	if (handleType == QAbstractVideoBuffer::NoHandle)
	{
		return QList<QVideoFrame::PixelFormat>()
				<< QVideoFrame::Format_RGB32
				<< QVideoFrame::Format_ARGB32
				<< QVideoFrame::Format_ARGB32_Premultiplied
				<< QVideoFrame::Format_RGB565
				<< QVideoFrame::Format_RGB555
				<< QVideoFrame::Format_NV12
				<< QVideoFrame::Format_NV21
				<< QVideoFrame::Format_YUV444
				<< QVideoFrame::Format_AYUV444
				<< QVideoFrame::Format_AYUV444_Premultiplied
				<< QVideoFrame::Format_YUV420P;
	}
	else if(handleType == QAbstractVideoBuffer::GLTextureHandle)
	{
		return QList<QVideoFrame::PixelFormat>()
				<< QVideoFrame::Format_RGB32
				<< QVideoFrame::Format_RGB24
				<< QVideoFrame::Format_ARGB32
				<< QVideoFrame::Format_ARGB32_Premultiplied
				<< QVideoFrame::Format_RGB565
				<< QVideoFrame::Format_BGR24
				<< QVideoFrame::Format_BGR32
				<< QVideoFrame::Format_RGB555;
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

		if (imageSize != format.frameSize())
		{
			prepareGeometryChange();
			imageSize = format.frameSize();
		}

		qDebug() << format.pixelFormat();

		return QAbstractVideoSurface::start(format);


		return true;
	} else {
		return false;
	}
}

void VideoItem::stop()
{
	currentFrame = QVideoFrame();
	QAbstractVideoSurface::stop();
}

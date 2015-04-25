
#include "qaudioout.hpp"
#include <QDebug>
#include <qglobal.h>

#include <pulse/thread-mainloop.h>

#define PALOCK palock palockobj##__line__(m_pa_mainloop)

PAOut::PAOut()
{
	m_pa_mainloop = pa_threaded_mainloop_new();

	auto _pa_mainloop_api = pa_threaded_mainloop_get_api(m_pa_mainloop);

	pa_ctx = pa_context_new(_pa_mainloop_api, "bilibili-player");

	pa_context_set_state_callback(pa_ctx, [](pa_context *c, void *userdata)
	{
		reinterpret_cast<PAOut*>(userdata)->pa_context_state_callback(c);
	}, this);

	qDebug() << pa_context_is_local(pa_ctx);

	pa_context_connect(pa_ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

	pa_threaded_mainloop_start(m_pa_mainloop);
}

PAOut::~PAOut()
{
	pa_stream_unref(astream);
	pa_context_unref(pa_ctx);

	pa_threaded_mainloop_stop(m_pa_mainloop);
	pa_threaded_mainloop_wait(m_pa_mainloop);
	pa_threaded_mainloop_free(m_pa_mainloop);
}

void PAOut::pa_context_state_callback(pa_context* c)
{
	qDebug() << "pa state stateChanged";
	m_pa_context_state = pa_context_get_state(c);

	qDebug() << "pa_context_get_state(c) = " << m_pa_context_state;

	switch(m_pa_context_state)
	{
		case PA_CONTEXT_READY:
			m_context_ready = true;
			m_context_ready_cond.wakeAll();
			qDebug() << "connect ready";
			break;
		case PA_CONTEXT_TERMINATED:
			return;
	}
}

bool PAOut::open(const QAudioFormat& f)
{
	ss.channels = f.channelCount();
	ss.rate =  f.sampleRate();

	switch(f.sampleType())
	{
		case QAudioFormat::Float:
			ss.format = PA_SAMPLE_FLOAT32NE;
			break;
		case QAudioFormat::SignedInt:
			ss.format = f.sampleSize() == 16 ?  PA_SAMPLE_S16NE : PA_SAMPLE_S32NE;
			break;
		case QAudioFormat::UnSignedInt:
			ss.format = PA_SAMPLE_U8;
	}

	PALOCK;

	while ( !m_context_ready)
		QThread::msleep(1);

	astream = pa_stream_new(pa_ctx, "audio track", &ss, 0 );
}

void PAOut::pa_stream_state_callback(pa_stream* s)
{
	switch (pa_stream_get_state(s))
	{
		case PA_STREAM_CREATING:
			break;
		case PA_STREAM_TERMINATED:
			stateChanged(QAudio::StoppedState);
			break;

		case PA_STREAM_READY:
			fprintf(stderr, ("Stream successfully created\n"));
			m_state = QAudio::ActiveState;
			break;

		case PA_STREAM_FAILED:
		default:
			fprintf(stderr, ("Stream errror: %s\n"), pa_strerror(pa_context_errno(pa_stream_get_context(s))));
			exit(1);
	}
}

void PAOut::start(QIODevice* _source)
{
	m_source = _source;
	int ret = 0;
	auto flags = PA_STREAM_INTERPOLATE_TIMING|PA_STREAM_AUTO_TIMING_UPDATE;

	PALOCK;

	pa_stream_set_state_callback(astream, [](pa_stream *s, void *userdata)
	{
		reinterpret_cast<PAOut*>(userdata)->pa_stream_state_callback(s);
	}, this);

	pa_stream_set_write_callback(astream, [](pa_stream* p, size_t nbytes, void *userdata)
	{
		reinterpret_cast<PAOut*>(userdata)->pa_stream_write_callback(p, nbytes);
	}, this);

	ret = pa_stream_connect_playback(astream, NULL, NULL, (pa_stream_flags_t)flags, NULL, NULL);

	connect(m_source, SIGNAL(readyRead()), this, SLOT(readyRead()));

	m_state = QAudio::ActiveState;

	stateChanged(m_state);
}

void PAOut::stop()
{
	raise(3);
// 	PALOCK;
// 	pa_stream_flush(astream);
// 	pa_stream_disconnect(astream);
}

void PAOut::reset()
{
	raise(3);
	PALOCK;
	pa_stream_flush(astream, [](pa_stream*astream, int success, void *userdata)
	{
	}, this);
}

qint64 PAOut::GetPlaybackClock()
{
	// return the current audio position

// 	pa_stream_get_time();

}

void PAOut::setNotifyInterval(qint64 interval)
{

}

void PAOut::close()
{
	raise(9);

	PALOCK;

	pa_context_ref(pa_ctx);
	pa_stream_ref(astream);
	pa_stream_flush(astream, [](pa_stream*astream, int success, void *userdata)
	{
		pa_stream_disconnect(astream);
		pa_stream_unref(astream);
		pa_context_unref(reinterpret_cast<PAOut*>(userdata)->pa_ctx);
	}, this);
	astream = nullptr;
}


void PAOut::readyRead()
{
	PALOCK;

	// feed more
// 	m_source->
// 			ret = pa_stream_write(p, buffer, samples_readed, pa_xfree, 0, PA_SEEK_RELATIVE);
// 	pa_stream_write_callback();
	auto bytesAvailable = m_source->bytesAvailable();
	if (bytesAvailable > 0)
		pa_stream_write_callback(astream, bytesAvailable);
}

void PAOut::pa_stream_write_callback(pa_stream* p, size_t nbytes)
{
	// 在这里读取 QIODevice 的数据，然后写入
	if (m_source)
	{
		void* buffer = nullptr;
		size_t buffer_size = nbytes;
// 		pa_stream_begin_write(p, &buffer, &buffer_size);
		buffer = pa_xmalloc(buffer_size);

		qDebug() << "in thread ?" << pa_threaded_mainloop_in_thread(m_pa_mainloop);

// 		buffer_size = std::min(nbytes, buffer_size);

		auto samples_readed =  m_source->read((char*)buffer, (qint64)buffer_size);
		int ret;

		if (samples_readed > 0)
			ret = pa_stream_write(p, buffer, samples_readed, pa_xfree, 0, PA_SEEK_RELATIVE);
		else{
// 			pa_stream_cancel_write(p);
			qDebug () << "read return 0!!! buffer may underrun";
			pa_xfree(buffer);
		}
		qDebug() << "pa_stream_write return :" << ret;
		notify();
	}
	else
		close();
}

void PAOut::suspend()
{
	raise(9);
	PALOCK;

	pa_stream_cork(astream, 1, [](pa_stream*s, int success, void *userdata)
	{
		reinterpret_cast<PAOut*>(userdata)->m_state = QAudio::SuspendedState;
		reinterpret_cast<PAOut*>(userdata)->stateChanged(QAudio::SuspendedState);
	}, this);
}

void PAOut::resume()
{
	raise(9);
	PALOCK;

	pa_stream_cork(astream, 0, [](pa_stream*s, int success, void *userdata)
	{
		reinterpret_cast<PAOut*>(userdata)->m_state = QAudio::ActiveState;
		reinterpret_cast<PAOut*>(userdata)->stateChanged(QAudio::ActiveState);
	}, this);
}


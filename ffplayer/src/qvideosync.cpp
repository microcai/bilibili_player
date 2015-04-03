
#include <functional>

#include <QDebug>

#include "qvideosync.hpp"


QAudioVideoSync::QAudioVideoSync(FFPlayer* parent)
{
	m_sync_thread = std::thread(std::bind(&QAudioVideoSync::sync_thread, this));

	m_audio_out = nullptr;
}

QAudioVideoSync::~QAudioVideoSync()
{
	m_sync_thread.join();
}

void QAudioVideoSync::stop()
{
	m_stop = true;
}

void QAudioVideoSync::sync_frame(const QVideoFrame& f)
{
	if (m_stop)
		return;

	QMutexLocker l(&m_lock);
	m_list.push_back(f);
	m_cond_decoder_to_sync.wakeAll();

	while (m_list.size() >= 50)
	{
// 		sleep until render complete!
		m_cond_sync_to_decoder.wait(&m_lock);
	}
}

void QAudioVideoSync::sync_audio(const QAudioBuffer& a)
{
	{
		QMutexLocker l(&m_alock);

		if(!m_audio_out)
		{
			m_audio_out = new QAudioOutput(a.format());
			m_audio_out->setNotifyInterval(1);
			QIODevice::open(QIODevice::ReadOnly);
			m_audio_out->start(this);

			connect(m_audio_out, SIGNAL(notify()), this, SLOT(audio_play_buffer_notify()));
		}

		// 挂入列队
		// 这里无需阻塞，因为视频部分会阻塞的.
		m_audiobuf_list.push_back(a);

	}
	Q_EMIT readyRead();
}

qint64 QAudioVideoSync::bytesAvailable() const
{
	QMutexLocker l(&m_alock);

	if(m_audiobuf_list.empty())
		return 0;

	return std::accumulate(m_audiobuf_list.begin(), m_audiobuf_list.end(), (qint64)0, [](qint64 s, const QAudioBuffer& f){
		return s + f.byteCount();
	});
}

qint64 QAudioVideoSync::writeData(const char* data, qint64 len)
{
	return 0;
}

qint64 QAudioVideoSync::readData(char* data, qint64 maxlen)
{
	QMutexLocker l(&m_alock);

	return readDataUnlocked(data, maxlen);
}

qint64 QAudioVideoSync::readDataUnlocked(char* data, qint64 maxlen)
{
	if(maxlen==0)
		return 0;

	if (m_tmp_buf_size == 0)
	{
		if(m_audiobuf_list.empty())
		{
			qDebug() << "return 0 for readData !!! might buffer underflow";
			return 0;
		}

		QAudioBuffer f = m_audiobuf_list.front();

		m_tmp_buf = f.constData<char>();
		m_tmp_buf_size = f.byteCount();
	}

	if (m_tmp_buf_size)
	{
		auto copy_size = qMin(maxlen, m_tmp_buf_size);

		if (copy_size < maxlen)
		{
			qDebug() << "copy_size < maxlen !!! might buffer underflow";
		}

		memcpy(data, m_tmp_buf, copy_size);

		m_tmp_buf += copy_size;
		m_tmp_buf_size -= copy_size;

		if (m_tmp_buf_size==0)
		{
			m_audiobuf_list.pop_front();
		}

		return copy_size + readDataUnlocked(data+copy_size, maxlen - copy_size);
	}

}

void QAudioVideoSync::audio_play_buffer_notify()
{
	m_avsync_notify.wakeAll();
}

void QAudioVideoSync::sync_thread()
{
	QVideoFrame f;

	for(;!m_stop;)
	{
		// 读取

		{
			QMutexLocker l(&m_lock);

			while(m_list.empty()){
				m_cond_decoder_to_sync.wait(&m_lock);
			}

			f = m_list.front();
			m_list.pop_front();
		}


		// 接着同步到时间点
		auto elapsed = 0;

		if (m_audio_out)
		{
			elapsed = m_audio_out->processedUSecs() / 1000;
		}else
		{
			elapsed = f.startTime();
		}

		auto startTime = f.startTime();

		while ( elapsed < (startTime - 5))
		{
			QThread::msleep(1);

			elapsed = m_audio_out->elapsedUSecs() / 1000;
		}
		render_frame(f);
		m_cond_sync_to_decoder.wakeAll();
	}

	m_lock.lock();
	m_list.clear();
	m_cond_decoder_to_sync.wakeAll();
	m_lock.unlock();
	return;
}


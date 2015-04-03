
#pragma once

#include <memory>
#include <thread>
#include <deque>

#include <QtCore>
#include <QVideoFrame>
#include <QAudioBuffer>
#include <QAudioOutput>

#include "ffplayer.hpp"
#include "ffmpeg.hpp"

class QAudioVideoSync : public QIODevice
{
	Q_OBJECT
public:
	QAudioVideoSync(FFPlayer* parent);
	virtual ~QAudioVideoSync();

	Q_SIGNAL void render_frame(const QVideoFrame&);

	Q_SIGNAL void need_more_frame() const;

	void stop();

	Q_SIGNAL void nomore_frames() const;
	Q_SIGNAL void frames_ready() const;

private:
	Q_SLOT void sync_frame(const QVideoFrame&);
	Q_SLOT void sync_audio(const QAudioBuffer&);

	Q_SLOT void audio_play_buffer_notify();


	void sync_thread();

private:

	// for QIODevice
	qint64 readDataUnlocked(char *data, qint64 maxlen);

	qint64 readData(char *data, qint64 maxlen);
	qint64 writeData(const char *data, qint64 len);
	qint64 bytesAvailable() const;

private:
	QAudioOutput* m_audio_out;

	QWaitCondition m_avsync_notify;

	bool m_stop = false;

	mutable QMutex m_ptslock;


	mutable QMutex m_lock;

	std::deque<QVideoFrame> m_list;


	mutable QMutex m_alock;
	std::deque<QAudioBuffer> m_audiobuf_list;

	const char* m_tmp_buf = 0;
	qint64 m_tmp_buf_size = 0;

	std::thread m_sync_thread;
	QTime play_time;

	qint64 played_audio_frame_time_stamp = 0;
};

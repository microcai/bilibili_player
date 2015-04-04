
#pragma once

#include <memory>
#include <thread>
#include <deque>

#include <QtCore>
#include <QVideoFrame>
#include <QAudioBuffer>
#include <QAudioOutput>

#include <boost/timer/timer.hpp>

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

	Q_SIGNAL void pause();
	Q_SIGNAL void resume();

	Q_SLOT void stop();

	Q_SIGNAL void clear_queue();

	Q_SIGNAL void nomore_frames() const;
	Q_SIGNAL void frames_ready() const;

	Q_SIGNAL void suspended() const;
	Q_SIGNAL void running() const;

	void start();

private:
	Q_SLOT void do_clear_queue();
	Q_SLOT void do_pause();
	Q_SLOT void do_resume();

	Q_SLOT void frame_seeked();
	Q_SLOT void sync_frame(const QVideoFrame&);
	Q_SLOT void sync_audio(const QAudioBuffer&);

	Q_SLOT void audio_play_buffer_notify();

    Q_SLOT void stateChanged(QAudio::State);


	void sync_thread();

private:

	// for QIODevice
	qint64 readDataUnlocked(char *data, qint64 maxlen);

	qint64 readData(char *data, qint64 maxlen);
	qint64 writeData(const char *data, qint64 len);
	qint64 bytesAvailable() const;

private:
	QPointer<QAudioOutput> m_audio_out;

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
	boost::timer::cpu_timer play_time;

	qint64 played_audio_frame_time_stamp = 0;
};

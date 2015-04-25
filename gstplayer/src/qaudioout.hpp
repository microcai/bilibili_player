
#pragma once

#include <QtCore>

#include <QAudioBuffer>
#include <QAudioFormat>
#include <QIODevice>

#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>

class PAOut: public QObject
{
	Q_OBJECT

	struct palock{
		palock(pa_threaded_mainloop*_l) : _ll(_l){
			if(!pa_threaded_mainloop_in_thread(_l))
				pa_threaded_mainloop_lock(_l);

		}
		~palock(){ if(!pa_threaded_mainloop_in_thread(_ll))pa_threaded_mainloop_unlock(_ll);}
		pa_threaded_mainloop* _ll;
	};

public:

	PAOut();
	~PAOut();

	bool open(const QAudioFormat&);
	void close();

	void suspend();
	void resume();

	void start(QIODevice*);
	void stop();
	void reset();

	// the clock that is been played
	qint64 GetPlaybackClock();
	void setNotifyInterval(qint64 interval);
	QAudio::State& state() { return m_state;}

Q_SIGNALS:

	void notify();
	void stateChanged(QAudio::State state);


private:
	pa_threaded_mainloop* m_pa_mainloop = nullptr;
	pa_context* pa_ctx = nullptr;

	pa_sample_spec ss;
	pa_stream* astream = nullptr;

	QPointer<QIODevice> m_source;

	QAudio::State m_state = QAudio::StoppedState;

	bool m_context_ready = false;
	QWaitCondition m_context_ready_cond;

	pa_context_state_t m_pa_context_state;

private:
	void pa_context_state_callback(pa_context *c);
	void pa_stream_state_callback(pa_stream *s);
	void pa_stream_write_callback(pa_stream *p, size_t nbytes);

	Q_SLOT void readyRead();
};

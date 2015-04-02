
#include <memory>

#include <QIODevice>

extern "C" {
#include <libavformat/avio.h>
}

// Qt 的 io 转换到 ffmepg 的 IO
struct QIO2AVIO
{
public:
	QIO2AVIO(QIO2AVIO&&) = default;
	QIO2AVIO& operator = (QIO2AVIO&&) = default;

	QIO2AVIO(QIODevice* qio);
    virtual ~QIO2AVIO();

	AVIOContext* avio_context()
	{
		return avio_ctx.get();
	}

private:
	QIO2AVIO(const QIO2AVIO&) = delete;
	QIO2AVIO& operator = (const QIO2AVIO&) = delete;

protected:
	// size of the buffer! 64MB for now
	// should be enough for smooth playback
	std::shared_ptr<unsigned char> m_buffer;

	QIODevice* m_qio;
	std::shared_ptr<AVIOContext> avio_ctx;

private:

	// for AVIOContext
	static int read_packet(void *opaque, uint8_t *buf, int buf_size)
	{
		reinterpret_cast<QIO2AVIO*>(opaque)->read_packet(buf, buf_size);
	}
	static int write_packet(void *opaque, uint8_t *buf, int buf_size)
	{
		reinterpret_cast<QIO2AVIO*>(opaque)->write_packet(buf, buf_size);
	}
	static int64_t seek_packet(void *opaque, int64_t offset, int whence)
	{
		reinterpret_cast<QIO2AVIO*>(opaque)->seek_packet(offset, whence);
	}

	// for AVIOContext
	virtual int read_packet(uint8_t *buf, int buf_size);
	virtual int write_packet(uint8_t *buf, int buf_size);
	virtual int64_t seek_packet(int64_t offset, int whence);

};


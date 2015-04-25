
#include <stdio.h>
#include "qio2avio.hpp"

const int buffer_size = 1024*1024*64;

QIO2AVIO::QIO2AVIO(QIODevice* qio)
	: m_qio(qio)
{
	m_buffer.reset((unsigned char*)av_malloc(buffer_size), av_free);

	AVIOContext* io_ctx;

	io_ctx = avio_alloc_context(m_buffer.get(), buffer_size, 0, this, &QIO2AVIO::read_packet, &QIO2AVIO::write_packet, &QIO2AVIO::seek_packet);
	avio_ctx.reset(io_ctx, av_free);
}

QIO2AVIO::~QIO2AVIO()
{

}

int QIO2AVIO::read_packet(uint8_t *buf, int buf_size)
{
	if(!m_qio->waitForReadyRead(1000))
		return 0;

	return m_qio->read(reinterpret_cast<char*>(buf), buf_size);
}

int QIO2AVIO::write_packet(uint8_t *buf, int buf_size)
{
	return -1;
}

int64_t QIO2AVIO::seek_packet(int64_t offset, int whence)
{
	switch(whence)
	{
		case AVSEEK_SIZE:
			return m_qio->size();
		case SEEK_SET:
			return m_qio->seek(offset);
		case SEEK_CUR:
			return m_qio->seek(m_qio->pos() + offset);
		case SEEK_END:
			return m_qio->seek(m_qio->size() - offset);
		case AVSEEK_FORCE:
		{
			// now, this is the tricky part, we need to request new HTTP connection
			// and return the new connection
			// but if the are data already downloaded, then we don't need to do that.
			// TODO close the device and reopen
		}
	}

	return -1;
}

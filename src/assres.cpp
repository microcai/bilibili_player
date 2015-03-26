
extern "C" {
#include <ass/ass.h>
}


#include "assres.hpp"
#include <QFile>


struct AssResPrivate
{
	AssResPrivate()
	{
		_ass = ass_library_init();
		_track = ass_new_track(_ass);
	}

	~AssResPrivate()
	{
		ass_free_track(_track);
		ass_library_done(_ass);
	}

	bool load_ass(QString assfilename)
	{
		QFile file(assfilename);

		file.open(QIODevice::ReadOnly);

		ass_read_memory(_ass, file.readAll().data(), file.size(), "utf-8");

// 		ass_read_file(_ass, assfilename);
	}

	AssRes *q_ptr;

	ASS_Library* _ass;

	ASS_Track* _track;

};

AssRes::AssRes(QString assfile, QObject* parent)
	: QObject(parent)
	, d_ptr(new AssResPrivate)
{
    d_ptr->q_ptr = this;

}

AssRes::~AssRes()
{
    delete d_ptr;
}

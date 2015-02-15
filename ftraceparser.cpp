#include <QtGlobal>
#include <QString>
#include <QTextStream>
#include "ftraceparser.h"
#include "tracefile.h"

bool FtraceParser::open(const QString &fileName)
{
	unsigned long long nr = 0;
	bool ok = false;
	TraceFile file(fileName.toLocal8Bit().data(), ok, 1024*1024);

	if (!ok)
		return ok;

	//vector.resize(0);
	vector.reserve(80000000);

#if 0
	if (!file.open(QIODevice::ReadOnly))
		return false;
#endif

	while(!file.atEnd()) {
		TraceLine line;
		quint32 n = file.ReadLine(&line);
		vector.append(line);
		nr += n;
	}
	QTextStream(stdout) << nr << "\n";
	return true;
}

FtraceParser::FtraceParser()
{
}

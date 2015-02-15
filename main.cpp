#include <QTime>
#include <QString>
#include <QFile>
#include <QtGlobal>
#include <QTextStream>
#include <QVector>
#include <cstdlib>
#include <new>
#include <ftraceparser.h>

int main(int argc, char* argv[]) {
	QString valstr;
	QString fileName;
	FtraceParser parser;

	if (argc < 2) {
		QTextStream(stdout) << "Usage:" << argv[0] <<
			" <filename>\n";
		return 0;
	}
	
	fileName = QString::fromLocal8Bit(argv[1]);

	QTextStream(stdout) << "opening " << fileName << "\n";
	
	if (!parser.open(fileName)) {
		QTextStream(stdout) << "failed to open " << fileName << "\n";
		return 0;
	}

	return 0;
}

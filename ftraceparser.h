#ifndef FTRACEPARSER_H
#define FTRACEPARSER_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QList>

#include "traceline.h"

class FtraceEvent;

class FtraceParser 
{
public:
	FtraceParser();
	bool open(const QString &fileName);
	QVector<TraceLine> vector;
private:
};

#endif

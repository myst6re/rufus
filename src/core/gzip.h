#ifndef GZIP_H
#define GZIP_H

#include <QtCore>

class GZIP
{
public:
	static QByteArray decompress(const QByteArray &data, int decSize);
	static QByteArray compress(const QByteArray &ungzip);
	static QByteArray decompress(const char *data, int size, int decSize);
	static QByteArray compress(const char *ungzip, int size);
};

#endif // GZIP_H

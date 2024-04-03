#include "gzip.h"
#include <zlib.h>
#undef compress // conflict with GZIP::compress

QByteArray GZIP::decompress(const QByteArray &data, int decSize)
{
	return decompress(data.constData(), data.size(), decSize);
}

QByteArray GZIP::compress(const QByteArray &ungzip)
{
	return compress(ungzip.constData(), ungzip.size());
}

QByteArray GZIP::decompress(const char *data, int size, int/* decSize*/)
{
	QByteArray ungzip;

	QTemporaryFile temp;
	if(!temp.open()) {
		return QByteArray();
	}
	temp.write(data, size);
	temp.close();
	gzFile file = gzopen(temp.fileName().toLatin1(), "rb");
	if(!file) {
		return QByteArray();
	}
	char buffer[10000];
	int r;
	while((r = gzread(file, buffer, 10000)) > 0) {
		ungzip.append(buffer, r);
	}
	gzclose(file);

	return ungzip;
}

QByteArray GZIP::compress(const char *ungzip, int size)
{
	QString tempPath = QDir::tempPath()+"/qt_temp.gz";

	gzFile file2 = gzopen(tempPath.toLatin1(), "wb9");
	if(!file2) {
		return QByteArray();
	}
	gzwrite(file2, ungzip, size);
	gzclose(file2);
	QFile finalFile(tempPath);
	if(!finalFile.open(QIODevice::ReadOnly)) {
		return QByteArray();
	}

	QByteArray data = finalFile.readAll();
	finalFile.remove();

	return data;
}

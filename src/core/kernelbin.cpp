#include "Kernelbin.h"
#include "gzip.h"
#include <QFile>
#include <QDebug>

KernelBin::KernelBin() :
    _isModified(false)
{
}

bool KernelBin::open(const QString &path)
{
	QFile f(path);
	if(!f.open(QIODevice::ReadOnly))	return false;

	return open(f.readAll());
}

bool KernelBin::open(const QByteArray &data)
{
	const char *constData = data.constData(),
			*constDataEnd = constData + data.size();
	QList<QByteArray> parts;
	QList<quint16> typeIds, decSizes;

	for (int i = 0; i < KERNEL_BIN_PART_COUNT; ++i) {
		quint16 size, decSize, typeId;

		if (constData + KERNEL_BIN_PART_HEADER_SIZE > constDataEnd) {
			return false;
		}

		memcpy(&size, constData, 2);
		memcpy(&decSize, constData + 2, 2);
		memcpy(&typeId, constData + 4, 2);

		if (constData + KERNEL_BIN_PART_HEADER_SIZE + size > constDataEnd) {
			return false;
		}

		parts << QByteArray(constData + KERNEL_BIN_PART_HEADER_SIZE, size);
		decSizes << decSize;
		typeIds << typeId;

		constData += KERNEL_BIN_PART_HEADER_SIZE + size;
	}

	_partsLzs = parts;
	_decSizes = decSizes;
	_typeIds = typeIds;
	_isModified = false;

	return true;
}

bool KernelBin::save(const QString &path) const
{
	QByteArray data;
	if(!save(data)) 	return false;

	QFile f(path);
	if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate))	return false;
	f.write(data);
	f.close();

	return true;
}

bool KernelBin::save(QByteArray &data) const
{
	if (_partsLzs.isEmpty()) {
		return false;
	}

	for (int id = 0; id < KERNEL_BIN_PART_COUNT; ++id) {
		QByteArray part = partPacked(id);
		if (part.isEmpty()) {
			return false;
		}
		data.append(part);
	}
	// Last section (empty)
	data.append(QByteArray("\x00\x00", 2));

	return true;
}

QByteArray KernelBin::part(int id) const
{
	return GZIP::decompress(_partsLzs.at(id), _decSizes.at(id));
}

void KernelBin::setPart(int id, const QByteArray &data)
{
	QByteArray part = data;
	if (data.size() % 2 != 0) {
		part.append('\xFF'); // Align
	}
	_partsLzs[id] = GZIP::compress(part);
	_decSizes[id] = part.size();
	_isModified = true;
}

QByteArray KernelBin::partPacked(int id) const
{
	QByteArray data, partLzs = _partsLzs.at(id);
	quint16 typeId = _typeIds.at(id),
	        uncompressedSize = _decSizes.at(id);
	const int compressedSize = partLzs.size();

	if (compressedSize > 65535) {
		qWarning() << "KernelBin::save compressedSize overflow" << compressedSize;
		return QByteArray();
	}

	data.append((char *)&compressedSize, 2);
	data.append((char *)&uncompressedSize, 2);
	data.append((char *)&typeId, 2);
	data.append(partLzs);

	return data;
}

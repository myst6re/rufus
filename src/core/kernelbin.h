#pragma once

#include <QList>
#include <QByteArray>
#include <QString>

#define KERNEL_BIN_PART_COUNT           27
#define KERNEL_BIN_PART_HEADER_SIZE     6
#define KERNEL_BIN_PART_FIRST_WITH_TEXT 9
#define COUNT_PER_BLOCKS_POS            0xF1C
#define COUNT_PER_BLOCKS_SIZE           0x40

class KernelBin
{
public:
	enum Part {
		Command=0, Attack, Battle,
		Init, Item, Weapon,
		Armor, Accessory, Materia
	};

	KernelBin();
	inline bool open(const char *path) {
		return open(QString(path));
	}
	bool open(const QString &path);
	bool open(const QByteArray &data);
	bool save(const QString &path) const;
	bool save(QByteArray &data) const;
	QByteArray part(int id) const;
	QByteArray partPacked(int id) const;
	void setPart(int id, const QByteArray &data);
	inline bool isModified() const {
		return _isModified;
	}
private:
	QList<QByteArray> _partsLzs;
	QList<quint16> _decSizes, _typeIds;
	bool _isModified;
};

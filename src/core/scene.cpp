#include "scene.h"
#include "gzip.h"
#include <QDebug>
#include <QFile>

QByteArray SceneScript::save() const
{
	return _data;
}

SceneScriptOpcode SceneScript::opcodeAt(int pos, bool &ok) const
{
	ok = false;

	if (pos < _data.size()) {
		quint8 key = _data.at(pos++),
				opInfos = opLength().value(key, 0),
				length = SCENE_SCRIPT_LENGTH(opInfos);
		const char *args = nullptr;

		if (length > 0) {
			if (pos + length <= _data.size()) {
				args = _data.constData() + pos;
				pos += length;
				ok = true;
			}
		} else {
			ok = true;
		}

		if (ok) {
			bool hasNull = SCENE_SCRIPT_LENGTH_HAS_NULL(opInfos),
					hasFF = SCENE_SCRIPT_LENGTH_HAS_FF(opInfos);
			quint8 endChar = 0;

			if (hasFF) {
				endChar = 0xFF;
			}

			if (hasNull || hasFF) {
				while (pos + length < _data.size() &&
					   quint8(_data.at(pos + length)) != endChar) {
					length++;
				}

				if (hasFF) {
					length++; // FF
				}

				if (length > 0) {
					args = _data.constData() + pos;
				}
			}
		}

		return SceneScriptOpcode(key, args, length);
	}
	return SceneScriptOpcode();
}

SceneScriptOpcode SceneScriptIterator::nextOpcode(int &pos)
{
	bool ok;
	SceneScriptOpcode ret = _script.opcodeAt(_cur, ok);
	if (ok) {
		pos = _cur;
		_oldCur = _cur;
		_cur += ret.length + 1;
	} else {
		pos = -1;
	}
	return ret;
}

SceneScriptOpcode SceneScriptIterator::nextOpcode(quint8 key, int &pos)
{
	forever {
		SceneScriptOpcode op = nextOpcode(pos);
		if (pos < 0) {
			break;
		}

		if (op.key == key) {
			return op;
		}
	}

	return SceneScriptOpcode();
}

QByteArray SceneScriptIterator::nextTextData(int &pos)
{
	SceneScriptOpcode op = nextOpcode(0x93, pos);
	if (pos >= 0) {
		return op.data();
	}
	return QByteArray();
}


SceneScriptOpcode SceneScriptMutableIterator::nextOpcode(int &pos)
{
	bool ok;
	SceneScriptOpcode ret = _script.opcodeAt(_cur, ok);
	if (ok) {
		pos = _cur;
		_oldCur = _cur;
		_cur += ret.length + 1;
	} else {
		pos = -1;
	}
	return ret;
}

SceneScriptOpcode SceneScriptMutableIterator::nextOpcode(quint8 key, int &pos)
{
	forever {
		SceneScriptOpcode op = nextOpcode(pos);
		if (pos < 0) {
			break;
		}

		if (op.key == key) {
			return op;
		}
	}

	return SceneScriptOpcode();
}

QByteArray SceneScriptMutableIterator::nextTextData(int &pos)
{
	SceneScriptOpcode op = nextOpcode(0x93, pos);
	if (pos >= 0) {
		return op.data();
	}
	return QByteArray();
}

void SceneScript::updateJumps(int from, int diff)
{
	SceneScriptIterator it(*this);

	forever {
		int pos;
		SceneScriptOpcode ret = it.nextOpcode(pos);
		if (pos < 0) {
			break;
		}

		// Jumps
		if (ret.key == 0x70
				|| ret.key == 0x71
				|| ret.key == 0x72) {
			quint16 offset;
			memcpy(&offset, ret.args, ret.length);
			if (offset > from) {
				offset += diff;
			}
			_data.replace(pos + 1, 2, (char *)&offset, 2);
		}
	}
}

int SceneScript::setTextData(int pos, const QByteArray &text)
{
	bool ok;
	SceneScriptOpcode op = opcodeAt(pos, ok);
	if (ok && op.key == 0x93) {
		int size = op.length;
		if (!text.endsWith('\xff')) {
			size -= 1;
		}
		_data.replace(pos + 1, size, text);
		updateJumps(pos, text.size() - size);
		return size;
	} else {
		qWarning() << "SceneScript::setText error" << pos;
		Q_ASSERT(false);
	}
	return -1;
}

void SceneScriptMutableIterator::setTextData(const QByteArray &text)
{
	int size = _script.setTextData(_oldCur, text);
	// Change cursor pos if text size is different
	int diff = text.size() - size;
	if (diff != 0) {
		_cur += diff;
	}
}

QMap<quint8, quint8> SceneScript::_opLength;

const QMap<quint8, quint8> &SceneScript::opLength()
{
	if (_opLength.isEmpty()) {
		_opLength[0x00] = 2;
		_opLength[0x01] = 2;
		_opLength[0x02] = 2;
		_opLength[0x03] = 2;
		_opLength[0x10] = 2;
		_opLength[0x11] = 2;
		_opLength[0x12] = 2;
		_opLength[0x13] = 2;
		_opLength[0x60] = 1;
		_opLength[0x61] = 2;
		_opLength[0x62] = 3;
		_opLength[0x70] = 2;
		_opLength[0x71] = 2;
		_opLength[0x72] = 2;
		_opLength[0x93] = SCENE_SCRIPT_LENGTH_FF(0);
		_opLength[0xA0] = SCENE_SCRIPT_LENGTH_NULL(1);
	}

	return _opLength;
}

SceneFile::SceneFile()
{
}

SceneFile::SceneFile(const QString &path)
{
	open(path);
}

SceneFile::SceneFile(const QByteArray &data)
{
	open(data);
}

bool SceneFile::open(const QString &path)
{
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) {
		return false;
	}

	return open(f.readAll());
}

bool SceneFile::open(const QByteArray &data, bool jp)
{
	_data = data.left(0x1E80);

	const quint8 nameLen = jp ? 16 : 32,
			ennemySectionLen = 0x98 + nameLen;

	_ennemyNames[0] = data.mid(0x0298, nameLen);
	_ennemyNames[1] = data.mid(0x0298 + ennemySectionLen, nameLen);
	_ennemyNames[2] = data.mid(0x0298 + ennemySectionLen * 2, nameLen);

	int pos = 0x0298 + 0x03C0 + ennemySectionLen * 3;

	if (data.size() < pos + nameLen*32) {
		qWarning() << "SceneFile::open too short" << data.size();
		return false;
	}

	_attackNames.clear();

	for (int i=0 ; i<32 ; ++i) {
		_attackNames.append(data.mid(pos, nameLen));
		pos += nameLen;
	}

	pos += 0x0200;

	quint16 ennemyOffsets[3];

	for (int i=0; i<3; ++i) {
		ennemyScripts[i].clear();

		memcpy(ennemyOffsets + i, data.constData() + pos + i * 2, 2);
	}

	for (int i=0; i<3; ++i) {
		if (ennemyOffsets[i] != 0xFFFF) {
			QList<int> toc;

			for (int j=0; j<16; ++j) {
				quint16 scriptOffset;
				if (pos + ennemyOffsets[i] + j * 2 + 2 > data.size()) {
					qWarning() << "ennemy offset too large" << ennemyOffsets[i] << data.size();
					return false;
				}
				memcpy(&scriptOffset, data.constData() + pos + ennemyOffsets[i] + j * 2, 2);
				if (scriptOffset != 0xFFFF) {
					if (pos + ennemyOffsets[i] + scriptOffset >= data.size()) {
						qWarning() << "script offset too large" << scriptOffset << data.size();
						return false;
					}
					toc.append(pos + ennemyOffsets[i] + scriptOffset);
				} else {
					toc.append(-1);
				}
			}
			if (i < 2) {
				if (ennemyOffsets[i + 1] != 0xFFFF) {
					toc.append(pos + ennemyOffsets[i + 1]);
				} else if (i == 0 && ennemyOffsets[i + 2] != 0xFFFF) {
					toc.append(pos + ennemyOffsets[i + 2]);
				} else {
					toc.append(data.size());
				}
			} else {
				toc.append(data.size());
			}

			for (int j=0; j<16; ++j) {
				if (toc.at(j) != -1) {
					int firstPos = toc.last();
					for (int k=j+1; k<16; ++k) {
						if (toc.at(k) != -1) {
							firstPos = toc.at(k);
							break;
						}
					}
					ennemyScripts[i].append(SceneScript(data.mid(toc.at(j),
														firstPos - toc.at(j))));
				} else {
					ennemyScripts[i].append(SceneScript());
				}
			}

			if (ennemyScripts[i].size() != 16) {
				qWarning() << "SceneFile::open wrong ennemyScripts size" << ennemyScripts[i].size();
				Q_ASSERT(false);
			}
		}
	}

	return true;
}

bool SceneFile::compile()
{
	// Ennemy names
	int pos = 0x0298;
	for (int ennemyID = 0; ennemyID < 3; ++ennemyID) {
		_data.replace(pos, 32, _ennemyNames[ennemyID].leftJustified(32, '\xFF', true));
		pos += 184;
	}

	// Attack names
	pos = 0x0880;
	for (const QByteArray &attackName : _attackNames) {
		_data.replace(pos, 32, attackName.leftJustified(32, '\xFF', true));
		pos += 32;
	}

	// Ennemy AI scripts
	pos = 0x0E80;
	const quint16 ennemiesTocSize = 3 * 2;
	QByteArray scriptsData;

	for (int ennemyID = 0; ennemyID < 3; ++ennemyID) {
		quint16 aiOffset;
		QByteArray ennemyScriptsData, ennemyScriptsToc;

		if (ennemyScripts[ennemyID].isEmpty()) {
			aiOffset = 0xFFFF;
		} else {
			aiOffset = ennemiesTocSize + scriptsData.size();
			const quint16 tocSize = 16 * 2;

			for (const SceneScript &sceneScript : ennemyScripts[ennemyID]) {
				quint16 ennemyScriptPos;

				if (sceneScript.isNull()) {
					ennemyScriptPos = 0xFFFF;
				} else {
					ennemyScriptPos = tocSize + ennemyScriptsData.size();
					ennemyScriptsData.append(sceneScript.save());
				}
				ennemyScriptsToc.append((char *)&ennemyScriptPos, 2);
			}

			// Even Alignment Required
			if (ennemyScriptsData.size() % 2 != 0) {
				ennemyScriptsData.append('\xFF');
			}

			if (ennemyScriptsToc.size() != tocSize) {
				qWarning() << "SceneFile::compile wrong toc size" << ennemyID << ennemyScriptsToc.size();
				Q_ASSERT(false);
			}
			scriptsData.append(ennemyScriptsToc);
			scriptsData.append(ennemyScriptsData);
		}
		// TOC
		_data.replace(pos + ennemyID * 2, 2, (char *)&aiOffset, 2);
	}

	pos = 0x0E86;
	_data.replace(pos, _data.size() - pos, scriptsData);
	_data = _data.leftJustified(7808, '\xFF');

	if (_data.size() > 7808) {
		for (int i = _data.size() - 1; i >= 7808; i--) {
			if (_data.at(i) != '\xFF') {
				qWarning() << "SceneFile::compile File too large" << _data.size();
				_data.truncate(7808);
				return false;
			}
		}
		_data.truncate(7808);
	}

	return true;
}

bool SceneFile::save(const QString &path) const
{
	QByteArray data;
	if (!save(data)) {
		return false;
	}

	QFile f(path);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return false;
	}
	f.write(data);
	f.close();

	return true;
}

bool SceneFile::save(QByteArray &data) const
{
	data = _data;

	return true;
}

Scene::Scene()
{
}

bool Scene::open(const QString &path, bool jp)
{
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) {
		return false;
	}

	return open(f.readAll(), jp);
}

bool Scene::open(const QByteArray &data, bool jp)
{
	_scenes.clear();
	if (data.size() % SCENE_BLOCK_SIZE != 0) {
		qWarning() << "Scene::open data size not multiple of" << SCENE_BLOCK_SIZE;
		return false;
	}

	for (int blockID = 0; blockID < data.size()/SCENE_BLOCK_SIZE; ++blockID) {
		quint32 firstPointer, pointer=0, i=4;
		QList<quint32> pointers;
		const char *blockData = data.constData() + blockID * SCENE_BLOCK_SIZE;

		memcpy(&firstPointer, blockData, 4);
		if (firstPointer == 0xFFFFFFFF) {
			continue;
		}
		if (firstPointer * 4 > SCENE_BLOCK_SIZE) {
			qWarning() << "Scene::open firstPointer >" << SCENE_BLOCK_SIZE << blockID << firstPointer;
			return false;
		}
		pointers.append(firstPointer * 4);

		while (i < SCENE_BLOCK_SIZE_HEADER) {
			memcpy(&pointer, blockData + i, 4);
			if (pointer == 0xFFFFFFFF) {
				break;
			}
			if (pointer * 4 > SCENE_BLOCK_SIZE) {
				qWarning() << "Scene::open pointer >" << SCENE_BLOCK_SIZE << blockID << pointer;
				return false;
			}
			pointers.append(pointer * 4);
			i += 4;
		}
		pointers.append(SCENE_BLOCK_SIZE);

		for (i = 0; i < (quint32)pointers.size() - 1; ++i) {
			QByteArray gzipped = data.mid(blockID * SCENE_BLOCK_SIZE + pointers.at(i), pointers.at(i+1) - pointers.at(i));
			SceneFile sceneFile;
			if (!gzipped.isEmpty() && !sceneFile.open(GZIP::decompress(gzipped, 0), jp)) {
				qWarning() << "Scene::open cannot open scene" << blockID << i << _scenes.size();
				return false;
			}
			_scenes.append(sceneFile);
		}
	}

	return true;
}

bool Scene::save(const QString &path, QList<quint8> *sceneCountPerBlock) const
{
	QByteArray data;
	if (!save(data, sceneCountPerBlock)) {
		return false;
	}

	QFile f(path);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		return false;
	}
	f.write(data);
	f.close();

	return true;
}

bool Scene::save(QByteArray &data, QList<quint8> *sceneCountPerBlock) const
{
	quint8 blockID = 0, sceneFileID = 0;
	QByteArray blockToc, blockData;

	for (const SceneFile &scene : _scenes) {
		// Save scene
		QByteArray sceneData;
		scene.save(sceneData);
		QByteArray compressedScene = GZIP::compress(sceneData);

		// FIXME: >= 15 or > 15? No matters, this case won't happen, and if so, it won't break the fileformat
		if (sceneFileID >= 15
				|| SCENE_BLOCK_SIZE_HEADER + blockData.size() + (4 - blockData.size() % 4) + compressedScene.size() > SCENE_BLOCK_SIZE) {
			// Create block
			data.append(blockToc.leftJustified(SCENE_BLOCK_SIZE_HEADER, '\xFF'))
					.append(blockData.leftJustified(SCENE_BLOCK_SIZE - SCENE_BLOCK_SIZE_HEADER, '\xFF'));

			if (data.size() % SCENE_BLOCK_SIZE != 0) {
				qWarning() << "Scene::save scene block size !=" << SCENE_BLOCK_SIZE << blockID << data.size();
				return false;
			}
			if (sceneCountPerBlock) {
				sceneCountPerBlock->append(sceneFileID);
			}

			// Next block
			blockToc.clear();
			blockData.clear();
			sceneFileID = 0;
			++blockID;
		}

		// 4-bytes alignment
		if (blockData.size() % 4 != 0) {
			blockData.append(QByteArray(4 - (blockData.size() % 4), '\xFF'));
		}

		quint32 pos = (SCENE_BLOCK_SIZE_HEADER + blockData.size()) / 4;
		blockToc.append((char *)&pos, 4);
		blockData.append(compressedScene);

		// Next scene
		++sceneFileID;
	}

	if (blockID + 1 > SCENE_BLOCK_MAX_COUNT) {
		qWarning() << "Scene::save too much blocks! should not be more than" << SCENE_BLOCK_MAX_COUNT << (blockID + 1);
		return false;
	}

	// Last block
	// Create block
	data.append(blockToc.leftJustified(SCENE_BLOCK_SIZE_HEADER, '\xFF'))
			.append(blockData.leftJustified(SCENE_BLOCK_SIZE - SCENE_BLOCK_SIZE_HEADER, '\xFF'));

	if (data.size() % SCENE_BLOCK_SIZE != 0) {
		qWarning() << "Scene::save scene block size !=" << SCENE_BLOCK_SIZE << blockID << data.size();
		return false;
	}

	if (sceneCountPerBlock) {
		sceneCountPerBlock->append(sceneFileID);
	}

	return true;
}

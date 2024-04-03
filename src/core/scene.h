/****************************************************************************
 ** Néo-Midgar Final Fantasy VII French Retranslation
 ** Copyright (C) 2009-2012 Arzel Jérôme <myst6re@gmail.com>
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#pragma once

#include <QByteArray>
#include <QString>
#include <QList>
#include <QMap>

#define SCENE_SCRIPT_LENGTH_NULL(length) \
	(length | 0x80)
#define SCENE_SCRIPT_LENGTH_HAS_NULL(length) \
	((length & 0x80) != 0)

#define SCENE_SCRIPT_LENGTH_FF(length) \
	(length | 0x40)
#define SCENE_SCRIPT_LENGTH_HAS_FF(length) \
	((length & 0x40) != 0)

#define SCENE_SCRIPT_LENGTH(length) \
	(length & 0x3F)

#define SCENE_BLOCK_SIZE           8192
#define SCENE_BLOCK_SIZE_HEADER    64
#define SCENE_BLOCK_MAX_COUNT      64

struct SceneScriptOpcode
{
	SceneScriptOpcode() : length(0) {}
	
	inline SceneScriptOpcode(quint8 key, const char *args, quint8 length)
	    : key(key), args(args), length(length) {}
	
	QByteArray data() const {
		return QByteArray(args, length);
	}
	
	quint8 key;
	const char *args;
	quint8 length;
};

class SceneScript;

class SceneScriptIterator
{
	friend class SceneScript;
public:
	inline SceneScriptIterator(const SceneScript &script)
	    : _script(script), _cur(0), _oldCur(0) {}
	
	inline void reset() {
		_oldCur = _cur = 0;
	}
	QByteArray nextTextData(int &pos);
	SceneScriptOpcode nextOpcode(int &pos);
	SceneScriptOpcode nextOpcode(quint8 key, int &pos);
private:
	const SceneScript &_script;
	int _cur, _oldCur;
};

class SceneScriptMutableIterator
{
	friend class SceneScript;
public:
	inline SceneScriptMutableIterator(SceneScript &script)
	    : _script(script), _cur(0), _oldCur(0) {}
	
	inline void reset() {
		_oldCur = _cur = 0;
	}
	QByteArray nextTextData(int &pos);
	SceneScriptOpcode nextOpcode(int &pos);
	SceneScriptOpcode nextOpcode(quint8 key, int &pos);
	void setTextData(const QByteArray &text);
private:
	SceneScript &_script;
	int _cur, _oldCur;
};

class SceneScript
{
	friend class SceneScriptIterator;
	friend class SceneScriptMutableIterator;
public:
	inline SceneScript() {}
	
	inline explicit SceneScript(const QByteArray &data)
	    : _data(data) {}
	
	QByteArray save() const;
	
	void updateJumps(int from, int diff);
	int setTextData(int pos, const QByteArray &text);
	inline bool isNull() const {
		return _data.isNull();
	}
private:
	SceneScriptOpcode opcodeAt(int pos, bool &ok) const;
	static const QMap<quint8, quint8> &opLength();
	QByteArray _data;
	static QMap<quint8, quint8> _opLength;
};

class SceneFile
{
public:
	SceneFile();
	explicit SceneFile(const QByteArray &data);
	explicit SceneFile(const QString &path);
	bool open(const QString &path);
	bool open(const QByteArray &data, bool jp = false);
	bool compile();
	bool save(const QString &path) const;
	bool save(QByteArray &data) const;
	inline const QList<SceneScript> &ennemyScript(quint8 id) const {
		return ennemyScripts[id];
	}
	inline const QList<QByteArray> &attackNames() const {
		return _attackNames;
	}
	inline void setAttackNames(const QList<QByteArray> &attackNames) {
		_attackNames = attackNames;
	}
	inline const QByteArray &ennemyName(quint8 id) const {
		return _ennemyNames[id];
	}
	inline void setEnnemyName(quint8 id, const QByteArray &ennemyName) {
		_ennemyNames[id] = ennemyName;
	}
	inline void setEnnemyScript(quint8 id, const QList<SceneScript> &ennemyScript) {
		ennemyScripts[id] = ennemyScript;
	}
private:
	QByteArray _data;
	QList<SceneScript> ennemyScripts[3];
	QByteArray _ennemyNames[3];
	QList<QByteArray> _attackNames;
};

class Scene
{
public:
	Scene();
	bool open(const QString &path, bool jp = false);
	bool open(const QByteArray &data, bool jp = false);
	bool save(const QString &path, QList<quint8> *sceneCountPerBlock = nullptr) const;
	bool save(QByteArray &data, QList<quint8> *sceneCountPerBlock = nullptr) const;
	inline bool isOpen() const {
		return !_scenes.isEmpty();
	}
	inline const QList<SceneFile> &scenes() const {
		return _scenes;
	}
	inline void setScene(int sceneId, const SceneFile &scene) {
		_scenes[sceneId] = scene;
	}
private:
	QList<SceneFile> _scenes;
};

#include <QCoreApplication>
#include <QTimer>
#include <QFile>
#include "arguments.h"
#include "core/scene.h"
#include "core/csvfile.h"
#include "core/ff7text.h"
#include "core/kernelbin.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName(PROG_NAME);
	QCoreApplication::setApplicationVersion(PROG_VERSION);

	Arguments args;

	QString scenePath = args.scenePath(), filePath = args.filePath(),
	        kernelPath = args.kernelPath();
	bool jp = args.jp();

	Scene scene;
	if (!scene.open(scenePath, jp)) {
		qWarning() << "Error: Cannot open scene.bin at" << scenePath;
		args.showHelp(1);
	} else if (args.import()) {
		QFile f(args.filePath());
		if (! f.open(QIODevice::ReadOnly)) {
			qWarning() << "Error: Cannot open source file" << f.errorString() << f.fileName();
			args.showHelp(1);
		}
		CsvFile csv(&f);
		QStringList line;
		int sceneId = 0;
		QList<SceneFile> scenes = scene.scenes();
		for (SceneFile sf: scenes) {
			for (int ennemyId = 0; ennemyId < 3; ++ennemyId) {
				qDebug() << QString("%1 %2 name").arg(sceneId).arg(ennemyId);
				if (! csv.readLine(line) || line.size() < 3) {
					qWarning() << "Error: No more lines";
					args.showHelp(1);
				}
				sf.setEnnemyName(ennemyId, FF7Text(line.at(2), jp).data());
				QList<SceneScript> scripts = sf.ennemyScript(ennemyId);
				for (SceneScript &script: scripts) {
					if (script.isNull()) {
						continue;
					}

					int scriptTextId = 0;
					SceneScriptMutableIterator it(script);
					forever {
						int pos = -1;
						it.nextTextData(pos);

						if (pos < 0) {
							break;
						}

						qDebug() << QString("%1 %2 %3 script").arg(sceneId).arg(ennemyId).arg(scriptTextId);
						if (! csv.readLine(line) || line.size() < 3) {
							qWarning() << "Error: No more lines";
							args.showHelp(1);
						}
						it.setTextData(FF7Text(line.at(2), jp).data());

						scriptTextId += 1;
					}
				}
				sf.setEnnemyScript(ennemyId, scripts);
			}

			QList<QByteArray> attackNames;
			for (int attackId = 0; attackId < sf.attackNames().size(); ++attackId) {
				qDebug() << QString("%1 %2 attack").arg(sceneId).arg(attackId);
				if (! csv.readLine(line) || line.size() < 3) {
					qWarning() << "Error: No more lines";
					args.showHelp(1);
				}
				attackNames.append(FF7Text(line.at(2), jp).data());
			}
			sf.setAttackNames(attackNames);

			scene.setScene(sceneId, sf);
			sceneId += 1;
		}

		QList<quint8> sceneCountPerBlock;
		if (!scene.save(scenePath, &sceneCountPerBlock)) {
			qWarning() << "Error: Cannot save scene.bin" << args.scenePath();
			args.showHelp(1);
		}

		if (!sceneCountPerBlock.isEmpty() && !kernelPath.isEmpty()) {
			if (sceneCountPerBlock.size() > COUNT_PER_BLOCKS_SIZE) {
				qWarning() << "KernelPatch::apply too much block count" << sceneCountPerBlock.size() << "max" << 0x40;
				args.showHelp(1);
			}

			KernelBin kernel;
			if (!kernel.open(kernelPath)) {
				qWarning() << "Cannot open kernel path";
				args.showHelp(1);
			}
			QByteArray battlePart = kernel.part(KernelBin::Battle),
					scenePosPerblock;
			int scenePos = 0;

			foreach (quint8 sceneCount, sceneCountPerBlock) {
				scenePosPerblock.append(char(scenePos));
				scenePos += sceneCount;
				if (scenePos > 0xFF) {
					if (scenePos != 0x100) {
						qWarning() << "KernelPatch::apply scenePos > 0x100" << scenePos;
						args.showHelp(1);
					}
					break;
				}
			}
			scenePosPerblock = scenePosPerblock.leftJustified(COUNT_PER_BLOCKS_SIZE, '\xFF', true);

			if (battlePart.mid(COUNT_PER_BLOCKS_POS, COUNT_PER_BLOCKS_SIZE) != scenePosPerblock) {
				battlePart.replace(COUNT_PER_BLOCKS_POS, COUNT_PER_BLOCKS_SIZE, scenePosPerblock);
				kernel.setPart(KernelBin::Battle, battlePart);
				if (!kernel.save(kernelPath)) {
					qWarning() << "KernelPatch::apply cannot save kernel.bin" << kernelPath;
					args.showHelp(1);
				}
			} else {
				qWarning() << "KernelPatch::apply battle scene part not updated (2)";
				args.showHelp(1);
			}
		}
	} else {
		QFile f(args.filePath());
		if (! f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			qWarning() << "Error: Cannot open target file" << f.errorString() << f.fileName();
			args.showHelp(1);
		}
		CsvFile csv(&f);
		int sceneId = 0;
		for (const SceneFile &sf: scene.scenes()) {
			QString sceneName = QString("scene %1").arg(sceneId);
			for (int ennemyId = 0; ennemyId < 3; ++ennemyId) {
				csv.writeLine(QStringList() << sceneName << QString("ennemy %1 name").arg(ennemyId) << FF7Text(sf.ennemyName(ennemyId)).text(jp));
				for (const SceneScript &script: sf.ennemyScript(ennemyId)) {
					if (script.isNull()) {
						continue;
					}

					int scriptTextId = 0;
					SceneScriptIterator it(script);
					forever {
						int pos = -1;
						QByteArray text = it.nextTextData(pos);

						if (pos < 0) {
							break;
						}

						csv.writeLine(QStringList() << sceneName << QString("ennemy %1 script %2").arg(ennemyId).arg(scriptTextId) << FF7Text(text).text(jp));
						scriptTextId += 1;
					}
				}
			}

			int attackId = 0;
			for (const QByteArray &attackName: sf.attackNames()) {
				csv.writeLine(QStringList() << sceneName << QString("attack %1").arg(attackId) << FF7Text(attackName).text(jp));
				attackId += 1;
			}

			sceneId += 1;
		}
		f.close();
		qDebug() << "Output file saved to" << f.fileName();
	}

	QTimer::singleShot(0, &a, SLOT(quit()));

	return a.exec();
}

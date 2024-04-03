#include <QCoreApplication>
#include <QTimer>
#include <QFile>
#include "arguments.h"
#include "core/scene.h"
#include "core/csvfile.h"
#include "core/ff7text.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName(PROG_NAME);
	QCoreApplication::setApplicationVersion(PROG_VERSION);
	
	Arguments args;
	
	QString scenePath = args.scenePath(), filePath = args.filePath();
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
		for (const SceneFile &sf: scene.scenes()) {
			for (int ennemyId = 0; ennemyId < 3; ++ennemyId) {
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

						scriptTextId += 1;
					}
				}
			}
			
			QList<QByteArray> attackNames;
			for (int attackId = 0; attackId < sf.attackNames().size(); ++attackId) {
				if (! csv.readLine(line) || line.size() < 3) {
					qWarning() << "Error: No more lines";
					args.showHelp(1);
				}
				attackNames.append(FF7Text(line.at(2), jp));
			}
			sf.setAttackNames(attackNames);

			scene.setScene(sceneId, sf);
			sceneId += 1;
		}
		
		scene.save(scenePath);
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

#include "arguments.h"
#include <QCoreApplication>
#include <QDir>

Arguments::Arguments()
{
	_parser.addHelpOption();
	_parser.addVersionOption();
	
	RUFUS_ADD_ARGUMENT(RUFUS_OPTION_NAMES("f", "file"),
	                 "Input/output CSV file path.",
	                 "file", "");
	RUFUS_ADD_ARGUMENT(RUFUS_OPTION_NAMES("k", "kernel"),
	                 "kernel.bin path to update the pointers to the scene.bin.",
	                 "kernel", "");
	RUFUS_ADD_ARGUMENT(RUFUS_OPTION_NAMES("c", "col"),
	                 "When importing CSV into the scene.bin, use the 'column' number (0-based). Default 2.",
	                 "column", "");
	RUFUS_ADD_FLAG(RUFUS_OPTION_NAMES("i", "import"),
	                 "Import CSV file to scene.bin (export by default if this flag is absent).");
	RUFUS_ADD_FLAG(RUFUS_OPTION_NAMES("j", "japan"),
	                 "Use JP format for the scene.bin format and texts.");
	RUFUS_ADD_FLAG(QStringList("skip-script-texts"),
	                 "When importing CSV into the scene.bin, skip script texts. It can be useful when scripts are modified between two differents scene.bin");

	_parser.addPositionalArgument("scene", QCoreApplication::translate("Arguments", "scene.bin path."), "scene");

	_parser.process(*qApp);
}

QString Arguments::scenePath() const
{
	return _parser.positionalArguments().value(0);
}

QString Arguments::filePath() const
{
	QString ret = _parser.value("file");

	if (ret.isEmpty()) {
		return scenePath().append(".csv");
	}

	return ret;
}

QString Arguments::kernelPath() const
{
	return _parser.value("kernel");
}

uint Arguments::column()
{
	QString col = _parser.value("col");
	
	if (col.isEmpty()) {
		return 2;
	}
	
	bool ok = false;
	uint ret = col.toUInt(&ok);
	
	if (! ok) {
		qWarning() << "Column must be a positive number";
		showHelp(1);
	}
	
	return ret;
}

bool Arguments::import() const
{
	return _parser.isSet("import");
}

bool Arguments::jp() const
{
	return _parser.isSet("japan");
}

bool Arguments::skipScriptTexts() const
{
	return _parser.isSet("skip-script-texts");
}

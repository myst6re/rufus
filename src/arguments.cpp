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
	RUFUS_ADD_FLAG(RUFUS_OPTION_NAMES("i", "import"),
	                 "Import CSV file to scene.bin (export by default if this flag is absent).");
	RUFUS_ADD_FLAG(RUFUS_OPTION_NAMES("j", "japan"),
	                 "Use JP format for the scene.bin format and texts.");

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

bool Arguments::import() const
{
	return _parser.isSet("import");
}

bool Arguments::jp() const
{
	return _parser.isSet("japan");
}

#pragma once

#include <QCommandLineParser>

#define RUFUS_ADD_ARGUMENT(names, description, valueName, defaultValue) \
	_parser.addOption(QCommandLineOption(names, description, valueName, defaultValue));

#define RUFUS_ADD_FLAG(names, description) \
	_parser.addOption(QCommandLineOption(names, description));

#define RUFUS_OPTION_NAMES(shortName, fullName) \
	(QStringList() << shortName << fullName)

class Arguments
{
public:
	Arguments();
	inline void showHelp(int exitCode = 0) {
		_parser.showHelp(exitCode);
	}
	QString scenePath() const;
	QString filePath() const;
	QString kernelPath() const;
	bool import() const;
	bool jp() const;
private:
	QCommandLineParser _parser;
};

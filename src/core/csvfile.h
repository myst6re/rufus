#pragma once

#include <QtCore>

class CsvFile
{
public:
	enum CsvEncoding {
		Utf8
	};
	explicit CsvFile(QIODevice *io, const QChar &fieldSeparator = QChar(','), const QChar &quoteCharacter = QChar('"'),
	                 CsvEncoding encoding = Utf8);
	bool readLine(QStringList &line);
	bool writeLine(const QStringList &line);
private:
	enum State {
		Start,
		End,
		NextField,
		FieldQuoted,
		FieldQuotedEscape,
		FieldUnquoted
	};
	QIODevice *_io;
	QChar _fieldSeparator, _quoteCharacter;
	CsvEncoding _encoding;
};

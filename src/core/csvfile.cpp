#include "CsvFile.h"

CsvFile::CsvFile(QIODevice *io, const QChar &fieldSeparator, const QChar &quoteCharacter, CsvEncoding encoding) :
      _io(io), _fieldSeparator(fieldSeparator), _quoteCharacter(quoteCharacter), _encoding(encoding)
{
}

bool CsvFile::readLine(QStringList &line)
{
	QString field;
	line.clear();
	State state = Start;
	
	do {
		if (_io->atEnd()) {
			return false;
		}

		QByteArray lineData = _io->readLine();
		QString l;
		switch (_encoding) {
		case Utf8:
			l = QString::fromUtf8(lineData);
			break;
		}
		
		for (int i = 0; i < l.size(); ++i) {
			QChar c = l.at(i);
			
			switch (state) {
			case Start:
				if (c == _fieldSeparator) {
					state = NextField;
				} else if (c == '\n' || c == '\r') {
					state = End;
				} else if (c == _quoteCharacter) {
					state = FieldQuoted;
				} else {
					state = FieldUnquoted;
				}
				break;
			case End:
			case NextField:
				// Should not happen
				qWarning() << "Invalid state";
				break;
			case FieldQuoted:
				if (c == _quoteCharacter) {
					state = FieldQuotedEscape;
				} else {
					field.append(c);
				}
				break;
			case FieldQuotedEscape:
				if (c == _fieldSeparator) {
					state = NextField;
				} else if (c == '\n' || c == '\r') {
					state = End;
				} else if (c == _quoteCharacter) {
					field.append(c);
					state = FieldQuoted;
				} else {
					field.append(_quoteCharacter); // Abort escape, put the quote character of the previous iteration
					field.append(c);
					state = FieldQuoted;
				}
				break;
			case FieldUnquoted:
				if (c == _fieldSeparator) {
					state = NextField;
				} else if (c == '\n' || c == '\r') {
					state = End;
				} else {
					field.append(c);
					state = FieldUnquoted;
				}
				break;
			}
			
			if (state == NextField) {
				line.append(field);
				field = QString();
				if (i + 1 < l.size()) {
					state = Start;
				}
			} else if (state == End) {
				line.append(field);
				break;
			}
		}
		
		// Flush unfinished states
		switch (state) {
		case End:
		case NextField:
			// Nothing to do
			break;
		case FieldQuoted:
			// Will read another line
			break;
		case Start:
		case FieldQuotedEscape:
		case FieldUnquoted:
			line.append(field);
			break;
		}
	} while (state == FieldQuoted);
	
	return true;
}

bool CsvFile::writeLine(const QStringList &line)
{
	QString l, esc = QString().append(_quoteCharacter).append(_quoteCharacter);

	for (QString field: line) {
		l.append(_quoteCharacter)
		        .append(field.replace(_quoteCharacter, esc))
		        .append(_quoteCharacter)
		        .append(_fieldSeparator);
	}
	
	l[l.size() - 1] = '\r';
	l.append('\n');
	
	QByteArray lineData;
	
	switch (_encoding) {
	case Utf8:
		lineData = l.toUtf8();
		break;
	}
	
	return _io->write(lineData) == lineData.size();
}

#ifndef DEF_FF7TEXT
#define DEF_FF7TEXT

#include <QtCore>

class FF7Text
{
public:
	static int indexOfFF(const QByteArray &data, int indexOfText = 0);

	FF7Text(const QByteArray &data=QByteArray());
	FF7Text(const QString &text, bool jp);
	const QByteArray &data() const;
	QString text(bool jp, bool simplified=false) const;
	void setText(const QString &text, bool jp);
	inline bool operator ==(const FF7Text &t2) const {
		return data() == t2.data();
	}
	inline bool operator !=(const FF7Text &t2) const {
		return data() != t2.data();
	}

private:
	static QString getCaract(quint8 ord, quint8 table=0);
	static const char *caract[256];
	static const char *caract_jp[256];
	static const char *caract_jp_fa[256];
	static const char *caract_jp_fb[256];
	static const char *caract_jp_fc[256];
	static const char *caract_jp_fd[256];
	static const char *caract_jp_fe[256];
	QByteArray _data;
};

#endif

#include "ff7text.h"

int FF7Text::indexOfFF(const QByteArray &data, int indexOfText)
{
	int i = 0, size = data.size() - indexOfText;

	while (i < size) {
		quint8 c = data.at(indexOfText + i);

		if (c >= 0xEA && c <= 0xF0) { // Var
			i += 2;
			if (i >= size) {
				qWarning() << "FF7Text::indexOfFF var overflow";
			}
		} else if (c >= 0xF8 && c <= 0xFE) { // Window Color + Compression + JP tables
			i += 1;
			if (i >= size) {
				qWarning() << "FF7Text::indexOfFF overflow";
			}
		} else if (0xFF == c) { // End of text
			return indexOfText + i;
		}

		i += 1;
	}

	return -1;
}

FF7Text::FF7Text(const QByteArray &data)
{
	int index = indexOfFF(data);
	_data = index != -1 ? data.left(index) : data;
}

FF7Text::FF7Text(const QString &text, bool jp)
{
	setText(text, jp);
}

const QByteArray &FF7Text::data() const
{
	return _data;
}

QString FF7Text::text(bool jp, bool simplified) const
{
	QString trad, character;
	quint8 index;
//	bool jp = Config::value("jp_txt", false).toBool();
	int size = _data.size();

	for(quint16 i=0 ; i<size ; ++i)
	{
		index = (quint8)_data.at(i);
		if(index == 0xFF)	break;
		switch(index) {
		case 0xFA:
			++i;
			if(size<=i)	return trad.append(simplified ? "¶" : "{xfa}");
			if(jp) {
				character = getCaract(_data.at(i), 3);
				if(character.isEmpty()) {
					character = simplified ? "¶" : QString("{xfa}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0'));
				}
				trad.append(character);
			} else {
				trad.append(simplified ? "¶" : QString("{xfa}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0')));
			}
		break;
		case 0xFB:
			++i;
			if(size<=i)	return trad.append(simplified ? "¶" : "{xfb}");
			if(jp) {
				character = getCaract(_data.at(i), 4);
				if(character.isEmpty()) {
					character = simplified ? "¶" : QString("{xfb}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0'));
				}
				trad.append(character);
			} else {
				trad.append(simplified ? "¶" : QString("{xfb}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0')));
			}
		break;
		case 0xFC:
			++i;
			if(size<=i)	return trad.append(simplified ? "¶" : "{xfc}");
			if(jp) {
				character = getCaract(_data.at(i), 5);
				if(character.isEmpty()) {
					character = simplified ? "¶" : QString("{xfc}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0'));
				}
				trad.append(character);
			} else {
				trad.append(simplified ? "¶" : QString("{xfc}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0')));
			}
		break;
		case 0xFD:
			++i;
			if(size<=i)	return trad.append(simplified ? "¶" : "{xfd}");
			if(jp) {
				character = getCaract(_data.at(i), 6);
				if(character.isEmpty()) {
					character = simplified ? "¶" : QString("{xfd}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0'));
				}
				trad.append(character);
			} else {
				trad.append(simplified ? "¶" : QString("{xfd}{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0')));
			}
		break;
		case 0xFE:
			++i;
			if(size<=i) 	return trad.append(simplified ? "¶" : "{xfe}");;
			index = (quint8)_data.at(i);

			if(index == 0xE2) {
				if(!simplified) {
					trad.append("{xfe}{xe2}");
					++i;
					for(int i2=i+4 ; i<i2 ; ++i) {
						if(size<=i) 	return trad;
						trad.append(QString("{x%1}").arg((quint8)_data.at(i), 2, 16, QChar('0')));
					}
					--i;
				} else {
					i+=4;
				}
			} else if(index == 0xDD) {
				++i;
				if(!simplified) {
					if(size<=i)	return trad.append("{xfe}{xdd}");
					trad.append(QString("{PAUSE%1}").arg((quint8)_data.at(i), 3, 10, QChar('0')));
				}
			} else {
				if(jp) {
					character = getCaract(index, 7);
					if(index >= 0xd2 && simplified) {
						character = "¶";
					} else if(character.isEmpty()) {
						character = simplified ? "¶" : QString("{xfe}{x%1}").arg(index, 2, 16, QChar('0'));
					}
					trad.append(character);
				} else {
					character = QString();
					if(index >= 0xd2 && !simplified) {
						character = getCaract(index, 7);
					}
					if(character.isEmpty()) {
						character = simplified ? "¶" : QString("{xfe}{x%1}").arg(index, 2, 16, QChar('0'));
					}
					trad.append(character);
				}
			}
		break;
		default:
			character = getCaract(index, jp ? 2 : 0);
			if((index == 0xe0 || index == 0xe1 || index == 0xe7 || index == 0xe8) && simplified) {
				character = " ";
			}
			else if(character.isEmpty()) {
				character = simplified ? "¶" : QString("{x%1}").arg(index, 2, 16, QChar('0'));
			}
			trad.append(character);
		break;
		}
	}
	return trad;
}

void FF7Text::setText(const QString &string, bool jp)
{
	QChar comp;
	int stringSize = string.size(), i, table;
	bool ok;
	ushort value;
//	bool jp = Config::value("jp_txt", false).toBool();

	_data.clear();

	for(int c=0 ; c<stringSize ; ++c)
	{
		comp = string.at(c);
		if(comp=='\n') {//\n{New Page}\n,\n
			if(string.mid(c+1, 11).compare("{New Page}\n", Qt::CaseInsensitive) == 0) {
				_data.append('\xe8');
				c += 11;
			}
			else if(string.mid(c+1, 13).compare("{New Page 2}\n", Qt::CaseInsensitive) == 0) {
				_data.append('\xe9');
				c += 13;
			}
			else {
				_data.append('\xe7');
			}
			continue;
		}
		else if(comp=='{') {
			QString rest = string.mid(c);
			for(i=0 ; i<16 ; ++i) {
				if(rest.startsWith(getCaract(0xea + i), Qt::CaseInsensitive)) {//{Name},{Key}
					_data.append((char)(0xea + i));
					c += getCaract(0xea + i).size()-1;
					goto end;
				}
			}

			for(i=0 ; i<11 ; ++i) {
				if(rest.startsWith(getCaract(0xd2 + i, 7), Qt::CaseInsensitive)) {//{Colors},{PAUSE}
					_data.append('\xfe').append((char)(0xd2 + i));
					c += getCaract(0xd2 + i, 7).size()-1;
					goto end;
				}
			}

			for(i=0 ; i<4 ; ++i) {
				if(rest.startsWith(getCaract(0xde + i, 7), Qt::CaseInsensitive)) {//{Vars}
					_data.append('\xfe').append((char)(0xde + i));
					c += getCaract(0xde + i, 7).size()-1;
					goto end;
				}
			}

			if(rest.startsWith(getCaract(0xe9, 7), Qt::CaseInsensitive)) {//{SPACED CHARACTERS}
				_data.append("\xfe\xe9", 2);
				c += getCaract(0xe9, 7).size()-1;
				goto end;
			}

			if(rest.startsWith("{PAUSE", Qt::CaseInsensitive) && 9<rest.size() && rest.at(9)=='}') {//{PAUSE000}
				value = rest.mid(6,3).toUShort(&ok);
				if(ok) {
					_data.append("\xfe\xdd", 2).append((char)value);
					c += 9;
					goto end;
				}
			}

			if(!jp) {
				if(rest.startsWith(getCaract(0xe0), Qt::CaseInsensitive)) {//{CHOICE}
					_data.append('\xe0');
					c += 7;
					goto end;
				}

				for(i=0 ; i<3 ; ++i) {
					if(rest.startsWith(getCaract(0xe2 + i))) {//{, }{."}{?"}
						_data.append((char)(0xe2 + i));
						c += 3;
						goto end;
					}
				}
			}

			if(string.at(c+1)=='x' && string.at(c+4)=='}') {//{x00}
				value = string.mid(c+2,2).toUShort(&ok,16);
				if(ok && value != 0xff) {
					_data.append((char)value);
					c += 4;
					continue;
				}
			}

			_data.append('\x5b');// {

			continue;
		}

		for(i=0x00 ; i<=0xfe ; ++i)
		{
			if(QString::compare(comp, getCaract(i, jp ? 2 : 0))==0)
			{
				_data.append((char)i);
				goto end;
			}
		}

		if(jp) {
			for(table=3 ; table<8 ; ++table)
			{
				for(i=0x00 ; i<=0xfe ; ++i)
				{
					if(QString::compare(comp, getCaract(i, table))==0)
					{
						switch(table) {
						case 3:
							_data.append('\xfa');
							break;
						case 4:
							_data.append('\xfb');
							break;
						case 5:
							_data.append('\xfc');
							break;
						case 6:
							_data.append('\xfd');
							break;
						case 7:
							_data.append('\xfe');
							break;
						}
						_data.append((char)i);
						goto end;
					}
				}
			}
		}

		end:;
	}
}

QString FF7Text::getCaract(quint8 ord, quint8 table)
{
	switch(table) {
	case 2:
		return QString::fromUtf8(FF7Text::caract_jp[ord]);
	case 3:
		return QString::fromUtf8(FF7Text::caract_jp_fa[ord]);
	case 4:
		return QString::fromUtf8(FF7Text::caract_jp_fb[ord]);
	case 5:
		return QString::fromUtf8(FF7Text::caract_jp_fc[ord]);
	case 6:
		return QString::fromUtf8(FF7Text::caract_jp_fd[ord]);
	case 7:
		return QString::fromUtf8(FF7Text::caract_jp_fe[ord]);
	default:
		return QString::fromUtf8(FF7Text::caract[ord]);
	}
}

const char *FF7Text::caract[256] =
{
	" ","!","\"","#","$","%","&","'","(",")","*","+",",","-",".","/",
	"0","1","2","3","4","5","6","7","8","9",":",";","<","=",">","?",
	"@","A","B","C","D","E","F","G","H","I","J","K","L","M","N","O",
	"P","Q","R","S","T","U","V","W","X","Y","Z","[","\\","]","^","_",
	"`","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o",
	"p","q","r","s","t","u","v","w","x","y","z","{","|","}","~","",
	"Ä","","Ç","É","Ñ","Ö","Ü","á","à","â","ä","ã","å","ç","é","è",
	"ê","ë","í","ì","î","ï","ñ","ó","ò","ô","ö","õ","ú","ù","û","ü",
	"","°","¢","£","","","","ß","®","©","™","´","¨","≠","Æ","Ø",
	"∞","±","≤","≥","¥","µ","∂","Σ","Π","π","⌡","ª","º","Ω","æ","ø",
	"¿","¡","¬","√","ƒ","≈","∆","«","»","…","","À","Ã","Õ","Œ","œ",
	"–","—","“","”","‘","’","÷","◊","ÿ","Ÿ","","¤","‹","›","ﬁ","ﬂ",
	"■","▪","‚","„","‰","Â","Ê","Á","Ë","È","Í","Î","Ï","Ì","Ó","Ô",
	"","Ò","Ù","Û","","","","","","","","","","","","",
	"{CHOICE}","\t","{, }","{.\"}","{…\"}","","","\n","\n{NEW PAGE}\n","\n{NEW PAGE 2}\n","{CLOUD}","{BARRET}","{TIFA}","{AERIS}","{RED XIII}","{YUFFIE}",
	"{CAIT SITH}","{VINCENT}","{CID}","{MEMBER 1}","{MEMBER 2}","{MEMBER 3}","{CIRCLE}","{TRIANGLE}","{SQUARE}","{CROSS}",""/*jp_fa*/,""/*jp_fb*/,""/*jp_fc*/,""/*jp_fd*/,""/*jp_fe*/,""/*end*/
};

const char *FF7Text::caract_jp[256]=
{
	"バ","ば","ビ","び","ブ","ぶ","ベ","べ","ボ","ぼ","ガ","が","ギ","ぎ","グ","ぐ",
	"ゲ","げ","ゴ","ご","ザ","ざ","ジ","じ","ズ","ず","ゼ","ぜ","ゾ","ぞ","ダ","だ",
	"ヂ","ぢ","ヅ","づ","デ","で","ド","ど","ヴ","パ","ぱ","ピ","ぴ","プ","ぷ","ペ",
	"ぺ","ポ","ぽ","０","１","２","３","４","５","６","７","８","９","、","。","　",
	"ハ","は","ヒ","ひ","フ","ふ","ヘ","へ","ホ","ほ","カ","か","キ","き","ク","く",
	"ケ","け","コ","こ","サ","さ","シ","し","ス","す","セ","せ","ソ","そ","タ","た",
	"チ","ち","ツ","つ","テ","て","ト","と","ウ","う","ア","あ","イ","い","エ","え",
	"オ","お","ナ","な","ニ","に","ヌ","ぬ","ネ","ね","ノ","の","マ","ま","ミ","み",
	"ム","む","メ","め","モ","も","ラ","ら","リ","り","ル","る","レ","れ","ロ","ろ",
	"ヤ","や","ユ","ゆ","ヨ","よ","ワ","わ","ン","ん","ヲ","を","ッ","っ","ャ","ゃ",
	"ュ","ゅ","ョ","ょ","ァ","ぁ","ィ","ぃ","ゥ","ぅ","ェ","ぇ","ォ","ぉ","！","？",
	"『","』","．","＋","Ａ","Ｂ","Ｃ","Ｄ","Ｅ","Ｆ","Ｇ","Ｈ","Ｉ","Ｊ","Ｋ","Ｌ",
	"Ｍ","Ｎ","Ｏ","Ｐ","Ｑ","Ｒ","Ｓ","Ｔ","Ｕ","Ｖ","Ｗ","Ｘ","Ｙ","Ｚ","・","＊",
	"ー","～","…","％","／","：","＆","【","】","♥","→","α","β","「","」","（",
	"）","－","＝","","","","⑬","\n","\n{NEW PAGE}\n","\n{NEW PAGE 2}\n","{CLOUD}","{BARRET}","{TIFA}","{AERIS}","{RED XIII}","{YUFFIE}",
	"{CAIT SITH}","{VINCENT}","{CID}","{MEMBER 1}","{MEMBER 2}","{MEMBER 3}","{CIRCLE}","{TRIANGLE}","{SQUARE}","{CROSS}","","","","","",""
};

const char *FF7Text::caract_jp_fa[256]=
{
	"必","殺","技","地","獄","火","炎","裁","雷","大","怒","斬","鉄","剣","槍","海",
	"衝","聖","審","判","転","生","改","暗","黒","釜","天","崩","壊","零","式","自",
	"爆","使","放","射","臭","息","死","宣","告","凶","破","晄","撃","画","龍","晴",
	"点","睛","超","究","武","神","覇","癒","風","邪","気","封","印","吹","烙","星",
	"守","護","命","鼓","動","福","音","掌","打","水","面","蹴","乱","闘","合","体",
	"疾","迅","明","鏡","止","抜","山","蓋","世","血","祭","鎧","袖","一","触","者",
	"滅","森","羅","万","象","装","備","器","攻","魔","法","召","喚","獣","呼","出",
	"持","相","手","物","確","率","弱","投","付","与","変","化","片","方","行","決",
	"定","分","直","前","真","似","覚","列","後","位","置","防","御","発","回","連",
	"続","敵","全","即","効","果","尾","毒","消","金","針","乙","女","興","奮","剤",
	"鎮","静","能","薬","英","雄","榴","弾","右","腕","砂","時","計","糸","戦","惑",
	"草","牙","南","極","冷","結","晶","電","鳥","角","有","害","質","爪","光","月",
	"反","巨","目","砲","重","力","球","空","双","野","菜","実","兵","単","毛","茶",
	"色","髪","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","",""
};

const char *FF7Text::caract_jp_fb[256]=
{
	"安","香","花","会","員","蜂","蜜","館","下","着","入","先","不","子","供","屋",
	"商","品","景","交","換","階","模","型","部","離","場","所","仲","間","無","制",
	"限","殿","様","秘","氷","河","図","何","材","料","雪","上","進","事","古","代",
	"種","鍵","娘","紙","町","住","奥","眠","楽","最","初","村","雨","釘","陸","吉",
	"揮","叢","雲","軍","異","常","通","威","父","蛇","矛","青","偃","刀","戟","十",
	"字","裏","車","円","輪","卍","折","鶴","倶","戴","螺","貝","突","銀","玉","正",
	"宗","具","甲","烈","属","性","吸","収","半","減","土","高","級","状","態","縁",
	"闇","睡","石","徐","々","的","指","混","呪","開","始","歩","復","盗","小","治",
	"理","同","速","遅","逃","去","視","複","味","沈","黙","還","倍","数","瀕","取",
	"返","人","今","差","誰","当","拡","散","飛","以","外","暴","避","振","身","中",
	"旋","津","波","育","機","械","擲","炉","新","両","本","君","洞","内","作","警",
	"特","殊","板","強","穴","隊","族","亡","霊","鎖","足","刃","頭","怪","奇","虫",
	"跳","侍","左","首","潜","長","親","衛","塔","宝","条","像","忍","謎","般","見",
	"報","充","填","完","了","銃","元","経","験","値","終","獲","得","名","悲","蛙",
	"操","成","費","背","切","替","割","","","","","","","","","",
	"","","","","","","","","","","","","","","",""
};

const char *FF7Text::caract_jp_fc[256]=
{
	"由","閉","記","憶","選","番","街","底","忘","都","過","艇","路","運","搬","船",
	"墓","心","港","末","宿","西","道","艦","家","乗","竜","巻","迷","宮","絶","壁",
	"支","社","久","件","想","秒","予","多","落","受","組","余","系","標","起","迫",
	"日","勝","形","引","現","解","除","磁","互","口","廃","棄","汚","染","液","活",
	"令","副","隠","主","斉","登","温","泉","百","段","熱","走","急","降","奪","響",
	"嵐","移","危","戻","遠","吠","軟","骨","言","葉","震","叫","噴","舞","狩","粉",
	"失","敗","眼","激","盤","逆","鱗","踏","喰","盾","叩","食","凍","退","木","吐",
	"線","魅","押","潰","曲","翼","教","皇","太","陽","界","案","挑","援","赤","往",
	"殴","意","東","北","参","知","聞","来","仕","別","集","信","用","思","毎","悪",
	"枯","考","然","張","好","伍","早","各","独","配","腐","話","帰","永","救","感",
	"故","売","浮","市","加","流","約","宇","礼","束","母","男","年","待","宙","立",
	"残","俺","少","精","士","私","険","関","倒","休","我","許","郷","助","要","問",
	"係","旧","固","荒","稼","良","議","導","夢","追","説","声","任","柱","満","未",
	"顔","旅","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","",""
};

const char *FF7Text::caract_jp_fd[256]=
{
	"友","伝","夜","探","対","調","民","読","占","頼","若","学","識","業","歳","争",
	"苦","織","困","答","準","恐","認","客","務","居","他","再","幸","役","縮","情",
	"豊","夫","近","窟","責","建","求","迎","貸","期","工","算","湿","難","保","帯",
	"届","凝","笑","向","可","遊","襲","申","次","国","素","題","普","密","望","官",
	"泣","創","術","演","輝","買","途","浴","老","幼","利","門","格","原","管","牧",
	"炭","彼","房","驚","禁","注","整","衆","語","証","深","層","査","渡","号","科",
	"欲","店","括","坑","酬","緊","研","権","書","暇","兄","派","造","広","川","賛",
	"駅","絡","在","党","岸","服","捜","姉","敷","胸","刑","谷","痛","岩","至","勢",
	"畑","姿","統","略","抹","展","示","修","酸","製","歓","接","障","災","室","索",
	"扉","傷","録","優","基","讐","勇","司","境","璧","医","怖","狙","協","犯","資",
	"設","雇","根","億","脱","富","躍","純","写","病","依","到","練","順","園","総",
	"念","維","検","朽","圧","補","公","働","因","朝","浪","祝","恋","郎","勉","春",
	"功","耳","恵","緑","美","辺","昇","悩","泊","低","酒","影","競","二","矢","瞬",
	"希","志","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","",""
};

const char *FF7Text::caract_jp_fe[256]=
{
	"孫","継","団","給","抗","違","提","断","島","栄","油","就","僕","存","企","比",
	"浸","非","応","細","承","編","排","努","締","談","趣","埋","営","文","夏","個",
	"益","損","額","区","寒","簡","遣","例","肉","博","幻","量","昔","臓","負","討",
	"悔","膨","飲","妄","越","憎","増","枚","皆","愚","療","庫","涙","照","冗","壇",
	"坂","訳","抱","薄","義","騒","奴","丈","捕","被","概","招","劣","較","析","繁",
	"殖","耐","論","貴","称","千","歴","史","募","容","噂","壱","胞","鳴","表","雑",
	"職","妹","氏","踊","停","罪","甘","健","焼","払","侵","頃","愛","便","田","舎",
	"孤","晩","清","際","領","評","課","勤","謝","才","偉","誤","価","欠","寄","忙",
	"従","五","送","周","頑","労","植","施","販","台","度","嫌","諸","習","緒","誘",
	"仮","借","輩","席","戒","弟","珍","酔","試","騎","霜","鉱","裕","票","券","専",
	"祖","惰","偶","怠","罰","熟","牲","燃","犠","快","劇","拠","厄","抵","適","程",
	"繰","腹","橋","白","処","匹","杯","暑","坊","週","秀","看","軽","幕","和","平",
	"王","姫","庭","観","航","横","帳","丘","亭","財","律","布","規","謀","積","刻",
	"陥","類","{GREY}","{BLUE}","{RED}","{PURPLE}","{GREEN}","{CYAN}","{YELLOW}","{WHITE}","{BLINK}","{MULTICOLOUR}","{PAUSE}","","{VAR1}","{VAR2}",
	"{SCROLLING}","{VAR3}",""/*chocobo*/,"","","","","","","{SPACED CHARACTERS}","","","","","","",
	"","","","","","","","","","","","","","","",""
};

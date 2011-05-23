#include "client.h"

#include <QStringList>
#include <QFileInfo>
#include <QDateTime>
#include <QTextStream>

using namespace gloox;

Bot::Bot()
	: AdminJid("")
	, BotJid("a@b/c")
	, BotHost("")
	, BotPass("")
	, ProxyHost("")
	, ProxyPort(3128)
	, BotPort(5222)
	, Separator("/")
	, Prefix("Server")
{
	if(!readSettings())
		return;

	ping = false;

	j = new Client(BotJid.toStdString(), BotPass.toStdString(), BotPort);
	RM = new RosterManager(j);
	j->registerSubscriptionHandler(this);
	j->registerConnectionListener(this);
	j->registerMessageSessionHandler(this, 0);
	j->logInstance().registerLogHandler(LogLevelDebug, LogAreaAllClasses, this);
	if(BotHost != "") {
		j->setServer(BotHost.toStdString()); }
	j->disco()->setVersion("ATNF", "0.2.1", "");
	j->setPresence(PresenceAvailable, 0, "Send *HELP for more information");
	if(ProxyHost != "") {
		tcpcl = new ConnectionTCPClient(j->logInstance(), ProxyHost.toStdString(), ProxyPort);
		conn = new ConnectionHTTPProxy(j, tcpcl, j->logInstance(), BotHost.toStdString(), BotPort);
		j->setConnectionImpl(conn);
	}
	connect();
}

Bot::~Bot()
{
	j->disconnect();
	delete j;
	delete RM;
	delete conn;
	delete tcpcl;
}

void Bot::connect()
{
	int Time;
	QTime T;
	QString Mes;
	QString path = "lastcheck.txt";
	j->connect(false);
	je = j->recv(20000000);
	ce = ConnNoError;
	if(ProxyHost != "") {
		ce = conn->recv(1000);
	}
	while(je == ConnNoError && ce == ConnNoError)  {
		T = QTime::currentTime();
		Time = T.toString("hhmm").toInt();
		if( Time == 0 ) {
			foreach (M_Session S, Sessions) {
				j->disposeMessageSession(S.session);
			}
			Sessions.clear();
			error_report("Deleting sessions...");
		}
		Time = T.toString("hhmmss").toInt();
		if(Time > 130000 && Time < 130020) {
			QFile sub("sub.txt");
			QTextStream out(&sub);
			if(sub.exists()) {
				double d = readDate(path);
				foreach (QString val, BotFolders) {
					int Max = 0;
					Mes = "";
					MessFiles.clear();
					filesToMessage(Mes, Max, val, d);
					if(Mes != "") {
						MessFiles << Mes; }
					if(!MessFiles.isEmpty()) {
						sub.open(QIODevice::ReadOnly);
						QString Jid;
						foreach(QString M, MessFiles) {
							while(!out.atEnd()) {
								Jid = out.readLine();
								JID j(Jid.toStdString());
								sendMessage(j, M);
							}
						}
						MessFiles.clear();
						sub.close();
					}
				}
			}
			QFile lc(path);
			lc.open(QFile::WriteOnly);
			out.setDevice(&lc);
			out << QDateTime::currentDateTime().toString("yyyyMMddhhmmss") << endl;
		}
		if(ping) {
			j->whitespacePing();
		}
		je = j->recv(20000000);
		if(ProxyHost != "") {
			ce = conn->recv(1000);
		}
	}
}

void Bot::handleMessageSession(MessageSession *session)
{
	error_report("New Session!");
	int Date = QTime::currentTime().toString("hhmmss").toLong();
	M_Session S = { session, false, false, 0, Date, false, 0 };
	Sessions << S;
	session->registerMessageHandler(this);
}

void Bot::handleMessage( Stanza* stanza, MessageSession* session)   //Пытаемся разобрать, что от нас там хотят
{
	QString Mes;
	JID userJid = stanza->from();
	QString Body = QString::fromStdString(stanza->body());
	loger(userJid, Body);
	QStringList fields = Body.split(" ");
	QString val = fields.takeFirst();
	val = val.toUpper();
	if(userJid == tojid || QString::fromStdString(userJid.full()) == BotJid)
		return;

	if(val == "*HELP" || val == "?") {
		QFile in("help.txt");
		in.open(QIODevice::ReadOnly);
		QTextStream n(&in);
		Mes = n.readAll();
		session->send(Mes.toStdString());
	}
	else if(val == "*SUB") {
		QFile sub("sub.txt");
		QTextStream out(&sub);
		if(sub.open(QIODevice::ReadWrite)) {
			while(!out.atEnd()) {
				if(out.readLine() == QString::fromStdString(userJid.bare())) {
					session->send(QObject::trUtf8("Вы уже подписаны!").toStdString());
					return;
				}
			}
			out.seek(sub.size());
			out.setGenerateByteOrderMark(false);
			out << QString::fromStdString(userJid.bare()) << endl;
			session->send(QObject::trUtf8("Подписаны!").toStdString());
		}
	}
	else if(val == "*UNSUB") {
		QStringList List;
		QFile sub("sub.txt");
		QTextStream out(&sub);
		if(sub.open(QIODevice::ReadOnly)) {
			while(!out.atEnd()) {
				QString Line = out.readLine();
				if(Line != QString::fromStdString(userJid.bare())) {
					List << Line;
				}
			}
			sub.remove();
			if(sub.open(QIODevice::WriteOnly)) {
				while(!List.isEmpty()) {
					out.setGenerateByteOrderMark(false);
					out << List.takeFirst() << endl;
				}
			}
			session->send(QObject::trUtf8("Вы отказались от рассылки!").toStdString());
		}
	}
	else if(val == "*SINCE") {
		for(int i = Sessions.size(); i > 0;) {
			M_Session &S = Sessions[--i];
			if (S.session == session) {
				S.block = false;
				if(S.Since > 5) {
					session->send(QObject::trUtf8("Не надоедайте Боту! :)").toStdString());
				}
				else {
					bool k = fields.isEmpty();
					if(k)
						session->send(QObject::trUtf8("Неправильная дата!").toStdString());
					else {
						QString D = fields.takeFirst();
						int l = D.length();
						if(l != 8)
							session->send(QObject::trUtf8("Неправильная дата!").toStdString());
						else {
							int test = D.toInt();
							if(test < 20080101)
								session->send(QObject::trUtf8("Дата старее 2008 года!").toStdString());
							else {
								D += "000000";
								double Dd = D.toDouble();
								createMessage(session, Dd);
								S.Since++;
							}
						}
					}
				}
			}
		}
        }
	else if(val == "*ALL") {
		for(int i = Sessions.size(); i > 0;) {
			M_Session &S = Sessions[--i];
			if (S.session == session) {
				S.block = false;
				if(S.All)
					session->send(QObject::trUtf8("Сегодня от Вас уже был такой запрос. Хватит! ;)").toStdString());
				else {
					QString D = "19500101000000";
					double d = D.toDouble();
					createMessage(session, d);
					S.All = true;
				}
			}
		}
	}
	else if(val == "*SEND") {
		if(!AdminJid.isEmpty() && QString::fromStdString(userJid.bare()) != AdminJid)
			return;
		fields = Body.split(":");
		Mes = fields.takeLast();
		bool k = fields.isEmpty();
		if(k)
			session->send(QObject::trUtf8("Что-то не так!").toStdString());
		else {
			val = fields.takeFirst();
			fields = val.split(" ");
			val = fields.takeLast();
			tojid = val.toStdString();
			sendMessage(tojid, Mes);
		}
	}
	else if(val == "*NEW") {
		for(int i = Sessions.size(); i > 0;) {
			M_Session &S = Sessions[--i];
			if (S.session == session) {
				S.block = false;
				if(S.New)
					session->send(QObject::trUtf8("Сегодня от Вас уже был такой запрос. Хватит! ;)").toStdString());
				else {
					dateToMessage(session);
					S.New = true;
				}
			}
		}
        }
	else if(val == "*QUIT") {
		if(!AdminJid.isEmpty() && QString::fromStdString(userJid.bare()) != AdminJid)
			return;
		j->disconnect();
	}
	else if(val == ";-)" || val == ";)" || val == ":)" || val == ":-)" || val == ":-(" || val == "8-)" || val == ":-D" || val == ":-P" ) {
		session->send(" :-P ");
	}
	else if(val == "*MULTYSEND") {
		if(!AdminJid.isEmpty() && QString::fromStdString(userJid.bare()) != AdminJid)
			return;
		while(!fields.isEmpty()) {
			Mes += fields.takeFirst() + " "; }
		QDir dir("Data");
		foreach(QString J, dir.entryList(QDir::Files)) {
			JID Jid(J.toStdString());
			sendMessage(Jid, Mes);
		}
	}
	else {
		for(int i = Sessions.size(); i > 0;) {
			M_Session &S = Sessions[--i];
			if (S.session == session) {
				if (S.block)
					return;
				else {
					session->send(QObject::trUtf8("Это не команда! Пошлите *HELP чтобы узнать больше!").toStdString());
					if(!S.N) {
						S.s_time = QTime::currentTime().toString("hhmmss").toLong();
						S.N++;
					}
					else {
						S.N++;
						int T = QTime::currentTime().toString("hhmmss").toLong()  - S.s_time;
						if(S.N == 5 && T < 5) {
							S.block = true;
						}
						else {
							if(T > 5)
								S.N = 0;
						}
					}
				}
			}
		}
	}
}

void Bot::sendMessage(JID jid, const QString& mes)  //Отсылка сообщений
{
	std::string Mes = mes.toStdString();
	Stanza *s = Stanza::createMessageStanza(jid.full(), Mes);
	j->send(s);
}

bool Bot::onTLSConnect(const CertInfo& /*info*/)
{
        return true;
}

void Bot::loger(JID userJid, const QString& Mes)
{
	QFile file("log.txt");
	if(file.open(QIODevice::WriteOnly | QIODevice::Append)) {
		QTextStream out(&file);
		QString nw = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") + " " + QString::fromStdString(userJid.full()) + ": " + Mes;
		out.setGenerateByteOrderMark(false);
		out << nw << endl;
	}
}

void Bot::filesToMessage(QString &Mess, int &Max, const QString &path, double dateIDLong)
{
	QDir dir(path);
	QString M;
	foreach (const QString& file, dir.entryList(QDir::Files)) {
		bool use = false;
		if(extentions.isEmpty())
			use = true;
		else {
			foreach(const QString& ext, extentions) {
				QRegExp r = QRegExp(ext, Qt::CaseInsensitive, QRegExp::Wildcard);
				if(file.contains(r)) {
					use = true;
					break;
				}
			}
		}
		if(!use)
			continue;

		double DateTimeLong = QFileInfo(dir, file).created().toString("yyyyMMddhhmmss").toDouble();
		if(DateTimeLong > dateIDLong) {
			M = path + QDir::separator() + file;
			QStringList list = M.split(QDir::separator());
			M = list.takeFirst();
			M = Prefix;
			while(!list.isEmpty()) {
				M +=  Separator + list.takeFirst();
			}
			Mess += M + "\n";
			Max++;
			if(Max > 200) {
				MessFiles << Mess;
				Max = 0;
				Mess = "";
			}
		}
	}
	foreach (QString subDir, dir.entryList(QDir::Dirs
					       | QDir::NoDotAndDotDot)) {
		filesToMessage(Mess, Max, path + QDir::separator() + subDir, dateIDLong);
	}
}

bool Bot::readSettings()
{
	using namespace std;
	QFile file("settings.ini");
	if(!file.exists()) {
		cerr << "Cannot open settings file" << endl;
		return false;
	}
	if(!file.open(QFile::ReadOnly)) {
		cerr << "Cannot open settings file" << endl;
		return false;
	}
	QTextStream inFile(&file);
	while (!inFile.atEnd()){
		const QString Settings = inFile.readLine();
		QStringList fields = Settings.split(" ", QString::SkipEmptyParts);
		if(fields.count() < 2)
			continue;
		QString val = fields.takeFirst();
		if(val == "jid") {
			BotJid = fields.takeLast(); }
		else if(val == "host") {
			BotHost = fields.takeLast(); }
		else if(val == "password") {
			BotPass = fields.takeLast(); }
		else if(val == "port") {
			QString port = fields.takeLast();
			BotPort = port.toInt(); }
		else if(val == "separator") {
			Separator = fields.takeLast();  }
		else if(val == "prefix" ) {
			Prefix = fields.takeLast();     }
		else if(val == "path") {
			BotFolders << fields.takeLast(); }
		else if(val == "proxyhost") {
			ProxyHost = fields.takeLast(); }
		else if(val == "proxyport") {
			QString p = fields.takeLast();
			ProxyPort = p.toInt();  }
		else if(val == "admin") {
			AdminJid = fields.takeLast(); }
		else if(val == "ext") {
			while(!fields.isEmpty()) {
				QString ext = fields.takeFirst();
				if(ext != "=")
					extentions.append(ext);
			}
		}
	}
	return true;
}

double Bot::readDate(const QString &path)
{
	double dateLong;
	QFile file(path);
	if(!file.exists()) {
		dateLong = QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toDouble();
		return dateLong;
	}
	file.open(QFile::ReadOnly);
	QTextStream in(&file);
	dateLong = in.readLine().toDouble();
	return dateLong;
}

void Bot::dateToMessage(MessageSession* session)
{
	JID userJid = session->target();
	QString id = "Data";
	id += QDir::separator() + QString::fromStdString(userJid.bare());
	double dateLong = readDate(id);
	createMessage(session, dateLong);
	QFile outmes(id);
	outmes.open(QFile::WriteOnly);
	QTextStream out(&outmes);
	out << QDateTime::currentDateTime().toString("yyyyMMddhhmmss") << endl;
}

void Bot::createMessage(MessageSession* session, double dateLong)  //Здесь мы считываем файлы
{
	QString Mes;
	Mes = QObject::trUtf8("Подождите........");
	session->send(Mes.toStdString());
	foreach (const QString& val, BotFolders) {
		int Max = 0;
		MessFiles.clear();
		Mes = "";
		filesToMessage(Mes, Max, val, dateLong);
		if(Mes != "") {
			MessFiles << Mes;
		}
		if(!MessFiles.isEmpty()) {
			foreach(QString M, MessFiles) {
				session->send(M.toStdString());
			}
		}
		else {
			QString M = val;
			QStringList L = M.split(QDir::separator());
			M = L.takeFirst();
			M = Prefix;
			while(!L.isEmpty()) {
				M +=  Separator + L.takeFirst();
			}
			session->send(QObject::trUtf8("В каталоге ").toStdString()
				      + M.toStdString() + QObject::trUtf8(" новых файлов нет.").toStdString());
		}
	}
	Mes = QObject::trUtf8("Готово!");
	session->send(Mes.toStdString());
	MessFiles.clear();
}

void Bot::error_report(std::string message)  //Вывод сообщений в консоль
{
	using namespace std;
	QString time = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
	cerr << time.toStdString() << " " << message << endl;
	QFile file("error.txt");
	if(file.size() > 10000000) {
		file.remove();
	}
	file.open(QIODevice::WriteOnly | QIODevice::Append);
	QTextStream out(&file);
	out.setGenerateByteOrderMark(false);
	out << time << " " << QString::fromStdString(message) << endl;
	return;
}

void Bot::onConnect()
{
	error_report("Connected!");
	ping = true;
}

void Bot::onDisconnect(ConnectionError e)
{
	switch (e) {
	case ConnAuthenticationFailed:
		error_report("Authentication failed. Username/password wrong or account does not exist.");
		break;

	case ConnUserDisconnected:
		error_report("The user (or higher-level protocol) requested a disconnect.");
		break;

	case ConnNotConnected:
		error_report("There is no active connection.");
		break;

	case ConnCompressionFailed:
		error_report("Negotiating/initializing compression failed.");
		break;

	case ConnTlsFailed:
		error_report("The server's certificate could not be verified or the TLS handshake did not complete successfully.");
		break;

	case ConnTlsNotAvailable:
		error_report("The server didn't offer TLS while it was set to be required or TLS was not compiled in.");
		break;

	case ConnNoSupportedAuth:
		error_report("The auth mechanisms the server offers are not supported or the server offered no auth mechanisms at all.");
		break;

	case ConnOutOfMemory:
		error_report("Out of memory.");
		break;

	case ConnDnsError:
		error_report("Resolving the server's hostname failed.");
		break;
	case ConnConnectionRefused:
		error_report("The connection was refused by the server (on the socket level).");
		break;

	case ConnParseError:
		error_report("An XML parse error occurred.");
		break;

	case ConnIoError:
		error_report("An I/O error occured.");
		break;

	case ConnProxyNoSupportedAuth:
		error_report("The HTTP/SOCKS5 proxy requires an unsupported auth mechanism.");
		break;

	case ConnProxyAuthFailed:
		error_report("HTTP/SOCKS5 proxy authentication failed.");
		break;

	case ConnProxyAuthRequired:
		error_report("The HTTP/SOCKS5 proxy requires authentication.");
		break;

	case ConnStreamClosed:
		error_report("The stream has been closed (by the server).");
		break;

	case ConnStreamVersionError:
		error_report("The incoming stream's version is not supported");
		break;

	case ConnStreamError:
		error_report("A stream error occured. The stream has been closed.");
		break;

	case ConnNoError:
		error_report("Disconnected from server without errors");
		break;

	}
	error_report("Disconnected." );
	j->disconnect();
	ping = false;
	if(ProxyHost != "") {
		tcpcl->cleanup();
		conn->cleanup();
	}

	QTime t;
	t.start();
	while(t.elapsed() < 30000) {}

	error_report("Reconnect...");
	connect();
}

void Bot::handleSubscription (Stanza *stanza)   //Аворизируем всех просящих
{
	JID userJid = stanza->from();
	loger(userJid, "Subscription request");
	RM->ackSubscriptionRequest(userJid, true);
	sendMessage(userJid, QObject::trUtf8(" Бот \"Are There New Files\" приветствует Вас!\n Чтобы узнать больше отправьте *HELP"));
	std::string ID = userJid.bare();
	QString id = "Data";
	id += QDir::separator() + QString::fromStdString(ID);
	QString CDate = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	QFile outmes(id);
	if(outmes.open(QFile::WriteOnly)) {
		QTextStream out(&outmes);
		out << CDate << endl;
	}
}

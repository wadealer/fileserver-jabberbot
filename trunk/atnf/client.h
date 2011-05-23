#ifndef CLIENT_H
#define CLIENT_H
#include <gloox/client.h>
#include <gloox/messagehandler.h>
#include <gloox/messagesessionhandler.h>
#include <gloox/connectionhttpproxy.h>
#include <gloox/connectiontcpclient.h>
#include <gloox/connectionlistener.h>
#include <gloox/rostermanager.h>
#include <gloox/disco.h>

#include <QVector>
#include <QDir>
#include <iostream>

using namespace gloox;

class Bot : public MessageSessionHandler, MessageHandler, ConnectionListener, SubscriptionHandler, LogHandler
{

public:
	Bot();
	virtual ~Bot();

private:
	void filesToMessage(QString &Mess, int &Max, const QString &path, double dateIDLong);
	double readDate(const QString &path);
	bool readSettings();

	void run();
	void sendMessage(JID jid, const QString& mes);
	void handleLog( LogLevel /*level*/, LogArea /*area*/, const std::string& message ){ error_report(message); }
	void connect();
	bool onTLSConnect(const CertInfo &info);
	void loger(JID userJid, const QString& Mes);
	void handleMessage( Stanza* stanza, MessageSession* session);
	void dateToMessage(MessageSession* session);
	void createMessage(MessageSession* session, double dateLong);
	void error_report(std::string message);
	void onConnect();
	void onDisconnect(ConnectionError e);
	void handleSubscription (Stanza *stanza);
	void handleMessageSession(MessageSession *session);

	Client * j;
	RosterManager * RM;
	JID tojid;
	ConnectionError je, ce;
	ConnectionTCPClient* tcpcl;
	ConnectionHTTPProxy* conn;
	bool ping;

	QString AdminJid;
	QString BotJid;
	QString BotHost;
	QString BotPass;
	QString ProxyHost;
	int ProxyPort;
	int BotPort;
	QVector<QString> BotFolders;
	QString Separator;
	QString Prefix;
	QStringList extentions;

	struct M_Session {
		MessageSession * session;
		bool New;
		bool All;
		int Since;
		long s_time;
		bool block;
		int N;
	};

	QVector<M_Session> Sessions;
	QVector<QString> MessFiles;
};

#endif // CLIENT_H

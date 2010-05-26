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

#ifdef Q_WS_WIN
#include <windows.h>
#endif

using namespace gloox;

class Bot : public MessageSessionHandler, MessageHandler, ConnectionListener, SubscriptionHandler, LogHandler
{

  public:
    Bot();
    bool ReadSettings();

private:
    void FilesToMessage(QString &Mess, int &Max, const QString &path, double dateIDLong);
    double ReadDate(const QString &path);

    void run();
    void SendMessage(JID jid, QString mes);
    void handleLog( LogLevel level, LogArea area, const std::string& message ){ error_report(message); }
    void Connect();
    bool onTLSConnect(const CertInfo &info);
    void Loger(JID userJid, QString Mes);
    void handleMessage( Stanza* stanza, MessageSession* session);
    void DateToMessage(MessageSession* session);
    void CreateMessage(MessageSession* session, double dateLong);
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

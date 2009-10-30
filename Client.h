#ifndef CLIENT_H
#define CLIENT_H
#include <gloox/client.h>
#include <gloox/messagehandler.h>
#include <gloox/connectionhttpproxy.h>
#include <gloox/connectiontcpclient.h>
#include <gloox/connectionlistener.h>
#include <gloox/rostermanager.h>
#include <gloox/disco.h>
#include <iostream>

#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QIODevice>
#include <QTextStream>
#include <QTextCodec>

#include <main.h>

using namespace gloox;

class Bot : public MessageHandler, ConnectionListener, SubscriptionHandler, LogHandler
{
  public:
 Bot(std::string id, std::string pass, std::string host, int port);
 bool ReadSettings();

private:
 void SendMessage(JID jid, QString mes);

QString FilesToMessage(QString &Mess, int &Max, const QString &path, double dateIDLong, JID jid);
double ReadDate(const QString &path);

/* virtual void handleDisconnect(const ConnectionBase * connection, ConnectionError e) {
     error_report("Proxy Disconnect");
 }
 virtual void handleConnect(const ConnectionBase* con) {
     error_report("Proxy Connect");
 }
virtual void handleReceivedData (const ConnectionBase *connection, const std::string &data) {
    error_report("Data!");
}
*/

 virtual void handleLog( LogLevel level, LogArea area, const std::string& message )   {
      error_report(message);
    }
 void Connect ();
 bool onTLSConnect(const CertInfo &info);
 void Loger(JID userJid, QString Mes);
 void handleMessage( Stanza* stanza, MessageSession* session);
 void DateToMessage(JID userJid);
 void CreateMessage(JID userJid, double dateLong);
 void error_report(std::string message);
 void onConnect();
 void onDisconnect(ConnectionError e);
 void handleSubscription (Stanza *stanza);

 Client * j;
   RosterManager * RM;
   ConnectionError ce;
};

Bot* b;

#endif // CLIENT_H

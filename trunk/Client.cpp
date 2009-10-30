#include <main.h>
#include <Client.h>

using namespace gloox;

Bot::Bot()    {
      ping = false;
      j = new Client(BotJid.toStdString(), BotPass.toStdString(), BotPort);
      RM = new RosterManager(j);
      j->registerSubscriptionHandler(this);
      j->registerConnectionListener(this);
      j->registerMessageSessionHandler(this, 0);
      j->logInstance().registerLogHandler(LogLevelDebug, LogAreaAllClasses, this);
      if(BotHost != "") {
          j->setServer(BotHost.toStdString()); }
      j->disco()->setVersion("ATNF", "0.1.4", "");
      j->setPresence(PresenceAvailable, 50, "Send *HELP for more information");
      if(ProxyHost != "") {
          tcpcl = new ConnectionTCPClient(j->logInstance(), ProxyHost.toStdString(), ProxyPort);
          conn = new ConnectionHTTPProxy(j, tcpcl, j->logInstance(), BotHost.toStdString(), BotPort);
          j->setConnectionImpl(conn);
      }
      Connect();
   }

void Bot::Connect() {    
    int Time;    
    QTime T;
    QString Mes;
    QString path = "lastcheck.txt";
    j->connect(false);
    je = j->recv(20000000);
    ce = ConnNoError;
    if(ProxyHost != "") {    ce = conn->recv(1000); }
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
                   double d = ReadDate(path);                                      
                   foreach (QString val, BotFolders) {
                       int Max = 0;
                       Mes = "";
                       MessFiles.clear();
                       FilesToMessage(Mes, Max, val, d);
                       if(Mes != "") {
                           MessFiles << Mes; }
                       if(!MessFiles.isEmpty()) {
                           sub.open(QIODevice::ReadOnly);
                           foreach(QString M, MessFiles) {
                               while(!out.atEnd()) {
                                   JID j(out.readLine().remove(0, 1).toStdString());
                                   SendMessage(j, M);
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
          if(ProxyHost != "") { ce = conn->recv(1000); }
      }
  } 

void Bot::handleMessageSession(MessageSession *session) {
      error_report("New Session!");
      int Date = QTime::currentTime().toString("hhmmss").toLong();
      M_Session S = { session, false, false, 0, Date, false, 0 };
      Sessions << S;
      session->registerMessageHandler(this);
  }

void Bot::handleMessage( Stanza* stanza, MessageSession* session)   { //Пытаемся разобрать, что от нас там хотят
       QString Mes;
       JID userJid = stanza->from();
       QString Body = QString::fromStdString(stanza->body());
       Loger(userJid, Body);
       QStringList fields = Body.split(" ");
       QString val = fields.takeFirst();
       val = val.toUpper();
       if(userJid == tojid) { goto E; }
       if(val == "*HELP" || val == "?") {
           QFile in("help.txt");
           in.open(QIODevice::ReadOnly);
           QTextStream n(&in);
           Mes = n.readAll();
           session->send(Mes.toStdString()); goto E; }

       if(val == "*SUB") {           
           QFile sub("sub.txt");
           QTextStream out(&sub);
           if(!sub.exists()) {
               sub.open(QIODevice::WriteOnly);
               out << "\n";
               sub.close();
           }
           sub.open(QIODevice::ReadWrite);
           while(!out.atEnd()) {               
               if(out.readLine().remove(0, 1) == QString::fromStdString(userJid.bare())) {
                   session->send("Вы уже подписаны!");
                   goto E;                }
           }
           out.seek(sub.size());
           out << QString::fromStdString(userJid.bare()) << endl;
           session->send("Подписаны!");
           goto E;
       }

       if(val == "*SINCE") {
            for(int i = Sessions.size(); i > 0;) {
               M_Session &S = Sessions[--i];
               if (S.session == session) {
                   S.block = false;
                   if(S.Since > 5) {
                       session->send("Не надоедайте Боту! :)");
                       goto E;                   }
                   else {
                       bool k = fields.isEmpty();
                       if(k)  { session->send("Неправильная дата!"); goto E;}
                       QString D = fields.takeFirst();
                       int l = D.length();
                       if(l != 8)   { session->send("Неправильная дата!"); goto E;}
                       int test = D.toInt();
                       if(test < 20080101) { session->send("Дата старее 2008 года!"); goto E; }
                       D += "000000";
                       double Dd = D.toDouble();
                       CreateMessage(session, Dd);
                       S.Since++;
                       goto E;       }
               }
           }
        }

       if(val == "*ALL") {
           for(int i = Sessions.size(); i > 0;) {
               M_Session &S = Sessions[--i];
               if (S.session == session) {
                   S.block = false;
                   if(S.All) {
                       session->send("Сегодня от Вас уже был такой запрос. Хватит! ;)");
                       goto E; }
                   else {
                       QString D = "19500101000000";
                       double d = D.toDouble();
                       CreateMessage(session, d);
                       S.All = true;
                       goto E; }
               }
           }
       }

       if(val == "*SEND") {
           fields = Body.split(":");
           Mes = fields.takeLast();
           bool k = fields.isEmpty();
           if(k)  { session->send("Что-то не так!"); goto E;}
           val = fields.takeFirst();
           fields = val.split(" ");
           val = fields.takeLast();
           tojid = val.toStdString();
           SendMessage(tojid, Mes);
          // SendMessage(userJid, "OK");
           goto E;      }

       if(val == "*NEW") {
            for(int i = Sessions.size(); i > 0;) {
               M_Session &S = Sessions[--i];
               if (S.session == session) {
                   S.block = false;
                   if(S.New) {
                       session->send("Сегодня от Вас уже был такой запрос. Хватит! ;)");
                       goto E; }
                   else {
                       DateToMessage(session);
                       S.New = true;
                       goto E;  }
               }
           }
        }

       if(val == "*QUIT") {
           j->disconnect();
           goto E; }

       if(val == ";-)" || val == ";)" || val == ":)" || val == ":-)" || val == ":-(" || val == "8-)" || val == ":-D" || val == ":-P" )
       { session->send(" :-P ");
           goto E; }

       if(val == "*MULTYSEND") {
           while(!fields.isEmpty()) {
               Mes += fields.takeFirst() + " "; }
           QDir dir("Data");
           foreach(QString J, dir.entryList(QDir::Files)) {
               JID Jid(J.toStdString());
               SendMessage(Jid, Mes); }
           goto E;       }

       for(int i = Sessions.size(); i > 0;) {
               M_Session &S = Sessions[--i];
               if (S.session == session) {
                   if (S.block) { goto E;    }
                    else {
                           session->send("Это не команда! Пошлите *HELP чтобы узнать больше!");
                           if(!S.N) {
                               S.s_time = QTime::currentTime().toString("hhmmss").toLong();
                               S.N++;
                               goto E;              }
                           else {
                               S.N++;
                               int T = QTime::currentTime().toString("hhmmss").toLong()  - S.s_time;
                               if(S.N == 5 && T < 5) {
                                   S.block = true;                                   
                                   goto E;            }
                               else {
                                   if(T > 5) {
                                       S.N = 0;      }
                               }
                           }
                       }                    
                    goto E;
                }
           }        

       E:  {}
    }

void Bot::SendMessage(JID jid, QString mes) { //Отсылка сообщений
       std::string Mes = mes.toStdString();
       Stanza *s = Stanza::createMessageStanza(jid.full(), Mes);
       j->send(s);  }

bool Bot::onTLSConnect(const CertInfo &info) {
        return true; }

void Bot::Loger(JID userJid, QString Mes) {       
       QFile file("log.txt");
       file.open(QIODevice::ReadWrite);
       QTextStream out(&file);
       out.seek(file.size());              
       QString nw = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") + " " + QString::fromStdString(userJid.full()) + ": " + Mes;
       out << nw << endl;
   }

void Bot::FilesToMessage(QString &Mess, int &Max, const QString &path, double dateIDLong) {
    QDir dir(path);
    QString M;
    QStringList L;
    foreach (QString file, dir.entryList(QDir::Files)) {
     double DateTimeLong = QFileInfo(dir, file).created().toString("yyyyMMddhhmmss").toDouble();
     if(DateTimeLong > dateIDLong) {
         M = path + QDir::separator() + file;
         L = M.split(QDir::separator());
         M = L.takeFirst();
         M = Prefix;
         while(!L.isEmpty()) {
             M +=  Separator + L.takeFirst();  }
         Mess += M + "\n";
         Max++;
         if(Max > 200) {
            MessFiles << Mess;
            Max = 0;
            Mess = ""; }
     }
 }
    foreach (QString subDir, dir.entryList(QDir::Dirs
                                           | QDir::NoDotAndDotDot)) {
        FilesToMessage(Mess, Max, path + QDir::separator() + subDir, dateIDLong);
    } 
    }

bool Bot::ReadSettings() {
    using namespace std;
    QFile file("settings.ini");
    if(!file.exists()) {
        cerr << "Cannot open settings file" << endl;
    return 0; }
    file.open(QFile::ReadOnly);
    QTextStream inFile(&file);
    while (!inFile.atEnd()){
         QString Settings = inFile.readLine();
         QStringList fields = Settings.split(" ");
         QString val = fields.takeFirst();
         if(val == "jid") {
              BotJid = fields.takeLast(); }
         if(val == "host") {
             BotHost = fields.takeLast(); }
         if(val == "password") {
             BotPass = fields.takeLast(); }
         if(val == "port") {
             QString port = fields.takeLast();
             BotPort = port.toInt(); }
         if(val == "path") {
             BotFolders << fields.takeLast();
         }
         if(val == "proxyhost") {
             ProxyHost = fields.takeLast(); }
         if(val == "proxyport") {
             QString p = fields.takeLast();
             ProxyPort = p.toInt();
         }
     }
    return 1;
}

double Bot::ReadDate(const QString &path) {
    double dateLong;
    QFile file(path);    
    if(!file.exists()) {
        dateLong = QDateTime::currentDateTime().toString("yyyyMMddhhmmss").toDouble();
        return dateLong; }
    file.open(QFile::ReadOnly);
    QTextStream in(&file);
    dateLong = in.readLine().toDouble();
    return dateLong;
}

void Bot::DateToMessage(MessageSession* session)
   {
       JID userJid = session->target();
       QString id = "Data";
       id += QDir::separator() + QString::fromStdString(userJid.bare());
       double dateLong = ReadDate(id);
       CreateMessage(session, dateLong);
       QFile outmes(id);
       outmes.open(QFile::WriteOnly);
       QTextStream out(&outmes);
       out << QDateTime::currentDateTime().toString("yyyyMMddhhmmss") << endl;
   }

void Bot::CreateMessage(MessageSession* session, double dateLong) {  //Здесь мы считываем файлы
       QString Mes;
       Mes = "Подождите........";
       session->send(Mes.toStdString());
       foreach (QString val, BotFolders) {
           int Max = 0;
           MessFiles.clear();
           Mes = "";
           FilesToMessage(Mes, Max, val, dateLong);
           if(Mes != "") {
               MessFiles << Mes; }
           if(!MessFiles.isEmpty()) {
               foreach(QString M, MessFiles) {
               session->send(M.toStdString());            }
           }
           else { session->send("В каталоге " + val.toStdString() + " новых файлов нет.");    }
       }
       Mes = "Готово!";
       session->send(Mes.toStdString());
       MessFiles.clear();
    }

void Bot::error_report(std::string message) { //Вывод сообщений в консоль
       using namespace std;
       QString time = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
       cerr << time.toStdString() << " " << message << endl;
       QFile file("error.txt");
       file.open(QIODevice::ReadWrite);
       QTextStream out(&file);
       if(file.size() < 10000000) {
       out.seek(file.size());
       }
       QString nw = QString::fromStdString(message);
       out << time << " " << nw << endl;
       return; }

void Bot::onConnect() {
    error_report("Connected!");
    ping = true;
}

void Bot::onDisconnect(ConnectionError e) {
       switch (e) {
                case ConnAuthenticationFailed:
                        error_report("Authentication failed. Username/password wrong or account does not exist.");
                        break;

                case ConnNotConnected:
                        error_report("There is no active connection.");
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

                case ConnProxyNoSupportedAuth:
                        error_report("The HTTP/SOCKS5 proxy requires an unsupported auth mechanism.");
                        break;

                case ConnProxyAuthFailed:
                        error_report("HTTP/SOCKS5 proxy authentication failed.");
                        break;

                case ConnProxyAuthRequired:
                        error_report("The HTTP/SOCKS5 proxy requires authentication.");
                        break;

                case ConnStreamVersionError:
                        error_report("The incoming stream's version is not supported");
                        break;

                case ConnStreamError:
                        error_report("A stream error occured. The stream has been closed.");
                        break;

                 }
                        error_report("Disconnected." );
                        ping = false;
                        if(ProxyHost != "") {                            
                             tcpcl->cleanup();
                             conn->cleanup();
                        }
#ifdef Q_WS_WIN
                        Sleep(30000);
#else
                        sleep (3);
#endif
                        error_report("Reconnect...");
                        Connect();
       }

void Bot::handleSubscription (Stanza *stanza) { //Аворизируем всех просящих
     JID userJid = stanza->from();
     Loger(userJid, "Subscription request");
     RM->ackSubscriptionRequest(userJid, true);
     SendMessage(userJid, " Бот \"Are There New Files\" приветствует Вас!\n Чтобы узнать больше отправьте *HELP");
     std::string ID = userJid.bare();
     QString id = "Data";
     id += QDir::separator() + QString::fromStdString(ID);    
     QString CDate = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
     QFile outmes(id);
     outmes.open(QFile::WriteOnly);
     QTextStream out(&outmes);
     out << CDate << endl;
 }

int main( int argc, char* argv[] ) {   
    codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QDir Data("Data");
    if(!Data.exists()) {
        QDir P = QDir::current();
        bool r = P.mkdir("Data");
        if(!r) {
             std::cerr << "Cannot create Data folder" << endl;
             return 0;}
    }
    bool Res;
    Res = b->ReadSettings();
    if(!Res) { return Res; }
    b = new Bot();
    return Res;
}


#include <main.h>
#include <Client.h>

using namespace gloox;



QString Bot::FilesToMessage(QString &Mess, int &Max, const QString &path, double dateIDLong, JID jid) {
    QDir dir(path);
    foreach (QString file, dir.entryList(QDir::Files)) {
     QDateTime DateTime = QFileInfo(dir, file).created();
     QString DateTimeStr = DateTime.toString("yyyyMMddhhmmss");
     double DateTimeLong = DateTimeStr.toDouble();
     if(DateTimeLong > dateIDLong) {
         QString dirpath = dir.absolutePath();
         Mess += dirpath + QDir::separator() + file + "\n";
         Max++;
     }
 }

    foreach (QString subDir, dir.entryList(QDir::Dirs
                                           | QDir::NoDotAndDotDot)) {
        FilesToMessage(Mess, Max, path + QDir::separator() + subDir, dateIDLong, jid);
         if(Max > 200) {
            SendMessage(jid, Mess);
            Max = 0;
            Mess = "";}
        }

        return Mess;
     }

bool Bot::ReadSettings() {
    using namespace std;
    int i = 0;
    QFile file("settings.ini");
    if(!file.exists()) {
        cerr << "Cannot open settings file" << endl;
    return 0; }
    file.open(QFile::ReadOnly);
    QTextStream inFile(&file);
   // inFile.setCodec(codec);
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
             bool *ok;
             BotPort = port.toInt(ok, 10); }
         if(val == "path") {
             BotFolders[i++] = fields.takeLast();
         }
     }
    NumOfFolders = i;
    return 1;
}

double Bot::ReadDate(const QString &path) {
    double dateLong;
    QFile file(path);
    QString DateTime;
    if(!file.exists()) {
        QDateTime d = QDateTime::currentDateTime();
        DateTime = d.toString("yyyyMMddhhmmss");
        dateLong = DateTime.toDouble();
        return dateLong; }
    file.open(QFile::ReadOnly);
    QTextStream in(&file);
   // in.setCodec("UTF::string-8");
    DateTime = in.readLine();
    dateLong = DateTime.toDouble();
    return dateLong;
}



  Bot::Bot(std::string id, std::string pass, std::string host, int port)   {
      JID jid(id);
      j = new Client(jid, pass, port);
      RM = new RosterManager(j);
      j->registerSubscriptionHandler(this);
      j->registerConnectionListener(this);
      j->registerMessageHandler(this);
      j->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, this);
      if(host != "") {
          j->setServer(host); }
      j->disco()->setVersion("ATNF", "0.0.2", "");
      j->setPresence(PresenceAvailable, 1, "Send *HELP for more information");

      const std::string proxy = "2.1.1.252";
      ConnectionTCPClient* tcpcl =  new ConnectionTCPClient(j->logInstance(), proxy, 3128);
      ConnectionHTTPProxy* conn =  new ConnectionHTTPProxy(j, tcpcl, j->logInstance(), host, port);         
      j->setConnectionImpl(conn);        
      Connect();

}

void Bot::Connect() {
      j->connect(false);
      ce = j->recv();
      while(ce == ConnNoError)  {
          ce = j->recv();
      }
     }

 void  Bot::handleMessage( Stanza* stanza, MessageSession* session = 0 )   { //Пытаемся разобрать, что от нас там хотят
       QString Mes;
       JID userJid = stanza->from();
       QString Body = QString::fromStdString(stanza->body());
       Loger(userJid, Body);
       QStringList fields = Body.split(" ");
       QString val = fields.takeFirst();
       if(val == "*HELP") {
           QFile in("help.txt");
           in.open(QIODevice::ReadOnly);
           QTextStream n(&in);
           QString Mes = n.readAll();
           //Mes = "Список команд:\n *HELP - выводит эту помощь :)\n *SINCE yyyyMMdd - показывает все новые файлы, начиная с даты yyyyMMdd. yyyy - год(2009), MM - месяц(02), dd - день(22)\n Пример: *SINCE 20090701 - покажет все новые файлы начиная с 01 июля 2009.\n Любое другое сообщение покажет все новые файлы со времени последного сообщения(или со времени авторизации).";
           SendMessage(userJid, Mes); goto E; }

       if(val == "*SINCE") {
           bool k = fields.isEmpty();
           if(k)  { SendMessage(userJid, "Неправильная дата!"); goto E;}
           QString D = fields.takeFirst();
           int l = D.length();
           if(l != 8)   { SendMessage(userJid, "Неправильная дата!"); goto E;}
           int test = D.toInt();
           if(test < 20080101) { SendMessage(userJid,"Дата старее 2008 года!"); goto E; }
           D += "000000";
           double Dd = D.toDouble();
           CreateMessage(userJid, Dd);
           goto E; }

       if(val == "*ALL") {
           QString D = "19500101000000";
           double d = D.toDouble();
           CreateMessage(userJid, d);
           goto E; }

       DateToMessage(userJid);

E:  {}
    }

void  Bot::SendMessage(JID jid, QString mes) { //Отсылка сообщений
       std::string Mes = mes.toStdString();
       Stanza *s = Stanza::createMessageStanza(jid.full(), Mes);
       j->send(s);  }

 bool  Bot::onTLSConnect(const CertInfo &info) {
        return true; }

 void  Bot::Loger(JID userJid, QString Mes) {
       std::string Jid = userJid.full();
       QFile file("log.txt");
       file.open(QIODevice::ReadWrite);
       QTextStream out(&file);
       QString old = file.readAll();
       QString jid = QString::fromStdString(Jid);
       QString date = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
       QString nw = date + " " + jid + ": " + Mes + "\n";
       out << nw;
   }

 void  Bot::DateToMessage(JID userJid)
   {
       std::string ID = userJid.bare();                                           //Дефолтный вариант - файлы с последнего
       QString userFolder = "Data";                                                //сообщения
       QString id = userFolder + QDir::separator() + QString::fromStdString(ID);
       double dateLong = ReadDate(id);
       CreateMessage(userJid, dateLong);
       QDateTime CurDate = QDateTime::currentDateTime();                           //Пишем текущую дату
       QString CDate = CurDate.toString("yyyyMMddhhmmss");
       QFile outmes(id);
       outmes.open(QFile::WriteOnly);
       QTextStream out(&outmes);
       out << CDate << endl;

   }

void   Bot::CreateMessage(JID userJid, double dateLong) {  //Здесь мы считываем файлы
       QString Mes;
       int Max = 0;
       Mes = "Подождите........";
       SendMessage(userJid, Mes);
       Mes = "";
       int i = NumOfFolders;
       while ( i-- ) {
           Mes = "\n" + FilesToMessage(Mes, Max, BotFolders[i], dateLong, userJid);
           if(Mes != "\n") {
               SendMessage(userJid, Mes);   }
       }
        Mes = "Готово!";
        SendMessage(userJid, Mes);
    }

 void  Bot::error_report(std::string message) { //Вывод сообщений в консоль
       using namespace std;
       cerr << message << endl;
       return; }

void   Bot::onConnect() {error_report("Connected!");   }

 void  Bot::onDisconnect(ConnectionError e) {
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

                        error_report("Disconnected. Reconnect.....");
                        Connect();
       }

void  Bot::handleSubscription (Stanza *stanza) { //Аворизируем всех просящих
     JID userJid = stanza->from();
     Loger(userJid, "Subscription request");
     RM->ackSubscriptionRequest(userJid, true);
     QString Mes = " Бот 'Are There New Files' приветствует Вас!\n Чтобы узнать больше отправьте *HELP";
     SendMessage(userJid,Mes);

     std::string ID = userJid.bare();
     QString userFolder = "Data";
     QString id = userFolder + QDir::separator() + QString::fromStdString(ID);
     QDateTime CurDate = QDateTime::currentDateTime();                           //Пишем текущую дату
     QString CDate = CurDate.toString("yyyyMMddhhmmss");
     QFile outmes(id);
     outmes.open(QFile::WriteOnly);
     QTextStream out(&outmes);
     out << CDate << endl;
 }


int main( int argc, char* argv[] ) {
   // QCoreApplication(argc, argv);
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
    Bot* b = new Bot(BotJid.toStdString(), BotPass.toStdString(), BotHost.toStdString(), BotPort);
    return Res;
}


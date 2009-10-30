#ifndef MAIN_H
#define MAIN_H
#include <QVector>
#include <QCoreApplication>

QString BotJid = "a@b/c";
QString BotHost = "";
QString BotPass = "";
QString ProxyHost = "";
int ProxyPort = 3128;
int BotPort = 5222;
QVector<QString> BotFolders;
QTextCodec *codec;
QString Separator = "/";                    //Помеять тут, чтобы получить
QString Prefix = "ftp://2.1.1.68";              //необходимые пути

#endif // MAIN_H

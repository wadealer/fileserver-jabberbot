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
QString Separator = "/";
QString Prefix = "Server";

#endif // MAIN_H

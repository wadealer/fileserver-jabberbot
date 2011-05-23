#include <QCoreApplication>
#include <QTextCodec>

#include "Client.h"

int main( int argc, char* argv[] ) {
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QDir Data("Data");
    if(!Data.exists()) {
        QDir P = QDir::current();
        bool r = P.mkdir("Data");
        if(!r) {
             std::cerr << "Cannot create Data folder\n";
             return 0;
         }
    }
    Bot *b = new Bot();
    delete (b);
    return 1;
}


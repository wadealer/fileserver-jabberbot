#include <QCoreApplication>
#include <QTextCodec>
#include <QDir>

#include "client.h"

int main( int argc, char* argv[] ) {
	QCoreApplication app(argc, argv);
	QTextCodec *codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForCStrings(codec);
	QDir Data("Data");
	if(!Data.exists()) {
		if(!QDir::current().mkdir("Data")) {
			std::cerr << "Cannot create Data folder\n";
			return 0;
		}
	}
	int ret = 0;
	Bot b;
	if(!b.start()) {
		ret = app.exec();
	}
	return ret;
}

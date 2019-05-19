#include "LaserTrackQt.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	LaserTrackQt w;
	w.show();
	return a.exec();
}

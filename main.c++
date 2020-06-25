/*
 * main.c++
 *
 * William Miller
 * Jul 24, 2020
 *
 * Main routine for PicoView minimal image viewer
 */

#include <QApplication>

#include "picoview.h"

int main(int argn, char** argv) {
	QApplication a (argn, argv);
	PicoView w;
	
	fs::path path(".");
	if (argn > 1) {
		path = fs::path(argv[1]);
		w.open(path);
	}

	w.setWindowTitle("PicoView");
	w.show();
	return a.exec();
}

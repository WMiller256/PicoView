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
	w.setWindowTitle("PicoView");
	w.showMaximized();
	w.resize(800, 600);
	return a.exec();
}

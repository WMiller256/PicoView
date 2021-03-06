/*
 * main.c++
 *
 * William Miller
 * Jul 24, 2020
 *
 * Main routine for PicoView minimal image viewer
 */

#include <QApplication>
#include <QTimer>

#include "picoview.h"

int main(int argn, char** argv) {
	QApplication a (argn, argv);
	a.setStyle("Fusion");

	// Create and set a dark theme
	QPalette palette = QPalette();
	palette.setColor(QPalette::Window, QColor(53, 53, 53));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Base, QColor(25, 25, 25));
	palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, Qt::white);
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Button, QColor(53, 53, 53));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::red);
	palette.setColor(QPalette::Link, QColor(42, 130, 218));
	palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
	palette.setColor(QPalette::HighlightedText, Qt::black);

	// Set button disabled color
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::gray);
	
	a.setPalette(palette);

	PicoView w(palette);
	
	fs::path path(".");
	if (argn > 1) {
		path = fs::path(argv[1]);
	}
	if (fs::exists(path)) w.open(path);
	else w.open(fs::path("."));

	w.setWindowTitle("PicoView");
	w.show();

	// Force expansion of {img_container}
	w.resize(w.size() + QSize(1, 1));

	return a.exec();
}

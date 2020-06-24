/*
 * picoview.c++
 *
 * William Miller
 * Jun 24, 2020
 *
 * Functions for use in PicoView minimal image viewer
 *
 */ 

#include "picoview.h"

PicoView::PicoView(QWidget* parent) : QMainWindow(parent) {
	// constructor
}

void PicoView::createLayout() { 
	layout = new QVboxLayout();
	QDir directory(selected_directory);
	QStringList files = director.entryList();
}

std::vector<fs::path> getFileList() {

}

/*
 * picoview.h
 *
 * William Miller
 * Jun 24, 2020
 *
 * Class declaration for PicoView, the base class
 * for PicoView minimal image viewer
 *
 */

#pragma once

// std
#include <string>
#include <vector>

// Qt
#include <QtWidgets/QMainWindow>
#include <QDir>
#include <QDebug>
#include <QVboxLayout>

class PicoView : public QMainWindow {
	Q_OBJECT

public:
	PicoView(QWidget* parent = Q_NULLPTR);

private:
	QVboxLayout* layout;
	std::string selected_directory;
};

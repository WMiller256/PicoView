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
#include <algorithm>
#include <experimental/filesystem>
#include <string>
#include <vector>

// Qt
#include <QColor>
#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QtWidgets/QMainWindow>
#include <QMovie>
#include <QPalette>
#include <QPushButton>
#include <QRect>
#include <QWidget>
#include <QVBoxLayout>

namespace fs = std::experimental::filesystem;

const std::vector<std::string> supported = {".gif", ".tif", ".jpg", ".png"};

template <typename T>
bool contains(const std::vector<T> &v, const T &e) { return std::find(v.begin(), v.end(), e) != v.end(); }

std::string tolower(const std::string &s);

class PicoView : public QMainWindow {
	Q_OBJECT

public:
	PicoView(QWidget* parent = Q_NULLPTR);

	void getFileList();
	void buildLayout();
	void buildControls();

	void current(const unsigned int &i);

public slots:
	void next();
	void prev();
	void delt();

private:
	std::string selected_directory;
	std::vector<fs::path> files;
	unsigned int cidx = 0;

	QWidget* w;

	QHBoxLayout* layout;
	QVBoxLayout* img_canvas;

	QMovie* img;
	QLabel* img_container;
	QRect img_size;

	QHBoxLayout* controls_layout;
	std::map<std::string, QPushButton*> controls;
	std::vector<std::string> _controls = {"Previous", "Delete", "Next"};
};

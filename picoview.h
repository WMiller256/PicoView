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
#include <iostream>
#include <string>
#include <vector>

// Qt
#include <QColor>
#include <QComboBox>
#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QtWidgets/QMainWindow>
#include <QMovie>
#include <QMenu>
#include <QMenuBar>
#include <QPalette>
#include <QPushButton>
#include <QRect>
#include <QShortcut>
#include <QSignalMapper>
#include <QSizePolicy>
#include <QWidget>
#include <QVBoxLayout>

namespace fs = std::experimental::filesystem;

const std::vector<std::string> supported = {".gif", ".tif", ".jpg", ".png", ".jpeg"};

enum SortMode { name, modified, type };

// Forward declarations
class PicoWidget;

class PicoView : public QMainWindow {
	Q_OBJECT

public:
	PicoView(QWidget* parent = Q_NULLPTR);

	void resizeEvent(QResizeEvent* e);

	void open(const fs::path &p);

	void getFileList();
	void buildLayout();
	void buildMenu();
	void buildControls();

	void current(const int &i);

	bool isMovie(fs::path f);

	void open_file(fs::path _file, bool checking = true);
	void open_dir(fs::path _dir, bool checking = true);

public slots:
	void open_file();
	void open_dir();

	void sortby(QString s);

	void firs();
	void prev();
	void delt();
	void next();
	void last();

private:
	fs::path path;
	std::vector<fs::path> files;
	unsigned int cidx = 0;

	QString sorting = "Modified";
	std::string filter = "(";

	PicoWidget* w;

	QHBoxLayout* layout;
	QVBoxLayout* img_canvas;
	QMenuBar* menu;

	QLabel* img_label;
	QMovie* mov;
	QImage img;
	QRect img_size;
	QSize label_size;

	QLabel* info;
	QLabel* dimensions;	

	QPushButton* _next;
	QPushButton* _delt;
	QPushButton* _prev;

	QVBoxLayout* controls_layout;
	std::map<std::string, QPushButton*> controls;
	std::vector<std::string> _controls = {"<<", "Previous", "Delete", "Next", ">>"};
	std::vector<void (PicoView::*)()> _controls_slots = {&PicoView::firs, &PicoView::prev, &PicoView::delt, &PicoView::next, &PicoView::last};

	QMenu* file;
	std::vector<std::string> _file_actions = {"Open File...", "Open Directory..."};
	std::vector<void (PicoView::*)()> _file_slots = {&PicoView::open_file, &PicoView::open_dir};

	QMenu* sort;
	std::map<std::string, SortMode> _sort_options = {{"Name", name},
													 {"Modified", modified}, 
													 {"Type", type}};
};

class PicoWidget : public QWidget {
	Q_OBJECT

public: 
	PicoWidget( PicoView* _window, QWidget* parent = Q_NULLPTR) : QWidget(parent), window(_window) {}

	void resizeEvent(QResizeEvent* e);

private:
	PicoView* window;
};

// Checks if vector {v} contains element {e}
template <typename T>
bool contains(const std::vector<T> &v, const T &e) { return std::find(v.begin(), v.end(), e) != v.end(); }

// Non-destrucitvely converts string {s} to lower case
std::string tolower(const std::string &s);

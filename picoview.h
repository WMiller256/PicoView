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
#include <QApplication>
#include <QColor>
#include <QComboBox>
#include <QDesktopWidget>
#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QtWidgets/QMainWindow>
#include <QMediaPlayer>
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
#include <QVideoWidget>

#include "colors.h"

namespace fs = std::experimental::filesystem;

extern std::vector<std::string> supported;

enum SortMode { name, modified, type };

// Forward declarations
class PicoWidget;

class PicoView : public QMainWindow {
	Q_OBJECT

public:
	PicoView(QPalette palette, QWidget* parent = Q_NULLPTR);

	void resizeEvent(QResizeEvent* e);

	void open(const fs::path &p);

	void getFileList(bool sort = true);
	void buildLayout();
	void buildMenu();
	void buildControls();

	void current(const int &i);

	bool isMovie(fs::path f);

	void open_file(fs::path _file, bool checking = true);
	void open_dir(fs::path _dir, size_t idx = 0, bool checking = true);

public slots:
	void open_file();
	void open_dir();

	void sortby(QString s);
	void refresh();
	void fullscreen();

	void movieLooper(int f);    // Native looping of WebP animations ocassionally fails with Qt 5.9.5, have to handle manually.

	void firs();
	void prev();
	void delt();
	void next();
	void last();

private:
	fs::path path;
	std::vector<fs::path> files;
	int cidx = -1;

	QString sorting = "Modified";
	std::string filter = "(";

	PicoWidget* w;
	QPalette palette;

	QHBoxLayout* layout;
	QVBoxLayout* img_canvas;
	QHBoxLayout* tbar_layout;
	QMenuBar* menu;

    QRect norm_geometry;
    bool is_fullscreen = false;

	QPushButton* _refr;
	QPushButton* _fullscreen;
	QLabel* img_label;
	QMovie* mov;
	QImage img;
	QVideoWidget* vid;
	QMediaPlayer* player;
	QRect img_size;
	QSize label_size;
	int nframes;

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

// Automatically elide label text
void setLabelText(QLabel* label, QString text);

void error(const std::string mess, const int line, const char* file);

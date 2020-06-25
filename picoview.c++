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
	w = new QWidget(this);
	this->setCentralWidget(w);
	
	selected_directory = QFileDialog::getExistingDirectory(this, tr("Directory"), ".").toStdString();
	img_container = new QLabel();
	img_container->setAlignment(Qt::AlignCenter);
	img = new QMovie();

	getFileList();
	buildLayout();
}

void PicoView::getFileList() {
	std::string ext;
	fs::path p;
	for (const auto &e : fs::directory_iterator(selected_directory)) {
		p = e.path();
		ext = tolower(p.extension().string());
		if (contains<std::string>(supported, ext)) {
			p.replace_extension(ext);
			files.push_back(p);
		}
	}
}

void PicoView::buildLayout() { 
	layout = new QHBoxLayout();
	img_canvas = new QVBoxLayout();

	buildControls();

	if (files.size() > 0) {
		current(0);
	}
	img_container->show();
	img_canvas->addWidget(img_container, Qt::AlignCenter);
	img_canvas->addLayout(controls_layout);
	layout->addLayout(img_canvas);
	w->setLayout(layout);
}

void PicoView::buildControls() {
	controls_layout = new QHBoxLayout();
	QPalette pal;
	for (const auto &e : _controls) {
		QPushButton* button = new QPushButton(e.c_str());
		controls.insert({e, button});
		controls_layout->addWidget(button);
	}
	QObject::connect(controls.find("Next")->second, &QPushButton::clicked, this, &PicoView::next);
	QObject::connect(controls.find("Delete")->second, &QPushButton::clicked, this, &PicoView::delt);
	QObject::connect(controls.find("Previous")->second, &QPushButton::clicked, this, &PicoView::prev);

	// Make the delete button red in color
	pal = controls.find("Delete")->second->palette();
	pal.setColor(QPalette::ButtonText, QColor(Qt::red));
	controls.find("Delete")->second->setPalette(pal);
}

void PicoView::next() {
	if (cidx < files.size() - 1) {
		current(++cidx);
	}
}
void PicoView::prev() {
	if (cidx > 0) {
		current(--cidx);
	}
}
void PicoView::delt() {
	
}

void PicoView::current(const unsigned int &i) {
	if (i < files.size()) {
		delete img;
		img = new QMovie(QString::fromStdString(files[i].string()));
		img_container->setMovie(img);
		img_size = img->frameRect();
		img->start();
	}
}

std::string tolower(const std::string &s) {
	std::string ls(s);
	std::transform(ls.begin(), ls.end(), ls.begin(), [](unsigned char c) { return std::tolower(c); }); 
	return ls;
}


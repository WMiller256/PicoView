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
	for (const auto &s : supported) {
		if (s != supported.back()) filter += "*"+s+" ";
		else filter += 	"*"+s+")";	
	}

	w = new QWidget(this);
	this->setCentralWidget(w);
	
	img_container = new QLabel();
	img_container->setAlignment(Qt::AlignCenter);
//	img_container->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	img = new QMovie();
	info = new QLabel();
	info->setAlignment(Qt::AlignCenter);

	buildLayout();
}

void PicoView::resizeEvent(QResizeEvent* event) {
	QMainWindow::resizeEvent(event);
	w->setMaximumSize(this->size());
	container_size = img_container->size();
}

void PicoView::open(const fs::path &p) {
	path = p.parent_path();
	getFileList();
	current(0);
}

void PicoView::getFileList() {
	std::string ext;
	fs::path p;
	for (const auto &e : fs::directory_iterator(path)) {
		p = e.path();
		ext = tolower(p.extension().string());
		if (contains<std::string>(supported, ext)) {
			p.replace_extension(ext);
			files.push_back(p);
		}
	}
	sortby(sorting);
}

void PicoView::buildLayout() { 
	layout = new QHBoxLayout;
	img_canvas = new QVBoxLayout;
	menu = new QMenuBar;

	buildMenu();
	buildControls();

	menu->addMenu(file);

	img_container->show();
	img_canvas->addWidget(menu);
	img_canvas->addWidget(img_container, Qt::AlignCenter);
	img_canvas->addLayout(controls_layout);
	
	layout->addLayout(img_canvas);
	w->setLayout(layout);
}
void PicoView::buildMenu() {
	file = new QMenu("&File", w);
	file->show();
	int idx = 0;
	for (const auto &e : _file_actions) {
		QAction* act = new QAction(e.c_str(), this);
		QObject::connect(act, &QAction::triggered, this, _file_slots[idx++]);
		file->addAction(act);
	}
}
void PicoView::buildControls() {
	// Layout for buttons
	QHBoxLayout* _layout = new QHBoxLayout();

	// Create keyboard shortcuts for buttons
	std::vector<Qt::Key> keys = {Qt::Key_Home, Qt::Key_Left, Qt::Key_Delete, Qt::Key_Right, Qt::Key_End};
	std::vector<QShortcut*> shortcuts;
	for (auto key : keys) shortcuts.push_back(new QShortcut(QKeySequence(key), this));
	
	int idx = 0;
	for (const auto &e : _controls) {
		QPushButton* button = new QPushButton(e.c_str());
		// Connect keyboard shortcut
		QObject::connect(shortcuts[idx], &QShortcut::activated, this, _controls_slots[idx]);
		// Connect button press to corresponding slot
		QObject::connect(button, &QPushButton::clicked, this, _controls_slots[idx++]);
		controls.insert({e, button});
		_layout->addWidget(button);
	}

	// Make the delete button red in color
	QPalette pal;
	pal = controls.find("Delete")->second->palette();
	pal.setColor(QPalette::ButtonText, QColor(Qt::red));
	controls.find("Delete")->second->setPalette(pal);

	// Add the info and controls to controls_layout
	controls_layout = new QVBoxLayout();
	controls_layout->addWidget(info);
	controls_layout->addLayout(_layout);

	// Adjust size hints for first and last buttons
	controls.find("<<")->second->setMaximumWidth(30);
	controls.find(">>")->second->setMaximumWidth(30);
}

// Slots
void PicoView::open_file() {

	std::string _file = QFileDialog::getOpenFileName(this, tr("Open Image"), path.string().c_str(), 
		tr(("Image Files "+filter).c_str())).toStdString();
	if (_file == "") return;
	fs::path file(_file);

	if (file.parent_path() != path) {
		path = file.parent_path();
		getFileList();
	}

	auto found = std::find(files.begin(), files.end(), file);
	if (found == files.end()) {
		info->setText(QString::fromStdString("Error opening "+file.filename().string()+"."));
		cidx = 0;
	}
	else cidx = std::distance(files.begin(), found); 
	current(cidx);
}
void PicoView::open_dir() {

	std::string _dir = QFileDialog::getExistingDirectory(this, tr("Directory"), path.string().c_str()).toStdString();
	if (_dir == "") return; 
	fs::path new_path(_dir);
	
	if (new_path != path) {
		path = new_path;
		getFileList();
		current(0);
	}
}

void PicoView::firs() {
	cidx = 0;
	current(cidx);
}
void PicoView::prev() {
	if (cidx > 0) current(--cidx);
}
void PicoView::delt() {
	bool success = fs::remove(files[cidx]);
	if (success) {
		info->setText(QString::fromStdString("Removed "+files[cidx].filename().string()+"."));
		files.erase(files.begin() + cidx);
		current(cidx);
	}
	else {
		info->setText(QString::fromStdString("Failed to remove "+files[cidx].filename().string()+"."));
	}
}
void PicoView::next() {
	if (cidx < files.size() - 1) current(++cidx);
}
void PicoView::last() {
	cidx = files.size() - 1;
	current(cidx);
}

void PicoView::current(const int &i) {
	if (i >= 0 && (unsigned int)i < files.size()) {
		delete img;
		img = new QMovie(QString::fromStdString(files[i].string()));

		// Have to start the movie before calling [->frameRect()]
		img_container->setMovie(img);
		img->start();
		img_size = img->frameRect();
		
		// If current size of window does not accomodate image of new size, scale down accordingly		
		if (img_size.height() > container_size.height() || img_size.width() > container_size.width()) {
			img->setScaledSize(container_size);
		}
		info->setText(QString::fromStdString(files[i].filename().string()));
	}

	_prev = controls.find("Previous")->second;
	_next = controls.find("Next")->second;

	// Disable next and previous buttons when appropriate
	if (i == 0) {
		_prev->setEnabled(false);
		controls.find("<<")->second->setEnabled(false);
	}
	if ((unsigned int)i == files.size() - 1) {
		_next->setEnabled(false);
		controls.find(">>")->second->setEnabled(false);
	}

	// Re-enable next and previous buttons as needed
	if (!_prev->isEnabled() && i != 0) {
		_prev->setEnabled(true);
		controls.find("<<")->second->setEnabled(true);
	}
	if (!_next->isEnabled() && (unsigned int)i != files.size() - 1) {
		_next->setEnabled(true);
		controls.find(">>")->second->setEnabled(true);
	}
}

void PicoView::sortby(SortMode m) {
	switch(m) {
		case SortMode::modified:
			std::sort(files.begin(), files.end(), [](auto &l, auto &r) { return fs::last_write_time(l) < fs::last_write_time(r); });
		break;
	}
}

std::string tolower(const std::string &s) {
	std::string ls(s);
	std::transform(ls.begin(), ls.end(), ls.begin(), [](unsigned char c) { return std::tolower(c); }); 
	return ls;
}


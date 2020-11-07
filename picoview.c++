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

std::vector<std::string> supported;

PicoView::PicoView(QPalette _palette, QWidget* parent) : QMainWindow(parent), palette(_palette) {
    // Get supported formats
    QList<QByteArray> fmts = QImageReader::supportedImageFormats();
    fmts += QMovie::supportedFormats();
    for (int ii = 0; ii < fmts.length(); ii ++) supported.push_back("."+fmts[ii].toStdString());
    for (auto const &s : supported) std::cout << s << std::endl;

	for (const auto &s : supported) {
		if (s != supported.back()) filter += "*"+s+" ";
		else filter += 	"*"+s+")";	
	}

	w = new PicoWidget(this, this);
	w->setPalette(palette);
	this->setCentralWidget(w);
	
	img_label = new QLabel;
	img_label->setAlignment(Qt::AlignCenter);
	label_size = QSize(300, 200);
	img_label->setMinimumSize(label_size);
	
	mov = new QMovie;
    
	// Create image title and dimensions labels	
	info = new QLabel;
	info->setMinimumSize(QSize(0, info->minimumSizeHint().height()));
	info->setAlignment(Qt::AlignCenter);
	dimensions = new QLabel;
	dimensions->setAlignment(Qt::AlignCenter);

	buildLayout();
	current(-1);
}

void PicoView::resizeEvent(QResizeEvent* e) {
	QMainWindow::resizeEvent(e);
	w->setMaximumSize(this->size());
	label_size = img_label->size();
	current(cidx);
}

void PicoView::open(const fs::path &p) {
	path = fs::canonical(p);
	if (fs::is_directory(path)) open_dir(path, false);
	else open_file(path, false);
}

void PicoView::getFileList(bool sort) {
	std::string ext;
	fs::path p;
	files = {};
	for (const auto &e : fs::directory_iterator(path)) {
		p = e.path();
		ext = tolower(p.extension().string());
		if (contains<std::string>(supported, ext)) {
			p.replace_extension(ext);
			files.push_back(p);
		}
	}
	if (sort) sortby(sorting);
}

void PicoView::buildLayout() { 
	layout = new QHBoxLayout;
	img_canvas = new QVBoxLayout;
	tbar_layout = new QHBoxLayout;
	menu = new QMenuBar;

	buildMenu();
	buildControls();

	// Create and connect refresh button with F5 shortcut
	_refr = new QPushButton(w);
	QShortcut* _refr_shortcut = new QShortcut(QKeySequence::Refresh, this);
	QObject::connect(_refr, &QPushButton::clicked, this, &PicoView::refresh);
	QObject::connect(_refr_shortcut, &QShortcut::activated, this, &PicoView::refresh);

	// Load the refresh icon and set it for the refresh button
	QIcon r(QPixmap(":/Refresh.png"));
	_refr->setIcon(r);
	_refr->setIconSize(QSize(25, 25));
	tbar_layout->addWidget(menu);
	tbar_layout->addWidget(_refr);
	
	img_label->show();
	img_canvas->addLayout(tbar_layout);
	img_canvas->addWidget(img_label, Qt::AlignCenter);
	img_canvas->addLayout(controls_layout);

	layout->addLayout(img_canvas);
	w->setLayout(layout);
}
void PicoView::buildMenu() {
	// Populate file menu from _file_actions vector
	file = new QMenu("&File", w);
	file->show();
	int idx = 0;
	for (const auto &e : _file_actions) {
		QAction* act = new QAction(e.c_str(), this);
		QObject::connect(act, &QAction::triggered, this, _file_slots[idx++]);
		file->addAction(act);
	}

	// Populate sorting menu based on _sort_options map
	QSignalMapper* mapper = new QSignalMapper(this);
	sort = new QMenu("&Sort", w);
	sort->show();
	for (const auto &e : _sort_options) {
		QAction* act = new QAction(e.first.c_str(), this);
		QObject::connect(act, SIGNAL(triggered()), mapper, SLOT(map()));
		mapper->setMapping(act, QString::fromStdString(e.first));
		sort->addAction(act);
	}
	QObject::connect(mapper, SIGNAL(mapped(QString)), this, SLOT(sortby(QString)));

	menu->addMenu(file);
	menu->addMenu(sort);
}
void PicoView::buildControls() {
	// Layout for buttons
	QHBoxLayout* _layout = new QHBoxLayout;
	
	// Layout for info labels
	QHBoxLayout* _info = new QHBoxLayout;

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
	pal.setColor(QPalette::Active, QPalette::ButtonText, QColor(Qt::red));
	controls.find("Delete")->second->setPalette(pal);

	// Add the information labels
	dimensions->setMaximumWidth(200);
	info->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
	_info->addWidget(dimensions);
	_info->addWidget(info);


	// Create and connect fullscreen button with F11 shortcut (broken)
/*	_fullscreen = new QPushButton("FS", w);
	QShortcut* _fullscreen_shortcut = new QShortcut(QKeySequence(tr("F11")), this);
	QObject::connect(_fullscreen, &QPushButton::clicked, this, &PicoView::fullscreen);
	QObject::connect(_fullscreen_shortcut, &QShortcut::activated, this, &PicoView::fullscreen);
	_info->addWidget(_fullscreen);
*/
	// Add the info and controls to controls_layout
	controls_layout = new QVBoxLayout();
	controls_layout->addLayout(_info);
	controls_layout->addLayout(_layout);

	// Adjust size hints for first and last buttons
	controls.find("<<")->second->setMaximumWidth(30);
	controls.find(">>")->second->setMaximumWidth(30);
}

void PicoView::current(const int &i) {
	cidx = i;
	if (i >= 0 && (unsigned int)i < files.size()) {
		if (mov != NULL) {
			delete mov;
			mov = NULL;
		}
		if (isMovie(files[i])) {
			mov = new QMovie(QString::fromStdString(files[i].string()));

            nframes = mov->frameCount();
            connect(mov, SIGNAL(frameChanged(int)), this, SLOT(movieLooper(int)));

			// Have to start the movie before calling [->frameRect()]
			img_label->setMovie(mov);
			mov->jumpToNextFrame();
			img_size = mov->frameRect();

			// Attempt to down scale the movie to fit container (if necessary), without compromising the native aspect
			double aspect = (double)img_size.width() / (double)img_size.height();
			QSize scaled = img_size.size();
			if (img_size.width() > label_size.width()) scaled = QSize(label_size.width(), label_size.width() / aspect);
			if (scaled.height() > label_size.height()) scaled = QSize(label_size.height() * aspect, label_size.height());

            // Apply scaling and start the movie
		    mov->setScaledSize(scaled);
			mov->start();
		}
		else {
			img.load(QString::fromStdString(files[i].string()));
			QPixmap p = QPixmap::fromImage(img);
			img_size = p.rect();

			// If the image's native resolution exceeds the container size, attempt to scale down accordingly
			if (img_size.height() > label_size.height() || img_size.width() > label_size.width()) {
				p = p.scaled(label_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			}

			img_label->setPixmap(p);
		}

		dimensions->setText(QString::fromStdString(std::to_string(img_size.width())+"x"+std::to_string(img_size.height())));
		setLabelText(info, QString::fromStdString(files[i].filename().string()));
	}

	_prev = controls.find("Previous")->second;
	_delt = controls.find("Delete")->second;
	_next = controls.find("Next")->second;

	// Disable next and previous buttons when appropriate
	bool empty = files.empty();
	if (i == 0 || empty) {
		_prev->setEnabled(false);
		controls.find("<<")->second->setEnabled(false);
	}
	if ((unsigned int)i == files.size() - 1 || empty) {
		_next->setEnabled(false);
		controls.find(">>")->second->setEnabled(false);
	}
	if (empty && _delt->isEnabled()) _delt->setEnabled(false);
	else _delt->setEnabled(true);
	if (empty && _refr->isEnabled()) _refr->setEnabled(false);
	else _refr->setEnabled(true);

	// Re-enable next and previous buttons as needed
	if (!_prev->isEnabled() && i != 0 && !empty) {
		_prev->setEnabled(true);
		controls.find("<<")->second->setEnabled(true);
	}
	if (!_next->isEnabled() && (unsigned int)i != files.size() - 1 && !empty) {
		_next->setEnabled(true);
		controls.find(">>")->second->setEnabled(true);
	}
}

// Slots
void PicoView::open_file() {
	std::string _file = QFileDialog::getOpenFileName(this, tr("Open Image"), path.string().c_str(), 
		tr(("Image Files "+filter).c_str())).toStdString();
	if (_file == "") return;
	open_file(fs::path(_file));
}

void PicoView::open_file(fs::path _file, bool checking) {
	fs::path file = fs::canonical(_file);

	if (checking) {
		if (file.parent_path() == path) goto _open_file;
	}
	path = fs::canonical(file).remove_filename();
	getFileList();

_open_file:
	auto found = std::find(files.begin(), files.end(), file);
	if (found == files.end()) {
		setLabelText(info, QString::fromStdString("Error opening "+file.filename().string()+"."));
		cidx = 0;
	}
	else cidx = std::distance(files.begin(), found); 
	current(cidx);
}
void PicoView::open_dir() {
	std::string _dir = QFileDialog::getExistingDirectory(this, tr("Directory"), path.string().c_str()).toStdString();
	if (_dir == "") return; 
	open_dir(fs::path(_dir));
	
}
void PicoView::open_dir(fs::path _dir, size_t idx, bool checking) {
	if (checking) {
		if (_dir == path) return;
	}
	path = fs::canonical(_dir);
	getFileList();
	current(idx);
}

void PicoView::sortby(QString s) {
	if (cidx < 0) return;
	SortMode m = _sort_options.find(s.toStdString())->second;
	fs::path _file = files[cidx];

	// Update file list in case of deletion/addition from external source
	getFileList(false);

	// Reselect the file if it still exists, else go to nearest previous index
	std::vector<fs::path>::iterator pos = std::find(files.begin(), files.end(), _file);
	if (pos == files.end()) {
	    cidx = pos - files.begin();
	    if ((unsigned)cidx > files.size() - 1) cidx = files.size() - 1;
	    fs::path _file = files[cidx];
	}
	
	sorting = s;
	switch(m) {
		case SortMode::name:
			std::sort(files.begin(), files.end(), [](auto &l, auto &r) { return l < r; });
			break;
			
		case SortMode::modified:
			std::sort(files.begin(), files.end(), [](auto &l, auto &r) { return fs::last_write_time(l) < fs::last_write_time(r); });
			break;
			
		case SortMode::type:
			std::sort(files.begin(), files.end(), [](auto &l, auto &r) { 
				return (l.extension() < r.extension()) || (l.extension() == r.extension() && l.filename() < r.filename()); 
			});
			break;
	}

	// TODO This can be improved: as it is animations restart on every [sortby]
	current(std::distance(files.begin(), std::find(files.begin(), files.end(), _file)));
}
void PicoView::refresh() {
	img_label->hide();
	open_dir(path, cidx, false);
	img_label->show();
}
void PicoView::fullscreen() {
// Broken
    isFullScreen() ? showNormal() : showFullScreen();
}

void PicoView::movieLooper(int f) {
    if (f == nframes - 1) {
        mov->jumpToFrame(0);
    }
}

void PicoView::firs() {
	current(0);
}
void PicoView::prev() {
	if (cidx > 0) current(--cidx);
}
void PicoView::delt() {
	bool success = fs::remove(files[cidx]);
	if (success) {
		setLabelText(info, QString::fromStdString("Removed "+files[cidx].filename().string()+"."));
		files.erase(files.begin() + cidx);
		current(cidx);
	}
	else {
		setLabelText(info, QString::fromStdString("Failed to remove "+files[cidx].filename().string()+"."));
	}
}
void PicoView::next() {
	if ((unsigned int)cidx < files.size() - 1) current(++cidx);
}
void PicoView::last() {
	current(files.size() - 1);
}

// General
bool PicoView::isMovie(fs::path f) {
	QMovie m(QString::fromStdString(f.string()));
	return m.frameCount() > 1;
}

void PicoWidget::resizeEvent(QResizeEvent* e) {	
	QWidget::resizeEvent(e);
	window->resizeEvent(e);
}

std::string tolower(const std::string &s) {
	std::string ls(s);
	std::transform(ls.begin(), ls.end(), ls.begin(), [](unsigned char c) { return std::tolower(c); }); 
	return ls;
}
void setLabelText(QLabel* label, QString text) {
	QFontMetrics metric(label->font());
	QString clipped = metric.elidedText(text, Qt::ElideRight, label->width());
	label->setText(clipped);
}

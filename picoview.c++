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
    for (int ii = 0; ii < fmts.length(); ii ++) {
        if (!contains<std::string>(supported, fmts[ii].toStdString())) supported.push_back("."+fmts[ii].toStdString());
    }
    supported.push_back(".mp4");

	for (const auto &s : supported) {
		if (&s != &supported.back()) filter += "*"+s+" ";
		else filter += 	"*"+s+")";	
	}

	label_size = QSize(600, 800);

	w = new PicoWidget(this, this);
	w->setPalette(palette);
	this->setCentralWidget(w);
	
	img_container = new QLabel;
	img_container->setAlignment(Qt::AlignCenter);
	img_container->setMinimumSize(label_size);
	
	mov = new QMovie;

	vid_container = new QWidget(w);
	vid_container->setStyleSheet("background: gray");
	vid_container->hide();

	vid = new QVideoWidget(vid_container);

    QVBoxLayout* vid_vert = new QVBoxLayout(vid_container);
    QHBoxLayout* vid_horz = new QHBoxLayout;
    
    vid_horz->addWidget(vid, 0, Qt::AlignCenter);
    vid_vert->addLayout(vid_horz);
    vid_vert->setAlignment(Qt::AlignCenter);
    
	player = new QMediaPlayer;
	player->setVideoOutput(vid);
    player->setNotifyInterval(10);

    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(videoLooper(qint64)));
    
	// Create image title and dimensions labels	
	info = new QLabel;
	info->setMinimumSize(QSize(0, info->minimumSizeHint().height()));
	info->setAlignment(Qt::AlignCenter);
	
	dimensions = new QLabel;
	dimensions->setAlignment(Qt::AlignCenter);

	buildLayout();
	current(-1);

	norm_geometry = frameGeometry();
}

void PicoView::resizeEvent(QResizeEvent* e) {
	QMainWindow::resizeEvent(e);
	w->setMaximumSize(this->size());
	if (img_container->isVisible()) label_size = img_container->size();
	else label_size = vid_container->size();
	current(cidx);

    if (frameGeometry().topLeft() != QPoint(0, 0)) {
        norm_geometry = frameGeometry();
    }
}

void PicoView::open(const fs::path &p) {
	path = fs::canonical(p);
	if (fs::is_directory(path)) open_dir(path, 0, false);
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
	canvas = new QVBoxLayout;
	tbar_layout = new QHBoxLayout;
	media = new QHBoxLayout;
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
	
    media->addWidget(img_container, Qt::AlignCenter);
    media->addWidget(vid_container, Qt::AlignCenter);
	img_container->show();
	canvas->addLayout(tbar_layout);
	canvas->addLayout(media);
	canvas->addLayout(controls_layout);

	layout->addLayout(canvas);
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
	_fullscreen = new QPushButton("FS", w);
	_fullscreen->setFixedSize(QSize(25, 25));
	QShortcut* _fullscreen_shortcut = new QShortcut(QKeySequence(tr("F11")), this);
	QObject::connect(_fullscreen, &QPushButton::clicked, this, &PicoView::fullscreen);
	QObject::connect(_fullscreen_shortcut, &QShortcut::activated, this, &PicoView::fullscreen);
	_info->addWidget(_fullscreen);

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
		if (vid_container->isVisible() && !isVideo(files[i])) {
		    player->stop();
            vid_container->hide();
            img_container->show();
		}
		if (isMovie(files[i])) {
			mov = new QMovie(QString::fromStdString(files[i].string()));

            nframes = mov->frameCount();
            connect(mov, SIGNAL(frameChanged(int)), this, SLOT(movieLooper(int)));

			// Have to start the movie before calling [->frameRect()]
			img_container->setMovie(mov);
			mov->jumpToNextFrame();
			img_rect = mov->frameRect();

            // Apply scaling and start the movie
		    mov->setScaledSize(calculateScale());
			mov->start();
		}
		else if (isVideo(files[i])) {
		    player->setMedia(QUrl::fromLocalFile(QString::fromStdString(files[i].string())));
		    img_container->hide();
		    
		    // Use ffmpeg's ffprobe to extract resolution information
            img_rect.setSize(extractResolution(files[i].string()));
            vid->setFixedSize(calculateScale());

            vid->show();
            player->play();
		}
		else {
			img.load(QString::fromStdString(files[i].string()));
			QPixmap p = QPixmap::fromImage(img);
			img_rect = p.rect();

			// If the image's native resolution exceeds the container size, attempt to scale down accordingly
			if (img_rect.height() > label_size.height() || img_rect.width() > label_size.width()) {
				p = p.scaled(label_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			}

			img_container->setPixmap(p);
		}

		dimensions->setText(QString::fromStdString(std::to_string(img_rect.width())+"x"+std::to_string(img_rect.height())));
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
	try {
	    getFileList();
	}
    catch (...) {
        error("failed to open "+colors::yellow+path.string()+colors::res, __LINE__ - 3, __FILE__);
        return;
    }
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
	img_container->hide();
	open_dir(path, cidx, false);
	img_container->show();
}
void PicoView::fullscreen() {
    QRect g = frameGeometry();
    if (is_fullscreen) {
        showNormal();
        is_fullscreen = isFullScreen();

        // For X11 window managers, have to manually reset geometry
        if (frameGeometry() == g) {
            setGeometry(norm_geometry);
            is_fullscreen = false;
        }
    }
    else {
        showFullScreen();
        is_fullscreen = isFullScreen();

        // If showFullScreen() didn't work, try to fake it -- often necessary with X11 window managers
        if (frameGeometry() == g) {
            setGeometry(QApplication::desktop()->availableGeometry());
            move(0, 0);
            is_fullscreen = true;
        }
    }
}

void PicoView::movieLooper(int f) {
    if (f == nframes - 1) {
        mov->jumpToFrame(0);
    }
}

void PicoView::videoLooper(qint64 p) {
    if (player->duration() && p >= player->duration() - 15) {
        player->setPosition(0);
        player->play();
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
bool PicoView::isVideo(fs::path f) {
    return f.extension() == ".mp4";
}

QSize PicoView::extractResolution(std::string f) {
    return split(exec("ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=s=,:p=0 "+f), ',');
}


QSize PicoView::calculateScale() {
	// Attempt to down scale the media to fit container (if necessary), without compromising the native aspect
	double aspect = (double)img_rect.width() / (double)img_rect.height();
	QSize scaled = QSize(img_rect.width(), img_rect.height());
	if (img_rect.width() > label_size.width()) scaled = QSize(label_size.width(), label_size.width() / aspect);
	if (scaled.height() > label_size.height()) scaled = QSize(label_size.height() * aspect, label_size.height());
	return scaled;
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
void error(const std::string mess, const int line, const char* file) {
    std::cout << std::flush;
    printf(std::string("\r"+colors::red+colors::white_back+" Error :"+colors::res+" \"%s\""+" in File "+colors::yellow+"%s"+colors::res+
           " on line "+colors::bright+colors::red+"%d"+colors::res+"\n").c_str(), mess, file, line);
}

template <typename T>
void split(const std::string &s, char delim, T result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}
QSize split(const std::string &s, char delim) {
    std::vector<std::string> v;
    split(s, delim, std::back_inserter(v));
    return QSize(std::stoi(v[0]), std::stoi(v[1]));
}

std::string exec(std::string command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

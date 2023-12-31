#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass) {
  ui->setupUi(this);

  installEventFilter(this);

  setWindowTitle("Video Player");
  resize(1600, 900);

  renderWidget = new QWidget(this);
  renderWidget->setStyleSheet("background-color: black;");
  renderWidget->resize(1581, 828);

  QBoxLayout *renderLayout = new QVBoxLayout();
  renderLayout->addWidget(renderWidget);

  mainLayout = new QVBoxLayout(ui->centralWidget);
  renderWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  bottomMenuLayout = new QHBoxLayout();
  bottomMenuLayout->addWidget(ui->playButton);
  bottomMenuLayout->addWidget(ui->soundButton);
  bottomMenuLayout->addWidget(ui->currentContentDuration);
  bottomMenuLayout->addWidget(ui->slider);

  mainLayout->addLayout(renderLayout);
  mainLayout->addLayout(bottomMenuLayout);

  ui->centralWidget->setLayout(mainLayout);

  HWND hwnd = reinterpret_cast<HWND>(renderWidget->winId());
  m_videoPlayer = QSharedPointer<VideoPlayer>::create(hwnd);

  connectSignalsAndSlots();

  hideUI();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);

  UINT32 newWidth = renderWidget->width();
  UINT32 newHeight = renderWidget->height();
  m_videoPlayer->GetDxHelper()->ResizeRenderTarget(newWidth, newHeight);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (obj == this && event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_F11) {
      if (isFullScreen()) {
        showNormal();
        menuBar()->show();
        mainLayout->addLayout(bottomMenuLayout);
      } else {
        menuBar()->hide();
        showFullScreen();
        mainLayout->removeItem(bottomMenuLayout);
        renderWidget->resize(1920, 1080);
      }
      return true;
    }
  }
  return QMainWindow::eventFilter(obj, event);
}

void MainWindow::hideUI() {
  ui->slider->hide();
  ui->playButton->hide();
  ui->soundButton->hide();
  ui->volumeSlider->hide();
  ui->currentContentDuration->hide();
}

void MainWindow::setupUI() {
  const int maxVolumeSliderValue = 10;

  ui->slider->show();
  ui->playButton->show();
  ui->soundButton->show();
  //ui->volumeSlider->show();
  ui->currentContentDuration->show();

  ui->volumeSlider->setRange(0, maxVolumeSliderValue);
  ui->volumeSlider->setValue(maxVolumeSliderValue);
}

void MainWindow::connectSignalsAndSlots() {
  connect(ui->actionOpen_file, &QAction::triggered, this,
          &MainWindow::onFileOpen);
  connect(ui->volumeSlider, &QSlider::valueChanged, this,
          &MainWindow::onVolumeChanged);
  QObject::connect(this->m_videoPlayer.data(), &VideoPlayer::positionChanged,
                   this, &MainWindow::onPositionChanged);
  
  QObject::connect(this->m_videoPlayer.data(), &VideoPlayer::endOfStream,
                   this, &MainWindow::onEndOfStream);

  connect(ui->slider, &QSlider::sliderMoved, this, &MainWindow::onSliderMoved);
  connect(ui->slider, &QSlider::sliderPressed, this,
          &MainWindow::onSliderPressed);
  connect(ui->slider, &QSlider::sliderReleased, this,
          &MainWindow::onSliderReleased);
  connect(ui->playButton, &QPushButton::clicked, this,
          &MainWindow::onPlayPauseVideo);
  connect(ui->soundButton, &QPushButton::clicked, this, &MainWindow::onMute);

}

void MainWindow::onEndOfStream() {
  ui->playButton->setIcon(QIcon("Resources/icons/media-play.png"));

  m_videoPlayer->SetPosition(0);  
  m_videoPlayer->PlayPauseVideo();  
}

void MainWindow::onFileOpen() {
  QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                                                  tr("Video Files (*.mp4)"));

  if (!filePath.isEmpty()) {
    const wchar_t *wFilePath =
        reinterpret_cast<const wchar_t *>(filePath.utf16());
    m_videoPlayer->OpenURL(wFilePath);

    ui->slider->setRange(0, m_videoPlayer->GetDuration() / 100);

    setupUI();

    ////TODO: work out second time opening video
  }
}

void MainWindow::onSliderMoved(const int &position) {
  qint64 hnsPosition = static_cast<qint64>(position);
  m_videoPlayer->SetPosition(hnsPosition * 100);
}

void MainWindow::onPlayPauseVideo() {
  if (m_videoPlayer->GetIsPaused()) {
    ui->playButton->setIcon(QIcon("Resources/icons/media-pause.png"));
  } else {
    ui->playButton->setIcon(QIcon("Resources/icons/media-play.png"));
  }
  m_videoPlayer->PlayPauseVideo();
}

void MainWindow::onSliderPressed() {
  if (!m_videoPlayer->GetIsPaused()) {
    m_videoPlayer->PlayPauseVideo();
  }
}

void MainWindow::onSliderReleased() {
  if (m_videoPlayer->GetIsPaused()) {
    m_videoPlayer->PlayPauseVideo();
    ui->playButton->setIcon(QIcon("Resources/icons/media-pause.png"));
  }
}

void MainWindow::onPositionChanged(const qint64 &currentPosition) {
  ui->slider->setValue(static_cast<int>(currentPosition));
  updateDurationInfo(currentPosition * 100);
}

void MainWindow::onVolumeChanged(const int &value) {
  float volume = static_cast<float>(value) /
                 static_cast<float>(ui->volumeSlider->maximum());
  m_videoPlayer->GetSoundEffect()->ChangeVolume(volume);
}

void MainWindow::onMute() {
  if (m_videoPlayer->GetSoundEffect()->IsMute()) {
    m_videoPlayer->GetSoundEffect()->Unmute();
    ui->soundButton->setIcon(QIcon("Resources/icons/volume-unmute.png"));
  } else {
    ui->soundButton->setIcon(QIcon("Resources/icons/volume-mute.png"));
    m_videoPlayer->GetSoundEffect()->Mute();
  }
}

void MainWindow::updateDurationInfo(const qint64 &currentPosition) {
  const qint64 duration = this->m_videoPlayer->GetDuration();

  QTime currentTime(0, 0);
  currentTime = currentTime.addMSecs(currentPosition / 10000);

  QTime totalTime(0, 0);
  totalTime = totalTime.addMSecs(duration / 10000);

  QString format = "hh:mm:ss";
  QString currentTimeStr = currentTime.toString(format);
  QString totalTimeStr = totalTime.toString(format);

  QString tStr = currentTimeStr + " / " + totalTimeStr;
  ui->currentContentDuration->setText(tStr);
}
MainWindow::~MainWindow() {
  delete ui;
  m_videoPlayer.~QSharedPointer();
}

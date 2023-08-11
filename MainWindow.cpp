#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass) {
  ui->setupUi(this);

  setWindowTitle("Video Player");
  resize(1920, 1080);

  ui->renderWidget->setBaseSize(1280, 720);
  HWND hwnd = reinterpret_cast<HWND>(ui->renderWidget->winId());
  m_videoPlayer = QSharedPointer<VideoPlayer>::create(hwnd);

  connectSignalsAndSlots();

  hideUI();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);

  
  int newWidth = ui->renderWidget->width();
  int newHeight =
      static_cast<int>(newWidth * (9.0 / 16.0));

  ui->renderWidget->setFixedSize(newWidth, newHeight);

  if (m_videoPlayer) {
    m_videoPlayer->GetDxHelper()->ResizeSwapChain(newWidth, newHeight);
  }
  qDebug() << "Changed" << newWidth << newHeight;
}

void MainWindow::hideUI() {
  ui->slider->hide();
  ui->playButton->hide();
  ui->volumeSlider->hide();
  ui->currentContentDuration->hide();
}

void MainWindow::setupUI() {
  const int maxVolumeSliderValue = 10;
  ui->slider->show();
  ui->playButton->show();
  ui->volumeSlider->show();
  ui->currentContentDuration->show();

  ui->volumeSlider->setRange(0, maxVolumeSliderValue);
  ui->volumeSlider->setValue(maxVolumeSliderValue);
}

void MainWindow::connectSignalsAndSlots() {
  connect(ui->actionOpen_file, &QAction::triggered, this, &MainWindow::onFileOpen);
  connect(ui->volumeSlider, &QSlider::valueChanged, this,
          &MainWindow::onVolumeChanged);
  QObject::connect(this->m_videoPlayer.data(), &VideoPlayer::positionChanged,
                   this, &MainWindow::onPositionChanged);
  connect(ui->slider, &QSlider::sliderMoved, this, &MainWindow::onSliderMoved);
  connect(ui->slider, &QSlider::sliderPressed, this, &MainWindow::onSliderPressed);
  connect(ui->slider, &QSlider::sliderReleased, this, &MainWindow::onSliderReleased);
  connect(ui->playButton, &QPushButton::clicked, this, &MainWindow::onPlayPauseVideo);
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
  LONGLONG hnsPosition = static_cast<LONGLONG>(position);
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
  }
}

void MainWindow::onPositionChanged(const qint64 &currentPosition) {
  ui->slider->setValue(currentPosition);
  updateDurationInfo(currentPosition * 100);
}

void MainWindow::onVolumeChanged(const int &value) {
  float volume = static_cast<float>(value) /
                 static_cast<float>(ui->volumeSlider->maximum());
  m_videoPlayer->GetSoundEffect()->ChangeVolume(volume);
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

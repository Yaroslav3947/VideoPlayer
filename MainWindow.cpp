#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass) {
  ui->setupUi(this);

  setWindowTitle("Video Player");
  resize(1920, 1080);

  ui->renderWidget->setFixedSize(1280, 720);
  HWND hwnd = reinterpret_cast<HWND>(ui->renderWidget->winId());
  m_videoPlayer = QSharedPointer<VideoPlayer>::create(hwnd);

  connectSignalsAndSlots();

  /*m_videoPlayer->OpenURL(L"Resources/SampleVideo25fpsWIthAudio.mp4");

  ui->slider->setRange(0, m_videoPlayer->GetDuration());*/
}

void MainWindow::hideUI() {
  ui->slider->hide();
  ui->playButton->hide();
  ui->currentContentDuration->hide();
}

void MainWindow::setupUI() {
  ui->slider->show();
  ui->playButton->show();
  ui->currentContentDuration->show();
}

void MainWindow::connectSignalsAndSlots() {

  QObject::connect(this->m_videoPlayer.data(), &VideoPlayer::positionChanged,
                   this, &MainWindow::onPositionChanged);
  connect(ui->slider, &QSlider::sliderMoved, this, &MainWindow::onSliderMoved);
  connect(ui->slider, &QSlider::sliderPressed, this, &MainWindow::onSliderPressed);
  connect(ui->slider, &QSlider::sliderReleased, this, &MainWindow::onSliderReleased);
  connect(ui->playButton, &QPushButton::clicked, this, &MainWindow::onPlayPauseVideo);
}

void MainWindow::on_actionOpen_file_triggered() {
  QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                               tr("Video Files (*.mp4)"));

  if (!filePath.isEmpty()) {
    const wchar_t *wFilePath =
        reinterpret_cast<const wchar_t *>(filePath.utf16());
    m_videoPlayer->OpenURL(wFilePath);

    ui->slider->setRange(0, m_videoPlayer->GetDuration());

    setupUI();

    ////TODO: work out second time opening video
  }
}

void MainWindow::onSliderMoved(const int &position) {
  LONGLONG hnsPosition = static_cast<LONGLONG>(position);
  m_videoPlayer->SetPosition(hnsPosition);
}

void MainWindow::onPlayPauseVideo() {
  if (m_videoPlayer->GetIsPaused()) {
    ui->playButton->setText("Pause");
    ui->playButton->setIcon(QIcon("Resources/icons/media-pause.png"));
  } else {
    ui->playButton->setText("Play");
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
  updateDurationInfo(currentPosition);
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

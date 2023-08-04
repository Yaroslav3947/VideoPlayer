#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass) {
  ui->setupUi(this);

  setWindowTitle("Video Player");
  resize(1920, 1080);

  ui->renderWidget->setFixedSize(1280, 720);
  HWND hwnd = reinterpret_cast<HWND>(ui->renderWidget->winId());
  m_videoPlayer = VideoPlayer::Ptr(new VideoPlayer(hwnd));

  connectSignalsAndSlots();
}

void MainWindow::connectSignalsAndSlots() {
  connect(ui->playButton, &QPushButton::clicked, this, &MainWindow::pauseVideo);
  QObject::connect(this->m_videoPlayer.data(), &VideoPlayer::positionChanged,
                   this, &MainWindow::updateSliderPosition);
  connect(ui->slider, &QSlider::sliderMoved, this, &MainWindow::onSliderMoved);
  connect(ui->slider, &QSlider::sliderPressed, this, &MainWindow::pauseVideo);
  connect(ui->slider, &QSlider::sliderReleased, this, &MainWindow::pauseVideo);
}

void MainWindow::onSliderMoved(const int &position) {
  LONGLONG hnsPosition = static_cast<LONGLONG>(position);  
  m_videoPlayer->SetPosition(hnsPosition);
}

void MainWindow::on_actionOpen_file_triggered() {
  QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                                                  tr("Video Files (*.mp4)"));

  if (!filePath.isEmpty()) {
    const wchar_t *wFilePath =
        reinterpret_cast<const wchar_t *>(filePath.utf16());
    m_videoPlayer->OpenURL(wFilePath);

    ui->slider->setRange(0, m_videoPlayer->GetDuration());

    qDebug() << m_videoPlayer->GetDuration();
  }
}

void MainWindow::pauseVideo() { 
    m_videoPlayer->Pause(); 
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

void MainWindow::updateSliderPosition(const qint64 &currentPosition) {
  ui->slider->setValue(currentPosition);
  updateDurationInfo(currentPosition);
}

MainWindow::~MainWindow() { delete ui; }

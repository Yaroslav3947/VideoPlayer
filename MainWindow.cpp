#include "MainWindow.h"

#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass()) {
  ui->setupUi(this);

  setWindowTitle("Video Player");
  resize(1920, 1080);

  QWidget* mainContainer = new QWidget(this);
  setCentralWidget(mainContainer);

  QWidget* videoWidget = new QWidget(this);
  setCentralWidget(videoWidget);

  QVBoxLayout* layout = new QVBoxLayout(videoWidget);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  QWidget* renderWidget = new QWidget(videoWidget);
  renderWidget->setFixedSize(1280, 720);
  layout->addWidget(renderWidget, 0, Qt::AlignCenter);

  m_dxhelper = std::make_unique<DXHelper>(renderWidget);


  QTimer* renderTimer = new QTimer(this);
  connect(renderTimer, &QTimer::timeout, this, &MainWindow::RenderFrame);
  renderTimer->start(16);  // 60 FPS
}

MainWindow::~MainWindow() { delete ui; }

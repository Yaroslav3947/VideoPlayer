#include "MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass()) {
  ui->setupUi(this);

  CreateMenuBar();

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

  HWND hwnd = reinterpret_cast<HWND>(renderWidget->winId());

  m_videoPlayer = std::make_unique<VideoPlayer>(hwnd);
}

void MainWindow::OpenFile() {
  QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), "",
                                                  tr("Video Files (*.mp4)"));

  if (!filePath.isEmpty()) {
    const wchar_t* wFilePath =
        reinterpret_cast<const wchar_t*>(filePath.utf16());
    m_videoPlayer->OpenURL(wFilePath);
  }
}

void MainWindow::CreateMenuBar() {
  QMenuBar* menuBar = new QMenuBar(this);
  setMenuBar(menuBar);

  QMenu* fileMenu = menuBar->addMenu(tr("File"));

  QAction* openAction = new QAction(tr("Open"), this);
  fileMenu->addAction(openAction);

  connect(openAction, &QAction::triggered, this, &MainWindow::OpenFile);
}

MainWindow::~MainWindow() { delete ui; }

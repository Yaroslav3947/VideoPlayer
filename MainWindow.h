#pragma once

#include <QFileDialog>
#include <QTimer>
#include <QVBoxLayout>
#include <QtWidgets/QMainWindow>
#include <memory>

#include "DXHelper.h"
#include "VideoPlayer.h"
#include "ui_MainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindowClass;
};
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 private:
  void OpenFile();
  void CreateMenuBar();

 private:
  Ui::MainWindowClass *ui;
  std::unique_ptr<DXHelper> m_dxhelper;
  std::unique_ptr<VideoPlayer> m_videoPlayer;

 public slots:
};

#pragma once

#include <QFileDialog>
#include <QTimer>
#include <QVBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QMenuBar>
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
  void connectSignalsAndSlots();
 private:
  Ui::MainWindowClass *ui;

  std::unique_ptr<DXHelper> m_dxhelper;
  VideoPlayer::Ptr m_videoPlayer;

 public slots:
  void pauseVideo();
  void onSliderMoved(const LONGLONG &value);
  void on_actionOpen_file_triggered();
  void updateDurationInfo(const qint64 &currentPosition);
  void updateSliderPosition(const qint64 &currentPosition);
};

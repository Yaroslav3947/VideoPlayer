#pragma once

#include <QFileDialog>
#include <QMenuBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QtWidgets/QMainWindow>


#include "DX/DXHelper.h"
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
  void hideUI();
  void setupUI();
  void connectSignalsAndSlots();

 private:
  Ui::MainWindowClass *ui;

  std::unique_ptr<DXHelper> m_dxhelper;
  VideoPlayer::Ptr m_videoPlayer;

 private:
  QWidget *renderWidget;

 protected:
  void resizeEvent(QResizeEvent *event) override;

 private slots:
  void onMute();
  void onFileOpen();
  void onEndOfStream();
  void onSliderPressed();
  void onPlayPauseVideo();
  void onSliderReleased();
  void onVolumeChanged(const int &value);
  void onSliderMoved(const int &position);
  void updateDurationInfo(const qint64 &currentPosition);
  void onPositionChanged(const qint64 &currentPosition);
};

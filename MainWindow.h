#pragma once

#include <QTimer>
#include <QtWidgets/QMainWindow>
#include <memory>

#include "DXHelper.h"
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
  Ui::MainWindowClass *ui;
  std::unique_ptr<DXHelper> m_dxhelper;

 public slots:
  void RenderFrame() { m_dxhelper->RenderFrame(); }
};

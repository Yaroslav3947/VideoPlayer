#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <QApplication>
#include <QWidget>

using Microsoft::WRL::ComPtr;

class DXHelper : public QWidget {
 public:
  DXHelper(QWidget* widget);
  ~DXHelper() = default;

  void Init();
  void RenderFrame();

 private:
  QWidget* m_widget;
  ComPtr<ID3D11Device> m_device;
  ComPtr<ID3D11DeviceContext> m_deviceContext;
  ComPtr<IDXGISwapChain> m_swapChain;
  ComPtr<ID3D11RenderTargetView> m_renderTargetView;
};
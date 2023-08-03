#pragma once

#include <Mfidl.h>
#include <mfreadwrite.h>

#include <d3d11.h>
#include <d2d1_1.h>

#include <wrl/client.h>

#include <QApplication>
#include <QWidget>

using Microsoft::WRL::ComPtr;

class DXHelper : public QWidget {
 public:
  DXHelper(HWND& hwnd, QWidget* widget = nullptr);
  ~DXHelper() = default;

  HWND GetHWND() { return reinterpret_cast<HWND>(m_widget->winId()); }

  void Init(HWND& hwnd);

  void RenderBitmapOnWindow(ComPtr<ID2D1Bitmap> pBitmap);
  ComPtr<ID2D1Bitmap>CreateBitmapFromVideoSample(IMFSample* pSample);

 private:
  QWidget* m_widget;
  ComPtr<ID3D11Device> m_device;
  ComPtr<ID3D11DeviceContext> m_deviceContext;
  ComPtr<IDXGISwapChain> m_swapChain;

  ComPtr<ID2D1Factory1> m_factory;
  ComPtr<ID2D1RenderTarget> m_renderTarget;
};
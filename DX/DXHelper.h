#pragma once

#include "../WinInclude.h"

using Microsoft::WRL::ComPtr;

class DXHelper {
 public:
  DXHelper(HWND& hwnd);
  ~DXHelper() = default;

  void Init(HWND& hwnd);

  void RenderBitmapOnWindow(ComPtr<ID2D1Bitmap> pBitmap);
  ComPtr<ID2D1Bitmap> CreateBitmapFromVideoSample(IMFSample* pSample,
                                                  const UINT32& width,
                                                  const UINT32& height);

  void ResizeSwapChain(const UINT32& width, const UINT32& height);

 private:
  HWND m_hwnd;
  ComPtr<ID3D11Device> m_device;
  ComPtr<ID3D11DeviceContext> m_deviceContext;
  ComPtr<IDXGISwapChain> m_swapChain;

  ComPtr<ID2D1Factory1> m_factory;
  ComPtr<ID2D1RenderTarget> m_renderTarget;
};
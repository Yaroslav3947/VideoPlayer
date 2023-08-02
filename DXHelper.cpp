#include "DXHelper.h"

DXHelper::DXHelper(QWidget* widget) : m_widget(widget) { Init(); }

void DXHelper::Init() {
  DXGI_SWAP_CHAIN_DESC scd;
  ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
  scd.BufferCount = 2;
  scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  scd.OutputWindow = reinterpret_cast<HWND>(m_widget->winId());
  scd.SampleDesc.Count = 1;
  scd.Windowed = TRUE;

  D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                nullptr, 0, D3D11_SDK_VERSION, &scd,
                                &m_swapChain, &m_device, nullptr,
                                &m_deviceContext);

  ID3D11Texture2D* pBackBuffer;
  m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

  m_device->CreateRenderTargetView(pBackBuffer, nullptr, &m_renderTargetView);
  pBackBuffer->Release();
  m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

  D3D11_VIEWPORT viewport;
  ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width = m_widget->width();
  viewport.Height = m_widget->height();
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  m_deviceContext->RSSetViewports(1, &viewport);
}

void DXHelper::RenderFrame() {
  const float clearColor[] = {1.0f, .4f, 1.0f, 1.0f};
  m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);


  m_swapChain->Present(1, 0);
  
  qDebug() << "renderFrame()";
}
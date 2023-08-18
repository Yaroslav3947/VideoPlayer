#include "DXHelper.h"

DXHelper::DXHelper(const DXGI_SWAP_CHAIN_DESC& swapChainDesc)
    : m_swapChainDesc(swapChainDesc) {
  Init();
}

void DXHelper::Init() {
  D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_9_1,  D3D_FEATURE_LEVEL_9_2,
                                D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_10_0,
                                D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0,
                                D3D_FEATURE_LEVEL_11_1};

  UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

  D3D_FEATURE_LEVEL m_featureLevel;

  winrt::check_hresult(D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags,
      levels, ARRAYSIZE(levels), D3D11_SDK_VERSION, &m_swapChainDesc,
      m_swapChain.GetAddressOf(), m_device.GetAddressOf(), &m_featureLevel,
      m_deviceContext.GetAddressOf()));

  winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                         __uuidof(ID2D1Factory1), &m_factory));

  D2D1_RENDER_TARGET_PROPERTIES renderTargetProps =
      D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_HARDWARE,
                                   D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                                                     D2D1_ALPHA_MODE_IGNORE));

  ComPtr<IDXGISurface> dxgiBackbuffer;
  m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackbuffer));

  winrt::check_hresult(m_factory->CreateDxgiSurfaceRenderTarget(
      dxgiBackbuffer.Get(), &renderTargetProps, m_renderTarget.GetAddressOf()));
}

ComPtr<ID2D1Bitmap> DXHelper::CreateBitmapFromVideoSample(
    IMFSample* pSample, const UINT32& width, const UINT32& height) {
  ComPtr<IMFMediaBuffer> buffer;
  winrt::check_hresult(pSample->ConvertToContiguousBuffer(&buffer));

  BYTE* data = nullptr;
  DWORD maxDataLength = 0;
  DWORD currentDataLength = 0;

  winrt::check_hresult(buffer->Lock(&data, &maxDataLength, &currentDataLength));

  UINT32 pitch = width * sizeof(UINT32);

  D2D1_BITMAP_PROPERTIES bitmapProperties = {};
  ZeroMemory(&bitmapProperties, sizeof(bitmapProperties));
  bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
  bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

  ComPtr<ID2D1Bitmap> bitmap;
  winrt::check_hresult(m_renderTarget->CreateBitmap(
      D2D1::SizeU(width, height), data, pitch, bitmapProperties, &bitmap));

  winrt::check_hresult(buffer->Unlock());

  return bitmap;
}

void DXHelper::ResizeRenderTarget(const UINT32& width, const UINT32& height) {
  std::lock_guard<std::mutex> lock(m_resize_mtx);

  if (m_renderTarget) {
    m_renderTarget.Reset();
  }

  winrt::check_hresult(m_swapChain->ResizeBuffers(
      2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0));

  ComPtr<IDXGISurface> dxgiBackbuffer;
  winrt::check_hresult(
      m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackbuffer)));

  D2D1_RENDER_TARGET_PROPERTIES renderTargetProps =
      D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_HARDWARE,
                                   D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                                                     D2D1_ALPHA_MODE_IGNORE));

  winrt::check_hresult(m_factory->CreateDxgiSurfaceRenderTarget(
      dxgiBackbuffer.Get(), &renderTargetProps, m_renderTarget.GetAddressOf()));
}

void DXHelper::RenderBitmapOnWindow(ComPtr<ID2D1Bitmap> pBitmap) {
  m_renderTarget->BeginDraw();
  m_renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
  m_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

  D2D1_SIZE_F window = m_renderTarget->GetSize();
  D2D1_SIZE_F picture = pBitmap->GetSize();

  float windowAR = window.width / window.height;
  float pictureAR = picture.width / picture.height;

  float left = 0.0f, top = 0.0f, right = 0.0f, bottom = 0.0f;
  float scale = 0.0f;

  if (windowAR > pictureAR) {
    scale = window.height / 1080.0f;
    float newWidth = 1920 * scale;
    left = (window.width - newWidth) * 0.5f;
    right = left + newWidth;
    bottom = window.height;
  } else {
    scale = window.width / 1920.0f;
    float newHeight = scale * 1080;
    top = (window.height - newHeight) * 0.5f;
    bottom = top + newHeight;
    right = window.width;
  }

  D2D1_RECT_F destinationRect = D2D1::RectF(left, top, right, bottom);

  m_renderTarget->DrawBitmap(pBitmap.Get(), destinationRect);

  m_renderTarget->EndDraw();

  winrt::check_hresult(m_swapChain->Present(1, 0));
}

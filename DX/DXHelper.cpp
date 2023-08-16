#include "DXHelper.h"

DXHelper::DXHelper(HWND& hwnd) : m_hwnd(hwnd) { Init(hwnd); }

void DXHelper::Init(HWND& hwnd) {
  DXGI_SWAP_CHAIN_DESC desc = {};
  desc.BufferDesc.Width = 1920;
  desc.BufferDesc.Height = 1080;
  desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc.BufferDesc.RefreshRate.Numerator = 0;
  desc.BufferDesc.RefreshRate.Denominator = 0;
  desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
  desc.BufferCount = 2;
  desc.OutputWindow = hwnd;
  desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  desc.Windowed = true;

  D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_9_1,  D3D_FEATURE_LEVEL_9_2,
                                D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_10_0,
                                D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0,
                                D3D_FEATURE_LEVEL_11_1};

  UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

  D3D_FEATURE_LEVEL m_featureLevel;

  winrt::check_hresult(D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags,
      levels, ARRAYSIZE(levels), D3D11_SDK_VERSION, &desc,
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

void DXHelper::ChangeSize(UINT32 width, UINT32 height) {

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

  //// TODO: change logic, recreation of renderTarget => error
}

void DXHelper::RenderBitmapOnWindow(ComPtr<ID2D1Bitmap> pBitmap) {
  m_renderTarget->BeginDraw();
  m_renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
  m_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

  D2D1_SIZE_F windowSize = m_renderTarget->GetSize();
  D2D1_SIZE_F pictureSize = pBitmap->GetSize();


  float windowAR = windowSize.width / windowSize.height;

  float pictureAR = pictureSize.width / pictureSize.height;

  float left{0.0}, top{0.0}, right{0.0}, bottom{0.0};

  if (windowAR > pictureAR) {
    float scale = windowSize.height / 1080;

    float newWidth = 1920 * scale;
    float newHeight = windowSize.height;

     left = (windowSize.width - newWidth) / 2.0f;
     top = 0;
     right = left + newWidth;
     bottom = windowSize.height;
  } else {
    float scale = windowSize.width / 1920;

    float newWidth = windowSize.width;
    float newHeight = scale * 1080;

     left = 0;
     top = (windowSize.height - newHeight) / 2.0f;
     right = windowSize.width;
     bottom = top + newHeight;
  }

  qDebug() << left << top << right << bottom;

  qDebug() << "Render Target: " << m_renderTarget->GetSize().width
           << m_renderTarget->GetSize().height;

  D2D1_RECT_F destinationRect = D2D1::RectF(left, top, right, bottom);

  m_renderTarget->DrawBitmap(pBitmap.Get(), destinationRect);

  m_renderTarget->EndDraw();

  winrt::check_hresult(m_swapChain->Present(1, 0));

  return;
}

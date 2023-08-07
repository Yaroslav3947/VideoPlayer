#include "DXHelper.h"

DXHelper::DXHelper(HWND& hwnd) : m_hwnd(hwnd) {
  Init(hwnd);
}

void DXHelper::Init(HWND& hwnd) {
  DXGI_SWAP_CHAIN_DESC desc = {};
  desc.BufferDesc.Width = 0;
  desc.BufferDesc.Height = 0;
  desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc.BufferDesc.RefreshRate.Numerator = 0;
  desc.BufferDesc.RefreshRate.Denominator = 0;
  desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;
  desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  desc.BufferCount = 2;
  desc.OutputWindow = hwnd;
  desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  desc.Windowed = true;

  D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_9_1,  D3D_FEATURE_LEVEL_9_2,
                                D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_10_0,
                                D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0,
                                D3D_FEATURE_LEVEL_11_1};

  UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

  D3D_FEATURE_LEVEL m_featureLevel;

  HRESULT hr = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags,
      levels, ARRAYSIZE(levels), D3D11_SDK_VERSION, &desc,
      m_swapChain.GetAddressOf(), m_device.GetAddressOf(), &m_featureLevel,
      m_deviceContext.GetAddressOf());

  D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1),
                    &m_factory);

  D2D1_RENDER_TARGET_PROPERTIES renderTargetProps =
      D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_HARDWARE,
                                   D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                                                     D2D1_ALPHA_MODE_IGNORE));

  IDXGISurface* dxgiBackbuffer;
  m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackbuffer));

  hr = m_factory->CreateDxgiSurfaceRenderTarget(
      dxgiBackbuffer, &renderTargetProps, m_renderTarget.GetAddressOf());
}

ComPtr<ID2D1Bitmap> DXHelper::CreateBitmapFromVideoSample(IMFSample* pSample) {
  ////TODO: get rid of hardcode
  int width = 1920;
  int height = 1080;

  ComPtr<IMFMediaBuffer> buffer;
  HRESULT hr = pSample->ConvertToContiguousBuffer(&buffer);

  BYTE* data = nullptr;
  DWORD maxDataLength = 0;
  DWORD currentDataLength = 0;

  hr = buffer->Lock(&data, &maxDataLength, &currentDataLength);

  UINT32 pitch = width * sizeof(UINT32);

  D2D1_BITMAP_PROPERTIES bitmapProperties = {};
  ZeroMemory(&bitmapProperties, sizeof(bitmapProperties));
  bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
  bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

  ComPtr<ID2D1Bitmap> bitmap;
  m_renderTarget->CreateBitmap(D2D1::SizeU(width, height), data, pitch,
                               bitmapProperties, &bitmap);

  buffer->Unlock();

  return bitmap;
}

void DXHelper::RenderBitmapOnWindow(ComPtr<ID2D1Bitmap> pBitmap) {
  m_renderTarget->BeginDraw();
  m_renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
  m_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::MediumPurple));

  D2D1_SIZE_F renderTargetSize = m_renderTarget->GetSize();
  D2D1_SIZE_F bitmapSize = pBitmap->GetSize();

  D2D1_POINT_2F upperLeftCorner =
      D2D1::Point2F((renderTargetSize.width - bitmapSize.width) / 2.f,
                    (renderTargetSize.height - bitmapSize.height) / 2.f);

  m_renderTarget->DrawBitmap(
      pBitmap.Get(), D2D1::RectF(upperLeftCorner.x, upperLeftCorner.y,
                                 upperLeftCorner.x + bitmapSize.width,
                                 upperLeftCorner.y + bitmapSize.height));

  m_renderTarget->EndDraw();

  m_swapChain->Present(1, 0);

  return;
}
#include "VideoPlayer.h"

VideoPlayer::VideoPlayer(HWND &hwnd)
    : m_hwnd(hwnd), m_nRefCount(1), m_reader(nullptr) {
  Initialize();
}

//-----------------------------------------------------------------------------
// IUnknown Methods
//-----------------------------------------------------------------------------

STDMETHODIMP VideoPlayer::QueryInterface(REFIID iid, void **ppv) {
  if (ppv == nullptr) {
    return E_POINTER;
  }

  *ppv = nullptr;

  if (iid == IID_IUnknown || iid == IID_IMFAsyncCallback) {
    *ppv = static_cast<IMFAsyncCallback *>(this);
  } else if (iid == IID_IMFSourceReaderCallback) {
    *ppv = static_cast<IMFSourceReaderCallback *>(this);
  } else {
    return E_NOINTERFACE;
  }

  AddRef();

  return S_OK;
}

ULONG VideoPlayer::AddRef() { return InterlockedIncrement(&m_nRefCount); }

ULONG VideoPlayer::Release() {
  ULONG uCount = InterlockedDecrement(&m_nRefCount);
  if (uCount == 0) {
    delete this;
  }
  return uCount;
}

HRESULT VideoPlayer::Initialize() {
  HRESULT hr = MFStartup(MF_VERSION);
  if (FAILED(hr)) {
    return hr;
  }

  m_dxhelper = std::make_unique<DXHelper>(m_hwnd);
  if (m_dxhelper == nullptr) {
    return E_FAIL;
  }

  return S_OK;
}

void VideoPlayer::OpenURL(const WCHAR *sURL) {
  if (!sURL) return;

  ComPtr<IMFAttributes> pAttributes;
  MFCreateAttributes(pAttributes.GetAddressOf(), 1);

  // Enable hardware transforms and video processing
  pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);

  pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

  // Set the output media type to RGB32 format
  ComPtr<IMFMediaType> pMediaTypeOut;
  MFCreateMediaType(pMediaTypeOut.GetAddressOf());

  pMediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);

  pMediaTypeOut->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);

  // Set the asynchronous callback for the source reader
  pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK,
                          static_cast<IMFSourceReaderCallback *>(this));

  MFCreateSourceReaderFromURL(sURL, pAttributes.Get(), m_reader.GetAddressOf());

  // Set the output media type for the video stream
  m_reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr,
                                pMediaTypeOut.Get());

  m_reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr,
                       nullptr, nullptr);
  return;
}

void VideoPlayer::Pause() {
  if (!m_reader || !m_videoStreamIndex) {
    return;
  }

  if (m_paused) {
    m_paused = false;
    m_reader->ReadSample(m_videoStreamIndex, 0, nullptr, nullptr, nullptr,
                         nullptr);
  } else {
    m_paused = true;
  }
}

LONGLONG VideoPlayer::GetDuration() {
  LONGLONG duration = 0;

  PROPVARIANT var;
  PropVariantInit(&var);

  HRESULT hr = m_reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE,
                                                  MF_PD_DURATION, &var);
  if (SUCCEEDED(hr)) {
    hr = PropVariantToInt64(var, &duration);
    PropVariantClear(&var);
  }
  return duration;
}

void VideoPlayer::SetPosition(const LONGLONG &hnsPosition) {
  if (!m_reader) return;

  PROPVARIANT var;
  PropVariantInit(&var);
  var.vt = VT_I8;
  var.hVal.QuadPart = hnsPosition;

  m_reader->SetCurrentPosition(GUID_NULL, var);

  PropVariantClear(&var);
}

//-----------------------------------------------------------------------------
// Playback Methods
//-----------------------------------------------------------------------------

HRESULT VideoPlayer::OnReadSample(HRESULT hr, DWORD dwStreamIndex,
                                  DWORD dwStreamFlags, LONGLONG llTimestamp,
                                  IMFSample *pSample) {
  if (m_paused) {
    return S_OK;
  }

  if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
    OutputDebugStringA("EndOfStream\n");
    return S_OK;
  }

  m_videoStreamIndex = dwStreamIndex;

  ComPtr<ID2D1Bitmap> bitmap = m_dxhelper->CreateBitmapFromVideoSample(pSample);
  m_dxhelper->RenderBitmapOnWindow(bitmap);

  emit positionChanged(llTimestamp);

  hr = m_reader->ReadSample(dwStreamIndex, 0, NULL, NULL, NULL, NULL);

  return S_OK;
}


VideoPlayer::~VideoPlayer() { MFShutdown(); }

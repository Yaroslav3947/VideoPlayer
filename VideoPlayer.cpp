#include "VideoPlayer.h"

VideoPlayer::VideoPlayer(HWND &hwnd)
    : m_hwnd(hwnd),
      m_nRefCount(1),
      m_reader(nullptr),
      m_mediaReader(nullptr),
      m_soundEffect(nullptr) {
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

  m_mediaReader = std::make_unique<MediaReader>();
  if (m_mediaReader == nullptr) {
    return E_FAIL;
  }

  m_soundEffect= std::make_unique<SoundEffect>();
  if (m_soundEffect == nullptr) {
    return E_FAIL;
  }

  m_audio = std::make_unique<Audio>();
  if (m_audio == nullptr) {
    return E_FAIL;
  }

  return S_OK;
}

void VideoPlayer::OpenURL(const WCHAR *sURL) {
  if (!sURL) return;

  ComPtr<IMFAttributes> pAttributes;
  HRESULT hr = MFCreateAttributes(pAttributes.GetAddressOf(), 1);
  if (FAILED(hr)) {
    //throw hr;
  }

  // Enable hardware transforms and video processing
  pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
  pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

  // Set the asynchronous callback for the source reader
  pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK,
                          static_cast<IMFSourceReaderCallback *>(this));


  MFCreateSourceReaderFromURL(sURL, pAttributes.Get(), m_reader.GetAddressOf());
  if (FAILED(hr)) {
    //throw hr;
  }

  m_reader->ReadSample(MF_SOURCE_READER_ANY_STREAM, 0, nullptr, nullptr,
                       nullptr, nullptr);

  return;
}

void VideoPlayer::PlayPauseVideo() {
  if (!m_reader || !m_videoStreamIndex) {
    return;
  }

  if (m_isPaused) {
    m_audio->ResumeAudio();
    m_isPaused = false;
    m_reader->ReadSample(m_videoStreamIndex, 0, nullptr, nullptr, nullptr,
                         nullptr);
  } else {
    m_audio->SuspendAudio();
    m_isPaused = true;
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

void VideoPlayer::SetPosition(const LONGLONG &hnsNewPosition) {
  if (!m_reader) return;

  PROPVARIANT var;
  PropVariantInit(&var);
  var.vt = VT_I8;
  var.hVal.QuadPart = hnsNewPosition;

  m_reader->SetCurrentPosition(GUID_NULL, var);

  PropVariantClear(&var);
}

//-----------------------------------------------------------------------------
// Playback Methods
//-----------------------------------------------------------------------------

bool VideoPlayer::IsAudioSample(DWORD streamIndex) {
  ComPtr<IMFMediaType> pMediaType;
  HRESULT hr =
      m_reader->GetCurrentMediaType(streamIndex, pMediaType.GetAddressOf());
  if (SUCCEEDED(hr)) {
    GUID majorType;
    hr = pMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
    if (SUCCEEDED(hr)) {
      return (majorType == MFMediaType_Audio);
    }
  }

  return false;
}


HRESULT VideoPlayer::OnReadSample(HRESULT hr, DWORD dwStreamIndex,
                                  DWORD dwStreamFlags, LONGLONG llTimestamp,
                                  IMFSample *pSample) {
  if (m_isPaused) {
    return S_OK;
  }

  if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
    qDebug() << "EndOfStream";
    return S_OK;
  }

  static int cnt{0};

  qDebug() << ++cnt << dwStreamIndex << IsAudioSample(dwStreamIndex);

  if (!IsAudioSample(dwStreamIndex)) {
    ComPtr<ID2D1Bitmap> bitmap;
    bitmap = m_dxhelper->CreateBitmapFromVideoSample(pSample);
    m_dxhelper->RenderBitmapOnWindow(bitmap);

    emit positionChanged(llTimestamp);

  } else {
    // Proccess audio
    qDebug() << "Audio";
  }
  
  hr = m_reader->ReadSample(dwStreamIndex, 0, NULL, NULL, NULL, NULL);

  return S_OK;
}


VideoPlayer::~VideoPlayer() { MFShutdown(); }

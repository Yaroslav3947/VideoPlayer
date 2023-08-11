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

  m_soundEffect = std::make_unique<SoundEffect>();
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

  m_reader.Reset();

  ComPtr<IMFAttributes> pAttributes;
  HRESULT hr = MFCreateAttributes(pAttributes.GetAddressOf(), 1);

  // Enable hardware transforms and video processing
  hr = pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
  hr = pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

  // Set the asynchronous callback for the source reader
  hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK,
                               static_cast<IMFSourceReaderCallback *>(this));

  hr = MFCreateSourceReaderFromURL(sURL, pAttributes.Get(),
                                   m_reader.GetAddressOf());

  hr = GetWidthAndHeight();

  m_fps = GetFPS();

  ComPtr<IMFMediaType> pAudioType;
  hr = MFCreateMediaType(&pAudioType);

  hr = pAudioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
  hr = pAudioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);

  hr = m_reader->SetCurrentMediaType(0, nullptr, pAudioType.Get());

  ComPtr<IMFMediaType> pVideoType;
  hr = MFCreateMediaType(&pVideoType);

  hr = pVideoType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
  hr = pVideoType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);

  hr = m_reader->SetCurrentMediaType(1, nullptr, pVideoType.Get());

  m_audio->CreateDeviceIndependentResources();

  m_soundEffect->Initialize(m_audio->SoundEffectEngine(),
                            m_mediaReader->GetWaveFormat(m_reader));

  m_reader->ReadSample(MF_SOURCE_READER_ANY_STREAM, 0, nullptr, nullptr,
                       nullptr, nullptr);

  return;
}

void VideoPlayer::PlayPauseVideo() {
  m_isPaused = !m_isPaused;

  if (!m_isPaused) {
    m_audio->ResumeAudio();
    m_reader->ReadSample(m_videoStreamIndex, 0, nullptr, nullptr,
                         nullptr, nullptr);
  } else {
    m_audio->SuspendAudio();
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

HRESULT VideoPlayer::GetWidthAndHeight() {
  ComPtr<IMFMediaType> pMediaType;
  HRESULT hr = m_reader->GetCurrentMediaType(1, &pMediaType);
  if (SUCCEEDED(hr)) {
    hr = MFGetAttributeSize(pMediaType.Get(), MF_MT_FRAME_SIZE, &m_width,
                            &m_height);
  }
  return hr;
}

float VideoPlayer::GetFPS() {
  ComPtr<IMFMediaType> pMediaType = nullptr;
  HRESULT hr = m_reader->GetCurrentMediaType(1, &pMediaType);
  if (SUCCEEDED(hr)) {
    UINT32 numerator, denominator;
    hr = MFGetAttributeRatio(pMediaType.Get(), MF_MT_FRAME_RATE, &numerator,
                             &denominator);

    if (SUCCEEDED(hr) && denominator > 0) {
      return static_cast<float>(numerator) / static_cast<float>(denominator);
    }
  }

  return 0.0f;
}

HRESULT VideoPlayer::OnReadSample(HRESULT hr, DWORD dwStreamIndex,
                                  DWORD dwStreamFlags, LONGLONG llTimestamp,
                                  IMFSample *pSample) {
  if (m_isPaused) {
    return S_OK;
  }

  if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
    m_audio->SuspendAudio();
    return S_OK;
  }

  m_videoStreamIndex = dwStreamIndex;


  if (dwStreamIndex == 1) {
    ComPtr<ID2D1Bitmap> bitmap;
    bitmap =
        m_dxhelper->CreateBitmapFromVideoSample(pSample, m_width, m_height);
    m_dxhelper->RenderBitmapOnWindow(bitmap);

    emit positionChanged(llTimestamp / 100);

  } else if (dwStreamIndex == 0) {
    auto soundData = m_mediaReader->LoadMedia(pSample);
    m_soundEffect->PlaySound(soundData);
  }

  hr = m_reader->ReadSample(MF_SOURCE_READER_ANY_STREAM, 0, NULL, NULL, NULL,
                            NULL);

  return S_OK;
}

VideoPlayer::~VideoPlayer() { MFShutdown(); }

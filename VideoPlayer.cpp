#include "VideoPlayer.h"

VideoPlayer::VideoPlayer(HWND &hwnd)
    : m_hwnd(hwnd),
      m_nRefCount(1),
      m_reader(nullptr),
      m_mediaReader(nullptr),
      m_soundEffect(nullptr) {
  Init();
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

void VideoPlayer::Init() {
  winrt::check_hresult(MFStartup(MF_VERSION));

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
  desc.OutputWindow = m_hwnd;
  desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  desc.Windowed = true;

  m_dxhelper = std::make_unique<DXHelper>(desc);
  m_mediaReader = std::make_unique<MediaReader>();
  m_soundEffect = std::make_unique<SoundEffect>();
  m_audio = std::make_unique<Audio>();
}

void VideoPlayer::OpenURL(const WCHAR *sURL) {
  if (!sURL) return;

  InitReader(sURL);

  winrt::check_hresult(GetWidthAndHeight());

  InitAudioAndVideoTypes();

  InitAudio();
  StartPlayback();

  return;
}

void VideoPlayer::InitAudio() {
  m_audio->CreateDeviceIndependentResources();

  m_soundEffect->Initialize(m_audio->SoundEffectEngine(),
                            m_mediaReader->GetWaveFormat(m_reader));
}

void VideoPlayer::StartPlayback() {
  m_reader->ReadSample(MF_SOURCE_READER_ANY_STREAM, 0, nullptr, nullptr,
                       nullptr, nullptr);
}

void VideoPlayer::InitReader(const WCHAR *sURL) {
  m_reader.Reset();

  ComPtr<IMFAttributes> pAttributes;
  winrt::check_hresult(MFCreateAttributes(pAttributes.GetAddressOf(), 1));

  // Enable hardware transforms and video processing
  pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
  pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

  // Set the asynchronous callback for the source reader
  winrt::check_hresult(
      pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK,
                              static_cast<IMFSourceReaderCallback *>(this)));

  winrt::check_hresult(MFCreateSourceReaderFromURL(sURL, pAttributes.Get(),
                                                   m_reader.GetAddressOf()));
}

void VideoPlayer::InitAudioAndVideoTypes() {
  ComPtr<IMFMediaType> pAudioType;
  winrt::check_hresult(MFCreateMediaType(&pAudioType));

  pAudioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
  pAudioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);

  winrt::check_hresult(m_reader->SetCurrentMediaType(
      (DWORD)StreamIndex::audioStreamIndex, nullptr, pAudioType.Get()));

  ComPtr<IMFMediaType> pVideoType;
  MFCreateMediaType(&pVideoType);

  pVideoType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
  pVideoType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);

  winrt::check_hresult(m_reader->SetCurrentMediaType(
      (DWORD)StreamIndex::videoStreamIndex, nullptr, pVideoType.Get()));
}

void VideoPlayer::PlayPauseVideo() {
  m_isPaused = !m_isPaused;

  if (!m_isPaused) {
    m_reader->ReadSample(MF_SOURCE_READER_ANY_STREAM, 0, nullptr, nullptr,
                         nullptr,
                         nullptr);
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

  //winrt::check_hresult(m_reader->SetCurrentPosition(GUID_NULL, var));
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
    UINT32 numerator{0}, denominator{0};
    winrt::check_hresult(MFGetAttributeRatio(pMediaType.Get(), MF_MT_FRAME_RATE,
                                             &numerator, &denominator));

    if (denominator > 0) {
      return static_cast<float>(numerator) / static_cast<float>(denominator);
    }
  }

  return 0.0f;
}


    
    

HRESULT VideoPlayer::OnReadSample(HRESULT hr, DWORD dwStreamIndex,
                                  DWORD dwStreamFlags, LONGLONG llTimestamp,
                                  IMFSample *pSample) {

  std::lock_guard<std::mutex> lock(GetDxHelper()->GetResizeMtx());

  if (m_isPaused) {
    return S_OK;
  }

  if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
    emit endOfStream();
    return S_OK;
  }

  m_streamIndex = dwStreamIndex;

  if (dwStreamIndex == (DWORD)StreamIndex::videoStreamIndex) {
    ComPtr<ID2D1Bitmap> bitmap;
    bitmap =
        m_dxhelper->CreateBitmapFromVideoSample(pSample, m_width, m_height);
    m_dxhelper->RenderBitmapOnWindow(bitmap);

    emit positionChanged(llTimestamp / 100);

  } else if (dwStreamIndex == (DWORD)StreamIndex::audioStreamIndex) {
    auto soundData = m_mediaReader->LoadMedia(pSample);
    m_soundEffect->PlaySound(soundData);
  }

  winrt::check_hresult(m_reader->ReadSample(MF_SOURCE_READER_ANY_STREAM, 0,
                                            NULL, NULL, NULL, NULL));

  return S_OK;
}

VideoPlayer::~VideoPlayer() { MFShutdown(); }

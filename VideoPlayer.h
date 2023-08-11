#pragma once

#include <QObject>

#include "WinInclude.h"

#include "Audio/Audio.h"
#include "Audio/MediaReader.h"
#include "Audio/SoundEffect.h"

#include "DX/DXHelper.h"


class VideoPlayer : public QObject, public IMFAsyncCallback, public IMFSourceReaderCallback {
  Q_OBJECT
 public:
  using Ptr = QSharedPointer<VideoPlayer>;

  VideoPlayer(HWND& hwnd);
  virtual ~VideoPlayer();

  HRESULT Initialize();
  void OpenURL(const WCHAR* sURL);

  // Playback
  void PlayPauseVideo();

  void SetPosition(const LONGLONG& hnsPosition);
  HRESULT ConfigureDecoder(DWORD dwStreamIndex);

  LONGLONG GetDuration();
  inline bool GetIsPaused() const { return m_isPaused; }

  SoundEffect* GetSoundEffect() const { return m_soundEffect.get(); }
  DXHelper* GetDxHelper() const { return m_dxhelper.get(); }

 signals:
  void positionChanged(qint64 position);

  private:
  HRESULT GetWidthAndHeight();
   float GetFPS();

 protected:
  // IUnknown methods
  STDMETHODIMP QueryInterface(REFIID iid, void** ppv) override;
  STDMETHODIMP_(ULONG) AddRef() override;
  STDMETHODIMP_(ULONG) Release() override;

  // IMFAsyncCallback methods
  STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*) override { return S_OK; }
  STDMETHODIMP OnFlush(DWORD) override { return S_OK; }
  STDMETHODIMP GetParameters(DWORD*, DWORD*) override { return E_NOTIMPL; }
  STDMETHODIMP Invoke(IMFAsyncResult* pResult) override { return S_OK; }

  // IMFSourceReaderCallback methods
  HRESULT OnReadSample(HRESULT hr, DWORD dwStreamIndex, DWORD dwStreamFlags,
                       LONGLONG llTimestamp, IMFSample* pSample) override;

 private:

  ComPtr<IMFSourceReader> m_reader;
  std::unique_ptr<DXHelper> m_dxhelper;
  std::unique_ptr<MediaReader> m_mediaReader;
  std::unique_ptr<SoundEffect> m_soundEffect;
  std::unique_ptr<Audio> m_audio;

  long m_nRefCount;
  HWND m_hwnd;

  bool m_isPaused = false;
  DWORD m_videoStreamIndex = 0;
  LONGLONG m_currentPosition = 0;

  float m_fps = 0.0;
  UINT32 m_width = 0;
  UINT32 m_height = 0;
};
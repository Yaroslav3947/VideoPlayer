#pragma once

#include <mfapi.h>
#include <shlwapi.h>
#include <propvarutil.h>

#include "DXHelper.h"


class VideoPlayer : public QObject, public IMFAsyncCallback, public IMFSourceReaderCallback {
  Q_OBJECT
 public:
  using Ptr = QSharedPointer<VideoPlayer>;

  VideoPlayer(HWND& hwnd);
  virtual ~VideoPlayer();

  HRESULT Initialize();

  // Playback
  void PlayPauseVideo();
  void OpenURL(const WCHAR* sURL);

  LONGLONG GetDuration();
  void SetPosition(const LONGLONG& hnsPosition);

  inline bool GetIsPaused() const { return m_isPaused; };

 signals:
  void positionChanged(qint64 position);

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

  long m_nRefCount;
  HWND m_hwnd;

  bool m_isPaused = false;
  DWORD m_videoStreamIndex = 0;
};
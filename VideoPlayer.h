#pragma once

#include <mfapi.h>

#include <shlwapi.h>

#include "DXHelper.h"

class VideoPlayer : public IMFAsyncCallback, public IMFSourceReaderCallback {
 public:
  VideoPlayer(HWND &hwnd);
  virtual ~VideoPlayer();
  HRESULT Initialize();

  // Playback
  void OpenURL(const WCHAR *sURL);
  /* void Play();
   void Pause();*/

  // IUnknown methods
  STDMETHODIMP QueryInterface(REFIID iid, void **ppv) override;
  STDMETHODIMP_(ULONG) AddRef() override;
  STDMETHODIMP_(ULONG) Release() override;

  // IMFAsyncCallback methods
  STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *) override { return S_OK; }
  STDMETHODIMP OnFlush(DWORD) override { return S_OK; }
  STDMETHODIMP
  GetParameters(DWORD *, DWORD *) override { return E_NOTIMPL; }
  STDMETHODIMP
  Invoke(IMFAsyncResult *pResult) override { return S_OK; }

  // IMFSourceReaderCallback methods
  HRESULT OnReadSample(HRESULT hr, DWORD dwStreamIndex, DWORD dwStreamFlags,
                       LONGLONG llTimestamp, IMFSample *pSample) override;

 protected:
  ComPtr<IMFSourceReader> m_reader;
  std::unique_ptr<DXHelper> m_dxhelper;

  long m_nRefCount;
  HWND m_hwnd;

};
#pragma once

#include "../WinInclude.h"

class MediaReader {
 public:
  MediaReader();

  //std::vector<byte> LoadMedia(const WCHAR *sURL);
  //WAVEFORMATEX* GetOutputWaveFormatEx();

  std::vector<byte> LoadMedia(ComPtr<IMFSample> pSample);
  WAVEFORMATEX *GetWaveFormat(ComPtr<IMFSourceReader> reader);

 private:
  WAVEFORMATEX m_waveFormat;
};
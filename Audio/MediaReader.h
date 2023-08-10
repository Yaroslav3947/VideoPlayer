#pragma once

#include "../WinInclude.h"

class MediaReader {
 public:
  MediaReader();

  std::vector<byte> LoadMedia(const WCHAR *sURL);
  WAVEFORMATEX* GetOutputWaveFormatEx();

 private:
  WAVEFORMATEX m_waveFormat;
};
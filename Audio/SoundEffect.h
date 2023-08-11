#pragma once

#include "../WinInclude.h"

class SoundEffect {
 public:
  SoundEffect();

  void Initialize(ComPtr<IXAudio2> masteringEngine, WAVEFORMATEX* sourceFormat);

  void PlaySound(std::vector<byte> const& soundData);
  void ChangeVolume(const float& volume);

 private:
  bool m_audioAvailable;
  IXAudio2SourceVoice* m_sourceVoice;
  std::vector<byte> m_soundData;
};
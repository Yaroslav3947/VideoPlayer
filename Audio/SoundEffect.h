#pragma once

#include "../WinInclude.h"

class SoundEffect {
 public:
  SoundEffect();

  void Initialize(ComPtr<IXAudio2> masteringEngine, WAVEFORMATEX* sourceFormat,
                  std::vector<byte> const& soundData);

  void PlaySound(float volume);

 private:
  WORD m_nBlockAlign;
  bool m_audioAvailable;
  IXAudio2SourceVoice* m_sourceVoice;
  std::vector<byte> m_soundData;
};
#pragma once

#include "../WinInclude.h"

class SoundEffect {
 public:
  SoundEffect();

  void Initialize(ComPtr<IXAudio2> masteringEngine, WAVEFORMATEX* sourceFormat);

  float GetVolume() const { return m_volume; };
  bool IsMute() const { return std::abs(m_volume) < 0.001; };

  void Mute();
  void Unmute();
  void ChangeVolume(const float& volume);
  void PlaySound(std::vector<byte> const& soundData);

 private:
  bool m_audioAvailable;
  std::vector<byte> m_soundData;
  IXAudio2SourceVoice* m_sourceVoice;
  WAVEFORMATEX* m_sourceFormat;

  float m_volume;
  bool m_isPlaying;
};
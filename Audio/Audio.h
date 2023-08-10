#include "../WinInclude.h"

#pragma once

class Audio {
 public:
  Audio();

  void Initialize();
  void CreateDeviceIndependentResources();
  ComPtr<IXAudio2> MusicEngine();
  ComPtr<IXAudio2> SoundEffectEngine();
  void SuspendAudio();
  void ResumeAudio();

 private:
  bool m_audioAvailable;
  ComPtr<IXAudio2> m_musicEngine;
  ComPtr<IXAudio2> m_soundEffectEngine;
  IXAudio2MasteringVoice* m_musicMasteringVoice;
  IXAudio2MasteringVoice* m_soundEffectMasteringVoice;
};
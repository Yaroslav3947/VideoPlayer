#include "Audio.h"

Audio::Audio() : m_audioAvailable(false) {}

void Audio::Initialize() {}

void Audio::CreateDeviceIndependentResources() {
  UINT32 flags = 0;

  XAudio2Create(m_musicEngine.GetAddressOf(), flags);

#if defined(_DEBUG)
  XAUDIO2_DEBUG_CONFIGURATION debugConfiguration = {0};
  debugConfiguration.BreakMask = XAUDIO2_LOG_ERRORS;
  debugConfiguration.TraceMask = XAUDIO2_LOG_ERRORS;
  m_musicEngine->SetDebugConfiguration(&debugConfiguration);
#endif

  HRESULT hr = m_musicEngine->CreateMasteringVoice(&m_musicMasteringVoice);
  if (FAILED(hr)) {
    // Unable to create an audio device
    m_audioAvailable = false;
    return;
  }

  XAudio2Create(m_soundEffectEngine.GetAddressOf(), flags);

#if defined(_DEBUG)
  m_soundEffectEngine->SetDebugConfiguration(&debugConfiguration);
#endif

  m_soundEffectEngine->CreateMasteringVoice(&m_soundEffectMasteringVoice);

  m_audioAvailable = true;
}

ComPtr<IXAudio2> Audio::MusicEngine() { return m_musicEngine; }

ComPtr<IXAudio2> Audio::SoundEffectEngine() { return m_soundEffectEngine; }

void Audio::SuspendAudio() {
  if (m_audioAvailable) {
    m_musicEngine->StopEngine();
    m_soundEffectEngine->StopEngine();
  }
}

void Audio::ResumeAudio() {
  if (m_audioAvailable) {
    m_musicEngine->StartEngine();
    m_soundEffectEngine->StartEngine();
  }
}
#include "SoundEffect.h"

SoundEffect::SoundEffect() : m_audioAvailable(false) {}

void SoundEffect::Initialize(ComPtr<IXAudio2> masteringEngine,
                             WAVEFORMATEX* sourceFormat,
                             std::vector<byte> const& soundData) {
  m_soundData = soundData;

  if (masteringEngine == nullptr) {
    m_audioAvailable = false;
    return;
  }

  masteringEngine->CreateSourceVoice(&m_sourceVoice, sourceFormat);
  m_audioAvailable = true;
}

void SoundEffect::PlaySound(float volume) {
  XAUDIO2_BUFFER buffer = {0};

  if (!m_audioAvailable) {
    // Audio is not available so just return.
    return;
  }

  // Interrupt sound effect if it is currently playing.
  m_sourceVoice->Stop();
  m_sourceVoice->FlushSourceBuffers();

  // Queue the memory buffer for playback and start the voice.
  buffer.AudioBytes = (UINT32)m_soundData.size();
  buffer.pAudioData = m_soundData.data();
  buffer.Flags = XAUDIO2_END_OF_STREAM;

  m_sourceVoice->SetVolume(volume);
  m_sourceVoice->SubmitSourceBuffer(&buffer);
  m_sourceVoice->Start();
}
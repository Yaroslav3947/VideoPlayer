#include "SoundEffect.h"

SoundEffect::SoundEffect() : m_audioAvailable(false) {}

void SoundEffect::Initialize(ComPtr<IXAudio2> masteringEngine,
                             WAVEFORMATEX* sourceFormat,
                             std::vector<byte> const& soundData) {
  m_nBlockAlign = sourceFormat->nBlockAlign;
  m_soundData = soundData;

  if (masteringEngine == nullptr) {
    m_audioAvailable = false;
    return;
  }

  masteringEngine->CreateSourceVoice(&m_sourceVoice, sourceFormat);
  m_audioAvailable = true;
}

void SoundEffect::PlaySound(float volume) {

  if (!m_audioAvailable) {
    // Audio is not available so just return.
    return;
  }

  // Interrupt sound effect if it is currently playing.
  m_sourceVoice->Stop();
  m_sourceVoice->FlushSourceBuffers();

  size_t bufferSize = m_soundData.size();
  size_t blockAlign = m_nBlockAlign;

  // Queue the memory buffer for playback and start the voice.
  XAUDIO2_BUFFER buffer = {0};
  buffer.AudioBytes = static_cast<UINT32>(bufferSize / blockAlign) * blockAlign;
  buffer.pAudioData = reinterpret_cast<BYTE*>(m_soundData.data());
  buffer.Flags = XAUDIO2_END_OF_STREAM;
  buffer.LoopCount = XAUDIO2_LOOP_INFINITE; 


  m_sourceVoice->SetVolume(volume);
  m_sourceVoice->SubmitSourceBuffer(&buffer);
  m_sourceVoice->Start();
}

void SoundEffect::ChangeVolume(const float &volume) {
  if (m_audioAvailable) {
    m_sourceVoice->SetVolume(volume);
  }
}
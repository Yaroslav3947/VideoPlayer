#include "SoundEffect.h"

SoundEffect::SoundEffect()
    : m_audioAvailable(false),
      m_volume{1.0},
      m_isPlaying{false},
      m_sourceFormat{nullptr} {}

void SoundEffect::Initialize(ComPtr<IXAudio2> masteringEngine,
                             WAVEFORMATEX* sourceFormat) {
  m_sourceFormat = sourceFormat;
  if (masteringEngine == nullptr) {
    m_audioAvailable = false;
    return;
  }

  winrt::check_hresult(
      masteringEngine->CreateSourceVoice(&m_sourceVoice, m_sourceFormat));
  m_audioAvailable = true;
}

void SoundEffect::PlaySound(std::vector<byte> const& soundData) {
  m_soundData = soundData;

  if (!m_audioAvailable) {
    // Audio is not available so just return.
    return;
  }

  // Interrupt sound effect if it is currently playing.
  winrt::check_hresult(m_sourceVoice->Stop());
  winrt::check_hresult(m_sourceVoice->FlushSourceBuffers());

  XAUDIO2_BUFFER buffer = {0};
  buffer.AudioBytes = (UINT32)m_soundData.size();
  buffer.pAudioData = m_soundData.data();
  buffer.Flags = XAUDIO2_END_OF_STREAM;

  winrt::check_hresult(m_sourceVoice->SubmitSourceBuffer(&buffer));

  winrt::check_hresult(m_sourceVoice->Start());
}



void SoundEffect::ChangeVolume(const float& volume) {
  if (m_audioAvailable) {
    m_volume = volume;
    winrt::check_hresult(m_sourceVoice->SetVolume(volume));
  }
}

void SoundEffect::Mute() { ChangeVolume(0.0); }
void SoundEffect::Unmute() { ChangeVolume(1.0); }

#include "MediaReader.h"

MediaReader::MediaReader() {
  ZeroMemory(&m_waveFormat, sizeof(m_waveFormat));
}

WAVEFORMATEX *MediaReader::GetOutputWaveFormatEx() { return &m_waveFormat; }

std::vector<byte> MediaReader::LoadMedia(const WCHAR *sURL) {
  ComPtr<IMFSourceReader> reader;
  HRESULT hr =
      MFCreateSourceReaderFromURL(sURL, nullptr, reader.GetAddressOf());

  ComPtr<IMFMediaType> mediaType;
  hr = MFCreateMediaType(mediaType.GetAddressOf());

  hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);

  hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);

  hr = reader->SetCurrentMediaType(
      static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0,
      mediaType.Get());

  ComPtr<IMFMediaType> outputMediaType;
  hr = reader->GetCurrentMediaType(
      static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM),
      outputMediaType.GetAddressOf());

  UINT32 size = 0;
  WAVEFORMATEX *waveFormat;
  hr = MFCreateWaveFormatExFromMFMediaType(outputMediaType.Get(), &waveFormat,
                                           &size);

  CopyMemory(&m_waveFormat, waveFormat, sizeof(m_waveFormat));
  CoTaskMemFree(waveFormat);

  PROPVARIANT propVariant;
  hr = reader->GetPresentationAttribute(
      static_cast<DWORD>(MF_SOURCE_READER_MEDIASOURCE), MF_PD_DURATION,
      &propVariant);

  LONGLONG duration = propVariant.uhVal.QuadPart;
  unsigned int maxStreamLengthInBytes = static_cast<unsigned int>(
      ((duration * static_cast<ULONGLONG>(m_waveFormat.nAvgBytesPerSec)) +
       10000000) /
      10000000);

  std::vector<byte> fileData(maxStreamLengthInBytes);

  ComPtr<IMFSample> sample;
  ComPtr<IMFMediaBuffer> mediaBuffer;
  DWORD flags = 0;

  int positionInData = 0;
  bool done = false;

  while (!done) {
    sample = nullptr;
    mediaBuffer = nullptr;

    hr = reader->ReadSample(
        static_cast<uint32_t>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr,
        &flags, nullptr, sample.GetAddressOf());

    if (sample != nullptr) {
      hr = sample->ConvertToContiguousBuffer(mediaBuffer.GetAddressOf());

      byte *audioData = nullptr;
      DWORD sampleBufferLength = 0;

      hr = mediaBuffer->Lock(&audioData, nullptr, &sampleBufferLength);

      // Determine the actual copy size
      DWORD copySize =
          (positionInData + sampleBufferLength <= maxStreamLengthInBytes)
              ? sampleBufferLength
              : maxStreamLengthInBytes - positionInData;

      // Copy the audio data into the fileData vector
      std::memcpy(&fileData[positionInData], audioData, copySize);
      positionInData += copySize;

      mediaBuffer->Unlock();
    }

    if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
      done = true;
    }
  }

  return fileData;
}

#include "MediaReader.h"

std::vector<byte> MediaReader::LoadMedia(ComPtr<IMFSample> pSample) {
  ComPtr<IMFMediaBuffer> mediaBuffer;
  winrt::check_hresult(
      pSample->ConvertToContiguousBuffer(mediaBuffer.GetAddressOf()));

  BYTE* audioData = nullptr;
  DWORD sampleBufferLength = 0;

  winrt::check_hresult(
      mediaBuffer->Lock(&audioData, nullptr, &sampleBufferLength));

  std::vector<byte> fileData(sampleBufferLength);
  memcpy(fileData.data(), audioData, sampleBufferLength);

  winrt::check_hresult(mediaBuffer->Unlock());

  return fileData;
}

WAVEFORMATEX* MediaReader::GetWaveFormat(ComPtr<IMFSourceReader> reader) {
  ComPtr<IMFMediaType> outputMediaType;
  winrt::check_hresult(
      reader->GetCurrentMediaType(0, outputMediaType.GetAddressOf()));

  UINT32 size = 0;
  WAVEFORMATEX* pWaveFormat = nullptr;
  winrt::check_hresult(MFCreateWaveFormatExFromMFMediaType(
      outputMediaType.Get(), &pWaveFormat, &size));

  WAVEFORMATEX* waveFormat = (WAVEFORMATEX*)CoTaskMemAlloc(size);
  if (waveFormat) {
    ZeroMemory(waveFormat, size);
    CopyMemory(waveFormat, pWaveFormat, min(size, sizeof(WAVEFORMATEX)));
    CoTaskMemFree(pWaveFormat);
    return waveFormat;
  } else {
    CoTaskMemFree(pWaveFormat);
    return nullptr;
  }
}

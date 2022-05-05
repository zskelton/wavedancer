#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <Mmsystem.h>

#define bswap_64(x) _byteswap_uint64(x)
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_16(x) _byteswap_ushort(x)

struct riff_header {
  char chunk_id[4];
  uint32_t chunk_size;
  char format[4];
};

struct chunk_header {
  char chunk_id[4];
  uint32_t chunk_size;
};

struct fmt_header {
  uint16_t format_tag;
  uint16_t channels;
  uint32_t samples_per_sec;
  uint32_t avg_bytes_per_sec;
  uint16_t block_align;
  uint16_t bits_per_sample;
};

struct data_header {
  char chunk_id[4];
  uint32_t chunk_size;
};

void header() {
  printf("Skelton Wave File Reader\n");
  printf("Copyright (C) 2022 Skelton Networks\n");
  printf("\n");
}

int main(int argc, char *argv[]) {
  header();

  if (argc != 2) {
    printf("Usage: %s <file>\n", argv[0]);
    return 1;
  }

  // Open File
  FILE *fp = fopen(argv[1], "rb");
  if (fp == NULL) {
    printf("Failed to open file\n");
    return 1;
  }

  // Read RIFF Header
  struct riff_header riff_header;
  fread(&riff_header, sizeof(riff_header), 1, fp);

  // Read Chunk Header
  struct chunk_header chunk_header;
  fread(&chunk_header, sizeof(chunk_header), 1, fp);

  // Read Format Header
  struct fmt_header fmt_header;
  fread(&fmt_header, sizeof(fmt_header), 1, fp);

  // Read Data Header
  struct data_header data_header;
  fread(&data_header, sizeof(data_header), 1, fp);

  // Read Data
  int16_t *data = malloc(data_header.chunk_size);
  fread(data, data_header.chunk_size, 1, fp);

  // Close File
  fclose(fp);

  // Print Riff Header
  printf("RIFF Header:\n");
  printf("  Chunk ID: %.4s\n", riff_header.chunk_id);
  printf("  Chunk Size: %d\n", riff_header.chunk_size);
  printf("  Format: %.4s\n", riff_header.format);

  // Print Chunk Header
  printf("Chunk Header:\n");
  printf("  Chunk ID: %.4s\n", chunk_header.chunk_id);
  printf("  Chunk Size: %d\n", chunk_header.chunk_size);

  // Print Format Header
  printf("Format Header:\n");
  printf("  Format Tag: %d\n", fmt_header.format_tag);
  printf("  Channels: %d\n", fmt_header.channels);
  printf("  Samples Per Sec: %d\n", fmt_header.samples_per_sec);
  printf("  Avg Bytes Per Sec: %d\n", fmt_header.avg_bytes_per_sec);
  printf("  Block Align: %d\n", fmt_header.block_align);
  printf("  Bits Per Sample: %d\n", fmt_header.bits_per_sample);

  // Print Data Header
  printf("Data Header:\n");
  printf("  Chunk ID: %.4s\n", data_header.chunk_id);
  printf("  Chunk Size: %d\n", data_header.chunk_size);

  // Prepare Header
  printf("\n\n----------------------------------------------------\n");
  printf("Preparing Header...\n");

  HWAVEOUT hWaveOut;
  WAVEFORMATEX wfx;
  MMRESULT res;

  wfx.nSamplesPerSec = fmt_header.samples_per_sec;
  wfx.wBitsPerSample = fmt_header.bits_per_sample;
  wfx.nChannels = fmt_header.channels;
  wfx.nBlockAlign = fmt_header.block_align;
  wfx.wFormatTag = fmt_header.format_tag;
  wfx.nAvgBytesPerSec = fmt_header.avg_bytes_per_sec;

  res = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
  if (res != MMSYSERR_NOERROR) {
    printf("Error opening waveout device\n");
    return 1;
  }

  printf("Waveout device opened\n");
  printf("Playing...\n");

  // Play Data
  WAVEHDR wavehdr;
  wavehdr.lpData = (LPSTR)data;
  wavehdr.dwBufferLength = data_header.chunk_size;
  wavehdr.dwFlags = 0;
  wavehdr.dwBytesRecorded = 0;
  wavehdr.dwUser = 0;
  wavehdr.lpNext = NULL;
  wavehdr.dwFlags = WHDR_PREPARED;

  res = waveOutPrepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR));
  if (res != MMSYSERR_NOERROR) {
    printf("Error preparing waveout header\n");
    return 1;
  }

  res = waveOutWrite(hWaveOut, &wavehdr, sizeof(WAVEHDR));
  if (res != MMSYSERR_NOERROR) {
    printf("Error playing sound\n");
    return 1;
  }

  // Wait for sound to finish
  while (waveOutUnprepareHeader(hWaveOut, &wavehdr, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING) {
      Sleep(100);
  }

  res = waveOutClose(hWaveOut);
  if (res != MMSYSERR_NOERROR) {
    printf("Error closing waveout device\n");
    return 1;
  }

  printf("Done.\n");

  // Exit
  return 0;
}

// SPDX-License-Identifier: MIT
#include "../examples/PushToTalk/PlaybackLevel.h"
#include <assert.h>
int main() {
  int16_t voice[] = {-2900, 0, 2900, 0};
  auto level = levelPlayback(voice, 4);
  assert(level.gain > 6.8f && level.gain < 7.0f);
  assert(level.outputPeak > 19900 && level.outputPeak <= 20000);
  assert(voice[0] == -voice[2]);
  int16_t offset[] = {-1900, 1000, 3900, 1000};
  level = levelPlayback(offset, 4);
  assert(offset[1] == 0 && offset[3] == 0);
  assert(offset[0] == voice[0] && offset[2] == voice[2]);
  int16_t weak[] = {-200, 200};
  level = levelPlayback(weak, 2);
  assert(level.gain == 8.0f && weak[0] == -1600 && weak[1] == 1600);
  int16_t noise[] = {-20, 20};
  level = levelPlayback(noise, 2);
  assert(level.gain == 1.0f && noise[0] == -20 && noise[1] == 20);
  int16_t silent[] = {1200, 1200};
  level = levelPlayback(silent, 2);
  assert(level.outputPeak == 0 && silent[0] == 0 && silent[1] == 0);
  int16_t loud[] = {-32768, 32767};
  level = levelPlayback(loud, 2);
  assert(level.gain < 1 && level.outputPeak <= 20000);
  assert(loud[0] < 0 && loud[1] > 0);
  assert(levelPlayback(nullptr, 0).outputPeak == 0);
  assert(levelPlayback(nullptr, 10).outputPeak == 0);
}

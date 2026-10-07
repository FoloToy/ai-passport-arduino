// SPDX-License-Identifier: MIT
#include "detail/ButtonState.h"
#include <assert.h>
#include <stdint.h>
using namespace folotoy;
int main() {
  using detail::decodeButton;
  assert(decodeButton(0) == Button::Up);
  assert(decodeButton(149) == Button::Up);
  assert(decodeButton(150) == Button::Down);
  assert(decodeButton(446) == Button::Down);
  assert(decodeButton(447) == Button::Ok);
  assert(decodeButton(1899) == Button::Ok);
  assert(decodeButton(1900) == Button::None);
  assert(decodeButton(3300) == Button::None);
  detail::ButtonState state;
  state.reset(0, 25, 500);
  state.update(Button::Up, 10);
  state.update(Button::None, 20); // Bounce must not generate a press.
  assert(state.pressed() == Button::None);
  state.update(Button::Up, 30);
  state.update(Button::Up, 55);
  assert(state.pressed() == Button::Up);
  state.update(Button::Up, 60);
  assert(state.pressed() == Button::None);
  state.update(Button::None, 100);
  state.update(Button::None, 125);
  assert(state.released() == Button::Up && state.clicked() == Button::Up);
  state.update(Button::Ok, 200);
  state.update(Button::Ok, 225);
  state.update(Button::Ok, 725);
  assert(state.longPressed() == Button::Ok);
  state.update(Button::Ok, 800);
  assert(state.longPressed() == Button::None);
  state.update(Button::None, 810);
  state.update(Button::None, 835);
  assert(state.clicked() == Button::None);
  // Sparse polling on release must not turn a long hold into a click.
  state.reset(0, 25, 500);
  state.update(Button::Down, 0);
  state.update(Button::Down, 25);
  state.update(Button::None, 900);
  state.update(Button::None, 925);
  assert(state.clicked() == Button::None);
  // Debounce and long press remain correct across uint32_t timer rollover.
  state.reset(UINT32_MAX - 15, 25, 500);
  state.update(Button::Up, UINT32_MAX - 15);
  state.update(Button::Up, 10);
  assert(state.pressed() == Button::Up);
  state.update(Button::Up, 510);
  assert(state.longPressed() == Button::Up);
  assert(detail::batteryMillivolts(0, 0) == 0);
  assert(detail::batteryMillivolts(0x30, 0) == 3840);
  assert(detail::batteryMillivolts(0xF0, 0) == 3840); // Reserved high bits masked.
  assert(detail::batteryMillivolts(0x3F, 0xFF) == 5119);
}

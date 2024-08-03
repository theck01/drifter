
#include "input-event.h"

uint16_t button_mask = 0b1111111111110000;
uint16_t action_mask = 0b0000000000001111;

input_event create_input_event(input_action_e action, input_button_e button) {
  uint16_t action_int = (uint16_t)action;
  uint16_t button_int = (uint16_t)button;
  return (button_int | action_int);
}

input_event create_nil_event(void) {
  return 0;
}

bool input_event_is_nil(input_event e) {
  return e == 0;
}

input_action_e input_event_action(input_event e) {
  return (input_action_e)(e & action_mask);
}

input_button_e input_event_button(input_event e) {
  return (input_button_e)(e & button_mask);
}

direction_e input_button_to_direction(input_button_e e) {
  return (direction_e)((e >> 4) & 0xF);
}

bool input_button_is_dpad(input_button_e btn) {
  return btn >= D_DOWN && btn <= D_LEFT;
}

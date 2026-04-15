// Wokwi Custom Chip - For docs and examples see:
// https://docs.wokwi.com/chips-api/getting-started
//
// SPDX-License-Identifier: MIT
// Copyright 2023 R.S. PATHIRAGE

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

#define VOLTS_TO_CM 100.0f

typedef struct {
  pin_t pin_trig;
  pin_t pin_echo;
  pin_t pin_dist_in;
  timer_t transmit_timer;
  timer_t echo_timer;
  uint32_t trig_high_time;
  float pending_distance_cm;
} chip_state_t;

// 3: end the echo pulse
void echo_timer_done(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;
  pin_write(chip->pin_echo, LOW);
}

// 2: simulate 2ms delay where sensor fires 40kHz sonic burst
void transmit_timer_done(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;

  // calculate duration
  uint32_t echo_duration_us = (uint32_t)(chip->pending_distance_cm * 58.0f);

  // echo HIGH and start timer to set LOW
  pin_write(chip->pin_echo, HIGH);
  timer_start(chip->echo_timer, echo_duration_us, false);
}

// 1: listen for the TRIG pulse
void trig_changed(void *user_data, pin_t pin, uint32_t value) {
  chip_state_t *chip = (chip_state_t*)user_data;

  if (value == HIGH) {
    // start timer
    chip->trig_high_time = get_sim_nanos();
  } else {
    // calculate the high pulse duraion
    uint32_t pulse_duration_ns = get_sim_nanos() - chip->trig_high_time;
    if (pulse_duration_ns >= 10000) {
      // read voltage
      float voltage = pin_adc_read(chip->pin_dist_in);

      // voltage to distance
      float distance_cm = voltage * VOLTS_TO_CM;

      // clamp distance (to mimic the HC-SR04 sensor)
      if (distance_cm < 2.0f) distance_cm = 2.0f;
      if (distance_cm > 400.0f) distance_cm = 400.0f;
      
      chip->pending_distance_cm = distance_cm;

      // start HW transmition delay
      timer_start(chip->transmit_timer, 2000, false);
    }
  }
}

void chip_init() {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  // initialze pins
  chip->pin_trig = pin_init("TRIG", INPUT);
  chip->pin_echo = pin_init("ECHO", OUTPUT);
  chip->pin_dist_in = pin_init("DIST_IN", ANALOG); // used for analog read

  // set up pin watcher
  const pin_watch_config_t config = {
    .edge = BOTH,
    .pin_change = trig_changed,
    .user_data = chip,
  };
  pin_watch(chip->pin_trig, &config);

  // Initialize timers
  const timer_config_t transmit_config = { .callback = transmit_timer_done, .user_data = chip };
  chip->transmit_timer = timer_init(&transmit_config);

  const timer_config_t echo_config = { .callback = echo_timer_done, .user_data = chip };
  chip->echo_timer = timer_init(&echo_config);

  // Ensure ECHO starts LOW
  pin_write(chip->pin_echo, LOW);

  printf("Hello from custom chip!\n");
}

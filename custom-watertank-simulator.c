#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

// --- TANK CONFIGURATION ---
#define TANK_HEIGHT_CM 200.0f
#define SENSOR_CLEARANCE_CM 20.0f   // Space from sensor to max water level
#define PUMP_FILL_RATE 20.0f        // cm/sec filled when pump is HIGH
#define CM_TO_VOLTS 0.01f           // 100cm = 1.0V

typedef struct {
  pin_t pin_pump_in;
  pin_t pin_lvl_out;
  
  uint32_t attr_use_sw;
  uint32_t attr_use_rate;
  uint32_t attr_leak_sw;
  uint32_t attr_leak_rate;
  
  timer_t physics_timer;
  float water_level_cm;
} chip_state_t;

// The Physics Loop (Runs every 100ms)
void physics_timer_done(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;
  float dt = 0.1f; // 100ms time step

  // 1. Calculate Fill Rate
  float current_fill = (pin_read(chip->pin_pump_in) == HIGH) ? PUMP_FILL_RATE : 0.0f;
  
  // 2. High-Precision Usage Drain Rate
  float current_use = 0.0f;
  if (attr_read(chip->attr_use_sw) == 1) {
    // Read integer (e.g., 15000) and convert to float (1.5000)
    current_use = (float)attr_read(chip->attr_use_rate) / 10000.0f;
  }

  // 3. High-Precision Leak Drain Rate
  float current_leak = 0.0f;
  if (attr_read(chip->attr_leak_sw) == 1) {
    // Read integer (e.g., 5000) and convert to float (0.5000)
    current_leak = (float)attr_read(chip->attr_leak_rate) / 10000.0f;
  }

  // 4. Apply physics
  chip->water_level_cm += (current_fill - current_use - current_leak) * dt;

  // 5. Clamp limits
  if (chip->water_level_cm > TANK_HEIGHT_CM) chip->water_level_cm = TANK_HEIGHT_CM;
  if (chip->water_level_cm < 0.0f) chip->water_level_cm = 0.0f;

  // 6. Calculate distance to water
  float distance_cm = SENSOR_CLEARANCE_CM + (TANK_HEIGHT_CM - chip->water_level_cm);

  // 7. Output DAC voltage
  float out_voltage = distance_cm * CM_TO_VOLTS;
  pin_dac_write(chip->pin_lvl_out, out_voltage);
}

void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  chip->pin_pump_in = pin_init("PUMP_IN", INPUT_PULLDOWN);
  chip->pin_lvl_out = pin_init("LVL_OUT", ANALOG); 

  chip->attr_use_sw    = attr_init("use_sw", 0);
  chip->attr_use_rate  = attr_init("use_rate", 15000); // 1.5000 cm/s
  chip->attr_leak_sw   = attr_init("leak_sw", 0);
  chip->attr_leak_rate = attr_init("leak_rate", 5000);  // 0.5000 cm/s

  const timer_config_t physics_config = { .callback = physics_timer_done, .user_data = chip };
  chip->physics_timer = timer_init(&physics_config);
  timer_start(chip->physics_timer, 100000, true); 

  chip->water_level_cm = 100.0f; 
}
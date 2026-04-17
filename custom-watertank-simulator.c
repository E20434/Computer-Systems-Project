#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include "scenario.h" // Import your automated script

// --- MODE TOGGLE ---
// 0 = Manual Mode (UI Sliders)
// 1 = Automated Mode (Reads from scenario.h)
#define SIMULATION_MODE 0 

// --- TANK CONFIGURATION ---
#define TANK_HEIGHT_CM 200.0f
#define SENSOR_CLEARANCE_CM 20.0f   
#define PUMP_FILL_RATE 20.0f        
#define CM_TO_VOLTS 0.01f           

typedef struct {
  pin_t pin_pump_in;
  pin_t pin_lvl_out;
  pin_t pin_use_out;
  
  uint32_t attr_use_sw;
  uint32_t attr_use_rate;
  uint32_t attr_leak_sw;
  uint32_t attr_leak_rate;
  
  timer_t physics_timer;
  float water_level_cm;

  // Automated Mode Tracking Variables
  int current_event_index;
  uint32_t auto_use_sw;
  uint32_t auto_use_rate;
  uint32_t auto_leak_sw;
  uint32_t auto_leak_rate;
} chip_state_t;

// The Physics Loop (Runs every 100ms)
void physics_timer_done(void *user_data) {
  chip_state_t *chip = (chip_state_t*)user_data;
  float dt = 0.1f; 

  // --- AUTOMATED EVENT PROCESSOR ---
  if (SIMULATION_MODE == 1) {
    uint32_t current_time_ms = (uint32_t)(get_sim_nanos() / 1000000ULL);
    
    // Check if it is time to trigger the next event in the script
    while (chip->current_event_index < scenario_length && 
           scenario[chip->current_event_index].time_ms <= current_time_ms) {
      
      sim_event_t ev = scenario[chip->current_event_index];
      
      if (ev.action == ACT_USE_SW) chip->auto_use_sw = ev.value;
      else if (ev.action == ACT_USE_RATE) chip->auto_use_rate = ev.value;
      else if (ev.action == ACT_LEAK_SW) chip->auto_leak_sw = ev.value;
      else if (ev.action == ACT_LEAK_RATE) chip->auto_leak_rate = ev.value;

      chip->current_event_index++;
    }
  }

  // 1. Calculate Fill Rate
  float current_fill = (pin_read(chip->pin_pump_in) == HIGH) ? PUMP_FILL_RATE : 0.0f;
  
  // 2. High-Precision Usage Drain Rate & State Output
  float current_use = 0.0f;
  uint32_t active_use_sw = (SIMULATION_MODE == 1) ? chip->auto_use_sw : attr_read(chip->attr_use_sw);
  uint32_t active_use_rate = (SIMULATION_MODE == 1) ? chip->auto_use_rate : attr_read(chip->attr_use_rate);

  if (active_use_sw == 1) {
    current_use = (float)active_use_rate / 10000.0f;
    pin_write(chip->pin_use_out, HIGH); 
  } else {
    pin_write(chip->pin_use_out, LOW);  
  }

  // 3. High-Precision Leak Drain Rate
  float current_leak = 0.0f;
  uint32_t active_leak_sw = (SIMULATION_MODE == 1) ? chip->auto_leak_sw : attr_read(chip->attr_leak_sw);
  uint32_t active_leak_rate = (SIMULATION_MODE == 1) ? chip->auto_leak_rate : attr_read(chip->attr_leak_rate);

  if (active_leak_sw == 1) {
    current_leak = (float)active_leak_rate / 10000.0f;
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
  chip->pin_use_out = pin_init("USE_OUT", OUTPUT);

  chip->attr_use_sw    = attr_init("use_sw", 0);
  chip->attr_use_rate  = attr_init("use_rate", 15000); 
  chip->attr_leak_sw   = attr_init("leak_sw", 0);
  chip->attr_leak_rate = attr_init("leak_rate", 5000); 

  // Initialize automated tracking
  chip->current_event_index = 0;
  chip->auto_use_sw = 0;
  chip->auto_use_rate = 15000;
  chip->auto_leak_sw = 0;
  chip->auto_leak_rate = 5000;

  const timer_config_t physics_config = { .callback = physics_timer_done, .user_data = chip };
  chip->physics_timer = timer_init(&physics_config);
  timer_start(chip->physics_timer, 100000, true); 

  chip->water_level_cm = 100.0f; 
  pin_write(chip->pin_use_out, LOW);
}

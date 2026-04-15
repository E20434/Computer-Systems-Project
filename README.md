```markdown
# ESP32 Custom Ultrasonic Sensor Simulation

This project simulates an ESP32 reading a custom HC-SR04 ultrasonic sensor in VS Code. The distance is dynamically controlled using a potentiometer.

## Required VS Code Extensions
1. **PlatformIO IDE** (Compiles the ESP32 C++ code)
2. **Wokwi Simulator** (Runs the simulation)

## How to Run the Project

**1. Build the ESP32 Firmware**
Click the PlatformIO **Build** button (the `✓` checkmark icon in the bottom blue status bar) to compile `main.cpp`. Wait for the `SUCCESS` message.

**2. Install the Wokwi CLI**
The custom sensor is written in C and must be compiled into a WebAssembly (`.wasm`) file. Open a PowerShell terminal in VS Code and run:
```powershell
iwr [https://wokwi.com/ci/install.ps1](https://wokwi.com/ci/install.ps1) -useb | iex
```
*(Note: Close and reopen your terminal after this installs so Windows recognizes the command).*

**3. Compile the Custom Chip**
Make sure your terminal is in the project's root folder, then run:
```powershell
wokwi-cli chip compile custom-ultrasonic-sensor.c -o custom-ultrasonic-sensor.chip.wasm
```

**4. Start the Simulator**
Open the `diagram.json` file. Click the **Wokwi Play button** in the top-right corner of the editor (or press `Ctrl+Shift+P` and select `Wokwi: Start Simulator`). 

The simulation and Serial Monitor will start automatically. Turn the potentiometer knob to change the sensor's distance reading.

## Editing the Circuit Diagram
The free version of the Wokwi VS Code extension does not support graphical drag-and-drop editing. To change the circuit diagram:
1. Go to [wokwi.com](https://wokwi.com) and build your updated circuit in the web browser.
2. Click the `{}` (Code) tab in the web editor to view the underlying JSON.
3. Copy the updated code and paste it into your local `diagram.json` file.
```
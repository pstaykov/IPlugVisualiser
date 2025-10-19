# IPlugVisualizer

IPlugVisualizer is an audio-reactive VST3 plugin built with the [iPlug2](https://github.com/iPlug2/iPlug2) framework.  
It visualizes the incoming audio signal as a field of dynamically moving particles that expand and contract in response to the sound’s energy.

## Overview

The plugin implements a particle system rendered entirely within the iPlug2 graphics framework.  
Each particle reacts to the RMS level of the audio input, creating a pulsing, heartbeat-like motion.  
The visualization parameters are adjustable in real time through user-accessible controls.

## Features

- Audio-reactive particle field with smooth animation
- Adjustable pulse strength and speed parameters
- Real-time visualization directly inside the host environment
- Lightweight C++ implementation using iPlug2’s `IGraphics` and `ISender` modules
- Compatible with VST3 format (extendable to CLAP or AU)

## Technical Details

- **Framework:** iPlug2 (C++17)
- **Audio Processing:** RMS measurement in the DSP block  
  (RMS values transmitted to the UI via `SendControlMsgFromDelegate`)
- **Rendering:** CPU-based 2D particle system using `IControl`
- **Parameters:**  
  - `Pulse Strength` – controls the amplitude of particle motion  
  - `Pulse Speed` – adjusts responsiveness and damping

## Building

### Requirements
- [iPlug2](https://github.com/iPlug2/iPlug2) cloned to a local directory
- Visual Studio 2022 (Windows) or Xcode (macOS)
- VST3 SDK (included with iPlug2)

### Build Instructions
1. Place this project folder inside the `Examples/` directory of your iPlug2 checkout.  
   Example: iPlug2/Examples/IPlugVisualizer/
2. Open the Visual Studio or Xcode project file corresponding to your platform.
3. Build the target `IPlugVisualizer-vst3`.
4. The compiled plugin will be available under `build-win/vst3/x64/Release/IPlugVisualizer.vst3`
(or the equivalent macOS path).

## Usage

Load the plugin in any VST3-compatible host (e.g., Ableton Live, Reaper, FL Studio).  
Feed it an audio signal, and observe the responsive particle field.  
Adjust the sliders at the bottom of the interface to fine-tune the behavior:
- **Strength** – increases or decreases the particle displacement amplitude
- **Speed** – controls how quickly particles react and return toward the center

## Repository Structure

IPlugVisualizer/
│
├── src/ # Source code (.cpp / .h)
├── resources/ # Fonts, images, and other assets
├── config/ # iPlug2 project configuration
├── scripts/ # Build and packaging scripts
├── projects/ # Xcode project files (optional)
├── README.md
├── LICENSE
└── .gitignore

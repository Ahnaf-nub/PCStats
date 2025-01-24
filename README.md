# PCStats

This project combines a Pomodoro timer, system resource stats display, and animated GIF playback on an OLED screen. It also includes a buzzer for alerts when the Pomodoro timer completes.

## Features

### 1. **Pomodoro Timer**
- Displays a countdown timer for the Pomodoro technique (25-minute focus session).
- Includes a progress bar to visualize elapsed time.
- Plays a buzzer sound when the timer ends.

### 2. **System Stats Display**
- Fetches and displays real-time system stats such as:
  - CPU usage
  - RAM usage
  - Disk usage
  - Current time and day

### 3. **GIF Playback**
- Displays a series of frames as an animated GIF.
- Switches to GIF mode after displaying system stats for 5 seconds.

### 4. **Mode Switching**
- Switch between modes using a button.
  - Modes: System Stats, Pomodoro Timer, GIF Playback.
- Stops the buzzer sound when switching modes.
## Hardware
- Xiao ESP32 s3 microcontroller.
- OLED display (SH1106).
- Passive buzzer.
- Button for mode switching.

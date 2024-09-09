# imp-project
Final project for microcontrollers course at VUT FIT.

## Overview
This project implements a menu for a fictional alarm app, using the ESP32 microcontroller, an SPI display, and the APDS-9960 gesture sensor.

## Features
- Gesture-controlled navigation for an alarm application.
- User interface displayed on an SPI screen.
- Alarm time setting saved to internal memory.
- Predefined gestures to switch between screens and adjust settings.

## Project Functionality
1. **Home Screen**: Displays the current alarm time. If no time is set, it defaults to 08:00 on startup. Gestures allow users to access the help screen or settings menu.
2. **Help Screen**: Lists all available gestures and their respective functions. Users can scroll through the list if it's too long for the display screen.
3. **Alarm Settings Screen**: Allows the user to adjust the alarm time. The user can toggle between setting the hours and minutes using LEFT/RIGHT gestures and modify values using UP/DOWN gestures. The alarm time is saved by performing the FAR gesture.

## Pin Connections
### Display Pin Configuration
| Display Pin  | ESP32 Pin  |
|--------------|------------|
| GND          | GND        |
| VCC          | 3.3V       |
| D0           | GPIO18     |
| D1           | GPIO23     |
| RES          | GPIO14     |
| DC           | GPIO27     |
| CS           | GPIO5      |

### Gesture Sensor Pin Configuration
| Sensor Pin   | ESP32 Pin  |
|--------------|------------|
| GND          | GND        |
| VCC          | 3.3V       |
| SDA          | GPIO17     |
| SCL          | GPIO16     |

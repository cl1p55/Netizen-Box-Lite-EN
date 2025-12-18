# Netizen-Box-Lite-EN
# SmartAirBox Lite EN - Arduino-Based Intelligent Air Purification and Monitoring System

## Project Introduction
This project is an Arduino-based environmental monitoring system capable of real-time detection of PM2.5, carbon monoxide (CO), methane (CH4), and carbon dioxide (CO2) concentrations. It displays air quality status and triggers alarms when necessary. The goal is to protect people from air pollution hazards by controlling sensor-driven hardware devices.

## Features

- **Air Quality Monitoring**: Real-time monitoring of PM2.5, CO, CH4,and CO2 concentrations.
- **Alarm Functionality**: Provides warnings and extended alarms when pollutant concentrations exceed safe thresholds.
- **Ventilation Control**: Automatically activates ventilation devices to improve air quality.
- **LCD Display**: Real-time display of sensor readings with air quality indications.
- **Serial Data Output**: Supports debugging information output through serial communication.
- **Error Detection**: Integrated sensor error handling.

## System Components

### Hardware

- LiquidCrystal_I2C LCD (16x2)
- PM2.5 Sensor
- MQ-4 Methane Sensor
- JW01 Carbon Dioxide Sensor
- MQ-7 Carbon Monoxide Gas Sensor
- Dupont Wires, Circuit Board
- Programmable Relay
- Passive Buzzer
- Arduino Development Board

### Software and Frameworks
- Arduino IDE
- `LiquidCrystal_I2C` and `SoftwareSerial` libraries

## Quick Start

### Environment Setup

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Clone the repository to your local machine:
   ```bash
   git clone https://github.com/wym08281016/07--02--24.git
   ```
3. Configure the corresponding communication.

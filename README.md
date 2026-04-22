# STM32F407 + SIM808 | GPS Tracker via SMS

[![STM32](https://img.shields.io/badge/STM32-F407-blue)](https://www.st.com)
[![SIM808](https://img.shields.io/badge/SIM808-GSM%2FGPS-orange)](https://www.simcom.com)

## What it does

When **YOU** call this device, it:
1. Detects your phone number
2. Fetches live GPS coordinates  
3. Sends you a **Google Maps link** via SMS

## Hardware

| STM32F407 Pin | SIM808 Pin | Function |
|---------------|------------|----------|
| PA2 (TX) | RX | UART2 Transmit |
| PA3 (RX) | TX | UART2 Receive |
| PD12 | LED (optional) | Status Indicator |
| GND | GND | Common Ground |
| 3.3V/5V | VCC | Power |

## Quick Start

1. Change `MY_PHONE_NUMBER` in `main.c`
2. Connect SIM808 to USART2
3. Flash & call the device

## How it works

```
Incoming Call → Check Number → Get GPS → Send SMS
```

## 3 Scenarios (all tested)

- **Scenario 1** - Auto-call your phone
- **Scenario 2** - LED on when **YOU** call  
- **Scenario 3** - Send location via SMS

## SMS Example

> https://maps.google.com/?q=48.8584,2.2945

## Why this is useful

- Vehicle tracking
- Personal safety device  
- Pet/luggage locator

**No app needed. Just call, get location.**

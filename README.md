# STM32F407 + SIM808 | GPS Tracker via SMS

[![STM32](https://img.shields.io/badge/STM32-F407-blue)](https://www.st.com)
[![SIM808](https://img.shields.io/badge/SIM808-GSM%2FGPS-orange)](https://www.simcom.com)

## What it does

When **YOU** call this device, it:
1. Detects your phone number
2. Fetches live GPS coordinates  
3. Sends you a **Google Maps link** via SMS

## 🔧 Hardware

| Component | Pin |
|-----------|-----|
| STM32F407 | - |
| SIM808 | USART2 (PA2, PA3) |
| LED | PD12 |

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
- **Scenario 2** - LED on when you call  
- **Scenario 3** - Send location via SMS

## SMS Example

> https://maps.google.com/?q=48.8584,2.2945

## Why this is useful

- Vehicle tracking
- Personal safety device  
- Pet/luggage locator

**No app needed. Just call, get location.**

---

*Made for STM32F407 | SIM808 | Embedded C*

## 🏆 Why STM engineers will like it:

- **Clean macros** (`#define MY_PHONE_NUMBER`)
- **Modular functions** (GPIO, UART, GPS, GSM separated)
- **Commented scenarios** (shows understanding of 3 use cases)
- **Real hardware** (not just simulation)
- **Practical application** (solves real problem)

Go with **`stm32f407-sim808-gps-tracker`** — professional and searchable! 🚀

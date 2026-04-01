
# 🔊 AudioScout

> A compact autonomous robot that navigates by sound — not vision.

**Authors:** Adriana Crina Moraru & Elena Florentina Moraru  
**Year:** 2025

---

## About

AudioScout is a compact robot designed to observe its environment through sound, not images. It navigates rooms by continuously analyzing acoustic signals from two lateral microphones and steering toward the dominant sound source. A servo-mounted ultrasonic sensor handles collision avoidance, while an RGB LED reports status in real time.

---

## Hardware

### Movement
- 2× DC Motor 3V–6V with gearbox & wheel
- 1× Servomotor SG90, 180°, 9g
- L298N Dual H-Bridge Motor Driver

### Controller
- Arduino UNO R3 (ATMega328P-AU CH340G)
- Sensor Shield V5 expansion board

### Sensors
- 2× MAX4466 audio amplifier module with microphone
- HC-SR04 ultrasonic distance sensor

### Power & Extras
- 2× Samsung INR18650-25R Li-Ion 3.6V 2500mAh 20A
- RGB LED (common cathode)
- Rectangular chassis 10.8 × 23 cm, M3 & M1 screws + standoffs

---

## Pin Mapping

| Component | Function | Pin |
|---|---|---|
| HC-SR04 | Trig | A1 |
| HC-SR04 | Echo | A0 |
| Servomotor | PWM | A2 |
| MAX4466 Right mic | OUT | A3 |
| MAX4466 Left mic | OUT | A4 |
| DC Motor Right | Forward | D8 |
| DC Motor Right | Backward | D7 |
| DC Motor Right | PWM (ENB) | D6 |
| DC Motor Left | Forward | D4 |
| DC Motor Left | Backward | D2 |
| DC Motor Left | PWM (ENA) | D5 |
| RGB LED | Red | D9 |
| RGB LED | Green | D10 |
| RGB LED | Blue | D11 |

---

## LED Status

| Color | Meaning |
|---|---|
| 🔵 Blue | Scanning — actively searching for a sound source |
| 🟢 Green | Listening for sound, or maximum threshold reached (target found) |
| 🔴 Red | Obstacle detected (< 30 cm) or sound level too low to navigate |

---

## Logic Flow

**1. Obstacle check (priority 1)**  
Every cycle, the front distance is measured first. If distance < 30 cm: stop immediately, scan left & right with servo, turn toward the clearer side.

**2. Sound sampling**  
Robot stops for 3000 ms and reads both microphones continuously, recording min and max values per channel.

**3. Amplitude analysis**  
Amplitude = max − min per microphone. The difference between left and right channels indicates direction. Average amplitude is used for speed scaling.

**4. Speed & steering**  
Louder sound → slower approach for controlled targeting. Guards: no movement if avg < `25`; full stop if avg > `1000`. Steering triggers when the channel difference exceeds `20` units.

---

## Key Parameters

| Parameter | Value |
|---|---|
| Sample window | 3000 ms |
| Obstacle stop distance | 30 cm |
| Low sound cutoff | amplitude < 25 |
| Max sound cutoff | average > 1000 |
| Turn difference threshold | Δ > 20 |
| Speed High / Med / Low | 255 / 220 / 180 (PWM) |
| IDE | Arduino IDE |

---

## Challenges & Solutions

### Version 1 — Single rotating mic
A single MAX9814 microphone was mounted on the servomotor above the ultrasonic sensor. The plan was to rotate the mic, sample audio at each angle, and pick the loudest direction. **This failed** due to two issues:
- Servo motor noise interfered with microphone readings.
- Sound reflections from room objects caused confusion, making the robot spin in circles.

### Version 2 — Foam & cone shielding
Foam was added around the sensor and a cone was placed around the microphone to reduce reflections. The robot pointed in the correct direction in some tests, but the results were inconsistent — more coincidence than reliable logic.

### Version 3 — Final solution ✅
After researching working projects online, the approach was completely redesigned:
- Switched to **two fixed lateral MAX4466 microphones** (one left, one right), both facing outward.
- This placement eliminates servo-induced error and reduces motor noise interference.
- Crucially, **MAX4466 has no AGC (Automatic Gain Control)**, unlike the MAX9814. The AGC was the root cause of failure — it automatically equalized gain levels, making it impossible to detect which side received a stronger signal.

With fixed placement and no AGC, the robot successfully navigates toward a sound source.
> <img width="2048" height="1747" alt="image" src="https://github.com/user-attachments/assets/f141146d-9610-4940-a0dc-68ca59ebc31e" />

---

## References

- https://docs.arduino.cc/built-in-examples/
- https://www.sciencebuddies.org/science-fair-projects/project-ideas/Robotics_p048/robotics/sound-tracking-robot
- https://www.bitmi.ro/kituri-electronice/kit-robot-arduino-senzor-ultrasonic-hc-sr04-10083.html
- https://dewesoft.com/blog/sound-measurement-with-microphone-sensors

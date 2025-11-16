# STM32 Parking Sensor with FreeRTOS

This is a demo project based on the documentation from my **practice placement** at Preh Romania Automotive Technologies SRL.

The project consists of a functional parking sensor built on an STM32 Nucleo development board. It uses an HC-SR04 ultrasonic sensor to measure distance and a buzzer to provide audible alerts.

---

## 🛠️ Architecture & Logic

The application is built on the **FreeRTOS** real-time operating system and divides its logic into two main tasks to efficiently manage measurements and alerts.

### 1. The Measurement Task (Trig & Echo)

* **Trigger Pulse:** One task is responsible for sending a `TRIG` pulse to the HC-SR04 sensor every 60ms.
* **Time Measurement (Echo):** A second task (or the same one, depending on the logic) waits for the response on the `ECHO` pin.
* **Distance Calculation:** I used a timer (TIM6) configured to count in microseconds (similar to the `micros()` function on Arduino) to precisely measure the `ECHO` pulse duration. The distance is then calculated using the formula `duration_in_us / 58 = distance_in_cm`.

### 2. The Alert Task (Buzzer)

* **PWM Control:** Based on the calculated distance, the measurement task adjusts a **PWM** signal sent to the buzzer.
* **Progressive Logic:** The frequency and duration of the audible beeps change based on how close the obstacle is:
    * **Long distance (> 60-70 cm):** Slow, infrequent beeps.
    * **Medium distance (20-50 cm):** Medium-frequency beeps.
    * **Short distance (< 10-20 cm):** Fast, near-continuous beeps indicating imminent danger.

---

## 💡 Key Concepts

* **Embedded C Programming:** Using HAL libraries for peripheral control.
* **Real-Time Operating Systems (RTOS):** Managing concurrent tasks with FreeRTOS.
* **Peripheral Control:**
    * **GPIO:** Input (ECHO) and Output (TRIG) pins.
    * **TIM (Timers):** Using a timer for precise time measurement (Input Capture or manual `micros()` counter).
    * **PWM:** Generating a variable duty-cycle signal to control the buzzer.
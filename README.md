# STM32 Parking Sensor with FreeRTOS

This repository contains the complete source code for the Parking Assist System developed during my **practice placement** at Preh Romania Automotive Technologies SRL.

The project consists of a functional parking sensor built on an STM32 Nucleo development board. It uses an HC-SR04 ultrasonic sensor to measure distance and a buzzer to provide progressive audible alerts based on obstacle proximity.

---

## 🛠️ Architecture & Logic

The application is built on the **FreeRTOS** real-time operating system and divides its logic into two main tasks to efficiently manage measurements and alerts, following proper RTOS design principles.

### 1. The Sensor Task (`StartSensorTask`)

* **Measurement Cycle:** This task manages the entire interaction with the HC-SR04 hardware. It sends a 10µs `TRIG` pulse and waits for the response on the `ECHO` pin.
* **Precise Timing:** A timer (TIM6) is configured to count in microseconds, acting as a custom `micros()` function to measure the `ECHO` pulse duration accurately. 
* **Distance Calculation:** The duration is converted into centimeters using the standard formula (`duration_in_us / 58`). 
* **Data Sharing:** The calculated distance is stored in a shared global variable (`current_distance_cm`), making it accessible to the alert task. The task then yields for 60ms to allow the sensor to settle before the next reading.

### 2. The Alert Task (`StartBuzzerTask`)

* **Data Consumption:** This task continuously reads the latest distance calculated by the Sensor Task.
* **PWM Control:** Based on the distance, the task controls a **PWM** signal (via TIM2) to drive the buzzer.
* **Progressive Logic:** The frequency and duration of the audible beeps change dynamically depending on how close the obstacle is:
    * **Long distance (70-100 cm):** Slow, infrequent beeps.
    * **Medium distance (20-50 cm):** Medium-frequency beeps.
    * **Short distance (10-20 cm):** Fast beeps.
    * **Critical distance (<= 5 cm):** Continuous beep indicating imminent danger.

---

## 💡 Key Concepts Applied

* **Embedded C Programming:** Utilizing STM32 HAL libraries for low-level peripheral configuration.
* **Real-Time Operating Systems (RTOS):** Managing concurrent execution and separation of concerns using FreeRTOS tasks.
* **Peripheral Control:**
    * **GPIO:** Configuring and reading Input (ECHO) and Output (TRIG) pins.
    * **Hardware Timers (TIM):** Utilizing TIM6 as a precise microsecond counter for sensor timing, independent of the RTOS tick.
    * **PWM (Pulse Width Modulation):** Generating a variable signal (TIM2) to control the state and audio output of the buzzer.

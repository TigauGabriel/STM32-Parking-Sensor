/*
 * ============================================================================
 * main.c
 *
 * Parking Sensor Project (Practice Placement Preh Romania)
 * Student: Tigau Gabriel-Aurelian
 *
 * Demonstrates using FreeRTOS on an STM32 to read an
 * HC-SR04 sensor and control a buzzer.
 * ============================================================================
 */

/* Includes */
#include "main.h"
#include "cmsis_os.h" // For FreeRTOS
#include "tim.h"      // For TIM6 (micros) and TIM2 (PWM Buzzer)
#include "gpio.h"     // For TRIG and ECHO pins

/* Timer Handles (assumed to be defined by CubeIDE) */
extern TIM_HandleTypeDef htim2; // Timer for Buzzer PWM
extern TIM_HandleTypeDef htim6; // Timer for micros() function

/* Pin Definitions */
// TRIG Pin: PA9
// ECHO Pin: PA8

/* Task Function Prototypes */
void StartTrigTask(void *argument);
void StartMeasureTask(void *argument);
uint32_t micros(void);

/**
 * @brief  Main function (Entry point)
 * This is where HAL, Clock, and Peripherals would be initialized,
 * and FreeRTOS tasks would be created.
 */
int main(void)
{
  /* HAL Init, System Clock Config... */
  // HAL_Init();
  // SystemClock_Config();

  /* Initialize Peripherals (GPIO, TIM2, TIM6)... */
  // MX_GPIO_Init();
  // MX_TIM2_Init(); // PWM for Buzzer
  // MX_TIM6_Init(); // Timer for micros()

  /* Initialize FreeRTOS Kernel */
  // osKernelInitialize();

  /* Create Tasks */
  // As per the PDF, we have two tasks:
  // osThreadNew(StartTrigTask, NULL, NULL); // Task to send the TRIG pulse
  // osThreadNew(StartMeasureTask, NULL, NULL); // Task to measure and control the buzzer

  /* Start FreeRTOS Kernel */
  // osKernelStart();

  /* Should not reach here */
  while (1)
  {
  }
}

/**
 * @brief Helper function to get time in microseconds
 * Uses a 32-bit timer (TIM6).
 */
uint32_t micros(void)
{
  return _HAL_TIM_GET_COUNTER(&htim6);
}

/**
 * @brief Task that sends the TRIG pulse to the HC-SR04 sensor
 * Runs every 60ms.
 */
void StartTrigTask(void *argument)
{
  for (;;)
  {
    // Send a 10us pulse (PDF showed 1ms, but 10us is standard)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET); // TRIG HIGH
    osDelay(1); // 1ms delay (or use a dedicated microsecond delay)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); // TRIG LOW
    
    // Wait 60ms for the next pulse
    osDelay(60); 
  }
}

/**
 * @brief Task that reads the ECHO pin and controls the buzzer
 * based on the measured distance.
 */
void StartMeasureTask(void *argument)
{
  uint32_t t_start = 0;
  uint32_t t_stop = 0;
  uint32_t t_duration = 0;
  uint32_t distance_cm = 0;

  for (;;)
  {
    // 1. Wait for the ECHO pin to go HIGH
    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_RESET);
    t_start = micros();
    
    // 2. Wait for the ECHO pin to go back to LOW
    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_SET);
    t_stop = micros();

    // 3. Calculate duration and distance
    t_duration = t_stop - t_start;
    distance_cm = t_duration / 58; // Formula for HC-SR04

    /* 4. Buzzer Beep Logic */
    // Based on distance, toggle the PWM on TIM2
    
    if (distance_cm <= 10) // Very short distance
    {
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500); // Sound On
        osDelay(70);
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0); // Sound Off
        osDelay(70);
    }
    else if (distance_cm <= 20)
    {
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
        osDelay(90);
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
        osDelay(150);
    }
    else if (distance_cm <= 30)
    {
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
        osDelay(100);
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
        osDelay(250);
    }
    else if (distance_cm <= 50) // Medium distance
    {
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
        osDelay(100);
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
        osDelay(400);
    }
    else if (distance_cm <= 70) // Long distance
    {
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
        osDelay(50);
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
        osDelay(700);
    }
    else if (distance_cm <= 100)
    {
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
        osDelay(50);
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
        osDelay(1000);
    }
    else // Too far, no sound
    {
        HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
        osDelay(100); // Small delay to not overwhelm the task
    }
  }
}
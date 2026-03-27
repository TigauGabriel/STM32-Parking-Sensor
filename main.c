// ============================================================================
// Proiect: Senzor de parcare cu FreeRTOS
// Student: Tigau Gabriel-Aurelian
// Practica: Preh Romania
// ============================================================================

#include "main.h"
#include "cmsis_os.h"

// Timerele pe care le folosim: TIM2 face PWM pentru buzzer, TIM6 e pt delay in microsecunde
TIM_HandleTypeDef htim2;  
TIM_HandleTypeDef htim6;  

// Handle-urile pentru cele 2 task-uri din FreeRTOS
osThreadId_t sensorTaskHandle;
osThreadId_t buzzerTaskHandle;

// Variabila globala in care salvam distanta. 
// E volatile ca sa nu o optimizeze compilatorul, fiind folosita in ambele task-uri
volatile uint32_t current_distance_cm = 999; 

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM6_Init(void);

void StartSensorTask(void *argument);
void StartBuzzerTask(void *argument);

uint32_t micros(void);
void delay_us(uint16_t us);

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  // Initializam pinii si timerele configurate din CubeMX
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();

  // Pornim PWM-ul si timer-ul pt microsecunde inainte de RTOS
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_Base_Start(&htim6);

  osKernelInitialize();

  // Am impartit logica in 2 task-uri separate ca sa respectam principiile RTOS
  sensorTaskHandle = osThreadNew(StartSensorTask, NULL, NULL);
  buzzerTaskHandle = osThreadNew(StartBuzzerTask, NULL, NULL);

  osKernelStart();

  while (1)
  {
    // Aici nu ajunge niciodata daca RTOS-ul a pornit cu succes
  }
}

// ---------------- Configurari Hardware (Generate) ----------------

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) { Error_Handler(); }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();

  // PA9 e pinul de TRIG (il facem output)
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // PA8 e pinul de ECHO (il facem input sa citim raspunsul)
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void MX_TIM2_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};

  __HAL_RCC_TIM2_CLK_ENABLE();

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8 - 1;   
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000 - 1;   
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) { Error_Handler(); }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;  // initial tinem buzzerul oprit
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) { Error_Handler(); }

  // PA1 - Buzzer
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void MX_TIM6_Init(void)
{
  __HAL_RCC_TIM6_CLK_ENABLE();

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 8 - 1;  
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 0xFFFF;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK) { Error_Handler(); }
}

// ---------------- Functii Utilitare ----------------

// Functie care ne da timpul in microsecunde folosind timerul 6
uint32_t micros(void)
{
  return __HAL_TIM_GET_COUNTER(&htim6);
}

// Oprim executia cateva microsecunde (folositor pentru pulsul scurt de la senzor)
void delay_us(uint16_t us)
{
  uint32_t start = micros();
  while ((micros() - start) < us);
}

// ---------------- Task-uri FreeRTOS ----------------

// Task-ul asta doar se ocupa de senzor si calculeaza distanta
void StartSensorTask(void *argument)
{
  for (;;)
  {
    // Dam un puls scurt de 10us ca sa declansam senzorul HC-SR04
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
    delay_us(10); 
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);

    // Asteptam sa se faca ECHO 1. Am pus si un timeout ca sa nu se blocheze programul daca pateste ceva senzorul
    uint32_t timeout = micros();
    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_RESET) {
        if (micros() - timeout > 50000) break; 
    }

    uint32_t start = micros();

    // Asteptam sa se faca la loc 0 ca sa vedem cat a durat pulsul
    timeout = micros();
    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_SET) {
        if (micros() - timeout > 50000) break; 
    }

    uint32_t stop = micros();
    uint32_t duration = stop - start;

    // Formula din datasheet pt HC-SR04 ca sa aflam distanta in cm
    current_distance_cm = duration / 58;

    // Punem o pauza de 60ms intre masuratori ca sa se linisteasca senzorul (recomandat)
    osDelay(60); 
  }
}

// Task-ul asta ia distanta si face galagie din buzzer in functie de ea
void StartBuzzerTask(void *argument)
{
  for (;;)
  {
    // Ne salvam distanta local ca sa o verificam
    uint32_t distance = current_distance_cm;

    if (distance <= 5)
    {
      // Daca e super aproape, tiuie continuu
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
      osDelay(50);
    }
    else if (distance <= 10)
    {
      // Bipaie foarte des
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
      osDelay(70);
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
      osDelay(70);
    }
    else if (distance <= 20)
    {
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
      osDelay(90);
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
      osDelay(150);
    }
    else if (distance <= 30)
    {
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
      osDelay(100);
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
      osDelay(250);
    }
    else if (distance <= 50)
    {
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
      osDelay(100);
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
      osDelay(400);
    }
    else if (distance <= 70)
    {
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
      osDelay(50);
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
      osDelay(700);
    }
    else if (distance <= 100)
    {
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 500);
      osDelay(50);
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
      osDelay(1000);
    }
    else
    {
      // Suntem departe (> 1m), oprim buzzerul de tot
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
      osDelay(100);
    }
  }
}

void Error_Handler(void)
{
  while (1)
  {
  }
}

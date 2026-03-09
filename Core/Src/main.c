/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "i2c-lcd.h"
#include "led.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#define CODE_LENGTH 3

int bomb_defused = 0;
char* correct_code;
volatile int time_remaining = 120; // segundos
int base_penalty = 2; // segundos
double penalty_multiplier = 1.0;
volatile int last_error;

char input_code[CODE_LENGTH+1];
int key_count = 0;

char* msg_exploded = "BOMBA EXPLODIU";
char* msg_defused = "BOMBA DESARMADA";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char read_keypad(void) {
	char keys[4][3] = {
	        {'1','2','3'},
	        {'4','5','6'},
	        {'7','8','9'},
	        {'*','0','#'}
	};

    GPIO_TypeDef* row_ports[] = {GPIOA, GPIOA, GPIOB, GPIOA};
    uint16_t row_pins[] = {GPIO_PIN_8, GPIO_PIN_10, GPIO_PIN_3, GPIO_PIN_9};

    GPIO_TypeDef* col_ports[] = {GPIOB, GPIOB, GPIOB};
    uint16_t col_pins[] = {GPIO_PIN_10, GPIO_PIN_4, GPIO_PIN_5};

    for (int r = 0; r < 4; r++) {
        // 1. Reset: Coloca todas as linhas em HIGH
        for (int i = 0; i < 4; i++) {
            HAL_GPIO_WritePin(row_ports[i], row_pins[i], GPIO_PIN_SET);
        }

        // 2. Ativação: Coloca apenas a linha atual em LOW
        HAL_GPIO_WritePin(row_ports[r], row_pins[r], GPIO_PIN_RESET);
        HAL_Delay(2); // Estabilização de sinal

        // 3. Leitura: Verifica qual coluna respondeu
        for (int c = 0; c < 3; c++) {
            if (HAL_GPIO_ReadPin(col_ports[c], col_pins[c]) == GPIO_PIN_RESET) {
                HAL_Delay(50); // Debounce para evitar cliques duplos
                while(HAL_GPIO_ReadPin(col_ports[c], col_pins[c]) == GPIO_PIN_RESET);
                return keys[r][c];
            }
        }
    }
    return 0; // Nenhuma tecla detetada
}

char* generate_code(){
	char* new_code = malloc((CODE_LENGTH+1)*sizeof(char));

    if(!new_code){
        return NULL;
    }

	for (int i = 0; i < CODE_LENGTH; i++){
		new_code[i] = '0' + rand() % 10; // ASCII para transformar em char
	}

	new_code[CODE_LENGTH] = '\0';

	return new_code;
}

void applyPenalty(){
	penalty_multiplier += 0.5;
	int penalty = base_penalty * penalty_multiplier;

	if (time_remaining > penalty){
		time_remaining -= penalty;
	} else {
		time_remaining = 0;
	}

}

void giveHint(){
	lcd_clear();
    lcd_put_cur(0, 0);
	if (atoi(input_code) > atoi(correct_code)){
		lcd_send_string("MUITO ALTO");
	} else {
		lcd_send_string("MUITO BAIXO");
	}
}

void reset_game(){
	bomb_defused = 0;
	time_remaining = 120;
	penalty_multiplier = 1.0;

	if (correct_code)
		free(correct_code);

	correct_code = generate_code();

	for (int i = 0; i < CODE_LENGTH; i++)
		input_code[i] = 0;

	key_count = 0;

	lcd_display_codigo();
}

void wait_for_restart(){
	char tecla = 0;
	while (tecla != '#'){
		tecla = read_keypad();
		HAL_Delay(10); // debounce
	}
}

void endgame_routine(){
	char* msg = msg_exploded;
	if (bomb_defused){
        RGB_setGreen();
        msg = msg_defused;
	} else {
        RGB_setRed();
	}

	free(correct_code);
	lcd_display_final_message(msg);
    wait_for_restart();
    reset_game();
}

void checkCodeInput(){
    if (strcmp(input_code, correct_code) == 0) {
  	  bomb_defused = 1;
  	  endgame_routine();
    } else {
    	last_error = HAL_GetTick();
        giveHint();
        applyPenalty();
        key_count = 0;
    }
}

void handle_keypress(char tecla){
    if (key_count < CODE_LENGTH) {
        input_code[key_count] = tecla;
        lcd_put_cur(0, 7+key_count); // 7 é o comprimento de "CODIGO:"
        lcd_send_data(tecla);
        key_count++;
    }

    if (key_count == CODE_LENGTH) {
        input_code[CODE_LENGTH] = '\0';
        HAL_Delay(500);
        checkCodeInput();
    }
}

void update_display(){
    if(!bomb_defused) {
  	  if (time_remaining <= 0)
  		  endgame_routine();
  	  else {
  		  lcd_display_tempo(time_remaining);
  	  }
    }

    if (last_error != 0 && (HAL_GetTick() - last_error >= 1000))
    {
  	  lcd_display_codigo();
      last_error = 0;
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM2_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  RGB_init();
  lcd_init();
  srand(rtc_get_seed());
  correct_code = generate_code();
  HAL_TIM_Base_Start_IT(&htim2);
  lcd_display_codigo();
  lcd_display_tempo(time_remaining);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
    {
	  RGB_defuseBlink(time_remaining);

      char tecla = read_keypad();
      if (tecla)
    	  handle_keypress(tecla);

      update_display();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2) // verifica se é o timer certo
    {
        if(time_remaining > 0){
            time_remaining--;
        }
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

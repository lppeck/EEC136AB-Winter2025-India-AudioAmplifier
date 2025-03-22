/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "shared_memory.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c4;

SPI_HandleTypeDef hspi4;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
/* Define the I2C address for OLED and AS5600 */
//#define OLED_I2C_ADDRESS   0x78  // Assuming 7-bit address shifted left by 1
#define AS5600_I2C_ADDRESS 0x36 << 1  // 7-bit address for AS5600

#define SHARED_MEMORY_BASE_ADDR 0x20020000

volatile float* shared_data_ptr = (float*)0x38000000;

#define OLED_CS_PIN     GPIO_PIN_10 // Modify as per your setup (Chip select pin for SPI)
#define OLED_DC_PIN      GPIO_PIN_15
#define OLED_RST_PIN      GPIO_PIN_7
#define OLED_GPIO_PORT    GPIOE       // Modify as per your setup (Chip select GPIO port)
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_PAGES (OLED_HEIGHT/8)

uint8_t buffer[OLED_WIDTH * OLED_PAGES];

static float absoluteAngle = 0.0;
static uint16_t prevRawAngle = 0;
static uint32_t lastVolumeChangeTime = 0;
static int volume = 0;
static uint8_t showVolumeUI = 0;  // Boolean flag

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MX_GPIO_Init(void);
static void MX_I2C4_Init(void);
static void MX_SPI4_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void OLED_Init();
void OLED_Command(uint8_t cmd);
void OLED_Data(uint8_t data);
void OLED_Update();
void OLED_Clear();
void drawVolumeBar(int volume);
//void OLED_Init_SPI(void);

//void OLED_PrintVolume_SPI(uint8_t volume);
uint16_t readAS5600Angle();
uint8_t MapPositionToVolume(uint16_t position);

//read angle from encoder
void debug_print(const char *msg) {
    HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

// --- AS5600 Read Angle Function ---
uint16_t readAS5600Angle() {
    uint8_t reg = 0x0C; // RAW ANGLE register
    uint8_t buffer[2];
    uint16_t angle;

    HAL_I2C_Master_Transmit(&hi2c4, AS5600_I2C_ADDRESS, &reg, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(&hi2c4, AS5600_I2C_ADDRESS, buffer, 2, HAL_MAX_DELAY);

    angle = ((buffer[0] << 8) | buffer[1]) & 0x0FFF;  // 12-bit angle value
    return angle;
}

// --- OLED SPI Communication ---
void OLED_Command(uint8_t command) {
    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_DC_PIN, GPIO_PIN_RESET); // DC = 0 (Command)
    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_CS_PIN, GPIO_PIN_RESET); // CS = 0
    HAL_SPI_Transmit(&hspi4, &command, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_CS_PIN, GPIO_PIN_SET);   // CS = 1
}

void OLED_Data(uint8_t data) {
    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_DC_PIN, GPIO_PIN_SET); // DC = 1 (Data)
    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_CS_PIN, GPIO_PIN_RESET); // CS = 0
    HAL_SPI_Transmit(&hspi4, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_CS_PIN, GPIO_PIN_SET);   // CS = 1
}

// --- OLED Initialization ---
void OLED_Init() {
    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_RST_PIN, GPIO_PIN_RESET); // Reset OLED
    HAL_Delay(10);
    HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_RST_PIN, GPIO_PIN_SET);   // Release Reset

    OLED_Command(0xAE); // Display OFF
    OLED_Command(0xD5); OLED_Command(0x80); // Display Clock
    OLED_Command(0xA8); OLED_Command(0x3F); // Multiplex Ratio
    OLED_Command(0xD3); OLED_Command(0x00); // Display Offset
    OLED_Command(0x40); // Display Start Line
    OLED_Command(0xA1); // Segment Remap
    OLED_Command(0xC8); // COM Output Scan Direction
    OLED_Command(0xDA); OLED_Command(0x12); // COM Pins Configuration
    OLED_Command(0x81); OLED_Command(0x7F); // Contrast
    OLED_Command(0xD9); OLED_Command(0xF1); // Pre-charge Period
    OLED_Command(0xDB); OLED_Command(0x40); // VCOMH Deselect Level
    OLED_Command(0xA4); // Entire Display ON
    OLED_Command(0xA6); // Normal Display
    OLED_Command(0x8D); OLED_Command(0x14); // Enable Charge Pump
    OLED_Command(0xAF); // Display ON
}

// --- OLED Update (Page Addressing Mode) ---
void OLED_Update() {
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        OLED_Command(0xB0 + page); // Set Page Address
        OLED_Command(0x00); // Lower Column Start Address
        OLED_Command(0x10); // Upper Column Start Address

        HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_DC_PIN, GPIO_PIN_SET); // DC = 1 (Data)
        HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_CS_PIN, GPIO_PIN_RESET); // CS = 0
        HAL_SPI_Transmit(&hspi4, &buffer[page * OLED_WIDTH], OLED_WIDTH, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(OLED_GPIO_PORT, OLED_CS_PIN, GPIO_PIN_SET); // CS = 1
    }
}

// --- Clear Display Buffer ---
void OLED_Clear() {
    memset(buffer, 0, sizeof(buffer));
    OLED_Update();
}

void drawVolumeBar(int volume) {
    // Clear only the bar area
    memset(&buffer[(OLED_HEIGHT - 8) / 8 * OLED_WIDTH], 0, OLED_WIDTH);

    int width = (volume * OLED_WIDTH) / 100; // Scale volume to OLED width
    int y = OLED_HEIGHT - 8;  // Position at the bottom (last row of pixels)

    for (int x = 0; x < width; x++) {
        buffer[x + (y / 8) * OLED_WIDTH] = 0xFF;  // Draw a filled bar
    }
}

const uint8_t font6x8[][6] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00, 0x00}, // 1
    {0x72, 0x49, 0x49, 0x49, 0x46, 0x00}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31, 0x00}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39, 0x00}, // 5
    {0x3E, 0x49, 0x49, 0x49, 0x30, 0x00}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03, 0x00}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36, 0x00}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E, 0x00}, // 9
};

void drawChar(int x, int y, char c) {
    int index;
    if (c >= '0' && c <= '9') {
        index = c - '0'; // Get digit index
    } else if (c == '%') {
        index = 10; // '%' symbol
    } else {
        return; // Ignore unsupported characters
    }

    for (int col = 0; col < 6; col++) {
        for (int row = 0; row < 8; row++) {
            if (font6x8[index][col] & (1 << row)) {
                int pixelIndex = (y + row) / 8 * OLED_WIDTH + (x + col);
                buffer[pixelIndex] |= (1 << ((y + row) % 8));
            }
        }
    }
}

// Draw text at (x, y)
void drawText(int x, int y, const char *text) {
    while (*text) {
        drawChar(x, y, *text);
        x += 6; // Move cursor for next character
        text++;
    }
}

void drawWaveAnimation(uint8_t frameOffset) {
    for (int x = 0; x < OLED_WIDTH; x++) {
        int yOffset = 5 + (int)(4 * sin((x + frameOffset) * 0.2)); // Small wave oscillation
        int pixelIndex = (yOffset / 8) * OLED_WIDTH + x;
        buffer[pixelIndex] |= (1 << (yOffset % 8));
    }
}

void OLED_DrawPixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) return;

    if (color) {
        buffer[x + (y / 8) * OLED_WIDTH] |= (1 << (y % 8));  // Set pixel
    } else {
        buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8)); // Clear pixel
    }
}


void drawLine(int x1, int y1, int x2, int y2) {
    for (int x = x1; x <= x2; x++) {
        OLED_DrawPixel(x, y1, 1);  // Draw a pixel at (x, y1)
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
  /* Initialize peripherals */
	  HAL_Init();


  /* USER CODE END 1 */

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
	  /* HW semaphore Clock enable */
	    __HAL_RCC_HSEM_CLK_ENABLE();
	    /* Activate HSEM notification for Cortex-M4 */
	    HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));
	    /* Domain D2 goes to STOP mode (Cortex-M4 in deep-sleep) waiting for Cortex-M7 to
	       perform system initialization (system clock config, external memory configuration.. )
	    */
	    HAL_PWREx_ClearPendingEvent();
	    HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);
	    /* Clear HSEM flag */
	    __HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C4_Init();
  MX_SPI4_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
    OLED_Clear();
    uint32_t frameCounter = 0; // Keeps the wave moving continuously
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  uint16_t rawAngle = readAS5600Angle();
	        int delta = rawAngle - prevRawAngle;

	        if (delta > 2048) {
	            delta -= 4096;
	        } else if (delta < -2048) {
	            delta += 4096;
	        }

	        absoluteAngle += (delta * (360.0 / 4096.0));
	        if (absoluteAngle > 360.0) absoluteAngle = 360.0;
	        if (absoluteAngle < 0.0) absoluteAngle = 0.0;

	        /////////
	        *shared_data_ptr = absoluteAngle;
			/////////

	        int newVolume = (absoluteAngle * 100) / 360;

	        if (newVolume != volume) {
	            volume = newVolume;
	            lastVolumeChangeTime = HAL_GetTick();
	            showVolumeUI = 1;
	        }

	        uint32_t elapsedTime = HAL_GetTick() - lastVolumeChangeTime;

	        // Clear only the areas that need to be updated
	        memset(buffer, 0, sizeof(buffer));

	        // Show wave only if active within 3 seconds
	        if (elapsedTime < 3000) {
	            drawWaveAnimation(frameCounter); // Draw wave first (background)

	            drawVolumeBar(volume); // Draw volume bar
	            char volumeText[5];
	            sprintf(volumeText, "%d%%", volume);
	            drawText(64 - (strlen(volumeText) * 3), 32, volumeText);
	        }

	        OLED_Update();

	        prevRawAngle = rawAngle;
	        frameCounter++;  // Increment wave animation frame
	        /*uint8_t uart_buff[32];
	        uint8_t strlength = sprintf(uart_buff, "Volume: %u\r\n", newVolume);
	        HAL_UART_Transmit(&huart3, uart_buff, strlength, 10000);*/
	        HAL_Delay(16);   // ~60 FPS for smooth animation


	    }
  /* USER CODE END 3 */
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void)
{

  /* USER CODE BEGIN I2C4_Init 0 */

  /* USER CODE END I2C4_Init 0 */

  /* USER CODE BEGIN I2C4_Init 1 */

  /* USER CODE END I2C4_Init 1 */
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x10C0ECFF;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C4_Init 2 */

  /* USER CODE END I2C4_Init 2 */

}

/**
  * @brief SPI4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI4_Init(void)
{

  /* USER CODE BEGIN SPI4_Init 0 */

  /* USER CODE END SPI4_Init 0 */

  /* USER CODE BEGIN SPI4_Init 1 */

  /* USER CODE END SPI4_Init 1 */
  /* SPI4 parameter configuration*/
  hspi4.Instance = SPI4;
  hspi4.Init.Mode = SPI_MODE_MASTER;
  hspi4.Init.Direction = SPI_DIRECTION_2LINES;
  hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi4.Init.NSS = SPI_NSS_SOFT;
  hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi4.Init.CRCPolynomial = 0x0;
  hspi4.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi4.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi4.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi4.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi4.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi4.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi4.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi4.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi4.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi4.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI4_Init 2 */

  /* USER CODE END SPI4_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7|GPIO_PIN_10|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE7 PE10 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PE15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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

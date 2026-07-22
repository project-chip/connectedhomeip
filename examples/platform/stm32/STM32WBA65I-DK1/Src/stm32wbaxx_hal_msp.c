/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
extern DMA_HandleTypeDef handle_GPDMA1_Channel1;

extern DMA_HandleTypeDef handle_GPDMA1_Channel0;

extern DMA_HandleTypeDef handle_GPDMA1_Channel3;

extern DMA_HandleTypeDef handle_GPDMA1_Channel2;

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void)
{

    /* USER CODE BEGIN MspInit 0 */

    /* USER CODE END MspInit 0 */

    __HAL_RCC_PWR_CLK_ENABLE();

    /* System interrupt init*/
    /* PendSV_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

    /* Peripheral interrupt init */
    /* RCC_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(RCC_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(RCC_IRQn);
    /* RADIO_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(RADIO_IRQn, RADIO_INTR_PRIO_LOW, 0);
    HAL_NVIC_EnableIRQ(RADIO_IRQn);

    /* USER CODE BEGIN MspInit 1 */

    /* USER CODE END MspInit 1 */
}

/**
 * @brief CRC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hcrc: CRC handle pointer
 * @retval None
 */
void HAL_CRC_MspInit(CRC_HandleTypeDef * hcrc)
{
    if (hcrc->Instance == CRC)
    {
        /* USER CODE BEGIN CRC_MspInit 0 */

        /* USER CODE END CRC_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_CRC_CLK_ENABLE();
        /* USER CODE BEGIN CRC_MspInit 1 */

        /* USER CODE END CRC_MspInit 1 */
    }
}

/**
 * @brief CRC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hcrc: CRC handle pointer
 * @retval None
 */
void HAL_CRC_MspDeInit(CRC_HandleTypeDef * hcrc)
{
    if (hcrc->Instance == CRC)
    {
        /* USER CODE BEGIN CRC_MspDeInit 0 */

        /* USER CODE END CRC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CRC_CLK_DISABLE();
        /* USER CODE BEGIN CRC_MspDeInit 1 */

        /* USER CODE END CRC_MspDeInit 1 */
    }
}

/**
 * @brief I2C MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef * hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct       = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
    if (hi2c->Instance == I2C1)
    {
        /* USER CODE BEGIN I2C1_MspInit 0 */

        /* USER CODE END I2C1_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
        PeriphClkInit.I2c1ClockSelection   = RCC_I2C1CLKSOURCE_HSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**I2C1 GPIO Configuration
        PB2     ------> I2C1_SCL
        PB1     ------> I2C1_SDA
        */
        GPIO_InitStruct.Pin       = I2C1_SCL_Pin | I2C1_SDA_Pin;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __HAL_RCC_I2C1_CLK_ENABLE();
        /* USER CODE BEGIN I2C1_MspInit 1 */

        /* USER CODE END I2C1_MspInit 1 */
    }
}

/**
 * @brief I2C MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef * hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        /* USER CODE BEGIN I2C1_MspDeInit 0 */

        /* USER CODE END I2C1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C1_CLK_DISABLE();

        /**I2C1 GPIO Configuration
        PB2     ------> I2C1_SCL
        PB1     ------> I2C1_SDA
        */
        HAL_GPIO_DeInit(I2C1_SCL_GPIO_Port, I2C1_SCL_Pin);

        HAL_GPIO_DeInit(I2C1_SDA_GPIO_Port, I2C1_SDA_Pin);

        /* USER CODE BEGIN I2C1_MspDeInit 1 */

        /* USER CODE END I2C1_MspDeInit 1 */
    }
}

/**
 * @brief RAMCFG MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hramcfg: RAMCFG handle pointer
 * @retval None
 */
void HAL_RAMCFG_MspInit(RAMCFG_HandleTypeDef * hramcfg)
{
    /* USER CODE BEGIN RAMCFG_MspInit 0 */

    /* USER CODE END RAMCFG_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_RAMCFG_CLK_ENABLE();
    /* USER CODE BEGIN RAMCFG_MspInit 1 */

    /* USER CODE END RAMCFG_MspInit 1 */
}

/**
 * @brief RAMCFG MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hramcfg: RAMCFG handle pointer
 * @retval None
 */
void HAL_RAMCFG_MspDeInit(RAMCFG_HandleTypeDef * hramcfg)
{
    /* USER CODE BEGIN RAMCFG_MspDeInit 0 */

    /* USER CODE END RAMCFG_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RAMCFG_CLK_DISABLE();
    /* USER CODE BEGIN RAMCFG_MspDeInit 1 */

    /* USER CODE END RAMCFG_MspDeInit 1 */
}

/**
 * @brief RNG MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hrng: RNG handle pointer
 * @retval None
 */
void HAL_RNG_MspInit(RNG_HandleTypeDef * hrng)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
    if (hrng->Instance == RNG)
    {
        /* USER CODE BEGIN RNG_MspInit 0 */

        /* USER CODE END RNG_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RNG;
        PeriphClkInit.RngClockSelection    = RCC_RNGCLKSOURCE_HSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_RNG_CLK_ENABLE();
        /* USER CODE BEGIN RNG_MspInit 1 */

        /* USER CODE END RNG_MspInit 1 */
    }
}

/**
 * @brief RNG MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hrng: RNG handle pointer
 * @retval None
 */
void HAL_RNG_MspDeInit(RNG_HandleTypeDef * hrng)
{
    if (hrng->Instance == RNG)
    {
        /* USER CODE BEGIN RNG_MspDeInit 0 */

        /* USER CODE END RNG_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_RNG_CLK_DISABLE();
        /* USER CODE BEGIN RNG_MspDeInit 1 */

        /* USER CODE END RNG_MspDeInit 1 */
    }
}

/**
 * @brief RTC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspInit(RTC_HandleTypeDef * hrtc)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
    if (hrtc->Instance == RTC)
    {
        /* USER CODE BEGIN RTC_MspInit 0 */

        /* USER CODE END RTC_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInit.RTCClockSelection    = RCC_RTCCLKSOURCE_LSE;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_RTCAPB_CLK_ENABLE();
        /* RTC interrupt Init */
        HAL_NVIC_SetPriority(RTC_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(RTC_IRQn);
        /* USER CODE BEGIN RTC_MspInit 1 */

        /* USER CODE END RTC_MspInit 1 */
    }
}

/**
 * @brief RTC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef * hrtc)
{
    if (hrtc->Instance == RTC)
    {
        /* USER CODE BEGIN RTC_MspDeInit 0 */

        /* USER CODE END RTC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_RTCAPB_CLK_DISABLE();

        /* RTC interrupt DeInit */
        HAL_NVIC_DisableIRQ(RTC_IRQn);
        /* USER CODE BEGIN RTC_MspDeInit 1 */

        /* USER CODE END RTC_MspDeInit 1 */
    }
}

/**
 * @brief SPI MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef * hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct       = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
    if (hspi->Instance == SPI3)
    {
        /* USER CODE BEGIN SPI3_MspInit 0 */

        /* USER CODE END SPI3_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SPI3;
        PeriphClkInit.Spi3ClockSelection   = RCC_SPI3CLKSOURCE_HSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_SPI3_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**SPI3 GPIO Configuration
        PA0     ------> SPI3_SCK
        PB8     ------> SPI3_MOSI
        */
        GPIO_InitStruct.Pin       = SPI3_SCK_Pin;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
        HAL_GPIO_Init(SPI3_SCK_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = SPI3_MOSI_Pin;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
        HAL_GPIO_Init(SPI3_MOSI_GPIO_Port, &GPIO_InitStruct);

        /* USER CODE BEGIN SPI3_MspInit 1 */

        /* USER CODE END SPI3_MspInit 1 */
    }
}

/**
 * @brief SPI MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef * hspi)
{
    if (hspi->Instance == SPI3)
    {
        /* USER CODE BEGIN SPI3_MspDeInit 0 */

        /* USER CODE END SPI3_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_SPI3_CLK_DISABLE();

        /**SPI3 GPIO Configuration
        PA0     ------> SPI3_SCK
        PB8     ------> SPI3_MOSI
        */
        HAL_GPIO_DeInit(SPI3_SCK_GPIO_Port, SPI3_SCK_Pin);

        HAL_GPIO_DeInit(SPI3_MOSI_GPIO_Port, SPI3_MOSI_Pin);

        /* USER CODE BEGIN SPI3_MspDeInit 1 */

        /* USER CODE END SPI3_MspDeInit 1 */
    }
}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef * huart)
{
    GPIO_InitTypeDef GPIO_InitStruct       = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
    if (huart->Instance == USART1)
    {
        /* USER CODE BEGIN USART1_MspInit 0 */

        /* USER CODE END USART1_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
        PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_HSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PB12     ------> USART1_TX
        PA8     ------> USART1_RX
        */
        GPIO_InitStruct.Pin       = GPIO_PIN_12;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = GPIO_PIN_8;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART1 DMA Init */
        /* GPDMA1_REQUEST_USART1_RX Init */
        handle_GPDMA1_Channel1.Instance                   = GPDMA1_Channel1;
        handle_GPDMA1_Channel1.Init.Request               = GPDMA1_REQUEST_USART1_RX;
        handle_GPDMA1_Channel1.Init.BlkHWRequest          = DMA_BREQ_SINGLE_BURST;
        handle_GPDMA1_Channel1.Init.Direction             = DMA_PERIPH_TO_MEMORY;
        handle_GPDMA1_Channel1.Init.SrcInc                = DMA_SINC_FIXED;
        handle_GPDMA1_Channel1.Init.DestInc               = DMA_DINC_INCREMENTED;
        handle_GPDMA1_Channel1.Init.SrcDataWidth          = DMA_SRC_DATAWIDTH_BYTE;
        handle_GPDMA1_Channel1.Init.DestDataWidth         = DMA_DEST_DATAWIDTH_BYTE;
        handle_GPDMA1_Channel1.Init.Priority              = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA1_Channel1.Init.SrcBurstLength        = 1;
        handle_GPDMA1_Channel1.Init.DestBurstLength       = 1;
        handle_GPDMA1_Channel1.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        handle_GPDMA1_Channel1.Init.TransferEventMode     = DMA_TCEM_BLOCK_TRANSFER;
        handle_GPDMA1_Channel1.Init.Mode                  = DMA_NORMAL;
        if (HAL_DMA_Init(&handle_GPDMA1_Channel1) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmarx, handle_GPDMA1_Channel1);

        if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel1, DMA_CHANNEL_NPRIV) != HAL_OK)
        {
            Error_Handler();
        }

        /* GPDMA1_REQUEST_USART1_TX Init */
        handle_GPDMA1_Channel0.Instance                   = GPDMA1_Channel0;
        handle_GPDMA1_Channel0.Init.Request               = GPDMA1_REQUEST_USART1_TX;
        handle_GPDMA1_Channel0.Init.BlkHWRequest          = DMA_BREQ_SINGLE_BURST;
        handle_GPDMA1_Channel0.Init.Direction             = DMA_MEMORY_TO_PERIPH;
        handle_GPDMA1_Channel0.Init.SrcInc                = DMA_SINC_INCREMENTED;
        handle_GPDMA1_Channel0.Init.DestInc               = DMA_DINC_FIXED;
        handle_GPDMA1_Channel0.Init.SrcDataWidth          = DMA_SRC_DATAWIDTH_BYTE;
        handle_GPDMA1_Channel0.Init.DestDataWidth         = DMA_DEST_DATAWIDTH_BYTE;
        handle_GPDMA1_Channel0.Init.Priority              = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA1_Channel0.Init.SrcBurstLength        = 1;
        handle_GPDMA1_Channel0.Init.DestBurstLength       = 1;
        handle_GPDMA1_Channel0.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        handle_GPDMA1_Channel0.Init.TransferEventMode     = DMA_TCEM_BLOCK_TRANSFER;
        handle_GPDMA1_Channel0.Init.Mode                  = DMA_NORMAL;
        if (HAL_DMA_Init(&handle_GPDMA1_Channel0) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmatx, handle_GPDMA1_Channel0);

        if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel0, DMA_CHANNEL_NPRIV) != HAL_OK)
        {
            Error_Handler();
        }

        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        /* USER CODE BEGIN USART1_MspInit 1 */

        /* USER CODE END USART1_MspInit 1 */
    }
    else if (huart->Instance == USART2)
    {
        /* USER CODE BEGIN USART2_MspInit 0 */

        /* USER CODE END USART2_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
        PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_HSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART2 GPIO Configuration
        PB4(NJTRST)     ------> USART2_RX
        PA12     ------> USART2_TX
        */
        GPIO_InitStruct.Pin       = GPIO_PIN_4;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF3_USART2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin       = GPIO_PIN_12;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF3_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART2 DMA Init */
        /* GPDMA1_REQUEST_USART2_TX Init */
        handle_GPDMA1_Channel3.Instance                   = GPDMA1_Channel3;
        handle_GPDMA1_Channel3.Init.Request               = GPDMA1_REQUEST_USART2_TX;
        handle_GPDMA1_Channel3.Init.BlkHWRequest          = DMA_BREQ_SINGLE_BURST;
        handle_GPDMA1_Channel3.Init.Direction             = DMA_PERIPH_TO_MEMORY;
        handle_GPDMA1_Channel3.Init.SrcInc                = DMA_SINC_FIXED;
        handle_GPDMA1_Channel3.Init.DestInc               = DMA_DINC_FIXED;
        handle_GPDMA1_Channel3.Init.SrcDataWidth          = DMA_SRC_DATAWIDTH_BYTE;
        handle_GPDMA1_Channel3.Init.DestDataWidth         = DMA_DEST_DATAWIDTH_BYTE;
        handle_GPDMA1_Channel3.Init.Priority              = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA1_Channel3.Init.SrcBurstLength        = 1;
        handle_GPDMA1_Channel3.Init.DestBurstLength       = 1;
        handle_GPDMA1_Channel3.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        handle_GPDMA1_Channel3.Init.TransferEventMode     = DMA_TCEM_BLOCK_TRANSFER;
        handle_GPDMA1_Channel3.Init.Mode                  = DMA_NORMAL;
        if (HAL_DMA_Init(&handle_GPDMA1_Channel3) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmatx, handle_GPDMA1_Channel3);

        if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel3, DMA_CHANNEL_NPRIV) != HAL_OK)
        {
            Error_Handler();
        }

        /* GPDMA1_REQUEST_USART2_RX Init */
        handle_GPDMA1_Channel2.Instance                   = GPDMA1_Channel2;
        handle_GPDMA1_Channel2.Init.Request               = GPDMA1_REQUEST_USART2_RX;
        handle_GPDMA1_Channel2.Init.BlkHWRequest          = DMA_BREQ_SINGLE_BURST;
        handle_GPDMA1_Channel2.Init.Direction             = DMA_PERIPH_TO_MEMORY;
        handle_GPDMA1_Channel2.Init.SrcInc                = DMA_SINC_FIXED;
        handle_GPDMA1_Channel2.Init.DestInc               = DMA_DINC_FIXED;
        handle_GPDMA1_Channel2.Init.SrcDataWidth          = DMA_SRC_DATAWIDTH_BYTE;
        handle_GPDMA1_Channel2.Init.DestDataWidth         = DMA_DEST_DATAWIDTH_BYTE;
        handle_GPDMA1_Channel2.Init.Priority              = DMA_LOW_PRIORITY_LOW_WEIGHT;
        handle_GPDMA1_Channel2.Init.SrcBurstLength        = 1;
        handle_GPDMA1_Channel2.Init.DestBurstLength       = 1;
        handle_GPDMA1_Channel2.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
        handle_GPDMA1_Channel2.Init.TransferEventMode     = DMA_TCEM_BLOCK_TRANSFER;
        handle_GPDMA1_Channel2.Init.Mode                  = DMA_NORMAL;
        if (HAL_DMA_Init(&handle_GPDMA1_Channel2) != HAL_OK)
        {
            Error_Handler();
        }

        __HAL_LINKDMA(huart, hdmarx, handle_GPDMA1_Channel2);

        if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel2, DMA_CHANNEL_NPRIV) != HAL_OK)
        {
            Error_Handler();
        }

        /* USART2 interrupt Init */
        HAL_NVIC_SetPriority(USART2_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspInit 1 */

        /* USER CODE END USART2_MspInit 1 */
    }
}

/**
 * @brief UART MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef * huart)
{
    if (huart->Instance == USART1)
    {
        /* USER CODE BEGIN USART1_MspDeInit 0 */

        /* USER CODE END USART1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART1 GPIO Configuration
        PB12     ------> USART1_TX
        PA8     ------> USART1_RX
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);

        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8);

        /* USART1 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmarx);
        HAL_DMA_DeInit(huart->hdmatx);

        /* USART1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(USART1_IRQn);
        /* USER CODE BEGIN USART1_MspDeInit 1 */

        /* USER CODE END USART1_MspDeInit 1 */
    }
    else if (huart->Instance == USART2)
    {
        /* USER CODE BEGIN USART2_MspDeInit 0 */

        /* USER CODE END USART2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
        PB4(NJTRST)     ------> USART2_RX
        PA12     ------> USART2_TX
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_4);

        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12);

        /* USART2 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmatx);
        HAL_DMA_DeInit(huart->hdmarx);

        /* USART2 interrupt DeInit */
        HAL_NVIC_DisableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspDeInit 1 */

        /* USER CODE END USART2_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */
/**
 * @brief PKA MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hpka: PKA handle pointer
 * @retval None
 */
void HAL_PKA_MspInit(PKA_HandleTypeDef * hpka)
{
    if (hpka->Instance == PKA)
    {
        /* Peripheral clock enable */
        __HAL_RCC_PKA_CLK_ENABLE();
        /* PKA interrupt Init */
        HAL_NVIC_SetPriority(PKA_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(PKA_IRQn);

        HW_RNG_EnableClock(0x04);
    }
}

/**
 * @brief PKA MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hpka: PKA handle pointer
 * @retval None
 */
void HAL_PKA_MspDeInit(PKA_HandleTypeDef * hpka)
{
    if (hpka->Instance == PKA)
    {
        __HAL_RCC_PKA_CLK_DISABLE();

        /* PKA interrupt DeInit */
        HAL_NVIC_DisableIRQ(PKA_IRQn);

        HW_RNG_DisableClock(0x04);
    }
}
/* USER CODE END 1 */

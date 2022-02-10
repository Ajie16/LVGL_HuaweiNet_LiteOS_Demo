/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "usart.h"
#include "sdmmc.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
#include "lcd.h"
#include "lv_port_disp.h"
#include "lv_port_fs.h"
#include "transport.h"
#include "esp8266.h"
#include "huawei.h"
#include "sht30.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
FATFS   fs;			/* FATFS 文件系统对象 */
FRESULT fr; 		/* FATFS API 返回值 */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
osSemaphoreId_t DMA_SemaphoreHandle;
const osSemaphoreAttr_t DMA_Semaphore_attributes = {
  .name = "SPI_DMA_Semaphore"
};

osSemaphoreId_t TX_DMA_SemaphoreHandle;
const osSemaphoreAttr_t TX_DMA_Semaphore_attributes = {
  .name = "TX_DMA_Semaphore"
};

osSemaphoreId_t RX_DMA_SemaphoreHandle;
const osSemaphoreAttr_t RX_DMA_Semaphore_attributes = {
  .name = "RX_DMA_Semaphore"
};


osThreadId_t led_taskHandle;
const osThreadAttr_t led_task_attributes = {
  .name = "led_task",
  .stack_size = 512 * 1,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t lcd_taskHandle;
const osThreadAttr_t lcd_task_attributes = {
  .name = "lcd_task",
  .stack_size = 512 * 2,
  .priority = (osPriority_t) osPriorityNormal4,
};

osThreadId_t huawei_taskHandle;
const osThreadAttr_t huawei_task_attributes = {
  .name = "huawei_task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal3,
};


void Led_Task(void *argument);
void Lcd_Task(void *argument);
void Huawei_Task(void *argument);


void Led_Task(void *argument)
{
	while(1)
	{
		
		HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
		osDelay(1000);
	}
}


lv_img_dsc_t testimg1 = {
	.header.always_zero = 0,
	.header.w = 150,
	.header.h = 60,
	.data_size = 150 * 60 * 2,
	.header.cf = LV_IMG_CF_TRUE_COLOR,
};

lv_img_dsc_t bkg_bearpi = {
	.header.always_zero = 0,
	.header.w = 240,
	.header.h = 20,
	.data_size = 240 * 20 * 2,
	.header.cf = LV_IMG_CF_TRUE_COLOR,
};
//声明字体
LV_FONT_DECLARE(my_font);

void Lcd_Task(void *argument)
{
	lv_fs_res_t fs_res=LV_FS_RES_NOT_IMP;
	int offset;
	LCD_Init();
	lv_init();
	lv_port_disp_init();//lvgl 显示接口初始化,放在 lv_init()的后面
	lv_port_fs_init();
	
	//内存申请
	lv_fs_file_t *lv_file = (lv_fs_file_t *)lv_mem_alloc(sizeof(lv_fs_file_t *));
	lv_obj_t *bkg = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	lv_obj_t *homonoryimg = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	lv_style_t *style1 = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t *));
	uint8_t* framebuffer1 = (uint8_t*)lv_mem_alloc(sizeof(uint8_t)*18000);
	
	
	//对象操作
	
	lv_style_init(style1);
	lv_style_set_bg_color(style1, LV_STATE_DEFAULT,LV_COLOR_BLACK);
	lv_style_set_border_width(style1,LV_STATE_DEFAULT, 0);
	lv_style_set_radius(style1,LV_STATE_DEFAULT,0);
	
	bkg = lv_obj_create(lv_scr_act(),NULL);
	lv_obj_set_pos(bkg,0,0);
	lv_obj_set_size(bkg,240,240);
	lv_obj_add_style(bkg,LV_OBJ_PART_MAIN,style1);
	
	//鸿蒙启动界面
	homonoryimg = lv_img_create(lv_scr_act(), NULL);
	//文件打开
	fs_res = lv_fs_open(lv_file, "S:/os.bin", LV_FS_MODE_RD| LV_FS_MODE_WR);
	if ( fs_res != LV_FS_RES_OK )
		printf( "LVGL FS open error. (%d)\r\n", fs_res );
	offset = 0;
	offset += 4; //从offset=4
	lv_fs_seek(lv_file, offset);

	//计算bin文件里一共包含多少张图片，然后不断的给tft进行显示
//	for(int i = 0 ; i < 401 ; i++)
//	{
//		fs_res = lv_fs_read(lv_file, (uint8_t *)framebuffer1, 18000,NULL);
//		testimg1.data = (const uint8_t *)framebuffer1;
//		lv_img_set_src(homonoryimg, &testimg1);
//		lv_obj_align(homonoryimg, NULL, LV_ALIGN_CENTER, 0, 0);
//		lv_task_handler();
//		offset += 18004;
//		fs_res = lv_fs_seek(lv_file, offset);
//		osDelay(20);
//	}
	//资源回收
	lv_fs_close(lv_file);
	lv_obj_del(homonoryimg);
	
	//内存回收
	lv_mem_free(framebuffer1);
	framebuffer1 = NULL;
	lv_mem_free(homonoryimg);
	homonoryimg = NULL;

	//开机后背景
	lv_style_set_bg_color(style1, LV_STATE_DEFAULT,LV_COLOR_MAKE(0x1C,0x8A,0xDB));
	lv_obj_add_style(bkg,LV_OBJ_PART_MAIN,style1);
	
	//内存申请
	lv_obj_t *bkg_img_bearpi0 = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	lv_obj_t *bkg_img_bearpi1 = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	lv_obj_t *bkg_img_bearpi2 = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	lv_obj_t *bkg_img_bearpi3 = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	lv_obj_t *bkg_img_bearpi4 = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	lv_obj_t *bkg_img_bearpi5 = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	lv_obj_t *bkg_img_bearpi6 = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	
	uint8_t* framebuffer2 = (uint8_t*)lv_mem_alloc(sizeof(uint8_t)*9600);
	//图片界面

	bkg_img_bearpi0 = lv_img_create(lv_scr_act(), NULL);
	bkg_img_bearpi1 = lv_img_create(lv_scr_act(), NULL);
	bkg_img_bearpi2 = lv_img_create(lv_scr_act(), NULL);
	bkg_img_bearpi3 = lv_img_create(lv_scr_act(), NULL);
	bkg_img_bearpi4 = lv_img_create(lv_scr_act(), NULL);
	bkg_img_bearpi5 = lv_img_create(lv_scr_act(), NULL);
	bkg_img_bearpi6 = lv_img_create(lv_scr_act(), NULL);
	
	//文件打开
	fs_res = lv_fs_open(lv_file, "S:/bear.bin", LV_FS_MODE_RD| LV_FS_MODE_WR);
	if ( fs_res != LV_FS_RES_OK )
		printf( "LVGL FS open error. (%d)\r\n", fs_res );
	offset = 0;
	offset += 4; //从offset=4
	lv_fs_seek(lv_file, offset);
	
	//分开显示图片 6
	fs_res = lv_fs_read(lv_file, (uint8_t *)framebuffer2, 9600,NULL);
	bkg_bearpi.data = (const uint8_t *)framebuffer2;
	lv_img_set_src(bkg_img_bearpi0, &bkg_bearpi);
	lv_obj_align(bkg_img_bearpi0, NULL, LV_ALIGN_CENTER, 0, -60);
	lv_task_handler();
	offset += 9600;
	fs_res = lv_fs_seek(lv_file, offset);
	osDelay(20);
	
	fs_res = lv_fs_read(lv_file, (uint8_t *)framebuffer2, 9600,NULL);
	bkg_bearpi.data = (const uint8_t *)framebuffer2;
	lv_img_set_src(bkg_img_bearpi1, &bkg_bearpi);
	lv_obj_align(bkg_img_bearpi1, NULL, LV_ALIGN_CENTER, 0, -40);
	lv_task_handler();
	offset += 9600;
	fs_res = lv_fs_seek(lv_file, offset);
	osDelay(20);
	
		fs_res = lv_fs_read(lv_file, (uint8_t *)framebuffer2, 9600,NULL);
	bkg_bearpi.data = (const uint8_t *)framebuffer2;
	lv_img_set_src(bkg_img_bearpi2, &bkg_bearpi);
	lv_obj_align(bkg_img_bearpi2, NULL, LV_ALIGN_CENTER, 0, -20);
	lv_task_handler();
	offset += 9600;
	fs_res = lv_fs_seek(lv_file, offset);
	osDelay(20);
	
		fs_res = lv_fs_read(lv_file, (uint8_t *)framebuffer2, 9600,NULL);
	bkg_bearpi.data = (const uint8_t *)framebuffer2;
	lv_img_set_src(bkg_img_bearpi3, &bkg_bearpi);
	lv_obj_align(bkg_img_bearpi3, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_task_handler();
	offset += 9600;
	fs_res = lv_fs_seek(lv_file, offset);
	osDelay(20);
	
	fs_res = lv_fs_read(lv_file, (uint8_t *)framebuffer2, 9600,NULL);
	bkg_bearpi.data = (const uint8_t *)framebuffer2;
	lv_img_set_src(bkg_img_bearpi4, &bkg_bearpi);
	lv_obj_align(bkg_img_bearpi4, NULL, LV_ALIGN_CENTER, 0, 20);
	lv_task_handler();
	offset += 9600;
	fs_res = lv_fs_seek(lv_file, offset);
	osDelay(20);
	
	fs_res = lv_fs_read(lv_file, (uint8_t *)framebuffer2, 9600,NULL);
	bkg_bearpi.data = (const uint8_t *)framebuffer2;
	lv_img_set_src(bkg_img_bearpi5, &bkg_bearpi);
	lv_obj_align(bkg_img_bearpi5, NULL, LV_ALIGN_CENTER, 0, 40);
	lv_task_handler();
	offset += 9600;
	fs_res = lv_fs_seek(lv_file, offset);
	osDelay(20);

	fs_res = lv_fs_read(lv_file, (uint8_t *)framebuffer2, 9600,NULL);
	bkg_bearpi.data = (const uint8_t *)framebuffer2;
	lv_img_set_src(bkg_img_bearpi6, &bkg_bearpi);
	lv_obj_align(bkg_img_bearpi6, NULL, LV_ALIGN_CENTER, 0, 60);
	lv_task_handler();
	offset += 9600;
	fs_res = lv_fs_seek(lv_file, offset);
	osDelay(20);
	
	//资源再回收
	lv_fs_close(lv_file);
	lv_mem_free(framebuffer2);
	framebuffer2 = NULL;
	
	//顶部图标
	lv_style_t *style2 = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t *));
	lv_obj_t *top = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	top = lv_label_create(lv_scr_act(), NULL);
	lv_style_init(style2);
	lv_style_set_text_font(style2, LV_STATE_DEFAULT, &lv_font_montserrat_40);
	lv_style_set_text_color(style2, LV_STATE_DEFAULT,LV_COLOR_MAKE(0xFA,0xFA,0xFA));
	lv_obj_add_style(top,LV_OBJ_PART_MAIN,style2);
	lv_label_set_text(top, LV_SYMBOL_WIFI);
	lv_obj_set_pos(top,5,5);
	
	//----------底部图标--------------
	lv_style_t *style3 = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t *));
	lv_style_init(style3);
	lv_style_set_text_font(style3, LV_STATE_DEFAULT, &my_font);
	lv_style_set_text_color(style3, LV_STATE_DEFAULT,LV_COLOR_MAKE(0xFA,0xFA,0xFA));


	lv_obj_t *bottom1 = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	bottom1 = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(bottom1, LV_SYMBOL_TEMP);
	lv_obj_set_pos(bottom1,15,195);
	
	lv_obj_add_style(bottom1,LV_OBJ_PART_MAIN,style3);

	lv_obj_t *bottom2 = (lv_obj_t *)lv_mem_alloc(sizeof(lv_obj_t *));
	bottom2 = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(bottom2,LV_SYMBOL_LIGHT);
	lv_obj_set_pos(bottom2,110,195);
	lv_obj_add_style(bottom2,LV_OBJ_PART_MAIN,style3);

	while(1)
	{
		lv_task_handler();
		osDelay(20);
	}

}
void Huawei_Task(void *argument)
{
	int len = 0;
	int rc = 0;
	unsigned char topic[]= POST_TOPIC;
	unsigned char payload[100]={0};
	
	uint8_t recv_dat[6] = {0};
	float temperature = 0.0;
	float humidity = 0.0;
	
	printf("ESP8266 Ready: 2S!\r\n");
	osDelay(1000);
	printf("ESP8266 Ready: 1S!\r\n");
	osDelay(1000);
	printf("ESP8266 Init!\r\n");
	esp8266_Connect_IOTServer();
	huawei_connect();
	osDelay(100);
	
	while(1)
	{

		if(SHT30_Read_Dat(recv_dat) == HAL_OK)
		{
			if(SHT30_Dat_To_Float(recv_dat, &temperature, &humidity)==0)
			{
				printf("temperature = %f, humidity = %f\r\n", temperature, humidity);
			}
			else
			{
				printf("crc check fail\r\n");
			}
		}
		else
		{
			printf("read data from sht30 fail\r\n");
		}
		
		sprintf((char*)payload,PAY_LOAD,(uint32_t)temperature,(uint32_t)humidity);
		
		memset(buf,0,buflen);
		MQTTString topicString = MQTTString_initializer;
		topicString.cstring = (char *)topic;
		
		len=MQTTSerialize_publish(buf,buflen,0,0,0,0,topicString,payload,strlen((char *)payload));
		
		rc = transport_sendPacketBuffer(3, buf, len);
		if(!rc)
			printf("Send OK!\r\n");
		else
			printf("Send WRONG!\r\n");
		osDelay(5000);
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
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();
  MX_LPUART1_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	
	SHT30_reset();
	if(SHT30_Init() == HAL_OK)
			printf("sht30 init ok.\r\n");
	else
			printf("sht30 init fail\r\n");

	osKernelInitialize();
  /* creation of uart_task */
	HAL_GPIO_WritePin(MOTO_GPIO_Port,MOTO_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
	
	
	
	DMA_SemaphoreHandle = osSemaphoreNew(1, 1, &DMA_Semaphore_attributes);
	TX_DMA_SemaphoreHandle = osSemaphoreNew(1, 1, &TX_DMA_Semaphore_attributes);
	RX_DMA_SemaphoreHandle = osSemaphoreNew(1, 1, &RX_DMA_Semaphore_attributes);
	
  led_taskHandle = osThreadNew(Led_Task, NULL, &led_task_attributes);
	lcd_taskHandle = osThreadNew(Lcd_Task, NULL, &lcd_task_attributes);
	huawei_taskHandle = osThreadNew(Huawei_Task, NULL, &huawei_task_attributes);

	osKernelStart();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_LPUART1
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_SDMMC1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSE;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 8;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_48M2CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

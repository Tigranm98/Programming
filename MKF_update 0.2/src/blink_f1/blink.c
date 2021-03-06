/* Copyright Instigate Robotics CJSC 2017 */

/* SPL headers */
#include <STM32F10x_StdPeriph_Driver/misc.h>
#include <STM32F10x_StdPeriph_Driver/stm32f10x.h>

/* Standard libraries headers */

/* Heasers from this project */
#include <debug/delay.h>
#include <debug/assert.h>

GPIO_InitTypeDef GPIO_InitStructure;
ErrorStatus HSEStartUpStatus;

void RCC_Configuration(void);
void NVIC_Configuration(void);

void RCC_Configuration(void)
{
        /* RCC system reset(for debug purpose) */
        RCC_DeInit();

        /* Enable HSE */
        RCC_HSEConfig(RCC_HSE_ON);

        /* Wait till HSE is ready */
        HSEStartUpStatus = RCC_WaitForHSEStartUp();

        if(HSEStartUpStatus == SUCCESS)
        {
                /* Enable Prefetch Buffer */
                FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

                /* Flash 2 wait state */
                FLASH_SetLatency(FLASH_Latency_2);

                /* HCLK = SYSCLK */
                RCC_HCLKConfig(RCC_SYSCLK_Div1);

                /* PCLK2 = HCLK */
                RCC_PCLK2Config(RCC_HCLK_Div1);

                /* PCLK1 = HCLK/2 */
                RCC_PCLK1Config(RCC_HCLK_Div2);

                /* PLLCLK = 8MHz * 9 = 72 MHz */
                RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

                /* Enable PLL */
                RCC_PLLCmd(ENABLE);

                /* Wait till PLL is ready */
                while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
                {
                }

                /* Select PLL as system clock source */
                RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

                /* Wait till PLL is used as system clock source */
                while(RCC_GetSYSCLKSource() != 0x08)
                {
                }
        }
}

void NVIC_Configuration(void)
{
#ifdef  VECT_TAB_RAM  
        /* Set the Vector Table base location at 0x20000000 */
        NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
        /* Set the Vector Table base location at 0x08000000 */
        NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}

int main(void)
{
        /* Configure the system clocks */
        RCC_Configuration();
        /* NVIC Configuration */
        NVIC_Configuration();
        /* Enable GPIOC clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        /* Configure PC.4 as Output push-pull */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        while (1)
        {
                /* Turn on led connected to PC.4 pin */
                GPIO_SetBits(GPIOC, GPIO_Pin_13);
                /* Insert delay */
                delay(0xAFFFFF);
                /* Turn off led connected to PC.4 pin */
                GPIO_ResetBits(GPIOC, GPIO_Pin_13);
                /* Insert delay */
                delay(0xAFFFFF);
        }
}


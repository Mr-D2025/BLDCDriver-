#include "sys.h"
#include "delay.h"
#include "Drv_TIM.h"
#include "Drv_BLDC.h"
#include "LED.h"
#include "arm_math.h"
#include "math.h"
#include "OLED.h"


int main(void)
{
	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    delay_init(168);		  
    LED_Init();
    
    TIM3_Int_Init(5000,8400);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数5000次为500ms	
    TIM1_IO_Init();
    delay_ms(1000);
    PWM_Centre_Init();

    while(1)
    {

    
    
    }
}
	


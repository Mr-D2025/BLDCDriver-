#include "sys.h"
#include "Drv_BLDC.h"
#include "delay.h"

void TIM1_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIOA和GPIOB时钟（同时使能TIM1时钟，通常在TIM初始化函数中使能，这里只做注释）
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;            // 复用功能模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      // 高速
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;            // 上拉，防止引脚悬空，可根据实际电路调整
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 连接PAx到TIM1的AF 
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);   // PA8 -> TIM1_CH1 (A相)
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1);   // PA9 -> TIM1_CH2 (B相)
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_TIM1);  // PA10 -> TIM1_CH3 (C相)

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;            // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      // 高速
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;          // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 互补输出通常由定时器控制，可不带上拉，根据实际电路选择
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 连接PBx到TIM1的互补通道AF
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_TIM1);  // PB13 -> TIM1_CH1N (A相下桥)
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_TIM1);  // PB14 -> TIM1_CH2N (B相下桥)
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_TIM1);  // PB15 -> TIM1_CH3N (C相下桥)
    
}

void TIM1_TB_Init(uint32_t arr, uint32_t psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	
	// 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    // 配置TIM1时基单元
    TIM_TimeBaseStructure.TIM_Period = arr - 1;               // 自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = psc - 1;            // 预分频器
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1; // 向上向下计数模式
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
}

void TIM1_OC_Init(void)
{
    
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    
    // 配置PWM输出通道（只配置上桥臂的3个通道）
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;         // PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 使能输出
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; // 使能互补输出
    TIM_OCInitStructure.TIM_Pulse = 0;                        // 初始占空比为0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 高电平有效
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    
    // 初始化三个通道
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);  // CH1 - A相
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);  // CH2 - B相
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);  // CH3 - C相
    
    //使能预装载寄存器
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    
}

void TIM1_BDT_Init(void)
{
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
    
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;       // 运行模式下关闭状态使能
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;       // 空闲模式下关闭状态使能
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_1;            // 锁定级别1
    TIM_BDTRInitStructure.TIM_DeadTime = 0x20;                    // 死区时间值
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;           // 使能刹车输入
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low; // 刹车低电平有效
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Disable; // 自动输出使能
    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);
}

void TIM1_IT_Init(void)
{
    // 配置TIM1更新中断的NVIC（优先级低于EXTI）
    NVIC_InitTypeDef NVIC_InitStructure;
	
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  // 低于EXTI的0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
     // 清除更新标志
    TIM_ClearFlag(TIM1, TIM_FLAG_Update);

	// 使能TIM1更新中断（用于PWM周期控制）
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
}

void PWM_Centre_Init(void)
{
	TIM1_TB_Init(1000, 4);
	TIM1_OC_Init();
    TIM1_BDT_Init();
	TIM1_IT_Init();
    TIM_CtrlPWMOutputs(TIM1, ENABLE);	//使能主输出
    TIM_Cmd(TIM1, ENABLE);				//使能定时器
}

static const uint16_t sineWave[] = {
    514, 523, 532, 541, 549, 558, 567, 576, 585, 594, 603, 611, 620, 629, 638, 646,
    655, 663, 672, 680, 689, 697, 705, 714, 722, 730, 738, 746, 754, 762, 769, 777,
    785, 792, 800, 807, 814, 821, 828, 835, 842, 849, 856, 862, 869, 875, 881, 887,
    893, 899, 905, 911, 916, 922, 927, 932, 937, 942, 947, 952, 956, 960, 965, 969,
    973, 976, 980, 984, 987, 990, 993, 996, 999, 1002, 1004, 1007, 1009, 1011, 1013, 1015,
    1016, 1018, 1019, 1020, 1021, 1022, 1022, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1022, 1021,
    1020, 1019, 1018, 1017, 1015, 1014, 1012, 1010, 1008, 1005, 1003, 1000, 998, 995, 992, 989,
    985, 982, 978, 975, 971, 967, 963, 958, 954, 949, 945, 940, 935, 930, 924, 919,
    914, 908, 902, 896, 890, 884, 878, 872, 866, 859, 852, 846, 839, 832, 825, 818,
    811, 803, 796, 788, 781, 773, 766, 758, 750, 742, 734, 726, 718, 709, 701, 693,
    685, 676, 668, 659, 650, 642, 633, 624, 616, 607, 598, 589, 581, 572, 563, 554,
    545, 536, 527, 518, 509, 500, 491, 482, 474, 465, 456, 447, 438, 429, 420, 412,
    403, 394, 385, 377, 368, 360, 351, 343, 334, 326, 318, 309, 301, 293, 285, 277,
    269, 261, 254, 246, 238, 231, 223, 216, 209, 202, 195, 188, 181, 174, 167, 161,
    154, 148, 142, 136, 130, 124, 118, 112, 107, 101, 96, 91, 86, 81, 76, 71,
    67, 63, 58, 54, 50, 47, 43, 39, 36, 33, 30, 27, 24, 21, 19, 16,
    14, 12, 10, 8, 7, 5, 4, 3, 2, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 4, 5, 6, 8, 9, 11, 13, 15, 18, 20, 23,
    25, 28, 31, 34, 38, 41, 45, 48, 52, 56, 60, 65, 69, 74, 78, 83,
    88, 93, 99, 104, 109, 115, 121, 127, 133, 139, 145, 151, 157, 164, 171, 177,
    184, 191, 198, 205, 212, 220, 227, 235, 242, 250, 257, 265, 273, 281, 289, 297,
    305, 314, 322, 330, 338, 347, 355, 364, 373, 381, 390, 399, 407, 416, 425, 434,
    442, 451, 460, 469, 478, 487, 496, 505
};

// 计算正弦波表有多少个元素（应该是360个）
uint16_t SINE_POINT_NUM = sizeof(sineWave) / sizeof(sineWave[0]);


// TIM1更新中断服务函数
void TIM1_UP_TIM10_IRQHandler(void)
{
    static uint16_t sine_index = 0;         // 正弦波表索引
    static uint16_t step_cnt = 0;           // 步进计数，用于控制角度变化速度
    static int16_t direction = 1;           // 旋转方向：1正转，-1反转
    
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)	// 检查是否是TIM1更新中断
    {
		step_cnt++;
        
        // 每 step_class 次中断更新一次角度（控制电机转速）
        if (step_cnt >= 2)
        {
            // 更新角度位置（根据方向增减）
            sine_index += direction;
            
            // 360°循环处理
            if (sine_index >= SINE_POINT_NUM)
            {
                sine_index = 0;  // 回到0°
            }
            else if (sine_index < 0)  // 处理负数情况（如果用有符号变量）
            {
                sine_index = SINE_POINT_NUM - 1;
            }
            
            // 计算三相占空比（带120°相位差）
            uint16_t a_duty = sineWave[sine_index];                          // A相 0°
            uint16_t b_duty = sineWave[(sine_index + 119) % SINE_POINT_NUM]; // B相 120°
            uint16_t c_duty = sineWave[(sine_index + 239) % SINE_POINT_NUM]; // C相 240°
            
            // 根据当前速度系数调整占空比
            uint32_t temp_a = (uint32_t)a_duty * 100;
            uint32_t temp_b = (uint32_t)b_duty * 100;
            uint32_t temp_c = (uint32_t)c_duty * 100;
            
            // 右移10位（除以1024）归一化
            temp_a >>= 10;
            temp_b >>= 10;
            temp_c >>= 10;
            
            // 写入占空比到定时器比较寄存器
            TIM_SetCompare1(TIM1, (uint16_t)(1000-temp_a));  // A相
            TIM_SetCompare2(TIM1, (uint16_t)(1000-temp_b));  // B相
            TIM_SetCompare3(TIM1, (uint16_t)(1000-temp_c));  // C相
            
            step_cnt = 0;  // 重置步进计数
        }
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);		// 清除TIM1更新中断标志
    }
}

#ifndef __DRV_TIM_H
#define __DRV_TIM_H
#include "sys.h"


void TIM2_Init(void);
uint32_t Record_Commutation_Interval(void);
void TIM3_Int_Init(uint16_t arr,uint16_t psc);

#endif

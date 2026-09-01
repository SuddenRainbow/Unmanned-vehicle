#include "stm32f10x.h"
#include "sys.h"
#include "encoder.h"
#include "motor.h"
#include "speed_ctrl.h"

int main(void)
{
    Stm32_Clock_Init(9);				//外部时钟8Mhz 9倍频  8*9= 72mhz
    MY_NVIC_PriorityGroupConfig(2);	//=====中断优先级分组
    uart_init(115200);	            //=====串口初始化为115200
    JTAG_Set(JTAG_SWD_DISABLE);     //=====关闭JTAG接口
    JTAG_Set(SWD_ENABLE);           //=====打开SWD接口

    PWM_Init(7199,9);               //=====电机PWM初始化 1000Hz
    Encoder_Init_TIM2();            //=====左轮编码器初始化
    Encoder_Init_TIM3();            //=====右轮编码器初始化
    colorful_led_Init();            //=====炫彩灯初始化

    SysTick_Config(72000000/1000);  //滴答定时器, 每1ms触发一次中断

    printf("QST青软\r\n");
    printf("等待Hi3861协议帧: 0xFC 左方向 左速度 右方向 右速度 0xFD\r\n");
    /**主要程序**/
    while(1)
    {
    }
}

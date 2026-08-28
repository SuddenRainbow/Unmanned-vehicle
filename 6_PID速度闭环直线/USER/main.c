#include "stm32f10x.h"
#include "sys.h"
#include "encoder.h"
#include "motor.h"
#include "speed_ctrl.h"
#include "control_system.h"

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
#if AUTO_RUN
    delay_ms(3000);      //上电后延时3秒,先把小车放到地上
    Car_run=1;           //自动启动速度闭环,一直跑
    printf("自动启动, 目标=%d\r\n", Target_Speed);
#else
    printf("命令: START启动 STOPP停止 Txxxx设定目标速度(脉冲/100ms)\r\n");
#endif
    /**主要程序**/
    while(1)
    {
        if(USART_RX_STA==1)          //START:启动
        {
            Car_run=1;
            USART_RX_STA=0;
            printf("启动, 目标=%d\r\n", Target_Speed);
        }
        if(USART_RX_STA==2)          //STOPP:停止
        {
            Car_run=0;
            USART_RX_STA=0;
            printf("停止\r\n");
        }
        if(USART_RX_STA==3)          //Txxxx:设定目标速度
        {
            u16 t = (USART_RX_BUF[1]-'0')*1000 + (USART_RX_BUF[2]-'0')*100
                  + (USART_RX_BUF[3]-'0')*10  + (USART_RX_BUF[4]-'0');
            if(t<=7199) Target_Speed=t;
            USART_RX_STA=0;
            printf("目标速度=%d\r\n", Target_Speed);
        }
    }
}

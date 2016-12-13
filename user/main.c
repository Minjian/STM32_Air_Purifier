/*
Resources：
LED1~12
RGB_LED

USART1 for debug
USART5 for PM2.5 sensor

Fans:
	FANS_PWR
	FANS_SPEED
	FANS_PWM

3*4 Keyborad:
	TOUCH_COL1/2/3
	TOUCH_ROW1/2/3/4

BUZZER
TILT_IO, Tilt Sensor

ZIGBEE for remote control    

12 Button Functions
4*3
1--Power	12--ECO Mode 			11--Fan Speed Control
2--1h timer  9--Air Quality Good	 8--Breath LED Mode(RED)
3--2h timer  6--Air Quality Middle	10--Breath LED Mode(YELLOW)
4--4h timer  5--Air Quality Bad 	 7--Breath LED Mode(BLUE)

Load Tool:ST_link

The main function:
	1.3*4 Keyboard Handler
	2.Zigbee Handler
	3.Return the current status of the Air Purifier
*/

#include "stm32f4xx.h"
#include "fans.h"
#include "delay.h"
#include "usart.h"
#include "breath_led.h"
#include "key.h"
#include "pm2_5.h"
#include "led.h"
#include "zigbee.h"
#include "buzzer.h"
#include "dht11.h"
#include "voc.h"
#include "power_output.h"
#include "tilt.h"

#define FANS_OFF 100
#define SPEED_LEVEL_1 50
#define SPEED_LEVEL_2 30
#define SPEED_LEVEL_3 1

/*
Key and LED mapping:
key  led
1  10
2  11
3  12
4   7
5   8
6   9
7   4
8   5
9   6
10  1
11  2
12  3
*/

u8 led_flag = 1; //0 means close, 1 means open
u8 fans_speed = 0; //Can be Level 0/1/2/3


u32 bl = 8;  //Breath LED step
u8  bl_flag = 0;
u8  before_bl = 0; //for previous LED color
u8  now_bl = 0; //for current LED color
				//0--close  1--Red  2--Blue  3--Purple  4--Yellow
u8 bl_dir = 1; //Breath LED direction 0--descending 1--acsending
u8 bl_open_flag = 1; 
u8 bl_run_flag = 0;

u8 key_flag = 0;
u8 before_key_val = 0;//for previous key value
u8 eco_flag = 0; //ECO Mode, 
u8 key_time = 0; 

u8 led1_stat = 0;
u8 led2_stat = 0;
u8 led3_stat = 0;
u8 led4_stat = 0;
u8 led5_stat = 0;
u8 led6_stat = 0;
u8 led7_stat = 0;
u8 led8_stat = 0;
u8 led9_stat = 0;
u8 led10_stat = 0;
u8 led11_stat = 0;
u8 led12_stat = 0;

u32 sys_off_flag = 0xf;
u32  switch_flag = 0;
u32 sys_timer = 0;

u8  led_time_flag = 1;
u32 led_times = 0;

u8 tilt_step = 0;
u8 get_online_flag = 1;

//Protocal of Nanjing Internet of Things
u8 fb_cmd[32] = {0};
const u8  pm_id[]      = {'O', '0'};
const u8  fan_id[]     = {'O', '1'};
const u8  pm_type[]    = {0x10, 0x01};
const u8  r_head[]     = {0xff, 0x55};
const u8  r_tail[]     = {0xff, 0xaa};
const u8  t_head[]     = {0xff, 0xaa};
const u8  t_tail[]     = {0xff, 0x55};
const u8 changpai[]    = {0xe6, 0xb0, 
                         0xb8, 0xe5, 
                         0xa4, 0xa7, 
                         0xe7, 0x8e, 
                         0xaf, 0xe4, 
                         0xbf, 0x9d};

//License   EC92CEB5CC39
													
const u8  license[] = {'E', 'C', '9', '2', 'C', 'E', 
					   'B', '5', 'C', 'C', '3', '9'};

const u8  hangye[]       = {0x01, 0x02};
const u8  dev_type_pm[]  = {0x00, 0x08};
const u8  dev_type_fan[] = {0x00, 0x02};

const u8 event_type[][2] = {
    {0xf0, 0xf0}, //Online Request
    {0xf0, 0x02}, //Request ID
    {0xf0, 0x03}, //Request Initial Status
    {0xf0, 0x0f}, //Request Attributes
    {0xf0, 0x10}, //Reset, request ID again
    {0xf0, 0x11}, //Reset, request status
	{0xf0, 0xe0}, //Offline Request
};

u8  zigbee_cmd = 0;
u8  zigbee_state = 0;
u8  zigbee_cmd_len = 0;

u8  parse_cmd[32] = {0};
	
void  bl_all_off()
{
	led10_stat = 0;
	led7_stat = 0;
	led4_stat = 0;
	bl = 10;
	PCout(13) = 0;
	PCout(14) = 0;
	PCout(15) = 0;

	before_bl = 0;
	now_bl = 0;
	bl_dir=1;
}


void system_on()
{
 	//Turn on LED, Breath LED, Fans and Timers
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	led_flag = 1;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
}

void system_off()
{
	//Turn off LED, Breath LED, Fans and Timers
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,DISABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,DISABLE);
	led_all_off();
	led_flag = 0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, DISABLE);
}

//num: flash how man timers
//time: interval between two flashes
void flash_led(u8 num, u16 time)
{
	int i=0;
	for(;i<num;i++) {
		led_all_on();
		delay_ms(time);
		led_all_off2();
		delay_ms(time);
	}
}

void  standby_state()
{
	key_flag = 0;
	led_all_off();
}

u8 send_data_state = 0;
u8 dht_data[4];
u8 adc_value_h;
u8 adc_value_l;

extern struct pm2_5 pm_data;

int main()
{
	u8 i=0;
	u8 key = 0;
	u8 timer_hour = 0;
	u8 timer_min = 0;
	
	u32 pm2_5_step = 0;
	
	u32 vout_h_val = 0;
	u32 vout_l_val = 0;
	u32 pm_val = 0;
	u16 adc_result = 0;
	
	delay_init(168);

	uart_init(115200);
	led_init();
	
	flash_led(3, 300);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	fans_init(100-1, 84-1);
	TIM_SetCompare4(TIM1, FANS_OFF);
	breath_time4_init(2000-1,1);
	
	led_time5_init(2000-1,1);
	TIM_SetCompare1(TIM5, 1600);
	
	TIM3_Int_Init(50000-1,8400-1);
	TIM7_Int_Init(20-1,8400-1);
	
	breath_led_init();
	
	key_init();

	pm2_5_init();
	zigbee_init();
	buzzer_init();
	DHT11_Init();
	voc_init();
	power_output_init();
	tilt_init();

	while(1)  {

		//Tilt Sensor
		//0--normal, 1--tilt, need to power off and reset
		if(1 == TITL_IN){
			tilt_step++;
			if(100 == tilt_step) {
				tilt_step--;
				TIM_Cmd(TIM5, DISABLE);
				GPIO_SetBits(GPIOA, GPIO_Pin_0);
				led_all_on();
				delay_ms(500);
				GPIO_ResetBits(GPIOA, GPIO_Pin_0);
				led_all_off2();
				delay_ms(500);			
				key_flag = 0;
			}
		} else if((0 == TITL_IN)&&(99 == tilt_step)){ 		
			TIM_Cmd(TIM5, ENABLE);
			tilt_step = 0;
			GPIO_ResetBits(GPIOA, GPIO_Pin_0);
		}
		
		if(0 == get_online_flag) {
			TIM_Cmd(TIM5, DISABLE);
			flash_led(1, 300);
			TIM_Cmd(TIM5, ENABLE);
			key_flag = 0;
		}
		

		//Keyboard Handler
		key = key_scan();
		DHT11_Read_Data(&dht_data[2], &dht_data[3], &dht_data[0], &dht_data[1]);
		adc_result = get_voc_value();
		adc_value_l = adc_result & 0xff;
		adc_value_h = (adc_result >> 8) & 0xff;

		if((0 == key) && (1 == led_time_flag)) led_times++;
		else if( 0 != key){
			led_times = 0;
		}			
	
		if(500 == led_times ){
			led_time_flag = 0;
			led_times = 0;
			TIM_Cmd(TIM5, DISABLE);
			led_all_off2();
		}
			
		if(key != before_key_val) {
			if(0 == led_time_flag) {
				before_key_val = key;
				key = 0;
				TIM_Cmd(TIM5, ENABLE);
			}
			
			if (key != 0) {
				buzzer_action();
			}
			switch (key) {
				case 1:	
					led10_stat =  !led10_stat;
					led7_stat = 0;
					led4_stat = 0;
					now_bl = 2;
					if(0 == led10_stat) bl_all_off();
					break;
				case 2:
					key_time++;
					if(5 == key_time) {
						TIM_Cmd(TIM5, DISABLE);
						flash_led(1, 100);
						TIM_Cmd(TIM5, ENABLE);
						
						get_online_flag = 0 ;
						
						fb_cmd[0] = t_head[0];  //0xff
						fb_cmd[1] = t_head[1];  //0xaa
						fb_cmd[2] = 0x03;
						fb_cmd[3] = event_type[0][0]; //0xf0
						fb_cmd[4] = event_type[0][1]; //0xf0
						//fb_cmd[5] = fan_id[0]; //'0'
						//fb_cmd[6] = fan_id[1]; //'1'
						fb_cmd[5] = t_tail[0];  //0xff
						fb_cmd[6] = t_tail[1];  //0x55
						
						for (i = 0; i < 7; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}
						key_time = 0;
					}				
					break;
				case 3:
					led12_stat = !led12_stat;
					led6_stat = 0;
					led9_stat = 0;
					sys_off_flag = 4*60*60*500;
					switch_flag = led12_stat;			
					break;
				case 4:
					led7_stat = !led7_stat;
					led4_stat = 0;
					led10_stat = 0;
					now_bl = 4;
					if(0 == led7_stat) bl_all_off();
					break;
				case 5:			
					break;
				case 6:
					led9_stat = !led9_stat;
					led12_stat = 0;
					led6_stat = 0;
					sys_off_flag = 2*60*60*500;
					switch_flag = led9_stat;
					break;
				case 7:
					led4_stat = !led4_stat;
					led7_stat = 0;
					led10_stat = 0;		
					now_bl=3;
					if(0 == led4_stat) bl_all_off();
					break;
				case 8:
					//Offline request
					key_time++;
					if(5 == key_time) {
						TIM_Cmd(TIM5, DISABLE);
						flash_led(4, 200);
						TIM_Cmd(TIM5, ENABLE);
						fb_cmd[0] = t_head[0];  //0xff
						fb_cmd[1] = t_head[1];  //0xaa
						fb_cmd[2] = 0x03;
						fb_cmd[3] = event_type[6][0]; //0xf0
						fb_cmd[4] = event_type[6][1]; //0xe0
						fb_cmd[5] = t_tail[0];  //0xff
						fb_cmd[6] = t_tail[1];  //0x55
						for (i = 0; i < 7; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}
						key_time = 0;
					}
					break;
				case 9:
					led6_stat = !led6_stat;
					led12_stat = 0;
					led9_stat = 0;
					sys_off_flag = 60*60*500;
					switch_flag = led6_stat;
					break;
				case 10:
					led1_stat = 1;
					if(0 == eco_flag) {
						fans_speed++;
				
						if(fans_speed >= 4){
							fans_speed = 1;
						}
						
						switch (fans_speed) {
							case 1:
								TIM_SetCompare4(TIM1, SPEED_LEVEL_1);
								break;
							case 2:
								TIM_SetCompare4(TIM1, SPEED_LEVEL_2);
								break;
							case 3:
								TIM_SetCompare4(TIM1, SPEED_LEVEL_3);
								break;
							default:
								break;
						}
					}
					break;
				case 11:
          			eco_flag = !eco_flag;
					led2_stat = eco_flag;
					break;
				case 12: 
					if(0 == key_flag) {
						led3_stat = 1;
						key_flag = 1;
						fans_speed = 1;
						TIM_SetCompare4(TIM1, SPEED_LEVEL_1);
						power_output_on();
						get_online_flag = 1;
					} else if(1 == key_flag){
						led3_stat = 0;
						key_flag = 0;
					}
					break;
				default:
					break;
			}				
			if(1 == led_time_flag)before_key_val = key;
			else if(0 == led_time_flag)  led_time_flag = 1;
		}
		
		//Turn off system
		if((0 == key_flag) || (sys_timer == sys_off_flag)) {
			fans_speed = 0;
			TIM_SetCompare4(TIM1, FANS_OFF);		
			bl = 10;
			PCout(13) = 0;
			PCout(14) = 0;
			PCout(15) = 0;	
			before_bl = 0;
			now_bl = 0;
			bl_dir=1;
			led_all_off();
			
			led1_stat = 0;
			led2_stat = 0;
			led3_stat = 0;
			led4_stat = 0;
			//led5_stat = 0;
			led6_stat = 0;
			led7_stat = 0;
			//led8_stat = 0;
			led9_stat = 0;
			led10_stat = 0;
			//led11_stat = 0;
			led12_stat = 0;
						
			eco_flag = 0;
			power_output_off();
			switch_flag = 0;
		}
		
		if(50 == pm2_5_step) {
			
			vout_h_val = vout_h_val / 50;	
			vout_l_val = vout_l_val / 50;

			pm_val = vout_h_val * 256 + vout_l_val;
			
			if(pm_val < 0x20 ) { //Air Quality Good
				led5_stat = 1;
				led11_stat =0;
				led8_stat = 0;
				if((0 != eco_flag) &&(1 == key_flag)) {
					TIM_SetCompare4(TIM1, SPEED_LEVEL_1);
					fans_speed = 1;
				}
			}else if((pm_val >= 0x20) && (pm_val < 0xff) ) { //Air Quality Middle		
				led8_stat = 1;
				led5_stat = 0;
				led11_stat = 0;
				
				led_time_flag = 1;
				led_times = 0;
				TIM_Cmd(TIM5, ENABLE);
				if((0 != eco_flag)  && (1 == key_flag)) {
					TIM_SetCompare4(TIM1, SPEED_LEVEL_2);
					fans_speed = 2;
				}
			}else if(pm_val >= 0xff) { //Air Quality Bad
				TIM_Cmd(TIM5, ENABLE);
				led11_stat = 1;
				led5_stat = 0;
				led8_stat = 0;
				led_time_flag = 1;
				led_times = 0;
				TIM_Cmd(TIM5, ENABLE);
				if((0 != eco_flag) && (1 == key_flag)) {
					fans_speed = 3;
					TIM_SetCompare4(TIM1, SPEED_LEVEL_3);
				}
			}
			
			pm2_5_step = 0;
			vout_h_val = 0;
			vout_l_val = 0;
		}
		
		pm2_5_step++;
		vout_h_val += pm_data.data[1];
		vout_l_val += pm_data.data[2];
/*
    {0xf0, 0xf0}, //Online Request
    {0xf0, 0x02}, //Request ID
    {0xf0, 0x03}, //Request Initial Status
    {0xf0, 0x0f}, //Request Attributes
    {0xf0, 0x10}, //Reset, request ID again
    {0xf0, 0x11}, //Reset, request status
	{0xf0, 0xe0}, //Offline Request
*/
		if((0x1b == zigbee_state)) {
			if(0xf0 == parse_cmd[1]) {
				switch (parse_cmd[2]) {
					case 0xf0: //Online
						TIM_Cmd(TIM5, DISABLE);
						led_all_on();
						delay_ms(2000);
						led_all_off2();
						delay_ms(1000);
						get_online_flag = 1;
						
						TIM_Cmd(TIM5, ENABLE);
						break;
					case 0xe0: //Offline
						break;
					case 0x02:
						fb_cmd[0] = t_head[0];  //0xff
						fb_cmd[1] = t_head[1];  //0xaa
						fb_cmd[2] = 0x05;
						fb_cmd[3] = event_type[1][0]; //0xf0
						fb_cmd[4] = event_type[1][1]; //0x02
						fb_cmd[5] = fan_id[0]; //'0'
						fb_cmd[6] = fan_id[1]; //'1'
						fb_cmd[7] = t_tail[0];  //0xff
						fb_cmd[8] = t_tail[1];  //0x55
					
						for (i = 0; i < 9; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}				  	
						break;
					case 0x03:
						fb_cmd[0] = t_head[0];
						fb_cmd[1] = t_head[1];
						fb_cmd[2] = 0x05;
						fb_cmd[3] = event_type[2][0];
						fb_cmd[4] = event_type[2][1];
						fb_cmd[5] = 0; 
						fb_cmd[6] = 0; 
						fb_cmd[7] = t_tail[0];
						fb_cmd[8] = t_tail[1];
						for (i = 0; i < 9; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}
						break;
					case 0x0f:
						fb_cmd[0] = t_head[0];
						fb_cmd[1] = t_head[1];
						fb_cmd[2] = 0xf;
						fb_cmd[3] = event_type[3][0];
						fb_cmd[4] = 0x04;
						fb_cmd[5] = changpai[0];
						fb_cmd[6] = changpai[1];
						fb_cmd[7] = changpai[2];
						fb_cmd[8] = changpai[3];
						fb_cmd[9] = changpai[4];
						fb_cmd[10] = changpai[5];
						fb_cmd[11] = changpai[6];
						fb_cmd[12] = changpai[7];
						fb_cmd[13] = changpai[8];
						fb_cmd[14] = changpai[9];
						fb_cmd[15] = changpai[10];
						fb_cmd[16] = changpai[11];
						fb_cmd[17] = t_tail[0];
						fb_cmd[18] = t_tail[1];
						for (i = 0; i < 19; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}
						
						delay_ms(1000);
						fb_cmd[0] = t_head[0];
						fb_cmd[1] = t_head[1];
						fb_cmd[2] = 0x05;
						fb_cmd[3] = event_type[3][0];
						fb_cmd[4] = 0x05;
						fb_cmd[5] = dev_type_fan[0];
						fb_cmd[6] = dev_type_fan[1];
						fb_cmd[7] = t_tail[0];
						fb_cmd[8] = t_tail[1];
						for (i = 0; i < 9; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}
						delay_ms(1000);
						fb_cmd[0] = t_head[0];
						fb_cmd[1] = t_head[1];
						fb_cmd[2] = 0x0f;
						fb_cmd[3] = event_type[3][0];
						fb_cmd[4] = 0x06;
						fb_cmd[5] = license[0];
						fb_cmd[6] = license[1];
						fb_cmd[7] = license[2];
						fb_cmd[8] = license[3];
						fb_cmd[9] = license[4];
						fb_cmd[10] = license[5];
						fb_cmd[11] = license[6];
						fb_cmd[12] = license[7];
						fb_cmd[13] = license[8];
						fb_cmd[14] = license[9];
						fb_cmd[15] = license[10];
						fb_cmd[16] = license[11];
						//fb_cmd[17] = license[12];
						fb_cmd[17] = t_tail[0];
						fb_cmd[18] = t_tail[1];
						for (i = 0; i < 19; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}
						delay_ms(1000);
						fb_cmd[0] = t_head[0];
						fb_cmd[1] = t_head[1];
						fb_cmd[2] = 0x05;
						fb_cmd[3] = event_type[3][0];
						fb_cmd[4] = 0x07;
						fb_cmd[5] = 0x00;
						fb_cmd[6] = 0x00;
						fb_cmd[7] = t_tail[0];
						fb_cmd[8] = t_tail[1];
						for (i = 0; i < 9; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}
						delay_ms(1000);
						fb_cmd[0] = t_head[0];
						fb_cmd[1] = t_head[1];
						fb_cmd[2] = 0x05;
						fb_cmd[3] = event_type[3][0];
						fb_cmd[4] = 0x08;
						fb_cmd[5] = 0x00;
						fb_cmd[6] = 0x00;
						fb_cmd[7] = t_tail[0];
						fb_cmd[8] = t_tail[1];
						for (i = 0; i < 9; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}
						break;
					case 0x10:
						fb_cmd[0] = t_head[0];
						fb_cmd[1] = t_head[1];
						fb_cmd[2] = 0x05;
						fb_cmd[3] = event_type[4][0];
						fb_cmd[4] = event_type[4][1];
						fb_cmd[5] = fan_id[0]; //'0'
						fb_cmd[6] = fan_id[1]; //'1'
						fb_cmd[7] = t_tail[0];
						fb_cmd[8] = t_tail[1];
						for (i = 0; i < 9; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}
						break;
					case 0x11:
						fb_cmd[0] = t_head[0];
						fb_cmd[1] = t_head[1];
						fb_cmd[2] = 0x05;
						fb_cmd[3] = event_type[5][0];
						fb_cmd[4] = event_type[5][1];
						fb_cmd[5] = 0x00;
						fb_cmd[6] = 0x00;
						fb_cmd[7] = t_tail[0];
						fb_cmd[8] = t_tail[1];
						for (i = 0; i < 9; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}		
						break;
					default:
						break;
				}
			}
		
		if(0x10 == parse_cmd[1]) {		
				if ((0x04 == parse_cmd[3]) && (0x04 == parse_cmd[0])) {
					  fb_cmd[0] = t_head[0];
						fb_cmd[1] = t_head[1];
						fb_cmd[2] = 0x10;
						fb_cmd[3] = 0x10;
						fb_cmd[4] = 0x01;
						fb_cmd[5] = 0x04;
						fb_cmd[6] = pm_data.data[1];
						fb_cmd[7] = pm_data.data[2];
						if (send_data_state == 0) {
							fb_cmd[8] = dht_data[2];
							fb_cmd[9] = dht_data[3];
							fb_cmd[10] = dht_data[0];
							fb_cmd[11] = dht_data[1];
							send_data_state = 1;
						} else {
							fb_cmd[8] = 0xff;
							fb_cmd[9] = 0xff;
							fb_cmd[10] = adc_value_h;
							fb_cmd[11] = adc_value_l;
							send_data_state = 0;
						}
						fb_cmd[12] = key_flag;
						if(0 ==  switch_flag){
							timer_hour = 0;
							timer_min = 0;
						} else if(1 == switch_flag) {
							timer_hour = (sys_off_flag-sys_timer)/(60*60*500);
							timer_min = (sys_off_flag-sys_timer)/(500*60)%60;
						}
						
						
						fb_cmd[13] = timer_hour;			
						fb_cmd[14] = timer_min;
						fb_cmd[15] = eco_flag; 
						fb_cmd[16] = now_bl; 
						fb_cmd[17] = fans_speed;
						fb_cmd[18] = t_tail[0];
						fb_cmd[19] = t_tail[1];
						for (i = 0; i < 20; ++i) {
							while((USART3->SR&0X40)==0);
							USART3->DR = fb_cmd[i];
						}
				}
				
				if((0x05 == parse_cmd[0]) && (0x02 == parse_cmd[3])
					  && (0 == eco_flag) && (1 == key_flag)) {						
						if(0 == parse_cmd[4]) led1_stat = 0;
						else led1_stat = 1;
						switch (parse_cmd[4]) {
							case 0:
								fans_speed = 0;
								TIM_SetCompare4(TIM1, FANS_OFF);
								break;
							case 1:
								fans_speed = 1;
								TIM_SetCompare4(TIM1, SPEED_LEVEL_1);
								break;
							case 2:
								fans_speed = 2;
								TIM_SetCompare4(TIM1, SPEED_LEVEL_2);
								break;
							case 3:
								fans_speed = 3;
								TIM_SetCompare4(TIM1, SPEED_LEVEL_3);
								break;
							default:
								break;
						}
				}
						
				
				//0x1 0x00  System off
				//0x1 0x01  System on
				//0x1 0x02  Romantic Mode
				//0x1 0x03  Sleep Mode
				//0x1 0x04  Star Mode
				//0x1 0x05  Breath LED Off
				//0x1 0x06  ECO Mode On
				//0x1 0x07  ECO Mode Off
				//0x1 0x08  1h timer
				//0x1 0x09  2h timer
				//0x1 0x0a  4h timer
				//0x1 0x0b  Timer Off
				//0x1 0x0c  Back to current status     
				//CMD2 0x0/0x1(Power Button Status)
				//CMD3 0x0  remain time
				//CMD4 0x0  remain minutes
				//CMD5 0x0/0x01  ECO status
				//CMD6  0x0 0x1 0x2 0x3 Breath LED Status
				//CMD7 0X0 0X01 0X2 0X3  Fan Speed
				if((0x05 == parse_cmd[0]) && (0x01 == parse_cmd[3])) {	
					
						switch (parse_cmd[4]) {
							case 0x00:
								TIM_Cmd(TIM5, ENABLE);
								led3_stat = 0;
								key_flag = 0;
							  
								break;
							
							case 0x01:
								TIM_Cmd(TIM5, ENABLE);
								led3_stat = 1;
								key_flag = 1;
								fans_speed = 1;
								TIM_SetCompare4(TIM1, SPEED_LEVEL_1);
								//fans_speed++;
								power_output_on();
								get_online_flag = 1;
								break;
							
							case 0x02:
								TIM_Cmd(TIM5, ENABLE);
								led4_stat = 1;
								led7_stat = 0;
								led10_stat = 0;
								now_bl=3;
								break;
							
							case 0x03:
								TIM_Cmd(TIM5, ENABLE);
								led7_stat = 1;
								led4_stat = 0;
								led10_stat = 0;
								now_bl = 4;
								break;
							
							case 0x04:
								TIM_Cmd(TIM5, ENABLE);
								led10_stat =  1;
								led7_stat = 0;
								led4_stat = 0;
								now_bl = 2;	
								break;
							
							case  0x05:
								TIM_Cmd(TIM5, ENABLE);
								//buzzer_action();
								led10_stat =  0;
								led7_stat = 0;
								led4_stat = 0;
								bl = 8;
								PCout(13) = 0;
								PCout(14) = 0;
								PCout(15) = 0;	
								before_bl = 0;
								now_bl = 0;
								break;
							
							case 0x06:
								TIM_Cmd(TIM5, ENABLE);
								eco_flag = 1;
								led2_stat = 1;	
								break;
							
							case 0x07:
								TIM_Cmd(TIM5, ENABLE);
								eco_flag = 0;
								led2_stat = 0;
								break;
							
							case 0x08:
								TIM_Cmd(TIM5, ENABLE);
								led6_stat = 1;
								led12_stat = 0;
								led9_stat = 0; 
								sys_off_flag = 60*60*500;
								switch_flag = 1;
								break;

							case 0x09:
								TIM_Cmd(TIM5, ENABLE);
								led9_stat = 1;
								led12_stat = 0;
								led6_stat = 0;
								sys_off_flag = 2*60*60*500;
								switch_flag = 1;
								break;
							case 97:
								TIM_Cmd(TIM5, ENABLE);
								led12_stat = 1;
								led6_stat = 0;
								led9_stat = 0;
								sys_off_flag = 4*60*60*500;
								switch_flag = 1;
								break;		
							case 98:
								TIM_Cmd(TIM5, ENABLE);
								led12_stat = 0;
								led6_stat = 0;
								led9_stat = 0;
								switch_flag = 0; 
								break;
							default:
								break;
						}
				}	
			}
			zigbee_state = 0;  
			zigbee_cmd = 0;
		}
		
		if(before_bl !=  now_bl) {
			 TIM_Cmd(TIM4, ENABLE);
			 bl_run_flag = 1;	
		}  	
		delay_ms(20);
	}
}

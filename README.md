# STM32_Air_Purifier
Source Code for the STM32_Air_Purifier Project.

It is recommended using "Keil uversion5" to open and manage this project.

The chip used in this project is the STM32F405.

Drivers Info:
	breath_led.c -- PWM
	key.c -- GPIO Scan
	led.c -- GPIO Output
	buzzer.c -- GPIO Output
	power_output.c -- GPIO Output
	fans.c (XingDong DFH1225B) -- PWM
	pm2_5.c	(SHARP GP2Y1051AU0F) -- USART
	voc.c (Risym MQ-135) -- ADC
	tilt.c (Risym SW-520D) -- GPIO Input
	dht11.c	(Risym DHT11) -- One_wire
	zigbee.c (TELESKY DL-20 with CC2530 chip) -- USART

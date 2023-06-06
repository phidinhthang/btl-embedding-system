#include "stdio.h"
#include "stm32f10x.h"                  // Device header

#define i2c_FM 0x2d
#define ACK 0
#define NACK 1

void delayMs(unsigned long time);
void delayUs(unsigned long t);
void systick_init(void);
void i2c_init(char i2c, unsigned short speed_mode);
void i2c_start(char i2c);
void i2c_add(char i2c, char address, char RW);
void i2c_data(char i2c, char data);
void i2c_stop(char i2c);
void i2c_write(char i2c, char address, char data[]);
void lcd_i2c_init(void);
void lcd_i2c_msg(unsigned char line_1_2, unsigned char pos_0_16, char msg[]);
void MPU6050_rx(char reg, char *str, char data_len);
void MPU6050_tx(char reg, char data);
void MPU6050_i2c_init(void);

int main(void) {
	char a[1];
	char buffer[32];
	int step = 0;
	RCC->APB2ENR |= (1 << 4 | 1 << 3);
	systick_init();
	GPIOC->CRH &= 0xFF0FFFFF;
	GPIOC->CRH |= 0x00300000;
	
	//MPU6050_i2c_init();
	//MPU6050_rx(0x75, a, 1);	
	
	lcd_i2c_init();
	
	while (1) {
		//if (GPIOA->IDR & 1) {
			delayMs(1000);
			step = step + 1;
		//sprintf(buffer, "Step: %d", step);
		lcd_i2c_msg(1, 0, "a");
		//}
	}
} 

void systick_init(void) {
	SysTick->CTRL = 0;
	SysTick->LOAD = 0x00ffffff;
	SysTick->VAL = 0;
	SysTick->CTRL |= 5;
}

void delayMillis(void) {
	SysTick->LOAD = 0x11940;
	SysTick->VAL = 0;
	while((SysTick->CTRL & 0x00010000) == 0);
}

void delayMicro(void) {
	SysTick->LOAD = 72;
	SysTick->VAL = 0;
	while((SysTick->CTRL & 0x00010000) == 0);
}

void delayUs(unsigned long t)
{
	for(;t>0;t--)
		{
			delayMicro();
		}
}


void delayMs(unsigned long t)
{
	for(;t>0;t--)
		{
			delayMillis();
		}
}

void i2c_init(char i2c, unsigned short speed_mode) {
	RCC->APB2ENR |= 1;
	
	if (i2c == 1) {
		RCC->APB1ENR |= 0x200000;
	
		GPIOB->CRL &= 0x00FFFFFF;
		GPIOB->CRL |= (1 << 24) | (1 << 25) | (1 << 26) | (1 << 27) |
									(1 << 28) | (1 << 29) | (1 << 30) | (1 << 31);
		
		I2C1->CR1 |= 0x8000;
		I2C1->CR1 &= ~0x8000;
		I2C1->CR2 =0x8;
		I2C1->CCR = speed_mode;
		I2C1->TRISE = 0x9;
		I2C1->CR1 |= 1;
	} else if (i2c == 2){
		RCC->APB1ENR |= 0x400000;
		// Pin enable 
		GPIOB->CRH &= 0xFFFF00FF;
		GPIOB->CRH |= (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) |
									(1 << 12) | (0 << 13) | (1 << 14) | (1 << 15);
		I2C2->CR1 |= 0x8000;
		I2C2->CR1 &= ~0x8000;
		I2C2->CR2 =0x8;
		I2C2->CCR = speed_mode;
		I2C2->TRISE = 0x9;
		I2C2->CR1 |= 1;
	}
}

void i2c_start(char i2c) {
	if (i2c == 1) {
		I2C1->CR1 |= 0x100;
		while (!(I2C1->SR1 & 1)){};
	} else if (i2c == 2) {
		I2C2->CR1 |= 0x100;
		while (!(I2C2->SR1 & 1)){};
	}
}

void i2c_add(char i2c, char address,char RW)
{
	volatile int tmp;
	if (i2c == 1) {
		I2C1->DR = (address|RW);
		while((I2C1->SR1 & 2)==0){};
		while((I2C1->SR1 & 2)){
		tmp = I2C1->SR1;
		tmp = I2C1->SR2;
		if((I2C1->SR1 & 2)==0)
			{
				break;
			}
		}
	} else if (i2c == 2) {
		I2C2->DR = (address|RW);
		while((I2C2->SR1 & 2)==0){};
		while((I2C2->SR1 & 2)){
			tmp = I2C2->SR1;
			tmp = I2C2->SR2;
			if((I2C2->SR1 & 2)==0)
			{
				break;
			}
		}
	}

}

void i2c_data(char i2c, char data) {
	if (i2c == 1) {
		while((I2C1->SR1 & 0x80) == 0){}
		I2C1->DR = data;
		while((I2C1->SR1 & 0x80) == 0){}
	} else if (i2c == 2) {
		while((I2C2->SR1 & 0x80) == 0){}
		I2C2->DR = data;
		while((I2C2->SR1 & 0x80) == 0){}
	}
}

void i2c_stop(char i2c)
{
	volatile int tmp;
	if (i2c == 1) {
		tmp = I2C1->SR1;
		tmp = I2C1->SR2;
		I2C1->CR1 |= 0x200;
	} else if (i2c == 2) {
		tmp = I2C2->SR1;
		tmp = I2C2->SR2;
		I2C2->CR1 |= 0x200;
	}
}

void i2c_write(char i2c, char address,char data[])
{
	int i = 0;
	
	i2c_start(i2c);
	
	i2c_add(i2c, address,0);
	
	while(data[i]!='\0')
		{
			i2c_data(i2c, data[i]);
			i++;
		}
	i2c_stop(i2c);
}

char i2c_rx(char i2c, char ACK_NACK) {
	char temp;
	
	if(i2c==1)
	{
		I2C1->CR1 |= 0x0400;
		while((I2C1->SR1 & 0x40)==0){}
			temp = I2C1->DR;
		if(ACK_NACK)
		{
			I2C1->CR1 &= ~0x0400;
		}
		
	}
	else if(i2c==2)
	{
		I2C2->CR1 |= 0x0400;
		while((I2C2->SR1 & 0x40)==0){}
			temp = I2C2->DR;
		if(ACK_NACK)
		{
			I2C2->CR1 &= ~0x0400;
		}
		
	}
	return temp;
}

void PCF8574_add(char R_W) {
	i2c_add(1, 0x4E, R_W);
}

void PCF8574_tx_byte(char data) {
	i2c_start(1);
	PCF8574_add(0);
	i2c_data(1, data);
	i2c_stop(1);
}

void MPU6050_add(char R_W) {
	i2c_add(2, 0x68, R_W);
}

void MPU6050_tx(char reg,char data)
{
	i2c_start(2);
	MPU6050_add(0);
	i2c_data(2,reg);
	i2c_data(2,data);
	i2c_stop(2);
	
}

void MPU6050_rx(char reg, char *str, char data_len) {
	int i;
	i2c_start(2);
	delayMs(2);
	MPU6050_add(0);
	delayMs(2);
	i2c_data(2, reg);
	
	delayMs(2);
	i2c_start(2);
	delayMs(2);
	MPU6050_add(1);
	delayMs(2);
	for (i = 0; i < data_len-1; i++) {
		str[i] = i2c_rx(2,ACK);
		delayMs(2);
	}
	str[i] = i2c_rx(2, NACK);
	
	i2c_stop(2);
}

void lcd_i2c_data(unsigned char data)
{
	
	//lcd_rs(HIGH);
	//lcd_rw(LOW);
	PCF8574_tx_byte(0x09);
	delayUs(10);
	//lcd_e(HIGH);
	PCF8574_tx_byte(0x0D);
	delayUs(5);
	//GPIOA->ODR &= 0xff0f;
	//GPIOA->ODR |= (data & 0x00f0);
	PCF8574_tx_byte(((data & 0x00f0) | 0x0D));
	delayUs(10);
	//lcd_e(LOW);
	PCF8574_tx_byte(((data & 0x00f0) | 0x09));
	
	delayUs(20);
	
	//lcd_e(HIGH);
	PCF8574_tx_byte(0x0D);
	delayUs(5);
	//GPIOA->ODR &= 0xff0f;
	//GPIOA->ODR |= ((data << 4) & 0x00f0);
	PCF8574_tx_byte((((data << 4) & 0x00f0) | 0x0D));
	delayUs(10);
	//lcd_e(LOW);
	PCF8574_tx_byte((((data << 4) & 0x00f0) | 0x09));
}


void lcd_i2c_cmd(unsigned char data)
{
	
	//lcd_rs(LOW);
	//lcd_rw(LOW);
	PCF8574_tx_byte(0x08);
	delayUs(10);
	//lcd_e(HIGH);
	PCF8574_tx_byte(0x0C);
	delayUs(5);
	//GPIOA->ODR &= 0xff0f;
	//GPIOA->ODR |= (data & 0x00f0);
	PCF8574_tx_byte(((data & 0x00f0) | 0x0C));
	delayUs(10);
	//lcd_e(LOW);
	PCF8574_tx_byte(((data & 0x00f0) | 0x08));
	delayUs(20);
	
	//lcd_e(HIGH);
	PCF8574_tx_byte(0x0C);
	delayUs(5);
	//GPIOA->ODR &= 0xff0f;
	//GPIOA->ODR |= ((data << 4) & 0x00f0);
	PCF8574_tx_byte((((data << 4) & 0x00f0) | 0x0C));
	delayUs(10);
	//lcd_e(LOW);
	PCF8574_tx_byte((((data << 4) & 0x00f0) | 0x08));
}

void lcd_i2c_send(char str[])
{
	int i = 0;
		while(str[i])
		{
			lcd_i2c_data(str[i]);
			i++;
			delayUs(100);
		}
}
void lcd_i2c_msg(unsigned char line_1_2, unsigned char pos_0_16, char msg[])
{
	short pos = 0;
	if(line_1_2==1)
	{
		pos = 0;
	}
	else if(line_1_2==2)
	{
		pos = 0x40;
	}
	lcd_i2c_cmd(0x80 +pos + pos_0_16);
	delayUs(100);
	lcd_i2c_send(msg);
}

void MPU6050_i2c_init() {
	i2c_init(2, 1000);
	delayMs(20);
	MPU6050_tx(0x6B, 0x00);
}

void lcd_i2c_init()
{
  i2c_init(1, i2c_FM);
	delayMs(20);
	//lcd_rs(LOW);
	//lcd_rw(LOW);
	PCF8574_tx_byte(0x08);
	delayUs(10);
	//lcd_e(HIGH);
	PCF8574_tx_byte(0x0C);
	delayUs(5);
	//GPIOA->ODR &= 0xff0f;
	//GPIOA->ODR |= 0x30; // 8 bit communication mode 
	PCF8574_tx_byte(0x3C);
	delayUs(10);
	//lcd_e(LOW);
	PCF8574_tx_byte(0x38);
	
	delayMs(10);
	
	//lcd_rs(LOW);
	//lcd_rw(LOW);
	PCF8574_tx_byte(0x08);
	delayUs(10);
	//lcd_e(HIGH);
	PCF8574_tx_byte(0x0C);
	delayUs(5);
	//GPIOA->ODR &= 0xff0f;
	//GPIOA->ODR |= 0x30; // 8 bit communication mode 
	PCF8574_tx_byte(0x3C);
	delayUs(10);
	//lcd_e(LOW);
	PCF8574_tx_byte(0x38);
	
	delayMs(1);
	
	//lcd_rs(LOW);
	//lcd_rw(LOW);
	PCF8574_tx_byte(0x08);
	delayUs(10);
	//lcd_e(HIGH);
	PCF8574_tx_byte(0x0C);
	delayUs(5);
	//GPIOA->ODR &= 0xff0f;
	//GPIOA->ODR |= 0x30; // 8 bit communication mode 
	PCF8574_tx_byte(0x3C);
	delayUs(10);
	//lcd_e(LOW);
	PCF8574_tx_byte(0x38);
	
	delayMs(1);
	
	//lcd_rs(LOW);
	//lcd_rw(LOW);
	PCF8574_tx_byte(0x08);
	delayUs(10);
	//lcd_e(HIGH);
	PCF8574_tx_byte(0x0C);
	delayUs(5);
	//GPIOA->ODR &= 0xff0f;
	//GPIOA->ODR |= 0x20; // 4 bit communication mode 
	PCF8574_tx_byte(0x2C);
	delayUs(10);
	//lcd_e(LOW);
	PCF8574_tx_byte(0x28);
	
	
	lcd_i2c_cmd(0x2C); // 4 bit communication mode / 2 lines
	delayMs(5);
	lcd_i2c_cmd(0x0C); // Display ON
	delayMs(5);
	lcd_i2c_cmd(0x01); // Clear Display
	delayMs(5);
	lcd_i2c_cmd(0x02); // Get back to initial address
	delayMs(5);
}
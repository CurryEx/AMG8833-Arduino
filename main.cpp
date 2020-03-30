#include <Wire.h>

#define DEVICE_ADDR 0x69
#define RECT_ADDR 0x80
#define RECT_SIZE 64

unsigned int rawRect[RECT_SIZE];
double realRect[RECT_SIZE];

void setup() 
{
	Serial.begin(115200);

	Wire.begin();

	delay(1000);

	setProperty(0x00, 0x00);
	setProperty(0x01, 0x3F);
	setProperty(0x02, 0x00);
	setProperty(0x03, 0x00);
	memset(rawRect, 8, sizeof(unsigned int) * RECT_SIZE);
	memset(realRect, 8, sizeof(double) * RECT_SIZE);

	delay(2000);
}

void loop()
{
	readRawRect(rawRect);
	convertRect(rawRect, realRect);

	for (int i = 1; i <= RECT_SIZE; i++)
	{
		Serial.print(realRect[i - 1]);
		Serial.print(", ");
		if (i % 8 == 0) Serial.println();
	}
	Serial.println();

	delay(1000);
}

//因为我想要设置的都是寄存器的第1个bit 就简单一写了
void setProperty(int reg, int set)
{
	Wire.beginTransmission(DEVICE_ADDR);
	Wire.write((unsigned int)reg);
	Wire.write((unsigned int)set);
	Wire.endTransmission();
}

void readRawRect(unsigned int* buf)
{
	//从 0x80 到 0xFF 一共128bytes 每两个bit为一组 先低八位后高八位
	for (int i = 0; i < 128; i += 2)
	{
		//读低八位 8bit
		Wire.beginTransmission(DEVICE_ADDR);
		Wire.write((unsigned int)0x80 + (unsigned int)i);
		Wire.endTransmission();
		Wire.requestFrom(DEVICE_ADDR, 1);
		while (Wire.available() == 0) {}
		buf[i / 2] = Wire.read();

		//读高八位 8bit
		Wire.beginTransmission(DEVICE_ADDR);
		Wire.write((unsigned int)0x80 + (unsigned int)(i + 1));
		Wire.endTransmission();
		Wire.requestFrom(DEVICE_ADDR, 1);
		while (Wire.available() == 0) {}
		buf[i / 2] = ((unsigned int)Wire.read() << 8) | ((unsigned int)buf[i / 2]);
	}

}

void convertRect(unsigned int* raw, double* converted)
{
	for (int i = 0; i < RECT_SIZE; i++)
	{
		if (raw[i] >= 2048 && raw[i] <= 4095)
			converted[i] = 0 - ((4096 - raw[i]) * 0.25);
		//                1 0000 0000 - raw = 比0少了多少
		else
			converted[i] = raw[i] * 0.25;
	}
}
#pragma once
#include "stm32f4xx_hal.h"
#include "SSD1306Font.h"
#include "I2C.h"

// I2c address
#ifndef SSD1306_I2C_ADDR
#define SSD1306_I2C_ADDR        0x78
#endif // SSD1306_I2C_ADDR

// SSD1306 width in pixels
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH           128
#endif // SSD1306_WIDTH

// SSD1306 LCD height in pixels
#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT          32
#endif // SSD1306_HEIGHT
#define SSD1306_COM_LR_REMAP

class SSD1306
{
public:
	typedef enum {
		Black = 0x00,   // Black color, no pixel
		White = 0x01,   // Pixel is set. Color depends on LCD
	} SSD1306_COLOR;

	//
	//  Struct to store transformations
	//
	typedef struct {
		uint16_t CurrentX;
		uint16_t CurrentY;
		uint8_t Inverted;
		uint8_t Initialized;
	} SSD1306_t;

	// Screenbuffer
	uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

	// Screen object
	SSD1306_t ssd1306;

	I2C *i2c;
	void Init(I2C *i2c)
	{
		this->i2c = i2c;
		this->i2c->SetAddress(SSD1306_I2C_ADDR);

		// Wait for the screen to boot
		HAL_Delay(100);
		int status = 0;

		// Init LCD
		status += i2c->Write(0xAE);   // Display off
		status += i2c->Write(0x20);   // Set Memory Addressing Mode
		status += i2c->Write(0x10);   // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
		status += i2c->Write(0xB0);   // Set Page Start Address for Page Addressing Mode,0-7
		status += i2c->Write(0xC8);   // Set COM Output Scan Direction
		status += i2c->Write(0x00);   // Set low column address
		status += i2c->Write(0x10);   // Set high column address
		status += i2c->Write(0x40);   // Set start line address
		status += i2c->Write(0x81);   // set contrast control register
		status += i2c->Write(0xFF);
		status += i2c->Write(0xA1);   // Set segment re-map 0 to 127
		status += i2c->Write(0xA6);   // Set normal display

		status += i2c->Write(0xA8);   // Set multiplex ratio(1 to 64)
		status += i2c->Write(SSD1306_HEIGHT - 1);

		status += i2c->Write(0xA4);   // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
		status += i2c->Write(0xD3);   // Set display offset
		status += i2c->Write(0x00);   // No offset
		status += i2c->Write(0xD5);   // Set display clock divide ratio/oscillator frequency
		status += i2c->Write(0xF0);   // Set divide ratio
		status += i2c->Write(0xD9);   // Set pre-charge period
		status += i2c->Write(0x22);

		status += i2c->Write(0xDA);   // Set com pins hardware configuration
#ifdef SSD1306_COM_LR_REMAP
		status += i2c->Write(0x02);   // Enable COM left/right remap
#else
		status += i2c->Write(0x12);   // Do not use COM left/right remap
#endif // SSD1306_COM_LR_REMAP

		status += i2c->Write(0xDB);   // Set vcomh
		status += i2c->Write(0x20);   // 0x20,0.77xVcc
		status += i2c->Write(0x8D);   // Set DC-DC enable
		status += i2c->Write(0x14);   //
		status += i2c->Write(0xAF);   // Turn on SSD1306 panel

		if (status != 0) {
			return;
		}

		// Clear screen
		Fill(Black);

		// Flush buffer to screen
		UpdateScreen();

		// Set default values for screen object
		ssd1306.CurrentX = 0;
		ssd1306.CurrentY = 0;

		ssd1306.Initialized = 1;
	}

	//
	//  Fill the whole screen with the given color
	//
	void Fill(SSD1306_COLOR color)
	{
		// Fill screenbuffer with a constant value (color)
		uint32_t i;

		for (i = 0; i < sizeof(SSD1306_Buffer); i++)
		{
			SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
		}
	}

	//
	//  Write the screenbuffer with changed to the screen
	//
	void UpdateScreen()
	{
		for (uint8_t i = 0; i < SSD1306_HEIGHT/8; i++)
		{
			i2c->Write(0xB0 + i);
			i2c->Write(0x00);
			i2c->Write(0x10);
			i2c->WriteBuffer(&SSD1306_Buffer[SSD1306_WIDTH * i]);
		}
	}

	//
	//  Draw one pixel in the screenbuffer
	//  X => X Coordinate
	//  Y => Y Coordinate
	//  color => Pixel color
	//
	void DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
	{
		if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
		{
			// Don't write outside the buffer
			return;
		}

		// Check if pixel should be inverted
		if (ssd1306.Inverted)
		{
			color = (SSD1306_COLOR)!color;
		}

		// Draw in the correct color
		if (color == White)
		{
			SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
		}
		else
		{
			SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
		}
	}


	//
	//  Draw 1 char to the screen buffer
	//  ch      => Character to write
	//  Font    => Font to use
	//  color   => Black or White
	//
	char WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
	{
		uint32_t i, b, j;

		// Check remaining space on current line
		if (SSD1306_WIDTH <= (ssd1306.CurrentX + Font.FontWidth) ||
			SSD1306_HEIGHT <= (ssd1306.CurrentY + Font.FontHeight))
		{
			// Not enough space on current line
			return 0;
		}

		// Translate font to screenbuffer
		for (i = 0; i < Font.FontHeight; i++)
		{
			b = Font.data[(ch - 32) * Font.FontHeight + i];
			for (j = 0; j < Font.FontWidth; j++)
			{
				if ((b << j) & 0x8000)
				{
					DrawPixel(ssd1306.CurrentX + j, (ssd1306.CurrentY + i), (SSD1306_COLOR)color);
				}
				else
				{
					DrawPixel(ssd1306.CurrentX + j, (ssd1306.CurrentY + i), (SSD1306_COLOR)!color);
				}
			}
		}

		// The current space is now taken
		ssd1306.CurrentX += Font.FontWidth;

		// Return written char for validation
		return ch;
	}

	//
	//  Write full string to screenbuffer
	//
	char WriteString(char* str, FontDef Font, SSD1306_COLOR color)
	{
		// Write until null-byte
		while (*str)
		{
			if (WriteChar(*str, Font, color) != *str)
			{
				// Char could not be written
				return *str;
			}

			// Next char
			str++;
		}

		// Everything ok
		return *str;
	}

	//
	//  Invert background/foreground colors
	//
	void InvertColors(void)
	{
		ssd1306.Inverted = !ssd1306.Inverted;
	}

	//
	//  Set cursor position
	//
	void SetCursor(uint8_t x, uint8_t y)
	{
		ssd1306.CurrentX = x;
		ssd1306.CurrentY = y;
	}
};

extern SSD1306 oled;
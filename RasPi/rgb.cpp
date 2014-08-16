/***************************************************************
 *
 * blinkenstick.org - RGB Strip control
 *
 * Copyright 2014 Milosch Meriac <milosch@meriac.com>
 *
 ***************************************************************

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 2.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 */
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/spi/spidev.h>
#include "rgb.h"

#define CRGB_SPI_BITS 8
#define CRGB_DEFAULT_SPI_CLOCK (12000000UL)

CRGB::CRGB(uint32_t count)
{
	/* set defaults */
	m_fd = -1;
	m_count = count;

	/* allocate pixel strip */
	m_pixel = new CRGBpixel[count];
}

CRGB::~CRGB(void)
{
	delete m_pixel;
	close(m_fd);
}

int CRGB::spi(const char* spi_dev)
{
	int res;
	const uint8_t spi_bits = CRGB_SPI_BITS;
	const uint8_t spi_mode = SPI_MODE_0;

	/* close previously opened handles */
	if(m_fd>=0)
	{
		close(m_fd);
		m_fd = -1;
	}

	/* open SPI device */
	if((m_fd = open(spi_dev, O_RDWR))<0)
		return -1;

	/* set SPI mode */
	if((res = ioctl(m_fd, SPI_IOC_WR_MODE, &spi_mode))<0)
		res = -2;
	else
		/* set SPI bits per word */
		if((res = ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits))<0)
			res = -3;
		else
			/* update SPI speed */
			if(speed(CRGB_DEFAULT_SPI_CLOCK))
				return 0;

	/* close file handle upon error */
	close(m_fd);
	m_fd = -1;

	return res;
}

uint32_t CRGB::speed(uint32_t speed)
{
	/* set SPI clock & read back actual selected clock */
	if(ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)<0)
		return 0;

	if(ioctl(m_fd, SPI_IOC_RD_MAX_SPEED_HZ, &m_speed)<0)
	{
		m_speed = 0;
		return 0;
	}

	return m_speed;
}

uint32_t CRGB::speed(void)
{
	return m_speed;
}

bool CRGB::send(void)
{
	spi_ioc_transfer tx;

	/* prepare SPI transfer */
	memset(&tx, 0, sizeof(tx));
	tx.tx_buf = (unsigned long)m_pixel;
	tx.len = (uint32_t)(m_count*sizeof(m_pixel[0]));
	tx.speed_hz = m_speed;
	tx.bits_per_word = CRGB_SPI_BITS;

	/* transmit */
	return ioctl(m_fd, SPI_IOC_MESSAGE(1), &tx)>0;
}

bool CRGB::update(const CRGBpixel* source)
{
	uint32_t t;
	CRGBpixel *pixel = m_pixel;

	for(t=0; t<m_count; t++)
	{
		pixel->r = g_rgb_correction[source->r >> 1];
		pixel->g = g_rgb_correction[source->g >> 1];
		pixel->b = g_rgb_correction[source->b >> 1];

		pixel++;
		source++;
	}

	/* send pixels to strip */
	if(!send())
		return false;

	/* latch pixels in strip */
	memset(m_pixel, 0, sizeof(m_pixel[0])*m_count);
	return send();
}

const uint8_t CRGB::g_rgb_correction[128] = {
	0x80, 0x80, 0x80, 0x80, 0x80, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x82, 0x82,
	0x82, 0x82, 0x82, 0x82, 0x83, 0x83, 0x83, 0x83,
	0x83, 0x84, 0x84, 0x84, 0x84, 0x85, 0x85, 0x85,
	0x86, 0x86, 0x86, 0x87, 0x87, 0x87, 0x88, 0x88,
	0x89, 0x89, 0x8A, 0x8A, 0x8B, 0x8B, 0x8C, 0x8C,
	0x8D, 0x8D, 0x8E, 0x8E, 0x8F, 0x90, 0x90, 0x91,
	0x92, 0x92, 0x93, 0x94, 0x95, 0x95, 0x96, 0x97,
	0x98, 0x99, 0x9A, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E,
	0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA5, 0xA6, 0xA7,
	0xA8, 0xA9, 0xAB, 0xAC, 0xAD, 0xAE, 0xB0, 0xB1,
	0xB2, 0xB4, 0xB5, 0xB7, 0xB8, 0xBA, 0xBB, 0xBD,
	0xBF, 0xC0, 0xC2, 0xC3, 0xC5, 0xC7, 0xC9, 0xCA,
	0xCC, 0xCE, 0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA,
	0xDC, 0xDE, 0xE0, 0xE2, 0xE5, 0xE7, 0xE9, 0xEB,
	0xEE, 0xF0, 0xF3, 0xF5, 0xF7, 0xFA, 0xFC, 0xFF
};

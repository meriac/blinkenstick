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
#ifndef __RGB_H__
#define __RGB_H__

typedef struct {
	uint8_t b,r,g;
} __attribute__ ((packed)) CRGBpixel;

class CRGB {

private:
	int m_fd;
	uint32_t m_count, m_speed;
	CRGBpixel* m_pixel;
	static const uint8_t g_rgb_correction[128];
	bool send(void);

public:
	CRGB(uint32_t count);
	~CRGB(void);

	int spi(const char* spi_dev);
	uint32_t speed(uint32_t speed);
	uint32_t speed(void);

	bool update(const CRGBpixel *source);
};

#endif/*__RGB_H__*/

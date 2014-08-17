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
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <png++/png.hpp>

#include "rgb.h"

#define RGB_WAIT_MS 10
#define RGB_LEDS_COUNT 117

const char default_spi[] = "/dev/spidev0.0";

static bool update_priority(void)
{
	sched_param param;

	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	return sched_setscheduler(0, SCHED_FIFO, &param)>=0;
}

int main(int argc, char *argv[])
{
	int res;
	int height, width, x, y;
	png::rgb_pixel pixel;
	CRGBpixel line[RGB_LEDS_COUNT];
	CRGB rgb(RGB_LEDS_COUNT);
	timespec req, rem;

	/* ensure realtime priority */
	if(!update_priority())
	{
		fprintf(stderr, "WARNING: failed to switch to realtime priority\n");
		return -1;
	}

	/* ensure that an image file name is provided */
	if(argc!=2)
	{
		fprintf(stderr, "USAGE: %s image_file_name.png\n", argv[0]);
		return -1;
	}

	/* read image */
	png::image<png::rgb_pixel> png(argv[1]);
	height = (int)png.get_height();
	width  = (int)png.get_width();
	printf("READ: '%s',%i,%i\n", argv[1], width, height);
	if(height>RGB_LEDS_COUNT)
		height = RGB_LEDS_COUNT;

	/* open SPI driver */
	if((res = rgb.spi(default_spi))<0)
	{
		fprintf(stderr, "ERROR: failed to open SPI device '%s' [%i]\n", default_spi, res);
		return res;
	}
	fprintf(stderr, "SPI_SPEED: %i\n", rgb.speed());

	/* compile transfer and send out */
	for(x=0; x<width; x++)
	{
		for(y=0; y<height; y++)
		{
			pixel = png[height-y-1][x];
			line[y].r = pixel.red;
			line[y].g = pixel.green;
			line[y].b = pixel.blue;
		}

		/* send line away */
		rgb.update(line);

		fprintf(stdout,".");
		fflush(stdout);

		/* wait for time */
		req.tv_sec = 0;
		req.tv_nsec = RGB_WAIT_MS*1000000UL;
		if(nanosleep(&req, &rem))
			while(nanosleep(&rem, &rem));
	}

	/* ensure that strip is dark after last line */
	memset(&line, 0, sizeof(line));
	rgb.update(line);

	fprintf(stderr, "\nDONE\n");
}

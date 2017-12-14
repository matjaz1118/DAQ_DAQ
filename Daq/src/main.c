/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include "core.h"

daq_settings_t master_settings;
daq_measured_data_t calc_data;

void delay (void)
{
	volatile uint32_t n;
	for(n = 0; n < 1000000; n++) {}
}
void print_data (void)
{
	asm("NOP");
}

void calculate_data (void)
{
	uint16_t* raw_data_ptr;
	uint32_t n, m = 0;
	raw_data_ptr = core_get_raw_data_pntr();
	for(n = 0; n < 4; n++) // clear the array
	{
		calc_data.results[n] = 0;
	}
	for(n = 0; n < core_get_raw_data_size(); n++)
	{
		for(!(master_settings.channels & (0x01 << m)))
		{
			m++;
			if(m > 3)
			{
				m = 0;
			}
		}
		calc_data.results[m] += *(raw_data_ptr + n);
		m++;
	}
	for(n = 0; n < 4; n++)
	{
+		if((master_settings.channels & (0x01 << n))) // only divide enabled channels
		{
			calc_data.results[n] /= (core_get_raw_data_size() / 4);
		}
	}
	calc_data.new_data = 1;
}

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	wdt_disable(WDT);
	sysclk_init();
	board_init();
	core_init();
	
	master_settings.acquisitionNbr = 6;
	master_settings.acqusitionTime = 10000;
	master_settings.averaging = 2;
	master_settings.channels = (DAQ_CHANNEL_1);

	while(1)
	{
		core_configure(&master_settings);
		core_start();
		while(core_status_get() == CORE_RUNNING)
		{
			if(core_new_data_ready())
			{
				calculate_data();
				print_data();
				core_new_data_claer();
			}
		}
		delay();
	}
}

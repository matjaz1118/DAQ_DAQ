/*
 * core.h
 *
 * Created: 13. 12. 2017 10:41:46
 *  Author: matja
 */ 


#include <asf.h>

#ifndef CORE_H_
#define CORE_H_

//Defines
#define ADC_CORE_DEBUG	0 //set to 1 for aditional development debugging
#define ADC_CLK	16000000 // clock of ADC converter
#define ADC_RAW_DATA_SIZE 20 * 4 //maximum size of raw datra buffer for averaging

#ifdef ADC_CORE_DEBUG == 1				//definition of debugging pins (only if debugging enabled)
	#define ADC_DEBUG_PIN	PIO_PA9
	#define TIMER_DEBUG_PIN	PIO_PA10
#endif // ADC_CORE_DEBUG == 1


//Macros
#define CORE_DEBUG		#ifdef ADC_CORE_DEBUG == 1
#define CORE_DEBUG_END	#endif

#ifdef ADC_CORE_DEBUG == 1				//debug pins control
	#define ADC_DEBUG_PIN_SET	pio_set(PIOA, ADC_DEBUG_PIN);
	#define ADC_DEBUG_PIN_CLR	pio_clear(PIOA, ADC_DEBUG_PIN);
	#define TIMER_DEBUG_PIN_SET	pio_set(PIOA, TIMER_DEBUG_PIN);
	#define TIMER_DEBUG_PIN_CLR	pio_clear(PIOA, TIMER_DEBUG_PIN);
	#define ADC_DEBUG_PIN_TGL	pio_toggle_pin_group(PIOA, ADC_DEBUG_PIN);
	#define TIMER_DEBUG_PIN_TGL	pio_toggle_pin_group(PIOA, TIMER_DEBUG_PIN);
#endif // ADC_CORE_DEBUG == 1

//Typedefs
typedef enum		// bitmask for channel enabling
{
	ADC_CHANNEL_1 = 0x01;
	ADC_CHANNEL_2 = 0x02;
	ADC_CHANNEL_3 = 0x04;
	ADC_CHANNEL_4 = 0x08;
}channel_bitmask_t;

typedef struct
{
	uint32_t acqusitionTime;
	uint32_t acquisitionNbr;
	uint32_t averaging;
	channel_bitmask_t channels;	
}daq_settings_t;

//Functions


#endif /* CORE_H_ */
/*
 * core.h
 *
 * Created: 13. 12. 2017 10:41:46
 *  Author: matja
 */ 


#include <asf.h>

#ifndef CORE_H_
#define CORE_H_

//*******************************************************************Defines***************************************
//general
#define ADC_CORE_DEBUG	1 //set to 1 for aditional development debugging

#define ADC_RAW_DATA_SIZE 20 * 4 //maximum size of raw datra buffer for averaging

//ADC configs
#define ADC_CLK	16000000 // clock of ADC converter
#define ADC_IRQ_PRIORITY		4	

//TIMER configs
#define TIMER_CH				0			//channel# of used timer
#define TIMER_IRQ_PRIORITY		4			//IRQ priotity for timer
#define TIMER_MAX				50000		// maximum value of timer
#define TIMER_DIV				2			//division factor to convert from microseconds to TC counts


#ifdef ADC_CORE_DEBUG == 1				//definition of debugging pins (only if debugging enabled)
	#define ADC_DEBUG_PIN	PIO_PA9
	#define TIMER_DEBUG_PIN	PIO_PA10
#endif // ADC_CORE_DEBUG == 1


//*************************************************************Macros**************************************************

#ifdef ADC_CORE_DEBUG == 1				//debug pins control
	#define ADC_DEBUG_PIN_SET	pio_set(PIOA, ADC_DEBUG_PIN);
	#define ADC_DEBUG_PIN_CLR	pio_clear(PIOA, ADC_DEBUG_PIN);
	#define TIMER_DEBUG_PIN_SET	pio_set(PIOA, TIMER_DEBUG_PIN);
	#define TIMER_DEBUG_PIN_CLR	pio_clear(PIOA, TIMER_DEBUG_PIN);
	#define ADC_DEBUG_PIN_TGL	pio_toggle_pin_group(PIOA, ADC_DEBUG_PIN);
	#define TIMER_DEBUG_PIN_TGL	pio_toggle_pin_group(PIOA, TIMER_DEBUG_PIN);
#endif // ADC_CORE_DEBUG == 1

#define US_TO_TC(x) x / TIMER_DIV
//********************************************************Typedefs****************************************************
typedef enum		// bitmask for channel enabling
{
	DAQ_CHANNEL_1 = 0x01,
	DAQ_CHANNEL_2 = 0x02,
	DAQ_CHANNEL_3 = 0x04,
	DAQ_CHANNEL_4 = 0x08
}channel_bitmask_t;

typedef struct
{
	uint32_t acqusitionTime;
	uint32_t acquisitionNbr;
	uint32_t averaging;
	channel_bitmask_t channels;	
}daq_settings_t;
//**************************************************************Functions***********************************************
void core_init (void);
void timer_set_compare_time (uint32_t tim);
#endif /* CORE_H_ */
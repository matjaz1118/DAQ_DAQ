/*
 * core.c
 *
 * Created: 13. 12. 2017 10:41:36
 *  Author: matja
 */ 
#include <asf.h>
#include "core.h"
#include "parser.h"

pdc_packet_t adc_pdc;
Pdc *adc_pdc_pntr;

uint16_t adc_raw_data [ADC_RAW_DATA_SIZE];
volatile uint32_t rep_cntr, new_data = 0, acqusition_in_progress = 0;
uint32_t raw_data_size;



#ifdef ADC_CORE_DEBUG == 1
	void pio_init (void)
	{
		pmc_enable_periph_clk(ID_PIOA);
		pio_set_output(PIOA, ADC_DEBUG_PIN, LOW, DISABLE, DISABLE); //indicator pin for ADC
		pio_set_output(PIOA, TIMER_DEBUG_PIN, LOW, DISABLE, DISABLE);//indicator pin for Timer0
	}
#endif //ADC_CORE_DEBUG == 1




void core_init (void)
{
	//init adc
	pmc_enable_periph_clk(ID_ADC);
	adc_init(ADC, sysclk_get_cpu_hz(), ADC_CLK, 1);
	adc_configure_timing(ADC, 8, ADC_SETTLING_TIME_3, 1);
	adc_configure_trigger(ADC, ADC_TRIG_SW, ADC_MR_FREERUN_ON); //WARNING! Bug in ASF! ADC_MR_FREERUN_ON does't actualy enables freerun mode!
	ADC->ADC_MR |= ADC_MR_FREERUN; //due to a bug in ASF we enable freerun mode manualy
	ADC->ADC_COR |= (ADC_COR_DIFF0 | ADC_COR_DIFF1 | ADC_COR_DIFF2 | ADC_COR_DIFF3
					 | ADC_COR_DIFF4 | ADC_COR_DIFF5 | ADC_COR_DIFF6 | ADC_COR_DIFF7); // set channels to differential
	#ifdef ADC_CORE_DEBUG == 1
		pio_init();
	#endif //ADC_CORE_DEBUG == 1
	
	adc_pdc_pntr = adc_get_pdc_base(ADC); // init DMA
	adc_pdc.ul_addr = adc_raw_data;
	NVIC_ClearPendingIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, ADC_IRQ_PRIORITY);
	NVIC_EnableIRQ(ADC_IRQn);  
	
	//init timer 0	
	pmc_enable_periph_clk(ID_TC0);		
	tc_init(TC0, TIMER_CH, TC_CMR_TCCLKS_TIMER_CLOCK4 | TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC );
	tc_write_rc(TC0, TIMER_CH, 50000);
	tc_enable_interrupt(TC0, TIMER_CH, TC_IER_CPCS);
	NVIC_SetPriority(TC0_IRQn, TIMER_IRQ_PRIORITY);
	NVIC_EnableIRQ(TC0_IRQn);
}

void timer_set_compare_time (uint32_t tim)
{
	if(tim > 50000) tim = 50000;
	tc_write_rc(TC0, TIMER_CH, tim);
}

void core_configure (daq_settings_t *settings)
{
	uint32_t nb_enables_ch = 0, n;
	
	//validate settings
	validate_settings(settings);
	
	//enable enabled channels and count the number of them
	adc_disable_all_channel(ADC);
	for(n = 0; n < 4; n++)
	{
		if(settings->channels & (0x01 << n))
		{
			nb_enables_ch ++;
			adc_enable_channel(ADC, (n * 2));
		}
	}
	//configure dma
	raw_data_size = settings->averaging * nb_enables_ch;
	adc_pdc.ul_size = raw_data_size;
	pdc_rx_init(adc_pdc_pntr, &adc_pdc, NULL);
	adc_enable_interrupt(ADC, ADC_IER_RXBUFF);
	
	//set repetition counter
	rep_cntr = settings->acquisitionNbr;
	
	//set timer
	timer_set_compare_time(US_TO_TC(settings->acqusitionTime));
	
}

void core_start (void)
{
	pdc_enable_transfer(adc_pdc_pntr, PERIPH_PTCR_RXTEN);
	tc_start(TC0, TIMER_CH);
	acqusition_in_progress = 1;
}

void validate_settings (daq_settings_t *settings)
{
	if(settings->acqusitionTime < 10) {settings->acqusitionTime = 10;}
	if(settings->acqusitionTime > 100000) {settings->acqusitionTime = 100000;}
	if(settings->acquisitionNbr > DAQ_MAX_ACQ_NB) {settings->acquisitionNbr = DAQ_MAX_ACQ_NB;}
	if(settings->averaging > DAQ_MAX_AVG_NB) {settings->averaging = DAQ_MAX_AVG_NB;}
	if(settings->channels > (DAQ_CHANNEL_1 | DAQ_CHANNEL_2 | DAQ_CHANNEL_3 | DAQ_CHANNEL_4)) 
	{
		settings->averaging = (DAQ_CHANNEL_1 | DAQ_CHANNEL_2 | DAQ_CHANNEL_3 | DAQ_CHANNEL_4);
	}
}

core_status_t core_status_get (void)
{
	if(acqusition_in_progress) 
	{
		return CORE_RUNNING;
	}
	else
	{
		return CORE_STOPED;
	}
}


uint32_t core_new_data_ready (void)
{
	return new_data;
}

uint16_t* core_get_raw_data_pntr (void)
{
	return adc_raw_data;
}

uint32_t core_get_raw_data_size (void)
{
	return raw_data_size;
}

void ADC_Handler (void)
{
	if(ADC->ADC_ISR & ADC_ISR_RXBUFF) // thiss gets triggered when acquisition of all samples for one averaging is complete
	{
		ADC->ADC_MR &= (~ADC_MR_FREERUN) //stop adc
		#ifdef ADC_CORE_DEBUG == 1
			TIMER_DEBUG_PIN_TGL;
		#endif //ADC_CORE_DEBUG == 1
		//configure dma for nex acusition
		adc_pdc.ul_size = raw_data_size;
		pdc_rx_init(adc_pdc_pntr, &adc_pdc, NULL);
		adc_enable_interrupt(ADC, ADC_IER_RXBUFF);
		ADC->ADC_MR |= ADC_MR_FREERUN; //stop adc
		//report new data
		new_data = 1;
	}
}

void TC0_Handler (void)
{
	if((tc_get_status(TC0, 0) & TC_SR_CPCS))
	{
		#ifdef ADC_CORE_DEBUG == 1
			TIMER_DEBUG_PIN_TGL;
		#endif //ADC_CORE_DEBUG == 1
		if(--rep_cntr)
		{
			adc_start(ADC);
		}
		else
		{
			acqusition_in_progress = 0;
		}
	}
}
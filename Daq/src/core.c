/*
 * core.c
 *
 * Created: 13. 12. 2017 10:41:36
 *  Author: matja
 */ 
#include <asf.h>
#include "core.h"

pdc_packet_t adc_pdc;
Pdc *adc_pdc_pntr;

uint16_t adc_raw_data [ADC_RAW_DATA_SIZE];

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
	//adc_pdc.ul_size = 0;
	pdc_rx_init(adc_pdc_pntr, &adc_pdc, NULL);
	//pdc_enable_transfer(adc_pdc_pntr, PERIPH_PTCR_RXTEN);
	//adc_enable_interrupt(ADC, ADC_IER_RXBUFF);
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



void ADC_Handler (void)
{
	
}

void TC0_Handler (void)
{
	if((tc_get_status(TC0, 0) & TC_SR_CPCS))
	{
		TIMER_DEBUG_PIN_TGL;
	}
}
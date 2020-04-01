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

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"


void but1_callback(void);
void but2_callback(void);
void but3_callback(void);
void pisca_led(int n, int t);

#define LED1_PIO			PIOA
#define LED1_PIO_ID			ID_PIOA
#define LED1_PIO_IDX		0
#define LED1_PIO_IDX_MASK	(1 << LED1_PIO_IDX)

#define LED2_PIO			PIOC
#define LED2_PIO_ID			ID_PIOC
#define LED2_PIO_IDX		30
#define LED2_PIO_IDX_MASK	(1 << LED2_PIO_IDX)

#define LED3_PIO			PIOB
#define LED3_PIO_ID			ID_PIOB
#define LED3_PIO_IDX		3
#define LED3_PIO_IDX_MASK	(1 << LED3_PIO_IDX)

#define BUT1_PIO			PIOD
#define BUT1_PIO_ID			ID_PIOD
#define BUT1_PIO_IDX		28
#define BUT1_PIO_IDX_MASK	(1u << BUT1_PIO_IDX)

#define BUT3_PIO			PIOA
#define BUT3_PIO_ID			ID_PIOA
#define BUT3_PIO_IDX		19
#define BUT3_PIO_IDX_MASK	(1u << BUT3_PIO_IDX)

#define BUT2_PIO			PIOC
#define BUT2_PIO_ID			ID_PIOC
#define BUT2_PIO_IDX		31
#define BUT2_PIO_IDX_MASK	(1u << BUT2_PIO_IDX)

volatile char but_flag1;
volatile char but_flag2;
volatile char but_flag3;

int led_freq;

void but1_callback(void)
{
	//pisca_led(5, 200);
	led_freq++;
	but_flag1 = 1;
}
void but2_callback(void)
{
	but_flag2 = 1;
}
void but3_callback(void)
{
	led_freq--;
	but_flag3 = 1;
}

void pisca_led(int n, int t){
	for (int i=0;i<n;i++){
		pio_clear(LED2_PIO, LED2_PIO_IDX_MASK);
		delay_s(1/(2.0*t));
		pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
		delay_s(1/(2.0*t));
	}
}

void init(void){
	sysclk_init();
	
	WDT->WDT_MR=WDT_MR_WDDIS;
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(LED1_PIO_ID);
	pmc_enable_periph_clk(LED2_PIO_ID);
	pmc_enable_periph_clk(LED3_PIO_ID);	
	
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_PIO_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED2_PIO, PIO_OUTPUT_0, LED2_PIO_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED3_PIO, PIO_OUTPUT_0, LED3_PIO_IDX_MASK, PIO_DEFAULT);
	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);
	
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP);
	
	pio_handler_set(BUT1_PIO,
		BUT1_PIO_ID,
		BUT1_PIO_IDX_MASK,
		PIO_IT_FALL_EDGE,
		but1_callback);
	
	pio_handler_set(BUT2_PIO,
		BUT2_PIO_ID,
		BUT2_PIO_IDX_MASK,
		PIO_IT_FALL_EDGE,
		but2_callback);
	
	pio_handler_set(BUT3_PIO,
		BUT3_PIO_ID,
		BUT3_PIO_IDX_MASK,
		PIO_IT_FALL_EDGE,
		but3_callback);
	
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4);
	
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 6);
	
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 5); 
	
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
}

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	
	board_init();
	sysclk_init();
	init();
	char buffer[8];
	led_freq = 5;

	delay_init();

	gfx_mono_ssd1306_init();
	gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	gfx_mono_draw_string("world", 50,16, &sysfont);


  /* Insert application code here, after the board has been initialized. */
	while(1) {
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
		if(but_flag1){
			but_flag1 = 0;
		}
		if(but_flag2){
			pisca_led(5, led_freq);
			but_flag2 = 0;
		}
		if(but_flag3){
			but_flag3 = 0;
		}
		gfx_mono_draw_string("      ", 50,16, &sysfont);
		sprintf(buffer,"%d",led_freq);
		gfx_mono_draw_string(buffer, 50,16, &sysfont);
	}
}

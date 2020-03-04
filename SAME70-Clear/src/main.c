	/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/
#define LED_PIO				PIOC         
#define LED_PIO_ID			12
#define LED_PIO_IDX			8
#define LED_PIO_IDX_MASK	(1 << LED_PIO_IDX)

// Configuracoes do botao
#define BUT_PIO				PIOA
#define BUT_PIO_ID			10
#define BUT_PIO_IDX			11
#define BUT_PIO_IDX_MASK	(1u << BUT_PIO_IDX)

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void)
{
	sysclk_init();
	
	WDT->WDT_MR=WDT_MR_WDDIS;
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(LED_PIO_ID);
	//Inicializa PC8 como saída
	pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0);
	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT_PIO_ID);
	
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, PIO_DEFAULT);
	pio_pull_up(BUT_PIO,BUT_PIO_IDX_MASK,1);
	
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();
  
  char status =0;

  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1)
  {
	  if(pio_get(BUT_PIO,PIO_DEFAULT,BUT_PIO_IDX_MASK) != status){
		  for(int i=0;i<5;i++){
			pio_set(PIOC, LED_PIO_IDX_MASK);      // Coloca 1 no pino LED
			delay_ms(200);                        // Delay por software de 200 ms
			pio_clear(PIOC, LED_PIO_IDX_MASK);    // Coloca 0 no pino do LED
			delay_ms(200);
		  }
	  }
	status = pio_get(BUT_PIO,PIO_DEFAULT,BUT_PIO_IDX_MASK);
  }
  return 0;
}

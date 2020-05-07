/*
* Example RTOS Atmel Studio
*/

#include <asf.h>
#include "conf_board.h"

#define tskIDLE_PRIORITY ( ( UBaseType_t ) 0U )
#define TASK_MONITOR_STACK_SIZE            (2048/sizeof(portSTACK_TYPE))
#define TASK_MONITOR_STACK_PRIORITY        (tskIDLE_PRIORITY)
#define TASK_LED_STACK_SIZE                (1024/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY            (tskIDLE_PRIORITY)

#define TASK_LED1_STACK_SIZE (1024/sizeof(portSTACK_TYPE))
#define TASK_LED1_STACK_PRIORITY (tskIDLE_PRIORITY)

#define configUSE_IDLE_HOOK   1

/** Semaforo a ser usado pela task led 
    tem que ser var global! */

SemaphoreHandle_t xSemaphore_1;
SemaphoreHandle_t xSemaphore_2;
SemaphoreHandle_t xSemaphore_3;

/*Configurando LEDs OLED1*/

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
#define LED3_PIO_IDX		2
#define LED3_PIO_IDX_MASK	(1 << LED3_PIO_IDX)

/* Configurando Botões OLED*/

#define BUT1_PIO            PIOD
#define BUT1_PIO_ID         16
#define BUT1_PIO_IDX        28
#define BUT1_PIO_IDX_MASK   (1u << BUT1_PIO_IDX)

#define BUT2_PIO			PIOC
#define BUT2_PIO_ID			12
#define BUT2_PIO_IDX		31
#define BUT2_PIO_IDX_MASK	(1u << BUT2_PIO_IDX)

#define BUT3_PIO			PIOA
#define BUT3_PIO_ID			10
#define BUT3_PIO_IDX		19
#define BUT3_PIO_IDX_MASK	(1u << BUT3_PIO_IDX)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

#define configTICK_RATE_HZ                      ( 1000 )
#define xPortSysTickHandler SysTick_Handler
/**
 * \brief Called if stack overflow during execution
 */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
		signed char *pcTaskName)
{
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	 * identify which task has overflowed its stack.
	 */
	for (;;) {
	}
}

/**
 * \brief This function is called by FreeRTOS idle task
 */
extern void vApplicationIdleHook(void)
{
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

/**
 * \brief This function is called by FreeRTOS each tick
 */
extern void vApplicationTickHook(void)
{
}

extern void vApplicationMallocFailedHook(void)
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}

/* Funcoes */
void pin_toggle(Pio *pio, uint32_t mask){
	if (pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio, mask);
}

/**
* callback do botao
* libera semaforo: xSemaphore
*/
void but1_callback(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	printf("but1_callback \n");
	xSemaphoreGiveFromISR(xSemaphore_1, &xHigherPriorityTaskWoken);
	printf("semafaro Led 1 tx \n");
}

void but2_callback(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	printf("but2_callback \n");
	xSemaphoreGiveFromISR(xSemaphore_2, &xHigherPriorityTaskWoken);
	printf("semafaro Led 2 tx \n");
}

void but3_callback(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	printf("but3_callback \n");
	xSemaphoreGiveFromISR(xSemaphore_3, &xHigherPriorityTaskWoken);
	printf("semafaro Led 3 tx \n");
}


/**
 * \brief This task, when activated, send every ten seconds on debug UART
 * the whole report of free heap and total tasks status
 */
static void task_monitor(void *pvParameters)
{
    static portCHAR szList[256];
    UNUSED(pvParameters);

    for (;;) {
        printf("--- task ## %u", (unsigned int)uxTaskGetNumberOfTasks());
        vTaskList((signed portCHAR *)szList);
        printf(szList);
        vTaskDelay(3000);
    }
}

/**
 * \brief This task, when activated, make LED blink at a fixed rate
 */
static void task_led1(void *pvParameters){
	pmc_enable_periph_clk(LED1_PIO_ID);
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_PIO_IDX_MASK, PIO_DEFAULT);

	const TickType_t xDelay = 3000 / portTICK_PERIOD_MS;
	const TickType_t xDelayLed = 200 / portTICK_PERIOD_MS;

	for (;;) {
		for (uint i=0; i<5; i++){
			pio_clear(LED1_PIO, LED1_PIO_IDX_MASK);
			vTaskDelay(xDelayLed);
			pio_set(LED1_PIO, LED1_PIO_IDX_MASK);
			vTaskDelay(xDelayLed);
		}
		vTaskDelay(xDelay);
	}
}

/* Funções com semaforo */

static void task_led(void *pvParameters) {
  /* We are using the semaphore for synchronisation so we create a binary
  semaphore rather than a mutex.  We must make sure that the interrupt
  does not attempt to use the semaphore before it is created! */
  xSemaphore_1 = xSemaphoreCreateBinary();

  /* devemos iniciar a interrupcao no pino somente apos termos alocado
  os recursos (no caso semaforo), nessa funcao inicializamos 
  o botao e seu callback*/
  /* init botão */
  pmc_enable_periph_clk(BUT1_PIO_ID);
  pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP);
  pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but1_callback);
  pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
  NVIC_EnableIRQ(BUT1_PIO_ID);
  NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4

  if (xSemaphore_1 == NULL)
    printf("falha em criar o semaforo \n");

  for (;;) {
    if( xSemaphoreTake(xSemaphore_1, ( TickType_t ) 500) == pdTRUE ){
      LED_Toggle(LED0);
    }
  }
}

static void task_led2(void *pvParameters) {
  xSemaphore_2 = xSemaphoreCreateBinary();

  pmc_enable_periph_clk(BUT2_PIO_ID);
  pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP);
  pio_handler_set(BUT2_PIO, BUT2_PIO_ID, BUT2_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but2_callback);
  pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
  NVIC_EnableIRQ(BUT2_PIO_ID);
  NVIC_SetPriority(BUT2_PIO_ID, 4); // Prioridade 4

  if (xSemaphore_2 == NULL)
    printf("falha em criar o semaforo \n");

  for (;;) {
    if( xSemaphoreTake(xSemaphore_2, ( TickType_t ) 500) == pdTRUE ){
		printf("tentando ligar LED2");
		pin_toggle(LED2_PIO,LED2_PIO_IDX_MASK);
}
}
}
static void task_led3(void *pvParameters) {

  xSemaphore_3 = xSemaphoreCreateBinary();

  pmc_enable_periph_clk(BUT3_PIO_ID);
  pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP);
  pio_handler_set(BUT3_PIO, BUT3_PIO_ID, BUT3_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but3_callback);
  pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
  NVIC_EnableIRQ(BUT3_PIO_ID);
  NVIC_SetPriority(BUT3_PIO_ID, 4); // Prioridade 4

  if (xSemaphore_3 == NULL)
    printf("falha em criar o semaforo \n");

  for (;;) {
    if( xSemaphoreTake(xSemaphore_3, ( TickType_t ) 500) == pdTRUE ){
      pin_toggle(LED3_PIO,LED3_PIO_IDX_MASK);
    }
  }
}

/**
 * \brief Configure the console UART.
 */
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#if (defined CONF_UART_CHAR_LENGTH)
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#if (defined CONF_UART_STOP_BITS)
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
#if defined(__GNUC__)
	setbuf(stdout, NULL);
#else
	/* Already the case in IAR's Normal DLIB default configuration: printf()
	 * emits one character at a time.
	 */
#endif
}

/**
 *  \brief FreeRTOS Real Time Kernel example entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */
int main(void)
{
	pmc_enable_periph_clk(LED2_PIO_ID);
	pio_configure(LED2_PIO, PIO_OUTPUT_1, LED2_PIO_IDX_MASK, PIO_DEFAULT);
	pmc_enable_periph_clk(LED3_PIO_ID);
	pio_configure(LED3_PIO, PIO_OUTPUT_1, LED3_PIO_IDX_MASK, PIO_DEFAULT);
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Initialize the console uart */
	configure_console();
	
	/* Criando tarefas */
	xTaskCreate(task_monitor, "Monitor", TASK_MONITOR_STACK_SIZE, NULL,TASK_MONITOR_STACK_PRIORITY, NULL);

	xTaskCreate(task_led, "Led", TASK_LED_STACK_SIZE, NULL,TASK_LED_STACK_PRIORITY, NULL);
	
	xTaskCreate(task_led2, "Led2", TASK_LED_STACK_SIZE, NULL,TASK_LED_STACK_PRIORITY, NULL);
	
	xTaskCreate(task_led3, "Led3", TASK_LED_STACK_SIZE, NULL,TASK_LED_STACK_PRIORITY, NULL);
	
	xTaskCreate(task_led1, "Led1", TASK_LED1_STACK_SIZE, NULL, TASK_LED1_STACK_PRIORITY, NULL);

	/* Output demo information. */
	printf("-- Freertos Example --\n\r");
	printf("-- %s\n\r", BOARD_NAME);
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);


	/* Create task to monitor processor activity */
	if (xTaskCreate(task_monitor, "Monitor", TASK_MONITOR_STACK_SIZE, NULL,
			TASK_MONITOR_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create Monitor task\r\n");
	}

	/* Create task to make led blink */
	if (xTaskCreate(task_led, "Led", TASK_LED_STACK_SIZE, NULL,
			TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test led task\r\n");
	}
	
	if (xTaskCreate(task_led2, "Led", TASK_LED_STACK_SIZE, NULL,
	TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test led task\r\n");
	}
	
	if (xTaskCreate(task_led3, "Led", TASK_LED_STACK_SIZE, NULL,
	TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test led task\r\n");
	}

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}

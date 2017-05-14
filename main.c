/* ###################################################################
 **     Filename    : ProcessorExpert.c
 **     Project     : ProcessorExpert
 **     Processor   : MKL25Z128VLK4
 **     Version     : Driver 01.01
 **     Compiler    : GNU C Compiler
 **     Date/Time   : 2017-04-12, 10:56, # CodeGen: 0
 **     Abstract    :
 **         Main module.
 **         This module contains user's application code.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
 
/*!
 ** @file ProcessorExpert.c
 ** @version 01.01
 ** @brief
 **         Main module.
 **         This module contains user's application code.
 */
/*!
 **  @addtogroup ProcessorExpert_module ProcessorExpert module documentation
 **  @{
 */
 
/* MODULE ProcessorExpert */
/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "AS1.h"
#include "ASerialLdd1.h"
#include "Bit1.h"
#include "BitIoLdd1.h"
#include "Bit2.h"
#include "BitIoLdd2.h"
#include "Bit3.h"
#include "BitIoLdd3.h"
#include "TI1.h"
#include "TimerIntLdd1.h"
#include "TU1.h"
#include "Row.h"
#include "BitsIoLdd1.h"
#include "EE241.h"
#include "WAIT1.h"
#include "GI2C0.h"
#include "CI2C1.h"
#include "Bit4.h"
#include "BitIoLdd4.h"
#include "UTIL1.h"
#include "MCUC1.h"
#include "LED1.h"
#include "LEDpin1.h"
#include "BitIoLdd5.h"
#include "WAIT2.h"
#include "AD1.h"
#include "AdcLdd1.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include <string.h>

/* User includes (#include below this line is not maintained by Processor Expert) */

/*Declaracao de variaveis globais de controle*/
extern int UART_state; 			//variavel que armazena estado da UART
extern int keyboard_state; 		//variavel que armazena estado do teclado matricial
extern uint8_t LDR_value; 		//variavel de 8 bits para medicao ADC do sinal luminoso
extern uint8_t mem_addr;		//variavel que armazena um endereco da memoria
extern int flag_auto;			//variavel que habilita medicao automatica

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */

int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
	char out_buffer[30];

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/
	LED1_Init();

	while (1) {
		/*Apos execucao de uma das funcoes, variaveis que armazenam estados sao reinicializadas para execucao de funcoes futuras*/
		
		/*Funcoes da UART*/
		
		/*(PING) PONG: Caso usuario envie mensagem PING, recebera resposta PONG*/
		if (UART_state == 1) {
			strcpy(out_buffer, "PONG\r\n");
			send_string(out_buffer);
			UART_state = 0;
		}
		/*(ID) String de identificacao: Caso usuario envie mensagem ID, recebera uma string de identificacao*/
		if (UART_state == 2) {
			strcpy(out_buffer, "DATALOGGER DO TAFFA\r\n");
			send_string(out_buffer);
			UART_state = 0;
		}
		/*(MEASURE) Retorna valor de uma medicao*/
		if (UART_state == 3) {
			AD1_Measure(FALSE);		//realiza medicao ADC
			WAIT2_Waitms(200);		//Delay de 200ms para que programa tenha tempo de processar a medicao
			UART_state = 0;
		}
		/*(MEMSTATUS) Numero de elementos na memoria*/
		if (UART_state == 4) {
			n_elements();
			UART_state = 0;
		}
		/*(RESET) Apaga toda a memoria*/
		if (UART_state == 5) {
			reset_mem();
			UART_state = 0;
		}
		/*(RECORD) Realiza uma medicao e grava valor na memoria*/
		if (UART_state == 6) {
			AD1_Measure(FALSE);		//realiza medicao ADC
			WAIT2_Waitms(200);		//Delay de 200ms para que programa tenha tempo de processar a medicao
			file_system(LDR_value);
			UART_state = 0;
		}
		/*(GET N) Retorna o N-esimo elemento da memoria*/
		if (UART_state == 7) {
			get_mem_val(mem_addr);
			UART_state = 0;
		}

		/*Funcoes do teclado matricial*/
		
		/*Pisca um LED*/
		if (keyboard_state == 1) {
			WAIT2_Waitms(500);			
			LED1_Neg();				//nega valor do LED da placa
			WAIT2_Waitms(500);
			LED1_Neg();	
			keyboard_state = 0;
		}
		/*Realiza uma medicao e grava na memoria*/
		if (keyboard_state == 2) {
			AD1_Measure(FALSE);		//realiza medicao ADC
			WAIT2_Waitms(200);		//Delay de 200ms para que programa tenha tempo de processar a medicao
			file_system(LDR_value);
			keyboard_state = 0;
		}
		
		/*Ativo modo medicao automatica*/
		/*Neste caso, keyboard_state nao eh reinicializada para que a funcao seja executada ate que o usuario desative a medicao automatica*/
		if (keyboard_state == 3) {
			AD1_Measure(FALSE);		//realiza medicao ADC
			WAIT2_Waitms(200);		//Delay de 200ms para que programa tenha tempo de processar a medicao
		}
		
		/*Desativa modo medicao automatica*/
		/*Neste caso, programa retorna ao estado inicial*/
		if (keyboard_state == 4) {
			keyboard_state = 0;		
		}
	}

	/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
	/*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
#ifdef PEX_RTOS_START
	PEX_RTOS_START(); /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of RTOS startup code.  ***/
	/*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
	for (;;) {
	}
	/*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END ProcessorExpert */
/*!
 ** @}
 */
/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.3 [05.08]
 **     for the Freescale Kinetis series of microcontrollers.
 **
 ** ###################################################################
 */
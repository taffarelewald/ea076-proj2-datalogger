/* ===================================================================
**	Projeto 2: Datalogger
**  
**  Taffarel Cunha Ewald 147957
**  Daniel Rodrigues Silveira Freitas 145782
**        
**	Programa implementa um sistema Datalogger que captura dados de um
** 	sensor de dados ambientais, possuindo interface com o computador e 
**	um teclado matricial, e grava os dados obtidos em uma memoria EEPROM,
**	utilizando um sistema de arquivos.
** ===================================================================*/

/* MODULE Events */

#include "Cpu.h"
#include "Events.h"

#ifdef __cplusplus
extern "C" {
#endif 

/* User includes (#include below this line is not maintained by Processor Expert) */

/*
 ** ===================================================================
 **     Event       :  Cpu_OnNMIINT (module Events)
 **
 **     Component   :  Cpu [MKL25Z128LK4]
 */
/*!
 **     @brief
 **         This event is called when the Non maskable interrupt had
 **         occurred. This event is automatically enabled when the [NMI
 **         interrupt] property is set to 'Enabled'.
 */
/* ===================================================================*/
void Cpu_OnNMIINT(void) {
	/* Write your code here ... */
}

/* stdio.h contem rotinas para processamento de expressoes regulares */
#include <stdio.h>

/*Declaracao das variaveis globais de controle*/
int UART_state;		//variavel que armazena estado da UART
int keyboard_state;	//variavel que armazena estado do teclado matricial
uint8_t LDR_value;	//variavel de 8 bits para medicao ADC do sinal luminoso
uint8_t mem_addr;	//variavel que armazena um endereco da memoria
int flag_auto = 0;	//variavel que habilita medicao automatica

/*Rotina auxiliar para comparacao de strings*/
int str_cmp(char *s1, char *s2, int len) {
	/*Compare two strings up to length len. Return 1 if they are equal, and 0 otherwise.*/
	int i;
	for (i = 0; i < len; i++) {
		if (s1[i] != s2[i])
			return 0;
		if (s1[i] == '\0')
			return 1;
	}
	return 1;
}

/*Processo de bufferizacao. Caracteres recebidos sao armazenados em um buffer. 
Quando um caractere de fim de linha ('\n') eh recebido, todos os caracteres do buffer sao processados simultaneamente.*/

/*Buffer de dados recebidos*/
#define MAX_BUFFER_SIZE 20
typedef struct {
	char data[MAX_BUFFER_SIZE];
	unsigned int tam_buffer;
} serial_buffer;

volatile serial_buffer Buffer;

/*Limpa buffer*/
void buffer_clean() {
	Buffer.tam_buffer = 0;
}

/*Adiciona caractere ao buffer*/
int buffer_add(char c_in) {
	if (Buffer.tam_buffer < MAX_BUFFER_SIZE) {
		Buffer.data[Buffer.tam_buffer++] = c_in;
		return 1;
	}
	return 0;
}

/*Funcao que define tarefa a ser executada, a partir da entrada do usuario pelo computador*/
void UART_compare() {
	/*Apos definicao da tarefa a ser executada, ou seja, do valor de UART_state, o buffer eh limpo para que outra entrada possa ser recebida*/

	/*Caso em que usuario envia mensagem "PING"*/
	if (str_cmp(Buffer.data, "PING", 4)) {
		UART_state = 1;
		buffer_clean();
	}
	
	/*Caso em que usuario envia mensagem "ID"*/
	if (str_cmp(Buffer.data, "ID", 2)) {
		UART_state = 2;
		buffer_clean();
	}

	/*Caso em que usuario envia mensagem "MEASURE"*/
	if (str_cmp(Buffer.data, "MEASURE", 7)) {
		UART_state = 3;
		buffer_clean();
	}
	
	/*Caso em que usuario envia mensagem "MEMSTATUS"*/
	if (str_cmp(Buffer.data, "MEMSTATUS", 9)) {
		UART_state = 4;
		buffer_clean();
	}
	
	/*Caso em que usuario envia mensagem "RESET"*/
	if (str_cmp(Buffer.data, "RESET", 5)) {
		UART_state = 5;
		buffer_clean();
	}
	
	/*Caso em que usuario envia mensagem "RECORD"*/
	if (str_cmp(Buffer.data, "RECORD", 6)) {
		UART_state = 6;
		buffer_clean();
	}
	
	/*Caso em que usuario envia mensagem "GET N", sendo N o indice do endereco de memoria a ser acessado*/
	if (str_cmp(Buffer.data, "GET", 3)) {
		sscanf(Buffer.data, "%*s %d", &mem_addr);  //valor de N eh atribuido a variavel mem_addr
		UART_state = 7;
		buffer_clean();
	}
}

/*Funcao que apaga toda a memoria*/
void reset_mem() {
	uint8_t mem_clr = 0; 	//variavel utilizada para limpar memoria
	/*Escreve 0 na posicao 0 da memoria referente ao numero de registros existentes na memoria*/
	EE241_WriteByte(0, (byte) mem_clr);
	send_string("Memoria apagada\n\r");
}

/*Funcao que le o valor do endereco N na memoria*/
/*Funcao recebe endereco da memoria a ser lido*/
void get_mem_val(uint8_t mem_addr) {
	byte mem_val;		//variavel para armazenamento do valor a ser lido na posicao N
	byte n_elements; 	//variavel que recebe numero de elementos existentes na memoria
	EE241_ReadByte(0, &n_elements);
	
	/*Teste para verificar se existe a posicao na memoria*/
	if (mem_addr <= n_elements) {
		EE241_ReadByte(mem_addr, &mem_val);
		send_string("Elemento da posicao ");
		send_int((int) mem_addr);
		send_string(": ");
		send_int((int) mem_val);
		send_string("\n\r");
	} else {
		/*Caso em que posicao da memoria nao contem dado*/
		send_string("Posicao da memoria nao preenchida\r\n");
	}
}

/*Funcao que obtem numero de elementos existentes na memoria*/
void n_elements() {
	byte n_elements;	//variavel que recebe numero de elementos existentes na memoria
	EE241_ReadByte(0, &n_elements);
	send_string("Numero de elementos da memoria: ");
	send_int((int) n_elements);
	send_string("\n\r");
}

/*Funcao chamada ao receber evento da UART para criacao do buffer de entrada*/
void AS1_OnRxChar(void) {
	char c;
	while (AS1_RecvChar(&c) == ERR_OK) {
		if (c == '\r') {
			buffer_add('\0'); /*Se recebeu um fim de linha, coloca um terminador de string no buffer*/
			UART_compare();   //Funcao UART_compare eh chamada para definicao da tarefa de acordo com entrada do usuario armazenada no buffer
		} else {
			buffer_add(c);
		}
	}
}

/*Funcao que recebe variavel do tipo int e envia para terminal*/
void send_int(int val) {
	char str[16];
	UTIL1_Num16uToStr((uint8_t*) str, 16, val);
	send_string(str);
}

/*Funcao que recebe variavel do tipo char* e envia para terminal*/
void send_string(char* s) {
	int err;
	while (*s != '\0') {
		err = 1;
		while (err) {
			err = AS1_SendChar(*s);
		}
		s++;
	}
}

/*Funcao chamada apos medicao ADC*/
void AD1_OnEnd(void) {
	AD1_GetValue8(&LDR_value);		//valor ADC medido eh guardado na variavel LDR_value
	
	/*Caso em que usuario queira realizar uma medida, por meio da comunicacao pelo computador ou pelo teclado matricial*/
	if (UART_state == 3 || keyboard_state == 3) { 
		send_string("Valor medido: ");
		send_int(LDR_value);
		send_string("\n\r");
	}
}

/*Funcao que atua diretamente na memoria gravando as medidas realizadas*/
/*Funcao recebe dado da memoria a ser gravado*/
void file_system(uint8_t mem_data) {
	byte index; //variavel que armazena indice na memoria

	EE241_ReadByte(0, &index);	//le numero de elementos existentes na memoria e guarda na variavel index
	index++;					//atualiza indice na memoria para posicao a ser realizada a gravacao do dado

	/*Grava um dado no sistema de arquivos na posicao index*/
	EE241_WriteByte(index, (byte) mem_data);

	/*Atualiza valor do numero de elementos existentes na memoria do sistema de arquivos*/
	EE241_WriteByte(0, (byte) index);

	send_string("Valor gravado com sucesso\n\r");
}

void TI1_OnInterrupt(void) {

	char keyboard_buffer[4]; 	//string que recebe entrada do teclado matricial
	int i, j;
	int flag_write = 0;			//variavel responsavel por indicar que o buffer esta pronto para ser escrito
	int column[3];				//array que contem estado de cada coluna do teclado matricial

	/*Varredura do teclado matricial*/
	
	/*Controle do boucing*/
	WAIT1_Waitms(120);
	
	/*Laco que percorre as linhas*/
	for (i = 0; i <= 3; i++) {
		Row_ClrBit(i); 			//ativa a linha que será testada
		/*array recebe estado das colunas*/
		column[0] = Bit1_GetVal();
		column[1] = Bit2_GetVal();
		column[2] = Bit3_GetVal();
		
		/*Laco que varre as colunas*/
		for (j = 0; j <= 2; j++) {
			/*Caso as colunas estejam em nivel baixo, significa que o teclado foi pressionado*/
			if (!column[j]) {
				/*Decodificacao da matriz do teclado, atribuindo caracteres as posicoes*/
				if (i == 3 && j == 0) {
					keyboard_buffer[0] = '#';
				}
				if (i == 0 && j == 0) {
					keyboard_buffer[1] = '1';
				}
				if (i == 0 && j == 1) {
					keyboard_buffer[1] = '2';
				}
				if (i == 0 && j == 2) {
					keyboard_buffer[1] = '3';
				}
				if (i == 1 && j == 0) {
					keyboard_buffer[1] = '4';
				}
				if (i == 3 && j == 2) {
					keyboard_buffer[2] = '*';
					keyboard_buffer[3] = '\0';
					flag_write = 1;		//apos mensagem ser encerrada, buffer pode ser escrito
				}
			}
		}
		
		/*Desativa a linha que foi testada*/
		Row_SetBit(i);
	}

	/*Compara as strings recebidas pelo teclado para definir tarefa a ser executada*/
	if (str_cmp(keyboard_buffer, "#1*", 3)) {
		keyboard_state = 1;

	}
	if (str_cmp(keyboard_buffer, "#2*", 3)) {
		keyboard_state = 2;

	}
	if (str_cmp(keyboard_buffer, "#3*", 3)) {
		send_string("Medicao automatica ativada\n\r");
		keyboard_state = 3;

	}
	if (str_cmp(keyboard_buffer, "#4*", 3)) {
		send_string("Medicao automatica desativada\n\r");
		keyboard_state = 4;
	}
}

/* END Events */

#ifdef __cplusplus
} /* extern "C" */
#endif 

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
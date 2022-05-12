/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


/* Descri��o:
 Implementa��o de uma comunica��o serial simples controlada por software:
 - O pino PA0 opera em dreno aberto como pino de comunica��o entre duas placas
 - Os pinos PA0 de duas placas devem ser interligados
 - Os bot�es K0 e K1 acionam, respectivamente, os LEDs D2 e D3 da outra placa
 - H� um buzzer no pino PA0 para sinalizar o envio de um dado
*/

#include "stm32f4xx.h"		//inclus�o das defini��es do microcontrolador
#include "Setup_Fagner.h"

void envia_cmd(uint8_t);	//fun��o para enviar um comando no barramento
uint8_t recebe_cmd(void);	//fun��o para receber um comando
void buzzer(void);			//fun��o de ativa��o do buzzer

//fun��o principal
int main(void)
{
	Configure_Clock();			//configura o sistema de clock
	Delay_Start();				//inicializa fun��es de Delay

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOEEN;	//habilita o clock do GPIOA e GPIOE

	GPIOA->ODR |= (1<<7) | (1<<6) | 1;		//inicia com leds e buzzer desligados e sa�da COM em idle
	GPIOA->OTYPER |= 1;						//sa�da open-drain em PA0
	GPIOA->PUPDR |= 0b01;					//habilita pull-up em PA0
	GPIOA->MODER |= (0b01 << 14) | (0b01 << 12) | (0b01 << 2) | (0b01) ; 	//pinos PA0, PA1, PA6 e PA7 no modo sa�da
	GPIOE->PUPDR |= (0b01 << 8) | (0b01 << 6);								//habilita pull-up em PE4 e PE3

	while(1)
	{
		if(!(GPIOE->IDR & (1 << 4)))	//verifica se PE4 foi pressionado
		{
			envia_cmd(0b01);
			Delay_ms(75);
			while(!(GPIOE->IDR & (1 << 4)));	//aguarda o bot�o ser solto
		}

		if(!(GPIOE->IDR & (1 << 3)))	//verifica se PE3 foi pressionado
		{
			envia_cmd(0b10);
			Delay_ms(75);
			while(!(GPIOE->IDR & (1 << 3)));	//aguarda o bot�o ser solto
		}

		if(!(GPIOA->IDR & 1))	//verifica se h� comunica��o
		{
			uint8_t recebido = recebe_cmd();	//recebe o comando
			if(recebido == 0b01)
			{
				GPIOA->ODR ^= 1 << 6;	//alterna o estado do LED em PA6
			}
			if(recebido == 0b10)
			{
				GPIOA->ODR ^= 1 << 7;	//alterna o estado do LED em PA7
			}
		}
	}
}

//Fun��o para envio de um comando
void envia_cmd(uint8_t dado)
{
	GPIOA->ODR &= ~1;	//start bit
	Delay_us(50);		//aguarda tempo do start bit
	for(uint8_t counter=0; counter<2; ++counter)
	{
		if(dado & 1)			//envia o pr�ximo bit
			GPIOA->ODR |= 1;
		else
			GPIOA->ODR &= ~1;
		dado >>= 1;
		Delay_us(50);			//aguarda o tempo do bit
	}
	GPIOA->ODR |= 1;	//stop bit
	buzzer();			//sinaliza o fim do envio
}

//Fun��o para recebimento de um comando
uint8_t recebe_cmd(void)
{
	uint8_t dado_recebido = 0;
	uint8_t dado_retornado = 0;
	Delay_us(25);			//aguarda metade do start bit
	if(!(GPIOA->IDR & 1))	//confirma que houve um start bit
	{
		for(uint8_t counter=0; counter<2; ++counter)	//leitura dos bits
		{
			Delay_us(50);				//aguarda o tempo do bit
			if(GPIOA->IDR & 1)
				dado_recebido |= 1;
			else
				dado_recebido &= ~1;
			dado_recebido <<= 1;
		}

		Delay_us(50);			//aguarda para fazer leitura do stop bit
		if((GPIOA->IDR & 1))	//confirma que houve um stop bit
		{
			dado_recebido >>= 1;
			return dado_retornado = ((dado_recebido & 0b10) >> 1) | ((dado_recebido & 0b01) << 1); //retorna o dado recebido
		}
		else
			return 0;	//n�o houve recep��o do stop bit, aborta recep��o
	}
	else
		return 0;		//n�o houve recep��o do start bit, aborta recep��o
}

//Fun��o de sinaliza��o de fim de envio de dado
void buzzer(void)
{
	GPIOA->ODR ^= 1 << 1;
	Delay_ms(8);
	GPIOA->ODR ^= 1 << 1;
	Delay_ms(100);
	GPIOA->ODR ^= 1 << 1;
	Delay_ms(8);
	GPIOA->ODR ^= 1 << 1;
}

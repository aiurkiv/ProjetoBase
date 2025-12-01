/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_display.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include "app_display.h"
#include "config/default/peripheral/gpio/plib_gpio.h"
#include "config/default/peripheral/coretimer/plib_coretimer.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include "config/default/system/debug/sys_debug.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
QueueHandle_t xLcdQueue;
char lcd[4][20];

bool reiniciar_lcd; 
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_DISPLAY_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

//APP_DISPLAY_DATA app_displayData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_DISPLAY_Initialize ( void )

  Remarks:
    See prototype in app_display.h.
 */

void APP_DISPLAY_Initialize ( void )
{
    reiniciar_lcd = false;
    memset(lcd, ' ', sizeof(lcd));
    // Cria a fila de eventos de atualização do display
    xLcdQueue = xQueueCreate(10, sizeof(lcd));
    configASSERT(xLcdQueue != NULL);
}

/*
   lcd_send_nibble(char n)
   
   Envia un nibble (4 bits) às linhas de dados do lcd
   
   n: nible a ser enviado (os 4 bits menos significativos)
*/
void lcd_send_nibble(char n) 
{
	PINO_LCD_D4_OUT=(n&0x01);
	PINO_LCD_D5_OUT=(n&0x02)>>1;
	PINO_LCD_D6_OUT=(n&0x04)>>2;
	PINO_LCD_D7_OUT=(n&0x08)>>3;

	//delay_us(10);
	CORETIMER_DelayUs(6);	// Alterei depois de corrigir a precisão do tempo de delay
   
	PINO_LCD_EN=1;
	//delay_us(20);	
	CORETIMER_DelayUs(12);	// Alterei depois de corrigir a precisão do tempo de delay
 	PINO_LCD_EN=0;
}

/*
   lcd_send_byte(char _rs, char n)
   
   Envia um byte (um nibble por vez) ao LCD
   
   _rs: Valor a ser enviado ao pino RS
   n: O byte
*/
void lcd_send_byte(char _rs, char n)
{
	unsigned char lsn,msn;

	// Se for um null character, troca por um espaço em branco
	if (n=='\0')
	    n=' ';

	CORETIMER_DelayUs(64);	// Alterei depois de corrigir a precisão do tempo de delay
	PINO_LCD_EN=0;
	PINO_LCD_RS=_rs;	//RS=0 - Escreve um endereço / RS=1 - Escreve um dado
	PINO_LCD_RW=0;		//RW=0 - Escrita
	
	// Espera Tas = 30ns
	DELAY_25NS();
	DELAY_25NS();
	
	// Manda nibble menos significativo
	lsn=n >> 4;
	PINO_LCD_D4_OUT=(lsn&0x01);
	PINO_LCD_D5_OUT=(lsn&0x02)>>1;
	PINO_LCD_D6_OUT=(lsn&0x04)>>2;
	PINO_LCD_D7_OUT=(lsn&0x08)>>3;
	PINO_LCD_EN=1;

	// Espera Tpw = 150ns 
	DELAY_250NS();	// Mudei de 200ns para 400ns para ficar igual ao pulso que aparece no HGI7000 de 250ns - Assim ele funciona com o WINSTAR WH2004L-YYH-JTE#
	
	PINO_LCD_EN=0;

	// Antes de iniciar o próximo ciclo, espera até completar Tc=400ns
	DELAY_200NS();
	DELAY_200NS();
	
	// Manda nibble mais significativo
	msn=n & 0xf;
	PINO_LCD_D4_OUT=(msn&0x01);
	PINO_LCD_D5_OUT=(msn&0x02)>>1;
	PINO_LCD_D6_OUT=(msn&0x04)>>2;
	PINO_LCD_D7_OUT=(msn&0x08)>>3;
	PINO_LCD_EN=1;

	// Espera Tpw = 150ns
	DELAY_250NS();	// Mudei de 200ns para 400ns para ficar igual ao pulso que aparece no HGI7000 de 250ns - Assim ele funciona com o WINSTAR WH2004L-YYH-JTE#
	
	PINO_LCD_EN=0;

	// Antes de iniciar o próximo ciclo, espera até completar Tc=400ns
	DELAY_200NS();
	DELAY_200NS();
}

/*
   lcd_init()
   
   Inicia o LCD no modo 4 bits
*/
void lcd_init()
{
    PINO_LCD_RS=0;
    PINO_LCD_EN=0;
    PINO_LCD_RW=0;
    CORETIMER_DelayMs(10);

    lcd_send_nibble(3);
    CORETIMER_DelayMs(5);
    lcd_send_nibble(3);
    CORETIMER_DelayMs(5);
    lcd_send_nibble(3);
    CORETIMER_DelayMs(5);

    lcd_send_nibble(2);

    // Codigo de configuração
    lcd_send_byte(0, 0x28);	// Configura LCD em duas linhas, modo 4 bits
    lcd_send_byte(0, 0xC);	// display ON, cursor OFF, blink OFF
    lcd_send_byte(0, 0x1);	// Clear display
    lcd_send_byte(0, 0x6);	// shift right

    // Espera um pouco antes de poder escrever
    CORETIMER_DelayMs(10);
}

/******************************************************************************
  Function:
    void APP_DISPLAY_Tasks ( void )

  Remarks:
    See prototype in app_display.h.
 */

TaskHandle_t xAPP_DISPLAY_Tasks;    // Handle do lcd. Posso usar para notificação ou identificação da task.
				    // Até o momento sem uso.

void APP_DISPLAY_Tasks ( void )
{
    // Guarda o handle desta task para outra poder notificá-la
    xAPP_DISPLAY_Tasks = xTaskGetCurrentTaskHandle();
    
    unsigned char loop;

    // Configura o lcd
    lcd_init();
    
    while(true)
    {
        // Reconfigura o lcd
        if(reiniciar_lcd)
        {
            reiniciar_lcd = false;
            lcd_init();
        }
        if (xQueueReceive(xLcdQueue, lcd, portMAX_DELAY) == pdTRUE)
        {
            lcd_send_byte(0, 0x28);	

            for (loop=0 ; loop<20 ; loop++)
            lcd_send_byte(1, lcd[0][loop]);

            for (loop=0 ; loop<20 ; loop++)
            lcd_send_byte(1, lcd[2][loop]);

            for (loop=0 ; loop<20 ; loop++)
            lcd_send_byte(1, lcd[1][loop]);

            for (loop=0 ; loop<20 ; loop++)
            lcd_send_byte(1, lcd[3][loop]);
        }
    }
}

/* atualiza_lcd()
 * Coloca uma atualização na fila de atualizações do lcd.
 * Assim não gera concorrência caso mais de uma task tente
 * atualizar.
 */
void atualiza_lcd(char* x_lcd)
{
    // Coloca a atualização do lcd na fila
    xQueueSend(xLcdQueue, x_lcd, portMAX_DELAY);
    return;
}

/* reconfigura_lcd()
 * Caso o lcd trave, chamando essa função ela reconfigura e limpa o lcd
 */
void reconfigura_lcd()
{
    char x_lcd[4][20];
    memset(x_lcd, ' ', sizeof(x_lcd));
    reiniciar_lcd = true;
    atualiza_lcd((char*)x_lcd);
}

/*******************************************************************************
 End of File
 */

/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    medida_gb.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "MEDIDA_GB_Initialize" and "MEDIDA_GB_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "MEDIDA_GB_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _MEDIDA_GB_H
#define _MEDIDA_GB_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    MEDIDA_GB_STATE_INIT=0,
    MEDIDA_GB_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} MEDIDA_GB_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    MEDIDA_GB_STATES state;
    // Corrente instantânea medida (para mostrar no display)
    float correnteA;
} MEDIDA_GB_DATA;

// Torna o dado acessível em outros módulos
extern MEDIDA_GB_DATA medida_gbData;

/*
typedef struct medida_gb
{
    // As variáveis abaixo guardam as ultimas leituras numa medida de aterramento
	unsigned short	gb_corrente;
	unsigned short	gb_tensao;
	unsigned short	gb_resistencia_medida;		
	unsigned char	gb_limite_corrente_excedido;	// Flag que informa se o limite de corrente foi excedido na medida de aterramento
	unsigned char	gb_limite_corrente_excedido_cont;	// Contador que conta o numero de vezes que a corrente passou do limite.
	unsigned char	gb_limite_corrente_excedido_reiniciado;	// Conta quantas vezes o ajuste foi reiniciado por causa de sobrecorrente
	unsigned char	gb_semcarga;			// Flag que informa de não há carga na medida de aterramento
	unsigned short	gb_resistencia_max_medida;	// Guarda o maior valor de resistência lida durante a medida de aterramento
	unsigned char	gb_leituras_fora;		// Guarda a quantidade de leitura seguidas fora do intervalo durante uma medida
    unsigned char   gb_i_estavel_cont;      // Guarda em 100ms há quanto tempo a corrente já estabilizou
    
    // **** As variáveis abaixo são usadas no controle do tempo durante a medida

    unsigned short contador_samples;    // Contador do número de amostras lidas pelo AD
    // Número de leituras do AD (samples) que completam um ciclo de senoide
	// samples_ciclo =  peíodo da senoide / período de amostragem
    // Depende da frequencia da senoide (50Hz ou 60Hz)
	unsigned short samples_ciclo;
    // Contador de ciclos - é incrementado a cada ciclo de senoide
	unsigned short contador_ciclos;
	// Contador de decisegundos, é incrementado a cada 6 ciclos
	unsigned short contador_ds;
} MEDIDA_GB;
*/

// Config da rede
#define MAINS_FREQ_HZ        60U
#define HALF_CYCLE_TICKS     62500U   // ~8,33ms com TMR6 @ 7,5MHz

// Reservamos alguns ticks antes do zero pra desligar o gate
#define SAFETY_TICKS_TO_ZERO 4000U    // ~0,53ms
#define MIN_GATING_TICKS     1000U    // ~0,13ms

// Limites de potência
#define TRIAC_POWER_MIN      0U       // 0%
#define TRIAC_POWER_MAX      100U     // 100%

// Inicializa periféricos e variáveis do controle
void TRIAC_Control_Initialize(void);

// Ajusta potência alvo (0..100%)
void TRIAC_SetPowerPercent(uint8_t percent);

// Handler chamado pelo zero-cross (INTx)
void ZC_InterruptHandler(GPIO_PIN pin, uintptr_t context);

// Callback do Timer 6 (registrado no plib TMR6)
void TMR6_Callback(uint32_t status, uintptr_t context);

// Task de ensaio de corrente (5 segundos, "one-shot")
void MEDIDA_GB_RunTestTask(void *pvParameters);

// Handle da task (declarado em tasks.c)
extern TaskHandle_t xMEDIDA_GB_Tasks;

void MEDIDA_GB_Initialize ( void );
void MEDIDA_GB_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _MEDIDA_GB_H */

/*******************************************************************************
 End of File
 */


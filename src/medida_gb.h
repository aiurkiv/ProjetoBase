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
    /* The application's current state */
    MEDIDA_GB_STATES state;

    /* TODO: Define any additional data used by the application. */

} MEDIDA_GB_DATA;

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
    
    /**** As variáveis abaixo são usadas no controle do tempo durante a medida ****/

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

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void MEDIDA_GB_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    MEDIDA_GB_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    MEDIDA_GB_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void MEDIDA_GB_Initialize ( void );


/*******************************************************************************
  Function:
    void MEDIDA_GB_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    MEDIDA_GB_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

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


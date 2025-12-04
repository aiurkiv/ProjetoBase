/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    menu_display.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "MENU_DISPLAY_Initialize" and "MENU_DISPLAY_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "MENU_DISPLAY_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _MENU_DISPLAY_H
#define _MENU_DISPLAY_H

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
    MENU_DISPLAY_STATE_INIT=0,
    MENU_DISPLAY_STATE_HP,
    MENU_DISPLAY_STATE_GB,
    MENU_DISPLAY_STATE_TF,
    ENSAIO_GB_STATE_ENSAIANDO,
//    MENU_DISPLAY_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} MENU_DISPLAY_STATES;

// Telas / estados simples de menu (exemplo)
typedef enum
{
    MENU_SCREEN_HOME = 0,
    MENU_SCREEN_PARAM1,
    MENU_SCREEN_PARAM2,

} MENU_SCREEN;


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
    MENU_DISPLAY_STATES state;
    MENU_SCREEN currentScreen;
    uint8_t currentItem;         // item selecionado na tela

    char lcd[4][20];             // buffer 4x20 próprio do menu

} MENU_DISPLAY_DATA;

// Exporta a estrutura global
extern MENU_DISPLAY_DATA menu_displayData;

// Botões
typedef enum
{
    BTN_BACK = 0,
    BTN_ENTER,
    BTN_CIMA,
    BTN_BAIXO,
    BTN_COUNT,
    ACT_NONE,   // sem evento
} ACTION_ID;

typedef enum
{
    BTN_EVENT_PRESS,    // apertou botão
    BTN_EVENT_RELEASE,  // soltou botão
    BTN_EVENT_REPEAT,   // auto-repeat botão
    ACT_EVENT_DISPLAY_UPDATE      // Atualiza display
} ACTION_EVENT_TYPE;

typedef struct
{
    ACTION_ID        id;
    ACTION_EVENT_TYPE type;
} ACTION_EVENT;


/* MENU_DISPLAY_Initialize()
 * Define os parâmetros iniciais do MENU display e inicia variáveis.
 */
void MENU_DISPLAY_Initialize ( void );
void MENU_DISPLAY_Tasks( void );

/************** Daqui para baixo são os estados e funções de impressão no display **************/
void MENU_DISPLAY_DrawHome(void);
void MENU_DISPLAY_DrawHP(void);
void MENU_DISPLAY_DrawGB(void);
void MENU_DISPLAY_DrawTF(void);
void ENSAIO_GB_DrawEnsaiando(void);
void ACTION_SendEventFromTask(ACTION_ID id, ACTION_EVENT_TYPE type);

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _MENU_DISPLAY_H */

/*******************************************************************************
 End of File
 */


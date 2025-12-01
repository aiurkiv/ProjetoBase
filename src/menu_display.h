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
    MENU_DISPLAY_STATE_SERVICE_TASKS,
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
    BTN_MAIS,
    BTN_MENOS,
    BTN_COUNT
} BUTTON_ID;

typedef enum
{
    BTN_EVENT_PRESS,    // apertou
    BTN_EVENT_RELEASE,  // soltou
    BTN_EVENT_REPEAT    // auto-repeat
} BUTTON_EVENT_TYPE;

typedef struct
{
    BUTTON_ID        id;
    BUTTON_EVENT_TYPE type;
} BUTTON_EVENT;


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
    void MENU_DISPLAY_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    MENU_DISPLAY_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    MENU_DISPLAY_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void MENU_DISPLAY_Initialize ( void );

void setup_switches();
void switch_handler(GPIO_PIN pin, uintptr_t context);
void TMR3_Callback(uint32_t status, uintptr_t context);

/*******************************************************************************
  Function:
    void MENU_DISPLAY_Tasks ( void )

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
    MENU_DISPLAY_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void MENU_DISPLAY_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _MENU_DISPLAY_H */

/*******************************************************************************
 End of File
 */


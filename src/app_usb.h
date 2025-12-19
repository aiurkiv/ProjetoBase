/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_usb.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_USB_Initialize" and "APP_USB_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_USB_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_USB_H
#define _APP_USB_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
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
    /* Application pixels put*/
    APP_USB_STATE_INIT=0,
    APP_USB_STATE_OPEN_HOST_LAYER,
    APP_USB_STATE_WAIT_FOR_HOST_ENABLE,
    APP_USB_STATE_HOST_ENABLE_DONE,
    APP_USB_STATE_WAIT_FOR_DEVICE_ATTACH,
    APP_USB_STATE_DEVICE_ATTACHED,
    APP_USB_STATE_READ_HID,
    APP_USB_STATE_DEVICE_DETACHED,
    APP_USB_STATE_CHANGE_DEVICE_PARAMETERS,
//    APP_USB_USART_STATE_DRIVER_OPEN,
//    APP_USB_USART_STATE_CHECK_FOR_STRING_TO_SEND,
//    APP_USB_USART_STATE_DRIVER_WRITE,
    APP_USB_STATE_ERROR

} APP_USB_STATES;


typedef struct
{
    /* Application last data buffer */
    USB_HOST_HID_KEYBOARD_DATA data;

} APP_USB_DATA_LAST_DATA;


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
    /* USB Application's current state*/
    APP_USB_STATES state;
    
    /* USART Application task state */
    //APP_STATES usartTaskState;
    
    /* Unique handle to USB HID Host Keyboard driver */
    USB_HOST_HID_KEYBOARD_HANDLE handle;
    
    /* Unique handle to USART driver */
    //DRV_HANDLE usartDriverHandle;
    
    /* Number of bytes written by the USART write*/
    //uint32_t nBytesWritten;
    
    /* Size of the buffer to be written */
    uint32_t stringSize;
    
    /* Buffer used for USART writing */
    uint8_t string[64];
    
    /* Holds the current offset in the string buffer */
    uint16_t currentOffset;
    
    /* Flag used to determine if data is to be written to USART */
    bool stringReady;
    
    /* Flag used to select CAPSLOCK sequence */
    bool capsLockPressed;
    
    /* Flag used to select SCROLLLOCK sequence */
    bool scrollLockPressed;
    
    /* Flag used to select NUMLOCK sequence */
    bool numLockPressed;
    
    /* Holds the output Report*/
    uint8_t outputReport;

    /* Application current data buffer */
    USB_HOST_HID_KEYBOARD_DATA data;
    
    /* Application last data buffer */
    APP_USB_DATA_LAST_DATA  lastData;
    
    //DRV_USART_BUFFER_HANDLE WriteBufHandler;

} APP_USB_DATA;

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
    void APP_USB_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_USB_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_USB_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_USB_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_USB_Tasks ( void )

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
    APP_USB_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_USB_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_USB_H */

/*******************************************************************************
 End of File
 */


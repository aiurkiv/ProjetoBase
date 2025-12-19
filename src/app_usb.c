/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_usb.c

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

#include "app_usb.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
// Fora da função, em escopo estático do arquivo:
static uint8_t  s_lastKey = 0;
static uint32_t s_lastKeyTime = 0;
static const uint32_t KBD_DEBOUNCE_MS = 200U;   // mínimo entre ?toques?

/* Usage ID to Key map table */
const char *keyValue[] = { 
                          "No event indicated",
                          "ErrorRoll Over",
                          "POSTFail",
                          "Error Undefined",
                          "a",
                          "b",
                          "c",
                          "d",
                          "e",
                          "f",
                          "g",
                          "h",
                          "i",
                          "j",
                          "k",
                          "l",
                          "m",
                          "n",
                          "o",
                          "p",
                          "q",
                          "r",
                          "s",
                          "t",
                          "u",
                          "v",
                          "w",
                          "x",
                          "y",
                          "z",
                          "1",
                          "2",
                          "3",
                          "4",
                          "5",
                          "6",
                          "7",
                          "8",
                          "9",
                          "0",
                          "ENTER",
                          "ESCAPE",
                          "BACKSPACE",
                          "TAB",
                          "SPACEBAR",
                          "-",
                          "=",
                          "[",
                          "]",
                          "\\0",
                          "~",
                          ";",
                          "'",
                          "GRAVE ACCENT",
                          ",",
                          ".",
                          "/",
                          "CAPS LOCK",
                          "F1",
                          "F2",
                          "F3",
                          "F4",
                          "F5",
                          "F6",
                          "F7",
                          "F8",
                          "F9",
                          "F10",
                          "F11",
                          "F12"
                      };

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_USB_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_USB_DATA app_usbData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
/*******************************************************
 * USB HOST Layer Events - Host Event Handler
 *******************************************************/

USB_HOST_EVENT_RESPONSE APP_USBHostEventHandler (USB_HOST_EVENT event, void * eventData, uintptr_t context)
{
    switch(event)
    {
        case USB_HOST_EVENT_DEVICE_UNSUPPORTED:
            break;
        default:
            break;
    }
    return USB_HOST_EVENT_RESPONSE_NONE;
}

/*******************************************************
 * USB HOST HID Layer Events - Application Event Handler
 *******************************************************/

void APP_USBHostHIDKeyboardEventHandler(USB_HOST_HID_KEYBOARD_HANDLE handle, 
        USB_HOST_HID_KEYBOARD_EVENT event, void * pData)
{   
    UART2_Write("\r\nest11", 7);
    switch ( event)
    {
        case USB_HOST_HID_KEYBOARD_EVENT_ATTACH:
            app_usbData.handle = handle;
            app_usbData.state =  APP_USB_STATE_DEVICE_ATTACHED;
            //app_usbData.nBytesWritten = 0;
            //app_usbData.stringReady = false;
            memset(&app_usbData.string, 0, sizeof(app_usbData.string));
            memset(&app_usbData.lastData, 0, sizeof(app_usbData.lastData));
            app_usbData.stringSize = 0;
            app_usbData.capsLockPressed = false;
            app_usbData.scrollLockPressed = false;
            app_usbData.numLockPressed = false;
            app_usbData.outputReport = 0;
//			LED1_On();
            break;

        case USB_HOST_HID_KEYBOARD_EVENT_DETACH:
            app_usbData.handle = handle;
            app_usbData.state = APP_USB_STATE_DEVICE_DETACHED;
            //app_usbData.nBytesWritten = 0;
            //app_usbData.stringReady = false;
            //app_usbData.usartTaskState = APP_USART_STATE_CHECK_FOR_STRING_TO_SEND;
            memset(&app_usbData.string, 0, sizeof(app_usbData.string));
            memset(&app_usbData.lastData, 0, sizeof(app_usbData.lastData));
            app_usbData.stringSize = 0;
            app_usbData.capsLockPressed = false;
            app_usbData.scrollLockPressed = false;
            app_usbData.numLockPressed = false;
            app_usbData.outputReport = 0;
//			LED1_Off();
            break;

        case USB_HOST_HID_KEYBOARD_EVENT_REPORT_RECEIVED:
            app_usbData.handle = handle;
            app_usbData.state = APP_USB_STATE_READ_HID;
            /* Keyboard Data from device */
            memcpy(&app_usbData.data, pData, sizeof(app_usbData.data));
            break;

        default:
            break;
    }
    return;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
void APP_MapKeyToUsage(USB_HID_KEYBOARD_KEYPAD keyCode)
{
    uint8_t outputReport = 0;
    
    outputReport = app_usbData.outputReport;
    
    if((keyCode >= USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A &&
            keyCode <= USB_HID_KEYBOARD_KEYPAD_KEYBOARD_0_AND_CLOSE_PARENTHESIS) || 
            (keyCode == USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CAPS_LOCK) || 
            (keyCode == USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SCROLL_LOCK) || 
            (keyCode == USB_HID_KEYBOARD_KEYPAD_KEYPAD_NUM_LOCK_AND_CLEAR))
    {
        if(keyCode >= USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A &&
            keyCode <= USB_HID_KEYBOARD_KEYPAD_KEYBOARD_0_AND_CLOSE_PARENTHESIS)
        {
            memcpy(&app_usbData.string[app_usbData.currentOffset], keyValue[keyCode],
                            strlen(keyValue[keyCode]));
        }
        else
        {
            if(keyCode == USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CAPS_LOCK)
            {
                /* CAPS LOCK pressed */
                if(app_usbData.capsLockPressed == false)
                {
                    app_usbData.capsLockPressed = true;
                    outputReport = outputReport | 0x2;
                }
                else
                {
                    app_usbData.capsLockPressed = false;
                    outputReport = outputReport & 0xFD;
                }
            }
            if(keyCode == USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SCROLL_LOCK)
            {
                /* SCROLL LOCK pressed */
                if(app_usbData.scrollLockPressed == false)
                {
                    app_usbData.scrollLockPressed = true;
                    outputReport = outputReport | 0x4;
                }
                else
                {
                    app_usbData.scrollLockPressed = false;
                    outputReport = outputReport & 0xFB;
                }
            }
            if(keyCode == USB_HID_KEYBOARD_KEYPAD_KEYPAD_NUM_LOCK_AND_CLEAR)
            {
                /* NUM LOCK pressed */
                if(app_usbData.numLockPressed == false)
                {
                    app_usbData.numLockPressed = true;
                    outputReport = outputReport | 0x1;
                }
                else
                {
                    app_usbData.numLockPressed = false;
                    outputReport = outputReport & 0xFE;
                }
            }
            
            /* Store the changes */
            app_usbData.outputReport = outputReport;
            /* Send the OUTPUT Report */
            USB_HOST_HID_KEYBOARD_ReportSend(app_usbData.handle, outputReport);
        }
        if(app_usbData.capsLockPressed && (app_usbData.data.modifierKeysData.leftShift ||
                app_usbData.data.modifierKeysData.rightShift))
        {
            /* Small case should be displayed */
        }
        /* Check if it is within a - z */
        else if((app_usbData.capsLockPressed || app_usbData.data.modifierKeysData.leftShift ||
                app_usbData.data.modifierKeysData.rightShift) &&
                (keyCode >= USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A &&
                keyCode <= USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z))
        {
            app_usbData.string[app_usbData.currentOffset] = app_usbData.string[app_usbData.currentOffset] - 32;
        }        
        //app_usbData.currentOffset = app_usbData.currentOffset + sizeof(keyValue[keyCode]);
        
        //GPT
        app_usbData.currentOffset += strlen(keyValue[keyCode]);
        if (app_usbData.currentOffset >= sizeof(app_usbData.string) - 1)
            app_usbData.currentOffset = sizeof(app_usbData.string) - 1;
        app_usbData.string[app_usbData.currentOffset] = '\0';
    }
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_USB_Initialize ( void )

  Remarks:
    See prototype in app_usb.h.
 */

void APP_USB_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    memset(&app_usbData, 0, sizeof(app_usbData));
    app_usbData.state = APP_USB_STATE_INIT;

    //VBUSON_Set();
    //drvUSBFSInit0
    //pUSBDrvObj->rootHubInfo.portPowerEnable(0 /* Port 0 */, false); 

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_USB_Tasks ( void )

  Remarks:
    See prototype in app_usb.h.
 */

void APP_USB_Tasks ( void )
{
    //uint64_t sysCount = 0;
    //uint8_t count = 0;
    //uint8_t counter = 0;
    //bool foundInLast = false;
    
    //app_usbData.currentOffset = 0;
    
    /* Check the application's current state. */
    switch ( app_usbData.state )
    {
        /* Application's initial state. */
        case APP_USB_STATE_INIT:
            USB_HOST_EventHandlerSet(APP_USBHostEventHandler, 0);
            USB_HOST_HID_KEYBOARD_EventHandlerSet(APP_USBHostHIDKeyboardEventHandler);
            
			USB_HOST_BusEnable(USB_HOST_BUS_ALL);
			app_usbData.state = APP_USB_STATE_WAIT_FOR_HOST_ENABLE;
            break;
			
		case APP_USB_STATE_WAIT_FOR_HOST_ENABLE:
            /* Check if the host operation has been enabled */
            if(USB_HOST_BusIsEnabled(USB_HOST_BUS_ALL))
            {
                /* This means host operation is enabled. We can
                 * move on to the next state */
                app_usbData.state = APP_USB_STATE_HOST_ENABLE_DONE;
            }
            break;
        case APP_USB_STATE_HOST_ENABLE_DONE:
            app_usbData.state = APP_USB_STATE_WAIT_FOR_DEVICE_ATTACH;
            break;

        case APP_USB_STATE_WAIT_FOR_DEVICE_ATTACH:
            /* Wait for device attach. The state machine will move
             * to the next state when the attach event
             * is received.  */

            break;

        case APP_USB_STATE_DEVICE_ATTACHED:
            memset(&app_usbData.string, 0, sizeof(app_usbData.string));
            app_usbData.currentOffset = 0;
            app_usbData.stringSize    = 0;
            app_usbData.state = APP_USB_STATE_READ_HID;
            break;
            
        case APP_USB_STATE_READ_HID:
        {
           // Processa apenas as teclas não-modificadoras recém-pressionadas
            for (size_t i = 0; i < app_usbData.data.nNonModifierKeysData; i++)
            {
                if (app_usbData.data.nonModifierKeysData[i].event == USB_HID_KEY_PRESSED)
                {
                    APP_MapKeyToUsage(app_usbData.data.nonModifierKeysData[i].keyCode);
                }
            }
            break;
        }

/*
        case APP_USB_STATE_READ_HID:
        {
            uint32_t nowTicks = SYS_TIME_CounterGet();
            uint32_t freq     = SYS_TIME_FrequencyGet();

            // Percorre apenas as teclas não-modificadoras válidas
            for (size_t i = 0; i < app_usbData.data.nNonModifierKeysData; i++)
            {
                if (app_usbData.data.nonModifierKeysData[i].event == USB_HID_KEY_PRESSED)
                {
                    uint8_t key = app_usbData.data.nonModifierKeysData[i].keyCode;
                    bool accept = false;

                    if (key != s_lastKey)
                    {
                        // Tecla diferente da última: aceita sempre
                        accept = true;
                    }
                    else
                    {
                        // Mesma tecla: respeita tempo mínimo entre "toques"
                        uint32_t dt_ms = (uint32_t)(
                            1000u * (nowTicks - s_lastKeyTime) / freq
                        );

                        if (dt_ms >= KBD_DEBOUNCE_MS)
                        {
                            accept = true;
                        }
                    }

                    if (accept)
                    {
                        s_lastKey     = key;
                        s_lastKeyTime = nowTicks;

                        if (app_usbData.currentOffset >= sizeof(app_usbData.string) - 1)
                        {
                            app_usbData.currentOffset = 0;
                            memset(app_usbData.string, 0, sizeof(app_usbData.string));
                        }

                        APP_MapKeyToUsage(key);

                        app_usbData.stringSize  = app_usbData.currentOffset;
                        app_usbData.stringReady = true;
                    }
                }
            }

            // Atualiza o display
            menu_displayData.state = MENU_DISPLAY_STATE_TECLADO;
            ACTION_SendEventFromTask(ACT_NONE, ACT_EVENT_DISPLAY_UPDATE);
        }
        break;
*/  
/*
        case APP_USB_STATE_READ_HID:

 //           // Processa apenas as teclas não-modificadoras recém-pressionadas
 //           for (size_t i = 0; i < app_usbData.data.nNonModifierKeysData; i++)
 //           {
 //               if (app_usbData.data.nonModifierKeysData[i].event == USB_HID_KEY_PRESSED)
 //               {
 //                   APP_MapKeyToUsage(app_usbData.data.nonModifierKeysData[i].keyCode);
 //               }
 //           }

            
            //if(appData.usartTaskState == APP_USART_STATE_CHECK_FOR_STRING_TO_SEND)
            {
                // We need to display only the non modifier keys
                for(count = 0; count < 6; count++)
                {
                    if(app_usbData.data.nonModifierKeysData[count].event == USB_HID_KEY_PRESSED)
                    {
                        app_usbData.stringReady = false;
                        // We can send Data to USART but need to check
                        app_usbData.stringSize = 64;
                        memset(&app_usbData.string, 0, sizeof(app_usbData.string));
                        for(counter = 0; counter < 6; counter++)
                        {
                            if((app_usbData.lastData.data.nonModifierKeysData[counter].event == USB_HID_KEY_PRESSED)
                                &&((app_usbData.lastData.data.nonModifierKeysData[counter].keyCode == 
                                    app_usbData.data.nonModifierKeysData[count].keyCode)))
                            {
                                sysCount = SYS_TIME_CounterGet ();
                                if(200 <= 1000 * 
                                        (sysCount - app_usbData.lastData.data.nonModifierKeysData[counter].sysCount)
                                        / SYS_TIME_FrequencyGet())
                                {
                                    foundInLast = false;
                                }
                                else
                                {
                                    foundInLast = true;
                                }
                                break;
                            }
                        }
                        if(foundInLast == false)
                        {
                            app_usbData.stringReady = true;
                            APP_MapKeyToUsage(app_usbData.data.nonModifierKeysData[count].keyCode);
                        }
                        else
                        {
                            // Reset it it false for next iteration
                            foundInLast = false;
                        }
                    }
                }
                //Store the present to future
                memcpy(&app_usbData.lastData.data, &app_usbData.data, sizeof(app_usbData.data));
            }
	        // Garante que o estado do display fica em MENU_DISPLAY_STATE_TECLADO
            menu_displayData.state = MENU_DISPLAY_STATE_TECLADO;
            
            // Como leu um novo valor de corrente, empilha a ação para atualizar o display
            ACTION_SendEventFromTask(ACT_NONE, ACT_EVENT_DISPLAY_UPDATE);
            break;
*/
        case APP_USB_STATE_DEVICE_DETACHED:
            UART2_Write("\r\nest7", 6);
            app_usbData.state = APP_USB_STATE_HOST_ENABLE_DONE;
            break;

        case APP_USB_STATE_ERROR:
            UART2_Write("\r\nest8", 6);

            /* The application comes here when the demo
             * has failed. Provide LED indication .*/

            
            break;

        default:
            break;
    }
}

/*******************************************************************************
 End of File
 */

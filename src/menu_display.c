/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    menu_display.c

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
#include <stdio.h>  // no topo do arquivo, se ainda não tiver
#include "menu_display.h"
#include "app_display.h"     // atualiza_lcd()
#include "definitions.h"
#include "medida_gb.h"
#include "app_usb.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
extern APP_USB_DATA app_usbData;
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the MENU_DISPLAY_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

MENU_DISPLAY_DATA menu_displayData;


// Fila de eventos para o menu
QueueHandle_t xActionEventQueue = NULL;

#define BTN_DEBOUNCE_MS     15
#define BTN_LONGPRESS_MS    500   // tempo segurando para começar auto-repeat
#define BTN_REPEAT_MS       200   // após isso, repete a cada 100 ms

// Máscara de estado estável (debounced): bit 0=BACK, 1=ENTER, 2=MAIS, 3=MENOS
static volatile uint8_t g_stableMask = 0;

// Contador de debounce em ms (quando >0, estamos esperando estabilizar)
static volatile uint16_t g_debounceCounter = 0;

// Contadores de tempo "segurando" cada botão (para long-press/repeat)
static volatile uint16_t g_holdMs[BTN_COUNT];
static volatile bool     g_repeatActive[BTN_COUNT];


/* static inline uint8_t BUTTONS_ReadRawMask(void)
 * Função rápida para ler o estado atual dos pinos em forma de máscara
 */
static inline uint8_t BUTTONS_ReadRawMask(void)
{
    uint8_t mask = 0;

    // assumindo 0 = pressionado
    if (PINO_BTN_BACK_Get()  == 0) mask |= (1 << BTN_BACK);
    if (PINO_BTN_ENTER_Get() == 0) mask |= (1 << BTN_ENTER);
    if (PINO_BTN_CIMA_Get()  == 0) mask |= (1 << BTN_CIMA);
    if (PINO_BTN_BAIXO_Get() == 0) mask |= (1 << BTN_BAIXO);

    return mask;
}

/* static inline void ACTION_SendEventFromISR(ACTION_ID id, ACTION_EVENT_TYPE type,
                                           BaseType_t *pxHigherPriorityTaskWoken)
 * Envia evento de ação para fila de ação por ISR (interrupção)
 */
static inline void ACTION_SendEventFromISR(ACTION_ID id, ACTION_EVENT_TYPE type,
                                           BaseType_t *pxHigherPriorityTaskWoken)
{
    ACTION_EVENT ev;
    ev.id   = id;
    ev.type = type;

    xQueueSendFromISR(xActionEventQueue, &ev, pxHigherPriorityTaskWoken);
}

/* void ACTION_SendEventFromTask(ACTION_ID id, ACTION_EVENT_TYPE type)
 * Envia evento de ação para fila de ação via task
 */
void ACTION_SendEventFromTask(ACTION_ID id, ACTION_EVENT_TYPE type)
{
    ACTION_EVENT ev;
    ev.id   = id;
    ev.type = type;

    xQueueSend(xActionEventQueue, &ev, portMAX_DELAY);
}

/* void switch_handler(GPIO_PIN pin, uintptr_t context)
 * Função única que configuro no Callback dos botões.
 * Todos os botões chamam a mesma função pois identifico qual botão foi apertado
 * no debounce do timer 3.
 */
void switch_handler(GPIO_PIN pin, uintptr_t context)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Inicia (ou mantém) o contador de debounce
    g_debounceCounter = BTN_DEBOUNCE_MS;

    // Zera contadores de hold para todos (vai começar de novo)
    for (int i = 0; i < BTN_COUNT; i++)
    {
        g_holdMs[i] = 0;
        g_repeatActive[i] = false;
    }

    // Garante que o Timer de 1ms está rodando
    TMR3_Start();            // ou TMRx_Start conforme Harmony
    TMR3_InterruptEnable();  // se não estiver habilitado

    // Se você usar alguma API FreeRTOS aqui, finalize:
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}


/* void setup_switches()
 * Configura os Callback (interrupções) das GPIOs dos botões.
 */
void setup_switches()
{
    GPIO_PinInterruptCallbackRegister(PINO_BTN_BACK_PIN, switch_handler, 0);
    GPIO_PinInterruptCallbackRegister(PINO_BTN_ENTER_PIN, switch_handler, 0);
    GPIO_PinInterruptCallbackRegister(PINO_BTN_CIMA_PIN, switch_handler, 0);
    GPIO_PinInterruptCallbackRegister(PINO_BTN_BAIXO_PIN, switch_handler, 0);
    PINO_BTN_BACK_InterruptEnable();
    PINO_BTN_ENTER_InterruptEnable();
    PINO_BTN_CIMA_InterruptEnable();
    PINO_BTN_BAIXO_InterruptEnable();
}

/* TMR3_Callback()
 * Trata o Callback (interrupção) do Timer3.
 * Uso o timer 3 para lidar com o debounce dos botões + apertar e segurar um botão
 */
void TMR3_Callback(uint32_t status, uintptr_t context)  // ou TMR3_InterruptHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // 1) Tratamento de debounce
    if (g_debounceCounter > 0)
    {
        g_debounceCounter--;

        if (g_debounceCounter == 0)
        {
            // Momento de "congelar" o estado e comparar com o estável
            uint8_t newMask = BUTTONS_ReadRawMask();
            uint8_t diff    = newMask ^ g_stableMask; // bits que mudaram

            for (int i = 0; i < BTN_COUNT; i++)
            {
                uint8_t bit = (1 << i);

                if (diff & bit)
                {
                    if (newMask & bit)
                    {
                        // botão i agora está pressionado (debounced)
                        g_stableMask |= bit;
                        g_holdMs[i] = 0;
                        g_repeatActive[i] = false;

                        ACTION_SendEventFromISR((ACTION_ID)i, BTN_EVENT_PRESS,
                                                &xHigherPriorityTaskWoken);
                    }
                    else
                    {
                        // botão i agora está solto
                        g_stableMask &= ~bit;
                        g_holdMs[i] = 0;
                        g_repeatActive[i] = false;

                        ACTION_SendEventFromISR((ACTION_ID)i, BTN_EVENT_RELEASE,
                                                &xHigherPriorityTaskWoken);
                    }
                }
            }
        }
    }
    else
    {
        // 2) Debounce já terminou. Se existir botão pressionado, trata hold/repeat.

        if (g_stableMask != 0)
        {
            for (int i = 0; i < BTN_COUNT; i++)
            {
                uint8_t bit = (1 << i);

                if (g_stableMask & bit)
                {
                    g_holdMs[i]++;

                    if (!g_repeatActive[i])
                    {
                        if (g_holdMs[i] >= BTN_LONGPRESS_MS)
                        {
                            g_repeatActive[i] = true;
                            g_holdMs[i] = 0;

                            // opcional: já manda o primeiro repeat aqui
                            ACTION_SendEventFromISR((ACTION_ID)i, BTN_EVENT_REPEAT,
                                                    &xHigherPriorityTaskWoken);
                        }
                    }
                    else
                    {
                        if (g_holdMs[i] >= BTN_REPEAT_MS)
                        {
                            g_holdMs[i] = 0;
                            ACTION_SendEventFromISR((ACTION_ID)i, BTN_EVENT_REPEAT,
                                                    &xHigherPriorityTaskWoken);
                        }
                    }
                }
                else
                {
                    g_holdMs[i] = 0;
                    g_repeatActive[i] = false;
                }
            }
        }
        else
        {
            // NENHUM botão está pressionado e não tem debounce rolando:
            // podemos parar o timer para não gastar CPU à toa.
            TMR3_InterruptDisable();
            TMR3_Stop();
        }
    }
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}



/* void MENU_DISPLAY_Initialize ( void )
 * Inicia as variáveis e drives necessários para controle do display lcd
 */
void MENU_DISPLAY_Initialize ( void )
{
    menu_displayData.state         = MENU_DISPLAY_STATE_INIT;
    menu_displayData.currentScreen = MENU_SCREEN_HOME;
    menu_displayData.currentItem   = 0;
    
    menu_displayData.debug1 = 0;
    menu_displayData.debug2 = 0;

    // cria fila de eventos
    xActionEventQueue = xQueueCreate(16, sizeof(ACTION_EVENT));
    configASSERT(xActionEventQueue != NULL);

    g_stableMask = 0;
    g_debounceCounter = 0;
    memset((void*)g_holdMs, 0, sizeof(g_holdMs));
    memset((void*)g_repeatActive, 0, sizeof(g_repeatActive));
    
    // *** REGISTRA O CALLBACK DO TIMER 3 ***
    TMR3_CallbackRegister(TMR3_Callback, 0);

    // Configura botões (CN) normalmente
    setup_switches();

    // Configura TMR3 no Harmony para período de 1ms (mas NÃO precisa deixar startado)
    TMR3_Stop();
    TMR3_InterruptDisable();
}

// *** A partir daqui são as funções que tratam os eventos de ação (xActionEventQueue) *** //

/* void MENU_DISPLAY_STATE_INIT_DetectEvent(const ACTION_EVENT *ev)
 * Função que trata ações enquando equipamento no menu iniciar.
 */
void MENU_DISPLAY_STATE_INIT_DetectEvent(const ACTION_EVENT *ev)
{
    switch (ev->id)
    {
        case BTN_CIMA:
        {
            if (ev->type == BTN_EVENT_PRESS || ev->type == BTN_EVENT_REPEAT)
            {
                if (menu_displayData.currentItem > 1)
                    menu_displayData.currentItem--;
                else
                    menu_displayData.currentItem = 4;
            }
            break;
        }
        case BTN_BAIXO:
        {
            if (ev->type == BTN_EVENT_PRESS || ev->type == BTN_EVENT_REPEAT)
            {
                if (menu_displayData.currentItem < 4)
                    menu_displayData.currentItem++;
                else
                    menu_displayData.currentItem = 1;
            }
            break;
        }
        case BTN_ENTER:
        {
            if (ev->type == BTN_EVENT_PRESS)
            {
                if (menu_displayData.currentItem == 1) menu_displayData.state = MENU_DISPLAY_STATE_TECLADO;
                else if (menu_displayData.currentItem == 2) menu_displayData.state = MENU_DISPLAY_STATE_HP;
                else if (menu_displayData.currentItem == 3) menu_displayData.state = MENU_DISPLAY_STATE_GB;
                else if (menu_displayData.currentItem == 4) menu_displayData.state = MENU_DISPLAY_STATE_TF;
            }
            break;
        }
        case BTN_BACK:
        {
            // Faz nada
            if (ev->type == BTN_EVENT_PRESS)
            {
                
            }
            break;
        }
        default:
        {
            break;
        }  
    }
}

/* void MENU_DISPLAY_STATE_TECLADO_DetectEvent(const ACTION_EVENT *ev)
 * Função que trata ações enquando equipamento no menu ensaio HP.
 */
void MENU_DISPLAY_STATE_TECLADO_DetectEvent(const ACTION_EVENT *ev)
{
    switch (ev->id)
    {
        case BTN_BACK:
        {
            // Volta ao menu inicial
            if (ev->type == BTN_EVENT_PRESS)
            {
                menu_displayData.state = MENU_DISPLAY_STATE_INIT;
                menu_displayData.currentItem = 0;
            }
            break;
        }
        case BTN_ENTER:
        {
            if (ev->type == BTN_EVENT_PRESS)
            {
                menu_displayData.debug1 += 1;
            }
            break;
        }
        default:
        {
            break;
        }  
    }
}

/* void MENU_DISPLAY_STATE_HP_DetectEvent(const ACTION_EVENT *ev)
 * Função que trata ações enquando equipamento no menu ensaio HP.
 */
void MENU_DISPLAY_STATE_HP_DetectEvent(const ACTION_EVENT *ev)
{
    switch (ev->id)
    {
        case BTN_BACK:
        {
            // Volta ao menu inicial
            if (ev->type == BTN_EVENT_PRESS)
            {
                menu_displayData.state = MENU_DISPLAY_STATE_INIT;
                menu_displayData.currentItem = 0;
            }
            break;
        }
        default:
        {
            break;
        }  
    }
}

/* void MENU_DISPLAY_STATE_GB_DetectEvent(const ACTION_EVENT *ev)
 * Função que trata ações enquando equipamento no menu ensaio GB.
 */
void MENU_DISPLAY_STATE_GB_DetectEvent(const ACTION_EVENT *ev)
{
    switch (ev->id)
    {
        case BTN_ENTER:
        {
            if (ev->type == BTN_EVENT_PRESS)
            {
                menu_displayData.state = ENSAIO_GB_STATE_ENSAIANDO;

                // Garante que não cria duas tasks ao mesmo tempo
                if (xMEDIDA_GB_Tasks == NULL)
                {
                    // Cria uma task one-shot que existirá somente durante a
                    // execução do ensaio GB.
                    xTaskCreate(
                        MEDIDA_GB_RunTestTask,   // nossa task one-shot
                        "MEDIDA_GB",
                        1024,
                        NULL,
                        7U,
                        &xMEDIDA_GB_Tasks);
                }
            }
            break;
        }
        case BTN_BACK:
        {
            // Volta ao menu inicial
            if (ev->type == BTN_EVENT_PRESS)
            {
                menu_displayData.state = MENU_DISPLAY_STATE_INIT;
                menu_displayData.currentItem = 0;
            }
            break;
        }
        default:
        {
            break;
        }  
    }
}

/* void MENU_DISPLAY_STATE_TF_DetectEvent(const ACTION_EVENT *ev)
 * Função que trata ações enquando equipamento no menu ensaio TF.
 */
void MENU_DISPLAY_STATE_TF_DetectEvent(const ACTION_EVENT *ev)
{
    switch (ev->id)
    {
        case BTN_BACK:
        {
            // Volta ao menu inicial
            if (ev->type == BTN_EVENT_PRESS)
            {
                menu_displayData.state = MENU_DISPLAY_STATE_INIT;
                menu_displayData.currentItem = 0;
            }
            break;
        }
        default:
        {
            break;
        }  
    }
}

/* static void MENU_DISPLAY_HandleActionEvent(const ACTION_EVENT *ev)
 * Função usada em 'void MENU_DISPLAY_Tasks ( void ).
 * Ela é chamada assim que uma ação é inserida na pilha xActionEventQueue
 */
static void MENU_DISPLAY_HandleActionEvent(const ACTION_EVENT *ev)
{
    switch (menu_displayData.state)
    {
        case MENU_DISPLAY_STATE_INIT:
        {
            MENU_DISPLAY_STATE_INIT_DetectEvent(ev);
            break;
        }
        case MENU_DISPLAY_STATE_TECLADO:
        {
            MENU_DISPLAY_STATE_TECLADO_DetectEvent(ev);
            break;
        }
        case MENU_DISPLAY_STATE_HP:
        {
            MENU_DISPLAY_STATE_HP_DetectEvent(ev);
            break;
        }
        case MENU_DISPLAY_STATE_GB:
        {
            MENU_DISPLAY_STATE_GB_DetectEvent(ev);
            break;
        }
        case MENU_DISPLAY_STATE_TF:
        {
            MENU_DISPLAY_STATE_TF_DetectEvent(ev);
            break;
        }
        case ENSAIO_GB_STATE_ENSAIANDO:
        {
            break;
        }
        default:
        {
            // Em teoria nunca cai aqui, mas o ideal é pecar pelo excesso
            // Garantir que não ha ensaio sendo executado.
            // Forçar o menu para o inicio
            menu_displayData.state = MENU_DISPLAY_STATE_INIT;
            menu_displayData.currentItem = 0;
            break;
        }  
    }
}

/* void MENU_DISPLAY_Tasks ( void )
 * Função de atualização das ações e do display.
 * Cada tela do display é um estado. Para cada estado crio uma função que
 * apenas escreve em 'menu_displayData.lcd' o que aparecerá no display.
 * Após tem de chamar a função 'atualiza_lcd((char*)menu_displayData.lcd)'
 * que vai empilhar o escrito em uma lista de eventos para ser consumido
 * pela task de atualização do display.
 */
void MENU_DISPLAY_Tasks ( void )
{
    switch (menu_displayData.state)
    {
        case MENU_DISPLAY_STATE_INIT:
        {
            MENU_DISPLAY_DrawHome();
            atualiza_lcd((char*)menu_displayData.lcd);
            break;
        }
        case MENU_DISPLAY_STATE_TECLADO:
        {
            MENU_DISPLAY_DrawTeclado();
            atualiza_lcd((char*)menu_displayData.lcd);
            break;
        }
        case MENU_DISPLAY_STATE_HP:
        {
            MENU_DISPLAY_DrawHP();
            atualiza_lcd((char*)menu_displayData.lcd);
            break;
        }
        case MENU_DISPLAY_STATE_GB:
        {
            MENU_DISPLAY_DrawGB();
            atualiza_lcd((char*)menu_displayData.lcd);
            break;
        }
        case MENU_DISPLAY_STATE_TF:
        {
            MENU_DISPLAY_DrawTF();
            atualiza_lcd((char*)menu_displayData.lcd);
            break;
        }
        case ENSAIO_GB_STATE_ENSAIANDO:
        {
            ENSAIO_GB_DrawEnsaiando();
            atualiza_lcd((char*)menu_displayData.lcd);
            break;
        }
        default:
            menu_displayData.state = MENU_DISPLAY_STATE_INIT;
            break;
    }
    ACTION_EVENT ev;
    // Bloqueia esperando evento de ação
    // O evento de ação pode entrar por 2 caminhos:
    // - Função ACTION_SendEventFromISR, por evento de interrupção
    // - Função ACTION_SendEventFromTask, por evento em uma task
    if (xQueueReceive(xActionEventQueue, &ev, portMAX_DELAY) == pdPASS)
        MENU_DISPLAY_HandleActionEvent(&ev);
}
// ************** Daqui para baixo são as funções que atualizam o texto conforme o menu para o display ************** //

void MENU_DISPLAY_DrawHome(void)
{
    // Limpa o buffer
    memset(menu_displayData.lcd, ' ', sizeof(menu_displayData.lcd));
    
    if (menu_displayData.currentItem < 4)
    {
        snprintf(menu_displayData.lcd[0], 20, "   HGF148 - Teste");
        snprintf(menu_displayData.lcd[1], 20, "Teste teclado");
        snprintf(menu_displayData.lcd[2], 20, "Ensaio HP");
        snprintf(menu_displayData.lcd[3], 20, "Ensaio GB");
        menu_displayData.lcd[menu_displayData.currentItem][19] = '<';
    }
    else
    {
        snprintf(menu_displayData.lcd[0], 20, "Ensaio TF");
        menu_displayData.lcd[menu_displayData.currentItem - 4][19] = '<';
    }
}

void MENU_DISPLAY_DrawTeclado(void)
{
    // Limpa o buffer
    memset(menu_displayData.lcd, ' ', sizeof(menu_displayData.lcd));
    memcpy(menu_displayData.lcd[0], "     Teclado", 14);
    snprintf(menu_displayData.lcd[1], 20, "%s", app_usbData.string);
    snprintf(menu_displayData.lcd[2], 20, "%d %d", menu_displayData.debug1, menu_displayData.debug2);
    memcpy(menu_displayData.lcd[3], "<BACK>       <ENTER>", 20);
}

void MENU_DISPLAY_DrawHP(void)
{
    // Limpa o buffer
    memset(menu_displayData.lcd, ' ', sizeof(menu_displayData.lcd));
    memcpy(menu_displayData.lcd[0], "     Ensaio HP", 14);
    memcpy(menu_displayData.lcd[3], "<BACK>       <ENTER>", 20);
}

void MENU_DISPLAY_DrawGB(void)
{
    // Limpa o buffer
    memset(menu_displayData.lcd, ' ', sizeof(menu_displayData.lcd));
    memcpy(menu_displayData.lcd[0], "     Ensaio GB", 14);
    memcpy(menu_displayData.lcd[3], "<BACK>       <ENTER>", 20);
}

void MENU_DISPLAY_DrawTF(void)
{
    // Limpa o buffer
    memset(menu_displayData.lcd, ' ', sizeof(menu_displayData.lcd));
    memcpy(menu_displayData.lcd[0], "     Ensaio TF", 14);
    memcpy(menu_displayData.lcd[3], "<BACK>       <ENTER>", 20);
}

void ENSAIO_GB_DrawEnsaiando(void)
{
    memset(menu_displayData.lcd, ' ', sizeof(menu_displayData.lcd));

    //memcpy(menu_displayData.lcd[0], "  Ensaio GB 5 s  ", 18);

    // Linha 1: corrente "processada" (placeholder em volts, trocar para A depois)
    snprintf(menu_displayData.lcd[0], 20, "R = %3u", medida_gbData.resistencia);

    // Linha 2: valores brutos dos ADCs em RA1 e RB0
    snprintf(menu_displayData.lcd[1], 20, "A=%3u V=%3u", medida_gbData.corrente, medida_gbData.tensao);

    // Linha 3: status
    snprintf(menu_displayData.lcd[2], 20, "%d %d %d", medida_gbData.fl1, medida_gbData.fl2, medida_gbData.fl3);
}

/*******************************************************************************
 End of File
 */

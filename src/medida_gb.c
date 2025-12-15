/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    medida_gb.c

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

    Mapa ADCHS:
    CH1 ? ANx (RB0) ? ADCDATA1 ? tensão
    CH2 ? ANy (RA1) ? ADCDATA2 ? corrente
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "medida_gb.h"
#include "FreeRTOS.h"
#include "task.h"
#include "menu_display.h"   // para poder mudar estado do menu
#include "utils.h"

MEDIDA_GB_DATA medida_gbData;

static volatile uint8_t  g_powerPercent   = 0;       // 0..100 %
static volatile uint32_t g_delayTicks     = 0;       // atraso até disparo (em ticks TMR6)

// Macros para os canais do ADCHS
#define MEDIDA_GB_CH_RB0   ADCHS_CH1
#define MEDIDA_GB_CH_RA1   ADCHS_CH2

// Define a quantidade de cilos pora leitura
#define QUANT_CICLOS    127

// Protótipos dos callbacks
static void MEDIDA_GB_TMR2Callback(uint32_t status, uintptr_t context);

// Estado do Timer 6
typedef enum
{
    TMR6_STATE_IDLE = 0,
    TMR6_STATE_WAIT_DELAY,   // esperando momento de disparar
    TMR6_STATE_GATE_ON       // gate está em nível alto
} TMR6_STATE_t;

static volatile TMR6_STATE_t g_tmr6State = TMR6_STATE_IDLE;

// Converte porcentagem de potência em atraso (em ticks)
static uint32_t TRIAC_ComputeDelayTicks(uint8_t powerPercent)
{
    if (powerPercent > TRIAC_POWER_MAX)
        powerPercent = TRIAC_POWER_MAX;
    if (powerPercent < TRIAC_POWER_MIN)
        powerPercent = TRIAC_POWER_MIN;

    // 100% => delay = 0 ticks (disparo no início do semiciclo)
    // 0%   => delay = HALF_CYCLE_TICKS (sem condução)
    uint32_t delay = (uint32_t)(100U - powerPercent) * HALF_CYCLE_TICKS / 100U;

    // Evita delay tão grande que não sobra tempo pra gate ligado
    if (delay > (HALF_CYCLE_TICKS - (MIN_GATING_TICKS + SAFETY_TICKS_TO_ZERO)))
    {
        delay = HALF_CYCLE_TICKS - (MIN_GATING_TICKS + SAFETY_TICKS_TO_ZERO);
    }

    return delay;
}

// ====== API pública ======

void TRIAC_SetPowerPercent(uint8_t percent)
{
    taskENTER_CRITICAL();
    g_powerPercent = percent;
    g_delayTicks   = TRIAC_ComputeDelayTicks(g_powerPercent);
    taskEXIT_CRITICAL();
}

void TRIAC_Control_Initialize(void)
{
    // Garante que está desligado
    PINO_TRIAC_GB_Clear();
    // ---- TMR6 ----
    // No Harmony você já configurou o TMR6 com PBCLK=60MHz, prescale=1:8.
    // Aqui só paramos, definimos período inicial e registramos callback.
    TMR6_Stop();
    TMR6_InterruptDisable();
    TMR6_CounterSet(0);
    TMR6_PeriodSet(1000U);        // valor qualquer inicial

    TMR6_CallbackRegister(TMR6_Callback, (uintptr_t)NULL);

    g_tmr6State   = TMR6_STATE_IDLE;
    g_powerPercent = 0;
    g_delayTicks   = TRIAC_ComputeDelayTicks(g_powerPercent);

    // Potência inicial baixa
    TRIAC_SetPowerPercent(0);
}

// ====== Handler chamado pela interrupção de zero-cross ======

void ZC_InterruptHandler(GPIO_PIN pin, uintptr_t context)
{
    // Foi detectado um zero-cross: novo semiciclo iniciando.

    // Se potência zero, não disparamos TRIAC
    if (g_powerPercent == 0)
    {
        PINO_TRIAC_GB_Clear();
        g_tmr6State = TMR6_STATE_IDLE;
        TMR6_Stop();
        TMR6_InterruptDisable();
        return;
    }

    uint32_t delay = g_delayTicks;

    // Programa Timer6 para esperar o atraso até o disparo
    TMR6_Stop();
    TMR6_CounterSet(0);
    TMR6_PeriodSet((uint16_t)delay);  // HALF_CYCLE_TICKS cabe em 16 bits

    g_tmr6State = TMR6_STATE_WAIT_DELAY;
    TMR6_InterruptEnable();
    TMR6_Start();
}

// ====== Callback do Timer 6 ======

void TMR6_Callback(uint32_t status, uintptr_t context)
{
    (void)status;
    (void)context;

    switch (g_tmr6State)
    {
        case TMR6_STATE_WAIT_DELAY:
        {
            // Chegou a hora de disparar TRIAC
            PINO_TRIAC_GB_Set();
            g_tmr6State = TMR6_STATE_GATE_ON;

            // Calcula por quanto tempo manter o gate alto:
            // do ponto atual até próximo do zero-cross:
            uint32_t delay  = g_delayTicks;
            uint32_t gateTicks;

            if (delay >= HALF_CYCLE_TICKS)
            {
                gateTicks = MIN_GATING_TICKS;  // não deveria acontecer
            }
            else
            {
                uint32_t maxGate = HALF_CYCLE_TICKS - delay - SAFETY_TICKS_TO_ZERO;
                if (maxGate < MIN_GATING_TICKS)
                    gateTicks = MIN_GATING_TICKS;
                else
                    gateTicks = maxGate;
            }
            TMR6_CounterSet(0);
            TMR6_PeriodSet((uint16_t)gateTicks);
            break;
        }

        case TMR6_STATE_GATE_ON:
        {
            // Tempo de gate alto acabou, desliga gate
            PINO_TRIAC_GB_Clear();
            g_tmr6State = TMR6_STATE_IDLE;

            // Timer só volta a ser usado no próximo zero-cross
            TMR6_Stop();
            TMR6_InterruptDisable();
            break;
        }

        default:
        {
            // Recuperação simples
            PINO_TRIAC_GB_Clear();
            
            // Desativa o relé 2 ao final do teste
            PINO_RELE2_TAP_Clear();
            
            g_tmr6State = TMR6_STATE_IDLE;
            TMR6_Stop();
            TMR6_InterruptDisable();
            break;
        }
    }
}

/*
	i_gb_calcula()

	A partir do ultimo valor de corrente RMS binário medido, calcula a saída de aterramento em A*10.
	O coeficiente é multiplicado por 1024 para podermos fazer uma multiplicação com numeros inteiros sem perder casas decimais, o
	resultado é então dividido por 8192 (2^13).

	Ibin = Ia10 * (1/10) * (1/1000)  *    94     *  (18/33)  * (4096/2,5)
		       A->A10     TC       Resistor      AMP	       ADC
        Ibin=Ia10*8,4
	Ia10=Ibin*(1/8,4)

	Supondo um coeficiente que permita variar 0,1% do valor
	Ia10=Ibin*(1/8,4)
	Colocando os outros fatores numa potência de 2, exceto o coeficiente (aproximando)
	Ia10=Ibin*(975/2^13)
*/
uint32_t i_gb_calcula(uint32_t i_rms)
{
	unsigned int temp;	// Número de 32 bits para guardar a multiplicação de um numero de 12 bits (v_rms)
				// com um de aproximadamente 10 bits (coeficiente)

	//temp = ((unsigned int)i_rms) * ((unsigned int)sistema_config.coef_aj_i_gb);
	temp = ((unsigned int)i_rms) * 1129;
	temp = temp >> 13;

	return(temp);
}

/*
	r_gb_calcula()

	A partir do ultimo valor de corrente RMS binário medido, calcula a saída de aterramento em A*10.
	
	Vbin = V *   (1/3,3)   *  (4096/2,5)
		       amp          ADC
	V=Vbin*(1/496)

 	Ibin = Ia10 * (1/1000)  *    94     *  (18/33)  * (4096/2,5)
		         TC       Resistor      AMP	       ADC
	I=Ibin*(1/84) (corrente em A)

	Rreal = V/I = (Vbin/496) / (Ibin/84) = (Vbin/Ibin) * (84/496)
	Esse resultado é em Ohms, mas queremos em mOhms , então multiplicamos por 1000
	Rreal = (Vbin/Ibin) * (84000/496) =  (Vbin/Ibin) * 169
	
 	Transformando o coeficiente para um inteiro dividido por uma potência de 2:
	Rreal = (Vbin/Ibin) * (1354/2^3)
*/
uint32_t r_gb_calcula(uint32_t v_rms, uint32_t i_rms)
{
	uint32_t temp;	// Número de 32 bits para guardar a multiplicação de um numero de 12 bits (v_rms)
				// com um de aproximadamente 10 bits (coeficiente)

	if (i_rms>0){
		//temp = v_rms * ((unsigned int)sistema_config.coef_aj_r_gb);
		temp = v_rms * 1335;
		temp = temp / i_rms;
		temp = temp>>3;

		return(temp);
	}else{
		return(0xFFFFFFFF);
	}
}

static void MEDIDA_GB_TMR2Callback(uint32_t status, uintptr_t context)
{
    uint32_t volatile adc_i, adc_v;
    uint32_t i_rms, v_rms;

    // Lê os dois canais configurados no scan. Cada canal é 12bits
    adc_i  = ADCDATA2;
    adc_v = ADCDATA1;
    
    medida_gbData.soma_quad_v += adc_v * adc_v;
	medida_gbData.soma_quad_i += adc_i * adc_i;
    
    //medida_gbData.fl3 = medida_gbData.soma_quad_i;
    
    if(medida_gbData.cont_ciclos >= QUANT_CICLOS)
    {
        medida_gbData.cont_ciclos = 0;
        
        v_rms = calcula_rms(medida_gbData.soma_quad_v);
		i_rms = calcula_rms(medida_gbData.soma_quad_i);
        
        medida_gbData.soma_quad_v = 0;
        medida_gbData.soma_quad_i = 0;
        
        // MEDIDA_GB
        medida_gbData.resistencia = r_gb_calcula(v_rms, i_rms);
        medida_gbData.corrente = i_gb_calcula(i_rms);
    }
    medida_gbData.cont_ciclos++;
    // Dispara conversão global para ler na próxima chamada do timer2
    ADCHS_GlobalEdgeConversionStart();
}


void MEDIDA_GB_RunTestTask(void *pvParameters)
{
    const TickType_t testDuration = pdMS_TO_TICKS(5000);   // 5 segundos
    const TickType_t samplePeriod = pdMS_TO_TICKS(100);     // lê corrente a cada 100 ms
    TickType_t startTick;

    (void) pvParameters;

    // Estado inicial previsível
    medida_gbData.correnteA = 0.0f;
    medida_gbData.corrente = 0;
    medida_gbData.tensao = 0;
    medida_gbData.resistencia = 0;
    medida_gbData.cont_ciclos = 0;
    
    // Aciona relé 2, necessários fazer isso antes de iniciar a medida
    PINO_RELE2_TAP_Set();
    
    // Configura o MUX para GBT
    PINO_MUX_A_Set();
    PINO_MUX_B_Clear();

    // Inicializa controle do TRIAC (timer, callback etc.)
    TRIAC_Control_Initialize();

    // Potência baixa fixa (ex.: 20%)
    TRIAC_SetPowerPercent(20);

    // Habilita zero-cross (começa a disparar TRIAC via TMR6)
    PINO_ZERO_CROSS_InterruptEnable();

    // Dispara conversão global de todos canais configurados no scan (ADCCSS)
    ADCHS_GlobalEdgeConversionStart();
    // Espera o sampling time de 500ns. Vamos esperar mais para garantir
	DELAY_250NS();
	DELAY_250NS();
	DELAY_250NS();
    
    // === INÍCIO: habilita TMR2 para amostrar ADC ===
    TMR2_Stop();
    TMR2_CounterSet(0);
    // PR2 já está 7811 (130,2 us), mas se quiser reforçar:
    // TMR2_PeriodSet(7811U);
    TMR2_CallbackRegister(MEDIDA_GB_TMR2Callback, 0);
    TMR2_InterruptEnable();
    TMR2_Start();
    // === FIM: habilita TMR2 ===

    // Marca início do teste
    startTick = xTaskGetTickCount();

    while ((xTaskGetTickCount() - startTick) < testDuration)
    {
        // Garante que o estado do display fica em ENSAIO_GB_STATE_ENSAIANDO
        menu_displayData.state = ENSAIO_GB_STATE_ENSAIANDO;
        // Como leu um novo valor de corrente, empilha a ação para atualizar o display
        ACTION_SendEventFromTask(ACT_NONE, ACT_EVENT_DISPLAY_UPDATE);
        // Delay com valor de 'samplePeriod'
        vTaskDelay(samplePeriod);
    }

    // Fim do teste: desabilita TRIAC e zero-cross
    TRIAC_SetPowerPercent(0);
    PINO_TRIAC_GB_Clear();
    TMR6_Stop();
    TMR6_InterruptDisable();
    PINO_ZERO_CROSS_InterruptDisable();

    // Finaliza o MUX para GBT
    PINO_MUX_A_Clear();
    PINO_MUX_B_Clear();
    
    // Desliga o relé 2 ao final do teste
    PINO_RELE2_TAP_Clear();
    
    // === DESLIGA TMR2 ===
    TMR2_Stop();
    TMR2_InterruptDisable();
    // Opcional: desregistrar callback se quiser
    TMR2_CallbackRegister(NULL, 0);
    
    // Volta o menu para a tela GB
    menu_displayData.state = MENU_DISPLAY_STATE_GB;
    // Atualiza o display novamente para o menu GB
    ACTION_SendEventFromTask(ACT_NONE, ACT_EVENT_DISPLAY_UPDATE);

    // Limpa handle e auto-destrói a task (libera memória do stack)
    xMEDIDA_GB_Tasks = NULL;
    vTaskDelete(NULL);
}


/*******************************************************************************
  Function:
    void MEDIDA_GB_Initialize ( void )

  Remarks:
    See prototype in medida_gb.h.
 */
void MEDIDA_GB_Initialize ( void )
{
    // Place the App state machine in its initial state.
    medida_gbData.state = MEDIDA_GB_STATE_INIT;
    
    medida_gbData.correnteA = 0.0f;
    medida_gbData.corrente    = 0;
    medida_gbData.tensao    = 0;
    
    medida_gbData.teste = 0;
    medida_gbData.fl1 = 0;
    medida_gbData.fl2 = 0;
    medida_gbData.fl3 = 0;
    
    medida_gbData.adcRb0Raw = 0;
    medida_gbData.adcRa1Raw = 0;
    
    GPIO_PinInterruptCallbackRegister(PINO_ZERO_CROSS_PIN, ZC_InterruptHandler, 0);
    PINO_ZERO_CROSS_InterruptDisable();
}


/******************************************************************************
  Function:
    void MEDIDA_GB_Tasks ( void )

  Remarks:
    See prototype in medida_gb.h.
 */
void MEDIDA_GB_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( medida_gbData.state )
    {
        /* Application's initial state. */
        case MEDIDA_GB_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized)
            {

                medida_gbData.state = MEDIDA_GB_STATE_SERVICE_TASKS;
            }
            break;
        }

        case MEDIDA_GB_STATE_SERVICE_TASKS:
        {

            break;
        }
        default:
        {
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */

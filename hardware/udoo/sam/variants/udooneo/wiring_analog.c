/*
 Copyright (c) 2011 Arduino.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <mqx.h>
#include <bsp.h>

#include "Arduino.h"
#include "log_mqx.h"
#include "pwm_ifc.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ! BSPCFG_ENABLE_IO_SUBSYSTEM
#error This application requires BSPCFG_ENABLE_IO_SUBSYSTEM defined non-zero in user_config.h. Please recompile BSP with this option.
#endif

/*
#ifndef BSP_DEFAULT_IO_CHANNEL_DEFINED
#error This application requires BSP_DEFAULT_IO_CHANNEL to be not NULL. Please set corresponding BSPCFG_ENABLE_TTYx to non-zero in user_config.h and recompile BSP with this option.
#endif
*/

#ifndef BSP_DEFAULT_LWADC_MODULE
#error This application requires BSP_DEFAULT_LWADC_MODULE to be not defined in the BSP. Please recompile BSP with this option.
#endif

// PWM
#define NMAX_ARD_ANALOG_OUT_PIN			11

// mapping arduino pins to pwm channels
const uint8_t ardPinToPwmChn[NMAX_ARD_ANALOG_OUT_PIN+1] = {255, 255, 255, PWM_CH0, PWM_CH1, PWM_CH2, PWM_CH3, PWM_CH4, 255, PWM_CH5, PWM_CH7, PWM_CH6};

extern const LWADC_INIT_STRUCT BSP_DEFAULT_LWADC_MODULE;
extern const LWADC_INIT_STRUCT lwadc2_init;


typedef struct adc_arduino_struct {
    const char *    name;
    uint32_t         input;
} ADC_ARDUINO_STRUCT;

static LWADC_STRUCT_PTR    lwadc_inputs=NULL;


eAnalogReference analog_reference = AR_DEFAULT;
static int _readResolution = 12;
static int _writeResolution = 8;

/* This structure defines the generic ADCs available on this board. The structure will be populated based on
** what generic ADCs are defined in the BSP. The generic ADCs currently supported are:
**  Potentiometer:                  BSP_ADC_POTENTIOMETER
**  Up to 8 generic ADC inputs:     BSP_ADC_INPUT_0..BSP_ADC_INPUT_7
**  One TWRPI ADC interface:        BSP_ADC_TWRPI_PINx
**  Tower primary elevator          BSP_ADC_TWR_AN0..BSP_ADC_TWR_AN7
**  Tower secondary elevator        BSP_ADC_TWR_AN8..BSP_ADC_TWR_AN13
**  Core voltage:                   BSP_ADC_VDD_CORE
**  Temperature:                    BSP_ADC_TEMPERATURE */

const ADC_ARDUINO_STRUCT adc_inputs[] = {

   {"arduino adc #0", ADC_INPUT(0,0) },
   {"arduino adc #1", ADC_INPUT(0,1) },
   {"arduino adc #2", ADC_INPUT(0,2) },
   {"arduino adc #3", ADC_INPUT(0,3) },
   {"arduino adc #4", ADC_INPUT(1,0) },
   {"arduino adc #5", ADC_INPUT(1,1) },
};


void InitAdcs (void)
{
    LWADC_VALUE         i;

    lwadc_inputs = (LWADC_STRUCT_PTR) _mem_alloc_zero(ELEMENTS_OF(adc_inputs)*sizeof(LWADC_STRUCT));

    if (lwadc_inputs == NULL) {
        printf("Error, Insufficient memory to run full test\n.");
        _task_block();
    }

    for (i=0;i<ELEMENTS_OF(adc_inputs);i++) {
        /* Set last value to a value out of range of the ADC. */
        if ( !_lwadc_init_input(&lwadc_inputs[i],adc_inputs[i].input) ) {
            /* Failed to initialize this input. We will end up failing the reads below as well. */
            printf("Failed to initialize ADC input %s\n",adc_inputs[i].name);
        }
    }
}

void analogReadResolution(int res) {
	_readResolution = res;
}

void analogWriteResolution(int res) {
	_writeResolution = res;
}

static inline uint32_t mapResolution(uint32_t value, uint32_t from, uint32_t to) {
	if (from == to)
		return value;
	if (from > to)
		return value >> (from-to);
	else
		return value << (to-from);
}

void analogReference(eAnalogReference ulMode)
{
	analog_reference = ulMode;
}

uint32_t analogRead( uint32_t ulPin )
{
    LWADC_VALUE         raw, normRaw;
	bool done = false;

    if (lwadc_inputs == NULL) {
        printf("Error, Insufficient memory to run full test\n.");
        _task_block();
    }

    if (ulPin > A5) return (0);

	do {
    	done = _lwadc_wait_next(&lwadc_inputs[ulPin]);
    }while (!done);
	_lwadc_read_raw(&lwadc_inputs[ulPin], &raw);
	printf ("analog pin = %d sensorValue = %d\n", ulPin, raw);
	normRaw = mapResolution(raw, ADC_RESOLUTION, _readResolution);

	return normRaw;
}

void analogOutputInit(void) {
}

// ulPin matches to arduino pin (3, 4, 5, 6, 7, 9, 10, 11)
void analogWrite(uint32_t ulPin, uint32_t ulValue) {

	if (ulPin > NMAX_ARD_ANALOG_OUT_PIN) return;
	uint16_t pwmChn = ardPinToPwmChn[ulPin];
	if (pwmChn == 255) return;

	if (!PwmIsEnabled(pwmChn)) {
		// PWM Startup code
    	_bsp_pwm_io_init(pwmChn, 0xff);
		pwm_disable(pwmChn);

    	pwm_set_clock(pwmChn, kPwmClockSourceIpg);
    	pwm_set_fwm(pwmChn, 1);
    	pwm_set_prescaler(pwmChn, 0);
    	pwm_set_poutc(pwmChn, 0);
    	pwm_set_period(pwmChn, 0xfffe);
    	pwm_set_resolution(pwmChn, 16);
    	pwm_set_repeat(pwmChn, 1);
		pwm_enable(pwmChn);
		/*
		printf("MY_HW_PWM_PWMPR(%d)=%08X\n", pwmChn, MY_HW_PWM_PWMPR(pwmChn).U);
		printf("MY_HW_PWM_PWMCR(%d)=%08X\n", pwmChn, MY_HW_PWM_PWMCR(pwmChn).U);
		printf("MY_HW_PWM_PWMSAR(%d)=%08X\n", pwmChn, MY_HW_PWM_PWMSAR(pwmChn).U);
		*/
	}

	ulValue = mapResolution(ulValue, _writeResolution, pwm_get_resolution(pwmChn));
   	pwm_set_sample(pwmChn, (uint16_t)ulValue);
}

static void print_lwadc_attribute( LWADC_STRUCT_PTR lwadc_ptr, LWADC_ATTRIBUTE attribute, const char * name)
{
    uint32_t value;

    if (_lwadc_get_attribute(lwadc_ptr,attribute,&value)) {
        printf("%-30s = 0x%08x (%d)\n", name,value,value);
    } else {
        printf("%-30s not supported\n", name);
    }
}

#define PRINT_LWADC_ATTRIBUTE(p,x) print_lwadc_attribute(p,x,#x)

static void print_all_lwadc_attributes( LWADC_STRUCT_PTR lwadc_ptr)
{
    PRINT_LWADC_ATTRIBUTE(lwadc_ptr, LWADC_RESOLUTION);
    PRINT_LWADC_ATTRIBUTE(lwadc_ptr, LWADC_REFERENCE);
    PRINT_LWADC_ATTRIBUTE(lwadc_ptr, LWADC_FREQUENCY);
    PRINT_LWADC_ATTRIBUTE(lwadc_ptr, LWADC_DIVIDER);
    PRINT_LWADC_ATTRIBUTE(lwadc_ptr, LWADC_DIFFERENTIAL);
    PRINT_LWADC_ATTRIBUTE(lwadc_ptr, LWADC_POWER_DOWN);
    PRINT_LWADC_ATTRIBUTE(lwadc_ptr, LWADC_NUMERATOR);
    PRINT_LWADC_ATTRIBUTE(lwadc_ptr, LWADC_DENOMINATOR);
    PRINT_LWADC_ATTRIBUTE(lwadc_ptr, LWADC_FORMAT);
    PRINT_LWADC_ATTRIBUTE(lwadc_ptr, LWADC_INPUT_CONVERSION_ENABLE);
}

/*

static void monitor_all_inputs(void)
{
    LWADC_VALUE_PTR     last;
    LWADC_VALUE         i,scaled, raw, delta, max_delta = 160;

    printf("Monitoring all inputs\n");

    last         = (LWADC_VALUE_PTR)  _mem_alloc_zero(ELEMENTS_OF(adc_inputs)*sizeof(LWADC_VALUE));

    if ((lwadc_inputs == NULL) || (last==NULL)) {
        printf("Error, Insufficient memory to run full test\n.");
        _task_block();
    }

    for (i=0;i<ELEMENTS_OF(adc_inputs);i++) {
        // Set last value to a value out of range of the ADC.
        last[i] = MAX_UINT_32;
    }

    printf("Monitoring ADC Inputs element=%d\n", ELEMENTS_OF(adc_inputs));
    while (1) {
        for (i=0;i<ELEMENTS_OF(adc_inputs);i++) {
            // This waits until a new conversion is read on the channel
            if (_lwadc_wait_next(&lwadc_inputs[i])) {
                if (_lwadc_read(&lwadc_inputs[i], &scaled) &&
                    _lwadc_read_raw(&lwadc_inputs[i], &raw)) {

                    // Obtained data, is the change significant enough to display?
                    delta = (raw>last[i])?raw-last[i]:last[i]-raw;
                    if (delta > max_delta) {
                        printf("%-30s = %04x (%d mv)\n",adc_inputs[i].name, raw,scaled);
                        last[i]=raw;
                    }
                }
            }
        }
    }
}

void test_adc(void)
{
    printf("\n\n\nADC Test for %s\n",BSP_NAME);
    _lwadc_init(&lwadc1_init);
    _lwadc_init(&lwadc2_init);	//only two channels, vedi init_lwadc.c
    InitAdcs();
    print_all_lwadc_attributes(lwadc_inputs);
    monitor_all_inputs();
}
*/

void AdcBegin(void)
{
    printf("AdcBegin()\n");
    _lwadc_init(&lwadc1_init);
    _lwadc_init(&lwadc2_init);	//only two channels, vedi init_lwadc.c
    InitAdcs();
    print_all_lwadc_attributes(lwadc_inputs);
//    monitor_all_inputs();
}

#ifdef __cplusplus
}
#endif

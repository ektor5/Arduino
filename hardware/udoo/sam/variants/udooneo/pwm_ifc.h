/*
 * Copyright (c) 2011-2012, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __PWM_IFC__
#define __PWM_IFC__

#include "regspwm.h"

//! @addtogroup diag_pwm
//! @{

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

//! @brief Available PWM interrupts.
enum _pwm_interrupts {
    kPwmFifoEmptyIrq = 1 << 0,  //!< FIFO empty interrupt.
    kPwmRolloverIrq = 1 << 1,   //!< Rollover interrupt. The counter has reached the period and reset.
    kPwmCompareIrq = 1 << 2     //!< Compare interrupt. The counter value matches the current sample value
};

//! @brief Clock sources for the PWM.
enum _pwm_clksrc {
    kPwmClockSourceNone = 0,
    kPwmClockSourceIpg = 1,
    kPwmClockSourceCkih = 2,
    kPwmClockSourceCkil = 3
};

struct pwm_parms {
    uint32_t clock;              //!< Input clock frequency.
    uint32_t smp_cnt;
    uint16_t prescale;
    uint16_t period;
    uint16_t *sample;
    uint8_t interrupt;
    uint8_t active_pol;
    uint8_t repeat;
};

struct pwm_interrupt_status {
    uint32_t instance;          //!< The number of this module instance. The first instance is number 1.
    uint8_t interrupt;           //!< PWM interrupt.
};

//! Number of sample FIFO entries.
#define PWM_CNT_FIFO_SZ       (4)

const uint32_t PWM_BASE_REGS[] = {REGS_PWM1_BASE, REGS_PWM2_BASE, REGS_PWM3_BASE, REGS_PWM4_BASE, REGS_PWM5_BASE,
		REGS_PWM6_BASE, REGS_PWM7_BASE, REGS_PWM8_BASE};

#define MY_HW_PWM_PWMCR(x)		(*(volatile hw_pwm_pwmcr_t *) PWM_BASE_REGS[x])
#define MY_HW_PWM_PWMSR(x)		(*(volatile hw_pwm_pwmsr_t *) (PWM_BASE_REGS[x]+0x4))
#define MY_HW_PWM_PWMIR(x)		(*(volatile hw_pwm_pwmir_t *) (PWM_BASE_REGS[x]+0x8))
#define MY_HW_PWM_PWMSAR(x)		(*(volatile hw_pwm_pwmsar_t *) (PWM_BASE_REGS[x]+0xc))
#define MY_HW_PWM_PWMPR(x)		(*(volatile hw_pwm_pwmpr_t *) (PWM_BASE_REGS[x]+0x10))
#define MY_HW_PWM_PWMCNR(x)		(*(volatile hw_pwm_pwmcnr_t *) (PWM_BASE_REGS[x]+0x14))


// PWM
typedef struct _pwm_setup_struct {
	uint8_t enabled;
	uint8_t resolution;
	uint16_t period;
} PWM_SETUP_STRUCT;

////////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)
extern "C" {
#endif


/*!
 * @brief Enable PWM output.
 *
 * The PWM counter starts counting and the waveform is produced on the output
 * pin. Interrupts will be triggered upon fifo empty, compare and rollover.
 *
 * @param instance the PWM instance number.
 */
void pwm_enable(uint16_t instance);

/*!
 * @brief Disable PWM ouput.
 *
 * Stops the PWM counter.
 *
 * @param instance the PWM instance number.
 */
void pwm_disable(uint16_t instance);

/*!
 * @brief Get current frequency of corresponding PWM clock source.
 *
 * @param clock PWM clock souce. The @a clock parameter should be composed of one of the below:
 *    - #kPwmClockSourceIpg
 *    - #kPwmClockSourceCkih
 *    - #kPwmClockSourceCkil
 */
void pwm_set_clock(uint16_t instance, uint16_t clkSrc);
void pwm_set_fwm(uint16_t instance, uint16_t fwm);
void pwm_set_prescaler(uint16_t instance, uint16_t prescale);
void pwm_set_poutc(uint16_t instance, uint16_t poutc);
void pwm_set_period(uint16_t instance, uint16_t period);
void pwm_set_repeat(uint16_t instance, uint16_t repeat);
bool pwm_set_sample(uint16_t instance, uint16_t sample);
bool pwmIsEnable(uint16_t instance);
uint16_t pwm_get_period(uint16_t instance);
void pwm_set_resolution(uint16_t instance, uint16_t bits);
uint16_t pwm_get_resolution(uint16_t instance);
bool PwmIsEnabled(uint16_t instance);

#if defined(__cplusplus)
}
#endif
//! @}
#endif
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

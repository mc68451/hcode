/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p10/common/occ/ssx/ppc405/ppc405_core.c $        */
/*                                                                        */
/* OpenPOWER EKB Project                                                  */
/*                                                                        */
/* COPYRIGHT 2016,2019                                                    */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc405_core.c
/// \brief The final bits of SSX runtime code required to complete the PPC405
/// port.
///
/// The entry points in this file are considered 'core' routines that will
/// always be present during runtime in any SSX application.

#define __PPC405_CORE_C__

#include "ssx.h"

// Even though the external timebase is only a 32 bit register, we emulate
// a 64 bit timebase by keeping the upper 32 bits in SRAM.
volatile SsxTimebase  ppc405_64bit_ext_timebase = 0;

#if APPCFG_USE_EXT_TIMEBASE_FOR_TRACE
typedef union
{
    struct
    {
        uint32_t tbu;
        uint32_t tbl;
    };
    SsxTimebase tb64;
} SsxExtTimebase;


SsxTimebase ssx_ext_timebase_get(void)
{
    SsxExtTimebase              snapshot;
    volatile SsxExtTimebase*     cur_tb = (SsxExtTimebase*)&ppc405_64bit_ext_timebase;
    uint32_t                    tbr;
    uint32_t                    high;

    //read our in-memory timebase accumulator.
    //NOTE: 64 bit reads are not atomic on the ppc405.  This means that the
    //accumulator can be updated between reading the upper 32 bits and lower
    //32 bits.  It's ok if only the lower 32 bits changed, but if the upper
    //32 bits changed, then we will report the wrong time stamp.  Therefore,
    //we check the upper 32 bits after reading the lower 32 bits to make sure
    //it hasn't changed.
    do
    {
        snapshot.tbu = cur_tb->tbu;
        snapshot.tbl = cur_tb->tbl;
        high = cur_tb->tbu;
    }
    while(snapshot.tbu != high);

    //Now read the external timebase register
    tbr = in32(OCB_OTBR);

    //Check if we need to increment the upper 32 bits
    if(tbr < snapshot.tbl)
    {
        snapshot.tbu++;
    }

    snapshot.tbl = tbr;
    return snapshot.tb64;
}

#endif /* APPCFG_USE_EXT_TIMEBASE_FOR_TRACE */

/// Get the 64-bit timebase following the PowerPC protocol
///
/// Note that the only way to guarantee that the value returned is the value
/// \e right \e now is to call this API from a critical section.

SsxTimebase
ssx_timebase_get(void)
{
    Uint64 tb;
    uint32_t high;

    do
    {
        tb.word[0] = mftbu();
        tb.word[1] = mftb();
        high       = mftbu();
    }
    while (high != tb.word[0]);

    return tb.value;
}


/// Set the 64-bit timebase in an SSX_CRITICAL critical section
///
/// It is assumed that the caller knows what they are doing; e.g., is aware of
/// what may happen when time warps as a result of this call.

void
ssx_timebase_set(SsxTimebase timebase)
{
    SsxMachineContext ctx;
    Uint64 tb;

    tb.value = timebase;

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    mttbl(0);
    mttbu(tb.word[0]);
    mttbl(tb.word[1]);

    ssx_critical_section_exit(&ctx);
}


/// Enable interrupt preemption
///
/// This API can only be called from an interrupt context.  Threads will
/// always be preempted by interrupts unless they explicitly disable
/// interrupts with the \c ssx_interrupt_disable() API. It is legal to call
/// this API redundantly.
///
/// Be careful when enabling interrupt handler preemption that the interrupt
/// being handled does not/can not trigger again, as this could rapidly lead
/// to stack overflows.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_ILLEGAL_CONTEXT The API call was not made from an interrupt
/// context.

int
ssx_interrupt_preemption_enable()
{
    if (SSX_ERROR_CHECK_API)
    {
        SSX_ERROR_UNLESS_ANY_INTERRUPT_CONTEXT();
    }

    if (__ssx_kernel_context_noncritical_interrupt())
    {
        wrteei(1);
    }
    else
    {
        or_msr(MSR_CE);
    }

    return SSX_OK;
}


/// Disable interrupt preemption
///
/// This API can only be called from an interrupt context.  Threads will
/// always be preempted by interrupts unless they explicitly disable
/// interrupts with the \c ssx_interrupt_disable() API.  It is legal to call
/// this API redundantly.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_ILLEGAL_CONTEXT The API call was not made from an interrupt
/// context.

int
ssx_interrupt_preemption_disable()
{
    if (SSX_ERROR_CHECK_API)
    {
        SSX_ERROR_UNLESS_ANY_INTERRUPT_CONTEXT();
    }

    if (__ssx_kernel_context_noncritical_interrupt())
    {
        wrteei(0);
    }
    else
    {
        andc_msr(MSR_CE);
    }

    return SSX_OK;
}


#if SSX_TIMER_SUPPORT

// The tickless kernel timer mechanism for PPC405
//
// This routine must be called from an SSX_NONCRITICAL critical section.
//
// Tickless timeouts are provided by programming the PIT timer based on when
// the next timeout will occur.  If the timeout is for the end of time there's
// nothing to do - SSX does not use auto-reload mode so no more PIT interrupts
// will be arriving.  Otherwise, if the timeout is longer than the 32-bit PIT
// timer can handle, we simply schedule the timeout for 2**32 - 1 and
// __ssx_timer_handler() will keep rescheduling it until it finally occurs.
// If the \a timeout is in the past, we schedule the PIT interrupt for 1 tick
// in the future in accordance with the SSX specification.

void
__ssx_schedule_hardware_timeout(SsxTimebase timeout)
{
    SsxTimebase now;
    uint32_t pit;

    if (timeout != SSX_TIMEBASE_MAX)
    {

        now = ssx_timebase_get();

        if (timeout <= now)
        {
            pit = 1;
        }
        else if ((timeout - now) > 0xffffffff)
        {
            pit = 0xffffffff;
        }
        else
        {
            pit = timeout - now;
        }

        mtspr(SPRN_PIT, pit);

#if APPCFG_USE_EXT_TIMEBASE_FOR_TRACE
        ppc405_64bit_ext_timebase = ssx_ext_timebase_get();
#endif /* APPCFG_USE_EXT_TIMEBASE_FOR_TRACE */
    }
}


// Cancel the PPC405 tickless kernel timeout
//
// This routine must be called from an SSX_NONCRITICAL critical section.  SSX
// does not use auto-reload mode of the PIT, so simply writing the PIT with 0
// effectively cancels the timer.

void
__ssx_cancel_hardware_timeout()
{
    mtspr(SPRN_PIT, 0);
}


#endif  /* SSX_TIMER_SUPPORT */

#undef __PPC405_CORE_C__

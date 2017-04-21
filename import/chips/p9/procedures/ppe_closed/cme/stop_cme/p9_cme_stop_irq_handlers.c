/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/procedures/ppe_closed/cme/stop_cme/p9_cme_stop_irq_handlers.c $ */
/*                                                                        */
/* OpenPOWER HCODE Project                                                */
/*                                                                        */
/* COPYRIGHT 2015,2017                                                    */
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

#include "p9_cme_stop.h"
#include "p9_cme_stop_enter_marks.h"
#include "p9_cme_irq.h"

extern CmeStopRecord G_cme_stop_record;


void
p9_cme_stop_pcwu_handler(void* arg, PkIrqId irq)
{
    MARK_TRAP(STOP_PCWU_HANDLER)
    PK_TRACE_INF("PCWU Handler Trigger %d", irq);

    uint32_t  core_mask = 0;
    uint32_t  core      = (in32(CME_LCL_EISR) & BITS32(12, 2)) >> SHIFT32(13);
    data64_t  scom_data = {0};
    ppm_pig_t pig       = {0};

    for (core_mask = 2; core_mask; core_mask--)
    {
        if (core & core_mask)
        {
            CME_GETSCOM(CPPM_CPMMR, core_mask, CME_SCOM_AND, scom_data.value);

            // If notify_select == sgpe
            if (scom_data.words.upper & BIT32(13))
            {
                // In stop5 as using type2, send exit pig
                if (!(scom_data.words.upper & BIT32(10)))
                {
                    pig.fields.req_intr_type    = PIG_TYPE2;
                    pig.fields.req_intr_payload = 0x400;
                    CME_PUTSCOM(PPM_PIG, core_mask, pig.value);
                }

                // block pc for stop8,11 or stop5 as pig sent
                out32(CME_LCL_EIMR_OR, core_mask << SHIFT32(13));
                G_cme_stop_record.core_blockpc |= core_mask;
                core = core - core_mask;
            }
        }
    }

    // if still wakeup for core with notify_select == cme, go exit
    if (core)
    {
        out32(CME_LCL_EIMR_OR, BITS32(12, 6) | BITS32(20, 2));
        pk_semaphore_post((PkSemaphore*)arg);
    }
}


// When take an Interrupt on falling edge of SPWU from a CPPM.
// 1) Read EINR to check if another one has been set
//    in the meantime from the same core.  If so abort.
// 2) Clear Special Wakeup Done to that CPPM.
// 3) Read GPMMR[1] to see if any Special Wakeup has been sent to active
//    in the meantime.  If so, set Special Wakeup Done again and abort.
// 4) Otherwise flip polarity of Special Wakeup in EISR and clear PM_EXIT
//    (note for abort cases do not Do not flip polarity of Special Wakeup in EISR.)
void
p9_cme_stop_spwu_handler(void* arg, PkIrqId irq)
{
    MARK_TRAP(STOP_SPWU_HANDLER)
    //PK_TRACE_INF("SPWU Handler");

    PkMachineContext ctx;
    int      sem_post   = 0;
    uint32_t core_mask  = 0;
    uint32_t core_index = 0;
    uint32_t raw_spwu   = (in32(CME_LCL_EISR) & BITS32(14, 2)) >> SHIFT32(15);
    uint64_t scom_data  = 0;

    PK_TRACE_INF("SPWU Trigger %d Level %x State %x",
                 irq, raw_spwu, G_cme_stop_record.core_in_spwu);

    for(core_mask = 2; core_mask; core_mask--)
    {
        if (raw_spwu & core_mask)
        {
            core_index = core_mask & 1;
            PK_TRACE("Detect SPWU signal level change on core%d", core_index);

            // if failing edge == spwu drop:
            if (G_cme_stop_record.core_in_spwu & core_mask)
            {
                PK_TRACE("Falling edge of spwu, first clearing EISR");
                out32(CME_LCL_EISR_CLR, BIT32((14 + core_index)));

                // if spwu asserts again before we drop spwu_done, do nothing, else:
                if ((~(in32(CME_LCL_EINR))) & BIT32((14 + core_index)))
                {
                    PK_TRACE("SPWU drop confirmed, now dropping spwu_done");
                    out32(CME_LCL_SICR_CLR, BIT32((16 + core_index)));

                    CME_GETSCOM(PPM_GPMMR, core_mask, CME_SCOM_AND, scom_data);

                    // if spwu has been re-asserted after spwu_done is dropped:
                    if (scom_data & BIT64(1))
                    {
                        PK_TRACE("SPWU asserts again, re-asserting spwu_done");
                        out32(CME_LCL_SICR_OR, BIT32((16 + core_index)));
                    }
                    // if spwu truly dropped:
                    else
                    {
                        PK_TRACE("Flip EIPR to raising edge and drop pm_exit");
                        out32(CME_LCL_EIPR_OR,  BIT32((14 + core_index)));
                        out32(CME_LCL_SICR_CLR, BIT32((4  + core_index)));

                        // Core is now out of spwu, allow pm_active
                        G_cme_stop_record.core_in_spwu &= ~core_mask;
                        g_eimr_override                &= ~(IRQ_VEC_STOP_C0 >> core_index);
                    }
                }
            }
            // raising edge, Note do not clear EISR as thread will read and clear:
            else
            {
                PK_TRACE("Raising edge of spwu, clear EISR later in exit thread");
                sem_post = 1;
            }
        }
    }

    if (sem_post)
    {
        out32(CME_LCL_EIMR_OR, BITS32(12, 6) | BITS32(20, 2));
        PK_TRACE_INF("Launching exit thread");
        pk_semaphore_post((PkSemaphore*)arg);
    }
    else
    {
        pk_irq_vec_restore(&ctx);
    }
}



void
p9_cme_stop_rgwu_handler(void* arg, PkIrqId irq)
{
    MARK_TRAP(STOP_RGWU_HANDLER)
    PK_TRACE_INF("RGWU Handler Trigger %d", irq);
    out32(CME_LCL_EIMR_OR, BITS32(12, 6) | BITS32(20, 2));
    pk_semaphore_post((PkSemaphore*)arg);
}



void
p9_cme_stop_enter_handler(void* arg, PkIrqId irq)
{
    MARK_TRAP(STOP_ENTER_HANDLER)
    PK_TRACE_INF("PM_ACTIVE Handler Trigger %d", irq);
    out32(CME_LCL_EIMR_OR, BITS32(12, 6) | BITS32(20, 2));
    pk_semaphore_post((PkSemaphore*)arg);
}

void
p9_cme_stop_db1_handler(void* arg, PkIrqId irq)
{
    PkMachineContext ctx;
    MARK_TRAP(STOP_DB1_HANDLER)
    PK_TRACE_DBG("DB1 Handler Trigger %d", irq);
    pk_irq_vec_restore(&ctx);
}

void
p9_cme_stop_db2_handler(void* arg, PkIrqId irq)
{
    PkMachineContext ctx;
    MARK_TRAP(STOP_DB2_HANDLER)
    PK_TRACE_DBG("DB2 Handler Trigger %d", irq);

    // read and clear doorbell
    uint32_t core = (in32(CME_LCL_EISR) & BITS32(18, 2)) >> SHIFT32(19);
    CME_PUTSCOM(CPPM_CMEDB2, core, 0);
    G_cme_stop_record.core_blockpc &= ~core;

    // unmask pc interrupt pending to wakeup that is still pending
    core &= (~(G_cme_stop_record.core_running));
    out32(CME_LCL_EIMR_CLR, core << SHIFT32(13));

    pk_irq_vec_restore(&ctx);
}

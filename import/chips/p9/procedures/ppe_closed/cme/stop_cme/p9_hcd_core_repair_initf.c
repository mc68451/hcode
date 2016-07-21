/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p9/procedures/ppe_closed/cme/stop_cme/p9_hcd_core_repair_initf.c $ */
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


#include "plat_ring_traverse.h"
#include "p9_cme_stop.h"
#include "p9_cme_stop_exit_marks.h"
#include "p9_ringid_cme_enums.h"
int
p9_hcd_core_repair_initf(uint32_t core)
{
    int rc = CME_STOP_SUCCESS;
    int i = 0;

    // Markers needed for repair ininf
    if (core == 3)
    {
        for (i = 0; i < 2; ++i)
        {
            core = 2;

            if (i)
            {
                core = 1;
            }

            PK_TRACE("Scan ec_repr ring core value %d", core);
            putRing(core, CME_SCOM_EQ, ec_repr);
        }
    }
    else
    {
        PK_TRACE("Scan ec_repr ring core value %d", core);
        putRing(core, CME_SCOM_EQ, ec_repr);
    }

    return rc;
}

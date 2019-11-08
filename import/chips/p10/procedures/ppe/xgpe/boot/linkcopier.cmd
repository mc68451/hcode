/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p10/procedures/ppe/xgpe/boot/linkcopier.cmd $    */
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

#ifndef INITIAL_STACK_SIZE
#define INITIAL_STACK_SIZE 256
#endif

OUTPUT_FORMAT(elf32-powerpc);

// Need to do this so that memmap header uses defines for linkerscript
#define __LINKERSCRIPT__
#include <p10_hcd_memmap_homer.H>

#define MEM_START  HOMER_XGPE_BOOT_COPIER_ADDR
#define MEM_LENGTH XGPE_BOOT_COPIER_SIZE

MEMORY
{
  mem : ORIGIN = MEM_START, LENGTH = MEM_LENGTH
}

SECTIONS
{
    . = MEM_START;

    . = ALIGN(512);

    _VECTOR_START = .;

    .vectors  _VECTOR_START  : { *(.vectors) } > mem

    ////////////////////////////////
    // All non-vector code goes here
    ////////////////////////////////
    .text       : { *(.text) } > mem

    ////////////////////////////////
    // Read-only Data
    ////////////////////////////////

    . = ALIGN(8);
    _RODATA_SECTION_BASE = .;

    // SDA2 constant sections .sdata2 and .sbss2 must be adjacent to each
    // other.  Our SDATA sections are small so we'll use strictly positive
    // offsets.

    _SDA2_BASE_ = .;
   .sdata2 . : { *(.sdata2) } > mem
   .sbss2  . : { *(.sbss2) } > mem

   // Other read-only data.

   .rodata . : { *(.rodata*) *(.got2) } > mem

    _RODATA_SECTION_SIZE = . - _RODATA_SECTION_BASE;

    ////////////////////////////////
    // Read-write Data
    ////////////////////////////////

    . = ALIGN(8);
    _DATA_SECTION_BASE = .;

    // SDA sections .sdata and .sbss must be adjacent to each
    // other.  Our SDATA sections are small so we'll use strictly positive
    // offsets.

    _SDA_BASE_ = .;
    .sdata  . : { *(.sdata)  } > mem
    .sbss   . : { *(.sbss)   } > mem

    // Other read-write data
    // It's not clear why boot.S is generating empty .glink,.iplt

   .rela   . : { *(.rela*) } > mem
   .rwdata . : { *(.data) *(.bss) } > mem
//   .iplt . : { *(.iplt) } > mem

    . = ALIGN(8);
    _bootcopier_end = . - 0;

}

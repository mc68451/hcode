/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: import/chips/p10/common/occ/commonlib/string.h $              */
/*                                                                        */
/* OpenPOWER EKB Project                                                  */
/*                                                                        */
/* COPYRIGHT 2015,2019                                                    */
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
#ifndef __STRING_H__
#define __STRING_H__

/// \file string.h
/// \brief Replacement for <string.h>
///
/// The SSX library does not implement the entire <string.h> function.
/// However the real reason for this header was the finding that under certain
/// optimization modes, we were geting errors from the default <string.h>
/// supplied with the MPC environment.  So we created this replacement that
/// only calls out what is implemented, exactly as it is implemented for SSX.

#ifndef __ASSEMBLER__

    #include <stddef.h>
    #if !defined(__size_t)
        #include <stdint.h>
        typedef size_t uint32_t;
    #endif

    // APIs inmplemented by string.c

    size_t
    strlen(const char* s);

    int
    strcmp(const char* s1, const char* s2);

    int
    strncmp(const char* s1, const char* s2, size_t n);

    int
    strcasecmp(const char* s1, const char* s2);

    int
    strncasecmp(const char* s1, const char* s2, size_t n);

    char*
    strcpy(char* dest, const char* src);

    char*
    strncpy(char* dest, const char* src, size_t n);

    void*
    memcpy(void* dest, const void* src, size_t n);

    void*
    memset(void* s, int c, size_t n);

    int
    memcmp(const void* s1, const void* s2, size_t n);

    // APIs implemented by strdup.c

    char*
    strdup(const char* s);

#endif  /* __ASSEMBLER__ */

#endif  /* __STRING_H__ */

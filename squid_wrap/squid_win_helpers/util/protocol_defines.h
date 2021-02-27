/*
 * Copyright (C) 1996-2018 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

/*
 * Copyright (c) 2009-2014, Treehouse Networks Ltd. New Zealand
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SQUID_HELPERS_DEFINES_H
#define __SQUID_HELPERS_DEFINES_H

/*
 * This file contains several macro definitions which are
 * useful and shared between helpers.
 */

#include <iostream>

#define HELPER_INPUT_BUFFER 8196

//#define USE_CPP
//#define DEBUG_SEND

#ifdef USE_CPP
  #ifdef DEBUG_SEND
    #define SEND_GEN(f, v)  std::cout << f << x << std::endl; debug(f " %s\n", v)
  #else
    #define SEND_GEN(f, v)  std::cout << f << x << std::endl
  #endif
#else
  #ifdef DEBUG_SEND
    #define SEND_GEN(f, v)  printf(f "%s\n", v); debug(f "%s", v)
  #else
    #define SEND_GEN(f, v)  _tprintf(f _T("%s\n"), v)
  #endif
#endif

/* send OK result to Squid with a string parameter. */
#define SEND_OK(x)   SEND_GEN(_T("AF "), x)
//#define SEND_OK(x)   SEND_GEN(_T("OK "), x)

/* send ERR result to Squid with a string parameter. */
#define SEND_ERR(x) SEND_GEN(_T("NA "), x)
//#define SEND_ERR(x) SEND_GEN(_T("ERR "), x)
//#define SEND_ERR_MSG(x) _tprintf(_T("ERR message=\"%s\"\n"), x)

/* send BH result to Squid with a string parameter. */
#define SEND_BH(x)  SEND_GEN(_T("BH "), x)

/* constructs a message to Squid. */
#define HLP_MSG(text)  _T("message=\"") text _T("\"")

/* send TT result to Squid with a string parameter. */
#define SEND_TT(x)  SEND_GEN("TT ", x)

#endif /* __SQUID_HELPERS_DEFINES_H */

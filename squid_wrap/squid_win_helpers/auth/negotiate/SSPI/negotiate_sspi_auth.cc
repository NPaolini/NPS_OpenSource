/*
 * Copyright (C) 1996-2018 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

/*
 * negotiate_sspi_auth: helper for Negotiate Authentication for Squid Cache
 *
 * (C)2005 Guido Serassio - Acme Consulting S.r.l.
 *
 * Authors:
 *  Guido Serassio <guido.serassio@acmeconsulting.it>
 *  Acme Consulting S.r.l., Italy <http://www.acmeconsulting.it>
 *
 * With contributions from others mentioned in the change history section
 * below.
 *
 * Based on previous work of Francesco Chemolli and Robert Collins.
 *
 * Dependencies: Windows 2000 and later.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * History:
 *
 * Version 1.0
 * 29-10-2005 Guido Serassio
 *              First release.
 */

//#include "squid.h"
#include "base64.h"
#include "protocol_defines.h"
#include "ntlmauth/ntlmauth.h"
#include "ntlmauth/support_bits.cci"
#include "sspwin32.h"
#include "util.h"

#include <windows.h>
#include <sspi.h>
#include <security.h>
#include <getopt.h>
#include <ctype.h>

int Negotiate_packet_debug_enabled = 0;
static int have_serverblob;

/* A couple of harmless helper macros */
#define SEND(X) {debug(_T("sending '%s' to squid\n"),X); _tprintf(X _T("\n"));}
#ifdef __GNUC__
#error ttt
#define SEND2(X,Y...) {debug("sending '" X "' to squid\n",Y); printf(X "\n",Y);}
#define SEND3(X,Y...) {debug("sending '" X "' to squid\n",Y); printf(X "\n",Y);}
#else
/* no gcc, no debugging. varargs macros are a gcc extension */
#define SEND2(X,Y) {debug(_T("sending '") X _T("' to squid\n"),Y); _tprintf(X _T("\n"),Y);}
#define SEND3(X,Y,Z) {debug(_T("sending '") X _T("' to squid\n"),Y,Z); _tprintf(X _T("\n"),Y,Z);}
#endif
//----------------------------------
static int debug_enabled = 0;
//----------------------------------
static void debug(LPTSTR format, ...)
{
  if (debug_enabled) {
    va_list args;
    va_start(args, format);
    _ftprintf(stderr, _T("negotiate_nt_auth[%d]: "), getpid());
    _vftprintf(stderr, format, args);
    va_end(args);
    }
}
//----------------------------------
//char *negotiate_check_auth(SSP_blobP auth, int auth_length);

/*
 * options:
 * -d enable debugging.
 * -v enable verbose Negotiate packet debugging.
 */
//----------------------------------
LPTSTR my_program_name = NULL;
//----------------------------------
void usage()
{
  _ftprintf(stderr,
            _T("Usage: %s [-d] [-v] [-h]\n")
            _T(" -d  enable debugging.\n")
            _T(" -v  enable verbose Negotiate packet debugging.\n")
            _T(" -h  this message\n\n"),
            my_program_name);
}
//----------------------------------
void process_options(int argc, TCHAR *argv[])
{
  int opt, had_error = 0;

  opterr = 0;
  while (-1 != (opt = getopt(argc, argv, _T("hdv")))) {
    switch (opt) {
      case _T('d'):
        debug_enabled = 1;
        break;
      case _T('v'):
        debug_enabled = 1;
        Negotiate_packet_debug_enabled = 1;
        break;
      case _T('h'):
        usage();
        exit(0);
      case _T('?'):
        opt = optopt;
        /* fall thru to default */
      default:
        _ftprintf(stderr, _T("ERROR: unknown option: -%c. Exiting\n"), opt);
        usage();
        had_error = 1;
      }
    }
  if (had_error)
    exit(1);
}
//----------------------------------
#ifdef UNICODE
static char* encodedBuff = 0;
static int encodedBuffLen = 0;
#endif
//----------------------------------
static bool token_decode(size_t *decodedLen, uint8_t decoded[], LPCTSTR buf)
{
  int len = _tcslen(buf);
#ifdef UNICODE
  if(len > encodedBuffLen) {
    delete []encodedBuff;
    encodedBuffLen = len;
    encodedBuff = new char[len + 2];
    }
  for (int i = 0; i < len; ++i)
    encodedBuff[i] = (char)buf[i];
  encodedBuff[len] = 0;
#else
  #define encodedBuff buf
#endif
  *decodedLen = base64_decode((LPSTR)decoded, encodedBuff, len);
  if(!*decodedLen) {
    SEND(_T("BH base64 decode failed"));
    _ftprintf(stderr, _T("ERROR: base64 decoding failed for: '%s'\n"), buf);
    return false;
    }
  return true;
}
#undef encodedBuff
//----------------------------------
LPTSTR separateUser(LPTSTR cred, LPTSTR dom)
{
  LPTSTR p = cred;
  while(*p) {
    if(_T('\\') == *p) {
      cred = p + 1;
      break;
      }
    *dom++ = *p;
    ++p;
    }
  *dom = 0;
  return cred;
}
//----------------------------------
static void to_lower(LPTSTR s)
{
  while (*s) {
    *s = _totlower(*s);
    ++s;
    }
}
//----------------------------------
template <typename T1, typename T2>
int myCmp(T1* t1, T2* t2, size_t len)
{
  for (size_t i = 0; i < len; ++i) {
    int t = t1[i] - (T1)(t2[i]);
    if (t)
      return t;
    }
  return 0;
}
//-------------------------------------------------------------------
struct acc
{
  char a;
  char r;
};

static acc gAcc[] = {
  { 'è', 'e' },
  { 'é', 'e' },
  { 'à', 'a' },
  { 'ò', 'o' },
  { 'ù', 'u' },
  { 'ì', 'i' },
};

#define SIZE_A(a) (sizeof(a) / sizeof(a[0]))

static void replaceAcc(LPSTR mbBuff)
{
  LPSTR p = mbBuff;
  while(*p) {
    if((unsigned)*p > 127) {
      for(int i = 0; i < SIZE_A(gAcc); ++i) {
        if(*p == gAcc[i].a)
          *p = gAcc[i].r;
        }
      }
    ++p;
    }
}
//----------------------------------
int manage_request()
{
  TCHAR buf[HELPER_INPUT_BUFFER];
  uint8_t decoded[HELPER_INPUT_BUFFER];
  size_t decodedLen = 0;
  TCHAR helper_command[3];
  LPTSTR c;
  int status;
  int oversized = 0;
  LPTSTR ErrorMessage;
  static TCHAR cred[SSP_MAX_CRED_LEN + 1];
  BOOL Done = FALSE;

  do {
    if (_fgetts(buf, HELPER_INPUT_BUFFER, stdin) == NULL)
      return 0;

    c = static_cast<LPTSTR>(memchr(buf, _T('\n'), HELPER_INPUT_BUFFER));
    if (c) {
      if (oversized) {
        SEND(_T("BH illegal request received"));
        _ftprintf(stderr, _T("ERROR: Illegal request received: '%s'\n"), buf);
        return 1;
        }
      *c = '\0';
      }
    else {
      _ftprintf(stderr, _T("No newline in '%s'\n"), buf);
      oversized = 1;
      continue;
      }
    break;
    } while (true);

  if ((_tcslen(buf) > 3) && Negotiate_packet_debug_enabled) {
    decodedLen = _tcslen(buf) - 3;
    if (!token_decode(&decodedLen, decoded, buf + 3))
      return 1;
    _tcsncpy(helper_command, buf, 2);
    debug(_T("Got '%s' from Squid with data:\n"), helper_command);
    hex_dump(reinterpret_cast<unsigned char*>(decoded), decodedLen);
    }
  else
    debug(_T("Got '%s' from Squid\n"), buf);

  if (myCmp(buf, _T("YR "), 3) == 0) {   /* refresh-request */
        /* figure out what we got */
    if (!decodedLen /* already decoded */ && !token_decode(&decodedLen, decoded, buf+3))
      return 1;
    if (decodedLen < sizeof(ntlmhdr)) {     /* decoding failure, return error */
      SEND(_T("NA * Packet format error"));
      return 1;
      }
        /* Obtain server blob against SSPI */
    cred[0] = 0;
    c = (LPTSTR) SSP_MakeNegotiateBlob(decoded, decodedLen, &Done, &status, cred);

    if (status == SSP_OK) {
      if (Done) {
        to_lower(cred);   /* let's lowercase them for our convenience */
        replaceAcc(cred);
        have_serverblob = 0;
        Done = FALSE;
        if (Negotiate_packet_debug_enabled) {
          if (c && !token_decode(&decodedLen, decoded, c))
            return 1;
          debug(_T("sending 'AF' %s to squid with data:\n"), cred);
          if (c != NULL) {
            hex_dump(reinterpret_cast<unsigned char*>(decoded), decodedLen);
            }
          else {
            _ftprintf(stderr, _T("No data available.\n"));
            }
          _tprintf(_T("AF %s %s\n"), c, cred);
          }
        else {
#if 1
          TCHAR dom[256];
          LPTSTR p = separateUser(cred, dom);
          if(!c)
            c = dom;
          SEND3(_T("AF %s %s"), c, p);
#else
          SEND3("AF %s %s", c, cred);
#endif
          }
        }
      else {
        if (Negotiate_packet_debug_enabled) {
          if (c && !!token_decode(&decodedLen, decoded, c))
            return 1;
          debug(_T("sending 'TT' to squid with data:\n"));
          hex_dump(reinterpret_cast<unsigned char*>(decoded), decodedLen);
          _tprintf(_T("TT %s\n"), c);
          }
        else {
#ifdef _UNICODE
          TCHAR t[64000];
          for(int i = 0; i < SIZE_A(t); ++i) {
            t[i] = c[i];
            if(!t[i])
              break;
            }
          SEND2(_T("TT %s"), t);
#else
          SEND2(_T("TT %s"), c);
#endif
          }
        have_serverblob = 1;
        }
      }
    else
      SEND(_T("BH can't obtain server blob"));
    return 1;
    }
  if (myCmp(buf, _T("KK "), 3) == 0) {   /* authenticate-request */
    if (!have_serverblob) {
      SEND(_T("BH invalid server blob"));
      return 1;
      }
        /* figure out what we got */
    if (!decodedLen /* already decoded */ && !token_decode(&decodedLen, decoded, buf+3))
      return 1;
    if (decodedLen < sizeof(ntlmhdr)) {     /* decoding failure, return error */
      SEND(_T("NA * Packet format error"));
      return 1;
      }
        /* check against SSPI */
    c = (LPTSTR) SSP_ValidateNegotiateCredentials(decoded, decodedLen, &Done, &status, cred);

    if (status == SSP_ERROR) {
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    /* Default language */
                    (LPTSTR) & ErrorMessage,
                    0,
                    NULL);
      if (ErrorMessage[_tcslen(ErrorMessage) - 1] == _T('\n'))
        ErrorMessage[_tcslen(ErrorMessage) - 1] = _T('\0');
      if (ErrorMessage[_tcslen(ErrorMessage) - 1] == _T('\r'))
        ErrorMessage[_tcslen(ErrorMessage) - 1] = _T('\0');
      SEND2(_T("NA * %s"), ErrorMessage);
      LocalFree(ErrorMessage);
      return 1;
      }
    if (Done) {
      debug(_T("cred_1:%s\n"), cred);
      to_lower(cred);       /* let's lowercase them for our convenience */
      replaceAcc(cred);
      debug(_T("cred_2:%s\n"), cred);
      have_serverblob = 0;
      Done = FALSE;
      if (Negotiate_packet_debug_enabled) {
        debug(_T("encoded:%s\n"), c);
        if (c && !token_decode(&decodedLen, decoded, c)) {
          debug(_T("error!\n"));
          return 1;
          }
        debug(_T("sending 'AF' %s to squid with data:\n"), cred);
        if (c != NULL)
          hex_dump(reinterpret_cast<unsigned char*>(decoded), decodedLen);
        else
          _ftprintf(stderr, _T("No data available.\n"));
        _tprintf(_T("AF %s %s\n"), c, cred);
        }
      else {
        SEND3(_T("AF %s %s"), c, cred);
        }
      return 1;
      }
    else {
      if (Negotiate_packet_debug_enabled) {
        if (!token_decode(&decodedLen, decoded, c))
          return 1;
        debug(_T("sending 'TT' to squid with data:\n"));
        hex_dump(reinterpret_cast<unsigned char*>(decoded), decodedLen);
        _tprintf(_T("TT %s\n"), c);
        }
      else {
#ifdef _UNICODE
        TCHAR t[64000];
        for(int i = 0; i < SIZE_A(t); ++i) {
          t[i] = c[i];
          if(!t[i])
            break;
          }
        SEND2(_T("TT %s"), t);
#else
        SEND2(_T("TT %s"), c);
#endif
        }
      return 1;
      }
    }
  else {            /* not an auth-request */
    SEND(_T("BH illegal request received"));
    _ftprintf(stderr, _T("Illegal request received: '%s'\n"), buf);
    return 1;
    }
  SEND(_T("BH detected protocol error"));
  return 1;
    /********* END ********/
}
//----------------------------------
/* Version number of package */
#define VERSION _T("4.5-20190128-r568e66b7c")
#define SQUID_BUILD_INFO _T("")
//----------------------------------
int _tmain(int argc, TCHAR *argv[])
{
  my_program_name = argv[0];
  process_options(argc, argv);

  debug(_T("%s ") VERSION _T(" ") SQUID_BUILD_INFO _T(" starting up...\n"), my_program_name);

  if (LoadSecurityDll(SSP_NTLM, NEGOTIATE_PACKAGE_NAME) == NULL) {
    _ftprintf(stderr, _T("FATAL: %s: can't initialize SSPI, exiting.\n"), argv[0]);
    exit(1);
    }
  debug(_T("SSPI initialized OK\n"));

  atexit(UnloadSecurityDll);

    /* initialize FDescs */
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  while (manage_request()) {
        /* everything is done within manage_request */
    }
  exit(0);
}


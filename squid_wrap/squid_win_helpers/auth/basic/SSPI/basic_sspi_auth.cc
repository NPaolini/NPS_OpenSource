/*
 * Copyright (C) 1996-2018 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

/*
  NT_auth -  Version 2.0

  Returns OK for a successful authentication, or ERR upon error.

  Guido Serassio, Torino - Italy

  Uses code from -
    Antonino Iannella 2000
    Andrew Tridgell 1997
    Richard Sharpe 1996
    Bill Welliver 1999

 * Distributed freely under the terms of the GNU General Public License,
 * version 2 or later. See the file COPYING for licensing details
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
*/
#include <tchar.h>
#include <string.h>
#include "valid.h"
#include "protocol_defines.h"
#include "rfc1738.h"
#include "util.h"
#include "sspwin32.h"

#include <getopt.h>

static char NTGroup[256];
LPTSTR NTAllowedGroup;
LPTSTR NTDisAllowedGroup;
int UseDisallowedGroup = 0;
int UseAllowedGroup = 0;
int debug_enabled = 0;

/*
 * options:
 * -A can specify a Windows Local Group name allowed to authenticate.
 * -D can specify a Windows Local Group name not allowed to authenticate.
 * -O can specify the default Domain against to authenticate.
 */
static void
usage(LPCSTR name)
{
    fprintf(stderr, "Usage:\n%s [-A|D UserGroup][-O DefaultDomain][-d]\n"
            "-A can specify a Windows Local Group name allowed to authenticate\n"
            "-D can specify a Windows Local Group name not allowed to authenticate\n"
            "-O can specify the default Domain against to authenticate\n"
            "-d enable debugging.\n"
            "-h this message\n\n",
            name);
}
#ifdef _UNICODE
LPTSTR myDup(LPCSTR s)
{
	int len = strlen(s);
	LPTSTR t = new TCHAR[len + 2];
	for (int i = 0; i < len + 1; ++i)
		t[i] = s[i];
	return t;
}
void myStrncpy(LPTSTR t, LPCSTR s, int len)
{
	for (int i = 0; i < len; ++i) {
		t[i] = s[i];
		if (!t[i])
			break;
	}
}
#else
#define myDup _tcsdup
#define myStrncpy strncpy
#endif
void
process_options(int argc, TCHAR *argv[])
{
    int opt;
    while (-1 != (opt = getopt(argc, argv, _T("dhA:D:O:")))) {
        switch (opt) {
        case 'A':
            delete []NTAllowedGroup;
            NTAllowedGroup = myDup(optarg);
            UseAllowedGroup = 1;
            break;
        case 'D':
            delete []NTDisAllowedGroup;
            NTDisAllowedGroup= myDup(optarg);
            UseDisallowedGroup = 1;
            break;
        case 'O':
			myStrncpy(Default_NTDomain, optarg, DNLEN);
            break;
        case 'd':
            debug_enabled = 1;
            break;
        case 'h':
            usage(argv[0]);
            exit(0);
        case '?':
            opt = optopt;
        /* fall thru to default */
        default:
            fprintf(stderr, "FATAL: Unknown option: -%c\n", opt);
            usage(argv[0]);
            exit(1);
        }
    }
}
static bool isUnicode;
bool onUnicode() { return isUnicode; }

// http://www.brescianet.com/appunti/vari/unicode.htm
#if 1
bool isutf1(unsigned char c)
{
#if 1
	if(c < 0xC0)
		return false;
	unsigned char t = 0xfc;
	for (int i = 0; i < 5; ++i) {
		if ((c & t) == t)
			return true;
		t <<= 1;
	}
	return false;
#else
	unsigned char t[] = { 0xfc, 0xf8, 0xf0, 0xe0, 0xc0 };
	for (int i = 0; i < sizeof(t); ++i)
		if ((c & t[i]) == t[i])
			return true;
	return false;
#endif
}
bool isutf2(unsigned char c)
{
	return (c & 0x80) == 0x80;
}
#else
#define isutf(c) (((c)&0xC0)!=0x80)
#endif
bool checkUnicode(LPCSTR str)
{
	isUnicode = false;
	while(*str) {
		if(isutf1(*str) && isutf2(*(str + 1))) {
			isUnicode = true;
			return true;
			}
		++str;
		}
	return false;
}
/* Main program for simple authentication.
   Scans and checks for Squid input, and attempts to validate the user.
*/
int _tmain(int argc, TCHAR *argv[])
{
    char wstr[HELPER_INPUT_BUFFER];
    char username[256];
    char password[256];
    char *p;
    int err = 0;

    process_options(argc, argv);

    if (LoadSecurityDll(SSP_BASIC, NTLM_PACKAGE_NAME) == NULL) {
        fprintf(stderr, "FATAL: can't initialize SSPI, exiting.\n");
        exit(1);
    }
    debug(_T("SSPI initialized OK\n"));

    atexit(UnloadSecurityDll);

    /* initialize FDescs */
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    while (fgets(wstr, HELPER_INPUT_BUFFER, stdin) != NULL) {

        if (NULL == strchr(wstr, '\n')) {
            err = 1;
            continue;
        }
        if (err) {
            SEND_ERR(_T("Oversized message"));
            err = 0;
            fflush(stdout);
            continue;
        }

        if ((p = strchr(wstr, '\n')) != NULL)
            *p = '\0';      /* strip \n */
        if ((p = strchr(wstr, '\r')) != NULL)
            *p = '\0';      /* strip \r */
        /* Clear any current settings */
        username[0] = '\0';
        password[0] = '\0';
        sscanf(wstr, "%s %s", username, password);  /* Extract parameters */

        debug(_T("Got %s from Squid\n"), wstr);

        /* Check for invalid or blank entries */
        if ((username[0] == '\0') || (password[0] == '\0')) {
            SEND_ERR(_T("Invalid Request"));
            fflush(stdout);
            continue;
        }
        rfc1738_unescape(username);
        rfc1738_unescape(password);
				
		if(!checkUnicode((LPCSTR)username))
			checkUnicode((LPCSTR)password);
        debug(_T("Trying to validate; %s %s\n"), username, password);

        if (Valid_User(username, password, NTGroup) == NTV_NO_ERROR)
            { SEND_OK(_T("")); }
        else
            { SEND_ERR(errormsg); }
        err = 0;
        fflush(stdout);
    }
    return 0;
}


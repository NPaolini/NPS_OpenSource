/*
 * Copyright (C) 1996-2018 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

/*
  NT_auth -  Version 2.0

  Modified to act as a Squid authenticator module.
  Removed all Pike stuff.
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

//#include "squid.h"
#include "util.h"

/* Check if we try to compile on a Windows Platform */
#if !_SQUID_WINDOWS_
/* NON Windows Platform !!! */
#error NON WINDOWS PLATFORM
#endif

#include <tchar.h>
#include "valid.h"

extern bool onUnicode();

TCHAR Default_NTDomain[DNLEN+1] = NTV_DEFAULT_DOMAIN;
LPCTSTR errormsg;

const TCHAR NTV_SERVER_ERROR_MSG[] = _T("Internal server errror");
const TCHAR NTV_GROUP_ERROR_MSG[] = _T("User not allowed to use this cache");
const TCHAR NTV_LOGON_ERROR_MSG[] = _T("No such user or wrong password");
const TCHAR NTV_VALID_DOMAIN_SEPARATOR[] = _T("\\/");

/* returns 1 on success, 0 on failure */
int
Valid_Group(LPTSTR UserName, LPTSTR Group)
{
    int result = FALSE;

    LPLOCALGROUP_USERS_INFO_0 pBuf = NULL;
    LPLOCALGROUP_USERS_INFO_0 pTmpBuf;
    DWORD dwLevel = 0;
    DWORD dwFlags = LG_INCLUDE_INDIRECT;
    DWORD dwPrefMaxLen = -1;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    NET_API_STATUS nStatus;
    DWORD i;
    DWORD dwTotalCount = 0;
#ifdef _UNICODE
#define wszUserName UserName
#define wszGroup Group
#else
		UINT cp = onUnicode() ? CP_UTF8 : CP_ACP;
	/* Convert ANSI User Name and Group to Unicode */
	WCHAR wszUserName[256]; // Unicode user name
	WCHAR wszGroup[256];    // Unicode Group

    MultiByteToWideChar(cp, 0, UserName,
                        strlen(UserName) + 1, wszUserName,
                        sizeof(wszUserName) / sizeof(wszUserName[0]));
    MultiByteToWideChar(cp, 0, Group,
                        strlen(Group) + 1, wszGroup, sizeof(wszGroup) / sizeof(wszGroup[0]));
#endif
    /*
     * Call the NetUserGetLocalGroups function
     * specifying information level 0.
     *
     * The LG_INCLUDE_INDIRECT flag specifies that the
     * function should also return the names of the local
     * groups in which the user is indirectly a member.
     */
    nStatus = NetUserGetLocalGroups(NULL,
                                    wszUserName,
                                    dwLevel,
                                    dwFlags,
                                    (LPBYTE *) & pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries);
    /*
     * If the call succeeds,
     */
    if (nStatus == NERR_Success) {
        if ((pTmpBuf = pBuf) != NULL) {
            for (i = 0; i < dwEntriesRead; ++i) {
                if (pTmpBuf == NULL) {
                    result = FALSE;
                    break;
                }
                if (wcscmp(pTmpBuf->lgrui0_name, wszGroup) == 0) {
                    result = TRUE;
                    break;
                }
                ++pTmpBuf;
                ++dwTotalCount;
            }
        }
    } else
        result = FALSE;
    /*
     * Free the allocated memory.
     */
    if (pBuf != NULL)
        NetApiBufferFree(pBuf);
    return result;
#undef wszUserName
#undef wszGroup
}
#define SIZE_A(a) (sizeof(a) / sizeof(*a))
int
Valid_User(LPSTR UserName, LPSTR Password, LPSTR Group)
{
    int result = NTV_SERVER_ERROR;
    size_t i;
    TCHAR NTDomain[256];
    LPTSTR domain_qualify = NULL;
    TCHAR DomainUser[256];
    TCHAR User[256];
		UINT cp = onUnicode() ? CP_UTF8 : CP_ACP;

#ifdef _UNICODE
	/* Convert ANSI User Name and Password to Unicode */
	WCHAR wszUserName[256];
	WCHAR wszPassword[256];

	MultiByteToWideChar(cp, 0, UserName,
		strlen(UserName) + 1, wszUserName,
		SIZE_A(wszUserName));
	MultiByteToWideChar(cp, 0, Password,
		strlen(Password) + 1, wszPassword, SIZE_A(wszPassword));
#else
#define wszUserName UserName
#define wszPassword Password

#endif

    errormsg = NTV_SERVER_ERROR_MSG;
    _tcsncpy_s(NTDomain, wszUserName, SIZE_A(NTDomain));

    for (i=0; i < _tcslen(NTV_VALID_DOMAIN_SEPARATOR); ++i) {
        if ((domain_qualify = _tcschr(NTDomain, NTV_VALID_DOMAIN_SEPARATOR[i])) != NULL)
            break;
    }
    if (domain_qualify == NULL) {
        _tcscpy_s(User, NTDomain);
        _tcscpy_s(NTDomain, Default_NTDomain);
    } else {
        _tcscpy_s(User, domain_qualify + 1);
        domain_qualify[0] = _T('\0');
    }
    debug(_T("User=%s, Psw=%s,Domain=%s\n"), User, Password, NTDomain);

    /* Log the client on to the local computer. */
    if (!SSP_LogonUser(User, wszPassword, NTDomain)) {
        result = NTV_LOGON_ERROR;
        errormsg = NTV_LOGON_ERROR_MSG;
        debug(_T("%s\n"), errormsg);
    } else {
        result = NTV_NO_ERROR;
        if (_tcscmp(NTDomain, NTV_DEFAULT_DOMAIN) == 0)
            _tcscpy_s(DomainUser, User);
        else {
			_tcscpy_s(DomainUser, NTDomain);
			_tcscat_s(DomainUser, _T("\\"));
			_tcscat_s(DomainUser, User);
        }
	    debug(_T("DomainUser=%s\n"), DomainUser);
        if (UseAllowedGroup) {
            if (!Valid_Group(DomainUser, NTAllowedGroup)) {
                result = NTV_GROUP_ERROR;
                errormsg = NTV_GROUP_ERROR_MSG;
                debug(_T("%s\n"), errormsg);
            }
        }
        if (UseDisallowedGroup) {
            if (Valid_Group(DomainUser, NTDisAllowedGroup)) {
                result = NTV_GROUP_ERROR;
                errormsg = NTV_GROUP_ERROR_MSG;
                debug(_T("%s\n"), errormsg);
            }
        }
    }
    return result;
}


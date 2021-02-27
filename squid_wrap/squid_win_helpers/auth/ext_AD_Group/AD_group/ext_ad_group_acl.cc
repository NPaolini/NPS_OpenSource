/*
 * Copyright (C) 1996-2018 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

/*
 * ext_ad_group_acl: lookup group membership in a Windows
 * Active Directory domain
 *
 * (C)2008-2009 Guido Serassio - Acme Consulting S.r.l.
 *
 * Authors:
 *  Guido Serassio <guido.serassio@acmeconsulting.it>
 *  Acme Consulting S.r.l., Italy <http://www.acmeconsulting.it>
 *
 * With contributions from others mentioned in the change history section
 * below.
 *
 * Based on mswin_check_lm_group by Guido Serassio.
 *
 * Dependencies: Windows 2000 SP4 and later.
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
 * Version 2.1
 * 20-09-2009 Guido Serassio
 *              Added explicit Global Catalog query
 *
 * Version 2.0
 * 20-07-2009 Guido Serassio
 *              Global groups support rewritten, now is based on ADSI.
 *              New Features:
 *              - support for Domain Local, Domain Global ad Universal
 *                groups
 *              - full group nesting support
 * Version 1.0
 * 02-05-2008 Guido Serassio
 *              First release, based on mswin_check_lm_group.
 *
 * This is a helper for the external ACL interface for Squid Cache
 *
 * It reads from the standard input the domain username and a list of
 * groups and tries to match it against the groups membership of the
 * specified username.
 *
 * Returns `OK' if the user belongs to a group or `ERR' otherwise, as
 * described on http://devel.squid-cache.org/external_acl/config.html
 *
 */

#define _CRT_SECURE_NO_WARNINGS
#define HELPER_INPUT_BUFFER 8196

/* send OK result to Squid with a string parameter. */
#define SEND_OK(x)  _ftprintf(stdout, _T("OK %s\n"),x)

/* send ERR result to Squid with a string parameter. */
#define SEND_ERR(x) _ftprintf(stdout, _T("ERR %s\n"),x)

/* send BH result to Squid with a string parameter. */
#define SEND_BH(x)  _ftprintf(stdout, _T("BH %s\n"),x)

/* constructs a message to Squid. */
#define HLP_MSG(text)  _T("message=\"") text _T("\"")

/* send TT result to Squid with a string parameter. */
#define SEND_TT(x)  _ftprintf(stdout, _T("TT %s\n"),x)

//-------------------------------------------------------------------
/*
#include "squid.h"
#include "helpers/defines.h"
#include "include/util.h"
*/
#if true //_SQUID_CYGWIN_
#include <wchar.h>
#include <tchar.h>
//int _wcsicmp(const wchar_t *, const wchar_t *);
#endif

#undef assert
#include <cassert>
#include <cctype>
#include <cstring>
#if HAVE_GETOPT_H
#include <getopt.h>
#endif
#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <adsiid.h>
#include <iads.h>
#include <adshlp.h>
#include <adserr.h>
#include <lm.h>
#include <dsrole.h>
#include <sddl.h>
#include <stdio.h>
#include <process.h>
#include "rfc1738.h"
typedef int pid_t;
//-------------------------------------------------------------------
enum ADSI_PATH {
    LDAP_MODE,
    GC_MODE
} ADSI_Path;
//-------------------------------------------------------------------
int debug_enabled = 0;
int err = 0;
int use_global = 0;
LPTSTR program_name;
pid_t mypid;
LPTSTR machinedomain;
int use_case_insensitive_compare = 0;
LPTSTR DefaultDomain = NULL;
const TCHAR NTV_VALID_DOMAIN_SEPARATOR[] = _T("\\/");
int numberofgroups = 0;
int WIN32_COM_initialized = 0;
LPTSTR WIN32_ErrorMessage = NULL;
wchar_t **User_Groups;
int User_Groups_Count = 0;

wchar_t *My_NameTranslate(wchar_t *, int, int);
LPTSTR Get_WIN32_ErrorMessage(HRESULT);
//-------------------------------------------------------------------
LPWSTR makeW_char(LPCSTR source, UINT cp = CP_UTF8)
{
  wchar_t *wc = 0;
  int wcsize = MultiByteToWideChar(cp, 0, source, -1, wc, 0);
  wc = new wchar_t[wcsize];
  MultiByteToWideChar(cp, 0, source, -1, wc, wcsize);
  return wc;
}
//-------------------------------------------------------------------
LPSTR makeMB_char(LPCWSTR source)
{
  char *mb = 0;
  int mbsize = WideCharToMultiByte(CP_UTF8, 0, source, -1, mb, 0, NULL, NULL);
  mb = new char[mbsize];
  WideCharToMultiByte(CP_UTF8, 0, source, -1, mb, mbsize, NULL, NULL);
  return mb;
}
//-------------------------------------------------------------------
void debug(LPCTSTR format, LPCSTR info)
{
  if(debug_enabled) {
#ifdef UNICODE
		LPWSTR i1 = 0;
		if(info) 
			i1 = makeW_char(info);
		_ftprintf(stderr, format, i1);
		delete []i1;	
#else
    _ftprintf(stderr, format, info);
#endif
    }
}
//-------------------------------------------------------------------
void debug(LPCTSTR format, LPCWSTR info)
{
  if(debug_enabled) {
#ifndef UNICODE
		LPSTR i1 = 0;
		if(info) 
			i1 = makeMB_char(info);
		_ftprintf(stderr, format, i1);
		delete []i1;	
#else
    _ftprintf(stderr, format, info, info);
#endif
    }
}
//-------------------------------------------------------------------
void debug(LPCTSTR format, LPCSTR info, LPCSTR info2)
{
  if(debug_enabled) {
#ifdef UNICODE
		LPWSTR i1 = 0;
		LPWSTR i2 = 0;
		if(info) 
			i1 = makeW_char(info);
		if(info2) 
			i2 = makeW_char(info2);
		_ftprintf(stderr, format, i1, i2);
		delete []i1;	
		delete []i2;	
#else
    _ftprintf(stderr, format, info, info2);
#endif
    }
}
//-------------------------------------------------------------------
void debug(LPCTSTR format, LPCWSTR info, LPCWSTR info2)
{
  if(debug_enabled) {
#ifndef UNICODE
		LPSTR i1 = 0;
		LPSTR i2 = 0;
		if(info) 
			i1 = makeMB_char(info);
		if(info2) 
			i2 = makeMB_char(info2);
		_ftprintf(stderr, format, i1, i2);
		delete []i1;	
		delete []i2;	
#else
    _ftprintf(stderr, format, info, info2);
#endif
    }
}
//-------------------------------------------------------------------
void debug(LPCTSTR format, LPCSTR info, LPCWSTR info2)
{
  if(debug_enabled) {
#ifdef UNICODE
		LPWSTR i1 = 0;
		if(info) 
			i1 = makeW_char(info);
		_ftprintf(stderr, format, i1, info2);
		delete []i1;	
#else
		LPSTR i2 = 0;
		if(info2) 
			i2 = makeMB_char(info2);
		_ftprintf(stderr, format, info, i2);
		delete []i2;	
#endif
    }
}
//-------------------------------------------------------------------
void debug(LPCTSTR format, LPCWSTR info, LPCSTR info2)
{
  if(debug_enabled) {
#ifdef UNICODE
		LPWSTR i2 = 0;
		if(info2) 
			i2 = makeW_char(info2);
		_ftprintf(stderr, format, info, i2);
		delete []i2;	
#else
		LPSTR i1 = 0;
		if(info) 
			i1 = makeMB_char(info);
		_ftprintf(stderr, format, i1, info2);
		delete []i1;	
#endif
    }
}
//-------------------------------------------------------------------
void debug(LPCTSTR format)
{
  if (debug_enabled)
    _ftprintf(stderr, format);
}
//-------------------------------------------------------------------
void debug(LPCTSTR format, LPCTSTR info, size_t info2)
{
  if (debug_enabled)
    _ftprintf(stderr, format, info, info2);
}
//-------------------------------------------------------------------
void
CloseCOM(void)
{
    if (WIN32_COM_initialized == 1)
        CoUninitialize();
}
//-------------------------------------------------------------------
HRESULT
GetLPBYTEtoOctetString(VARIANT * pVar, LPBYTE * ppByte)
{
    HRESULT hr = E_FAIL;
    void HUGEP *pArray;
    long lLBound, lUBound, cElements;

    if ((!pVar) || (!ppByte))
        return E_INVALIDARG;
    if ((pVar->vt) != (VT_UI1 | VT_ARRAY))
        return E_INVALIDARG;

    hr = SafeArrayGetLBound(V_ARRAY(pVar), 1, &lLBound);
    hr = SafeArrayGetUBound(V_ARRAY(pVar), 1, &lUBound);

    cElements = lUBound - lLBound + 1;
    hr = SafeArrayAccessData(V_ARRAY(pVar), &pArray);
    if (SUCCEEDED(hr)) {
        LPBYTE pTemp = (LPBYTE) pArray;
        *ppByte = (LPBYTE) CoTaskMemAlloc(cElements);
        if (*ppByte)
            memcpy(*ppByte, pTemp, cElements);
        else
            hr = E_OUTOFMEMORY;
    }
    SafeArrayUnaccessData(V_ARRAY(pVar));

    return hr;
}
//-------------------------------------------------------------------
#define SIZE_A(a) (sizeof(a) / sizeof(a[0]))
//-------------------------------------------------------------------
wchar_t *
Get_primaryGroup(IADs * pUser)
{
    HRESULT hr;
    VARIANT var;
    unsigned User_primaryGroupID;
    TCHAR tmpSID[SECURITY_MAX_SID_SIZE * 2];
    wchar_t *result = NULL;

    VariantInit(&var);

    /* Get the primaryGroupID property */
    hr = pUser->Get(L"primaryGroupID", &var);
    if (SUCCEEDED(hr)) {
        User_primaryGroupID = var.uintVal;
    } else {
        debug(_T("Get_primaryGroup: cannot get primaryGroupID, ERROR: %s\n"), Get_WIN32_ErrorMessage(hr));
        VariantClear(&var);
        return result;
    }
    VariantClear(&var);

    /*Get the objectSid property */
    hr = pUser->Get(L"objectSid", &var);
    if (SUCCEEDED(hr)) {
        PSID pObjectSID;
        LPBYTE pByte = NULL;
        LPTSTR szSID = NULL;
        hr = GetLPBYTEtoOctetString(&var, &pByte);

        pObjectSID = (PSID) pByte;

        /* Convert SID to string. */
        ConvertSidToStringSid(pObjectSID, &szSID);
        CoTaskMemFree(pByte);

        *(_tcsrchr(szSID, _T('-')) + 1) = _T('\0');
        _sntprintf(tmpSID, SIZE_A(tmpSID)-1, _T("%s%u"), szSID, User_primaryGroupID);

#ifdef UNICODE        
        result = My_NameTranslate(tmpSID, ADS_NAME_TYPE_SID_OR_SID_HISTORY_NAME, ADS_NAME_TYPE_1779);
#else
        wchar_t *wc = 0;
        int wcsize = MultiByteToWideChar(CP_UTF8, 0, tmpSID, -1, wc, 0);
        wc = new wchar_t[wcsize];
        MultiByteToWideChar(CP_UTF8, 0, tmpSID, -1, wc, wcsize);

        result = My_NameTranslate(wc, ADS_NAME_TYPE_SID_OR_SID_HISTORY_NAME, ADS_NAME_TYPE_1779);
        delete []wc;
#endif
        LocalFree(szSID);
        if (result == NULL)
            debug(_T("Get_primaryGroup: cannot get DN for %s.\n"), tmpSID);
        else {
          debug(_T("Get_primaryGroup: Primary group DN: %s.\n"), result);
        }
    } else
        debug(_T("Get_primaryGroup: cannot get objectSid, ERROR: %s\n"), Get_WIN32_ErrorMessage(hr));
    VariantClear(&var);
    return result;
}
//-------------------------------------------------------------------
LPTSTR Get_WIN32_ErrorMessage(HRESULT hr)
{
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  hr,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) & WIN32_ErrorMessage,
                  0,
                  NULL);
    return WIN32_ErrorMessage;
}
//-------------------------------------------------------------------
wchar_t *
My_NameTranslate(wchar_t * name, int in_format, int out_format)
{
    IADsNameTranslate *pNto;
    HRESULT hr;
    BSTR bstr;
    wchar_t *wc;

    if (WIN32_COM_initialized == 0) {
        hr = CoInitialize(NULL);
        if (FAILED(hr)) {
            debug(_T("My_NameTranslate: cannot initialize COM interface, ERROR: %s\n"), Get_WIN32_ErrorMessage(hr));
            /* This is a fatal error */
            exit(1);
        }
        WIN32_COM_initialized = 1;
    }
    hr = CoCreateInstance(CLSID_NameTranslate,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IADsNameTranslate,
                          (void **) &pNto);
    if (FAILED(hr)) {
        debug(_T("My_NameTranslate: cannot create COM instance, ERROR: %s\n"), Get_WIN32_ErrorMessage(hr));
        /* This is a fatal error */
        exit(1);
    }
    hr = pNto->Init(ADS_NAME_INITTYPE_GC, L"");
    if (FAILED(hr)) {
        debug(_T("My_NameTranslate: cannot initialise NameTranslate API, ERROR: %s\n"), Get_WIN32_ErrorMessage(hr));
        pNto->Release();
        /* This is a fatal error */
        exit(1);
    }
    hr = pNto->Set(in_format, name);
    if (FAILED(hr)) {
        debug(_T("My_NameTranslate: cannot set translate of %S, ERROR: %s\n"), name, Get_WIN32_ErrorMessage(hr));
        pNto->Release();
        return NULL;
    }
    hr = pNto->Get(out_format, &bstr);
    if (FAILED(hr)) {
        debug(_T("My_NameTranslate: cannot get translate of %S, ERROR: %s\n"), name, Get_WIN32_ErrorMessage(hr));
        pNto->Release();
        return NULL;
    }
    debug(_T("My_NameTranslate: %s translated to %s\n"), name, bstr);
    wc = new wchar_t[wcslen(bstr) + 1];
    wcscpy(wc, bstr);
    SysFreeString(bstr);
    pNto->Release();
    return wc;
}
//-------------------------------------------------------------------
wchar_t *
GetLDAPPath(wchar_t * Base_DN, int query_mode)
{
    wchar_t *wc;
    int l = wcslen(Base_DN) + 8;
    wc = new wchar_t[l];

    if (query_mode == LDAP_MODE)
        wcscpy_s(wc, l, L"LDAP://");
    else
        wcscpy_s(wc, l, L"GC://");
    wcscat_s(wc, l, Base_DN);

    return wc;
}
//-------------------------------------------------------------------
LPTSTR
GetDomainName(void)
{
    static LPTSTR DomainName = NULL;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pDSRoleInfo;
    DWORD netret;

    if ((netret = DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (PBYTE *) & pDSRoleInfo) == ERROR_SUCCESS)) {
        /*
         * Check the machine role.
         */

        if ((pDSRoleInfo->MachineRole == DsRole_RoleMemberWorkstation) ||
                (pDSRoleInfo->MachineRole == DsRole_RoleMemberServer) ||
                (pDSRoleInfo->MachineRole == DsRole_RoleBackupDomainController) ||
                (pDSRoleInfo->MachineRole == DsRole_RolePrimaryDomainController)) {

            size_t len = wcslen(pDSRoleInfo->DomainNameFlat);

            /* allocate buffer for str + null termination */
            delete []DomainName;
            DomainName = new TCHAR[len + 1];
#ifdef UNICODE
            _tcscpy_s(DomainName, len + 1, pDSRoleInfo->DomainNameFlat);
#else
            /* copy unicode buffer */
            WideCharToMultiByte(CP_UTF8, 0, pDSRoleInfo->DomainNameFlat, -1, DomainName, len, NULL, NULL);

            /* add null termination */
            DomainName[len] = '\0';
#endif
            /*
             * Member of a domain. Display it in debug mode.
             */
            debug(_T("Member of Domain %s\n"), DomainName);
            debug(_T("Into forest %s\n"), pDSRoleInfo->DomainForestName);

        } else {
            debug(_T("Not a Domain member\n"));
        }
    } else
        debug(_T("GetDomainName: ERROR DsRoleGetPrimaryDomainInformation returned: %s\n"), Get_WIN32_ErrorMessage(netret));

    /*
     * Free the allocated memory.
     */
    if (pDSRoleInfo != NULL)
        DsRoleFreeMemory(pDSRoleInfo);

    return DomainName;
}
//-------------------------------------------------------------------
int
add_User_Group(wchar_t * Group)
{
    wchar_t **array;

    if (User_Groups_Count == 0) {
        User_Groups = new wchar_t*[1];
        *User_Groups = NULL;
        ++User_Groups_Count;
    }
    array = User_Groups;
    while (*array) {
        if (wcscmp(Group, *array) == 0)
            return 0;
        ++array;
    }
    wchar_t **tmp = new wchar_t*[User_Groups_Count + 1];
    for(int i = 0; i < User_Groups_Count; ++i)
      tmp[i] = User_Groups[i];
    tmp[User_Groups_Count] = NULL;
    tmp[User_Groups_Count - 1] = new wchar_t[wcslen(Group) + 1];
    wcscpy(tmp[User_Groups_Count - 1], Group);
    delete []User_Groups;
    User_Groups = tmp;
    ++User_Groups_Count;

    return 1;
}
//-------------------------------------------------------------------
/* returns 0 on match, -1 if no match */
static int
wccmparray(const wchar_t * str, const wchar_t * const* array)
{
    while (*array) {
        debug(_T("Windows group: %s, Squid group: %s\n"), str, *array);
        if (wcscmp(str, *array) == 0)
            return 0;
        ++array;
    }
    return -1;
}
//-------------------------------------------------------------------
/* returns 0 on match, -1 if no match */
static int
wcstrcmparray(const wchar_t * str, const TCHAR **array)
{
    while (*array) {
      debug(_T("Windows group: %s, Squid group: %s\n"), str, *array);
#ifdef UNICODE
      if ((use_case_insensitive_compare ? _wcsicmp(str, *array) : wcscmp(str, *array)) == 0)
        return 0;
#else
      WCHAR wszGroup[GNLEN + 1];  // Unicode Group
      MultiByteToWideChar(CP_UTF8, 0, *array,
                            strlen(*array) + 1, wszGroup, SIZE_A(wszGroup));
      if ((use_case_insensitive_compare ? _wcsicmp(str, wszGroup) : wcscmp(str, wszGroup)) == 0)
            return 0;
#endif
      ++array;
    }
    return -1;
}
//-------------------------------------------------------------------
template <typename T>
void safe_free(T& v)
{
  delete []v;
  v = 0;
}
//-------------------------------------------------------------------
HRESULT
Recursive_Memberof(IADs * pObj)
{
    VARIANT var;
    long lBound, uBound;
    HRESULT hr;

    VariantInit(&var);
    hr = pObj->Get(L"memberOf", &var);
    if (SUCCEEDED(hr)) {
        if (VT_BSTR == var.vt) {
            if (add_User_Group(var.bstrVal)) {
                wchar_t *Group_Path;
                IADs *pGrp;

                Group_Path = GetLDAPPath(var.bstrVal, GC_MODE);
                hr = ADsGetObject(Group_Path, IID_IADs, (void **) &pGrp);
                if (SUCCEEDED(hr)) {
                    hr = Recursive_Memberof(pGrp);
                    pGrp->Release();
                    safe_free(Group_Path);
                    Group_Path = GetLDAPPath(var.bstrVal, LDAP_MODE);
                    hr = ADsGetObject(Group_Path, IID_IADs, (void **) &pGrp);
                    if (SUCCEEDED(hr)) {
                        hr = Recursive_Memberof(pGrp);
                        pGrp->Release();
                    } else
                        debug(_T("Recursive_Memberof: ERROR ADsGetObject for %s failed: %s\n"), Group_Path, Get_WIN32_ErrorMessage(hr));
                } else
                    debug(_T("Recursive_Memberof: ERROR ADsGetObject for %s failed: %s\n"), Group_Path, Get_WIN32_ErrorMessage(hr));
                safe_free(Group_Path);
            }
        } else {
            if (SUCCEEDED(SafeArrayGetLBound(V_ARRAY(&var), 1, &lBound)) &&
                    SUCCEEDED(SafeArrayGetUBound(V_ARRAY(&var), 1, &uBound))) {
                VARIANT elem;
                while (lBound <= uBound) {
                    hr = SafeArrayGetElement(V_ARRAY(&var), &lBound, &elem);
                    if (SUCCEEDED(hr)) {
                        if (add_User_Group(elem.bstrVal)) {
                            wchar_t *Group_Path;
                            IADs *pGrp;

                            Group_Path = GetLDAPPath(elem.bstrVal, GC_MODE);
                            hr = ADsGetObject(Group_Path, IID_IADs, (void **) &pGrp);
                            if (SUCCEEDED(hr)) {
                                hr = Recursive_Memberof(pGrp);
                                pGrp->Release();
                                safe_free(Group_Path);
                                Group_Path = GetLDAPPath(elem.bstrVal, LDAP_MODE);
                                hr = ADsGetObject(Group_Path, IID_IADs, (void **) &pGrp);
                                if (SUCCEEDED(hr)) {
                                    hr = Recursive_Memberof(pGrp);
                                    pGrp->Release();
                                    safe_free(Group_Path);
                                } else
                                    debug(_T("Recursive_Memberof: ERROR ADsGetObject for %s failed: %s\n"), Group_Path, Get_WIN32_ErrorMessage(hr));
                            } else
                                debug(_T("Recursive_Memberof: ERROR ADsGetObject for %s failed: %s\n"), Group_Path, Get_WIN32_ErrorMessage(hr));
                            safe_free(Group_Path);
                        }
                        VariantClear(&elem);
                    } else {
                        debug(_T("Recursive_Memberof: ERROR SafeArrayGetElement failed: %s\n"), Get_WIN32_ErrorMessage(hr));
                        VariantClear(&elem);
                    }
                    ++lBound;
                }
            } else
                debug(_T("Recursive_Memberof: ERROR SafeArrayGetxBound failed: %s\n"), Get_WIN32_ErrorMessage(hr));
        }
        VariantClear(&var);
    } else {
        if (hr != E_ADS_PROPERTY_NOT_FOUND)
            debug(_T("Recursive_Memberof: ERROR getting memberof attribute: %s\n"), Get_WIN32_ErrorMessage(hr));
    }
    return hr;
}
//-------------------------------------------------------------------
static wchar_t **
build_groups_DN_array(const TCHAR **array, TCHAR *userdomain)
{
    wchar_t *wc = NULL;
    int source_group_format;
    TCHAR Group[GNLEN + 1];

    wchar_t **wc_array, **entry;

    entry = wc_array = new wchar_t *[numberofgroups + 1];

    while (*array) {
        if (_tcschr(*array, _T('/')) != NULL) {
          _tcsncpy(Group, *array, GNLEN);
          source_group_format = ADS_NAME_TYPE_CANONICAL;
        } else {
            source_group_format = ADS_NAME_TYPE_NT4;
            if (_tcschr(*array, _T('\\')) == NULL) {
                _tcscpy(Group, userdomain);
                _tcscat(Group, _T("\\"));
                _tcsncat(Group, *array, GNLEN - _tcslen(userdomain) - 1);
            } else
              _tcsncpy(Group, *array, GNLEN);
        }
#ifdef UNICODE
        *entry = My_NameTranslate(Group, source_group_format, ADS_NAME_TYPE_1779);
#else
        int wcsize = MultiByteToWideChar(CP_UTF8, 0, Group, -1, wc, 0);
        wc = new wchar_t[wcsize];
        MultiByteToWideChar(CP_UTF8, 0, Group, -1, wc, wcsize);
        *entry = My_NameTranslate(wc, source_group_format, ADS_NAME_TYPE_1779);
        safe_free(wc);
#endif
        ++array;
        if (*entry == NULL) {
            debug(_T("build_groups_DN_array: cannot get DN for '%s'.\n"), Group);
            continue;
        }
        ++entry;
    }
    *entry = NULL;
    return wc_array;
}
//-------------------------------------------------------------------
/* returns 1 on success, 0 on failure */
int
Valid_Local_Groups(TCHAR *UserName, const TCHAR **Groups)
{
    int result = 0;
    TCHAR *Domain_Separator;
    WCHAR wszUserName[UNLEN + 1];   /* Unicode user name */

    LPLOCALGROUP_USERS_INFO_0 pBuf;
    LPLOCALGROUP_USERS_INFO_0 pTmpBuf;
    DWORD dwLevel = 0;
    DWORD dwFlags = LG_INCLUDE_INDIRECT;
    DWORD dwPrefMaxLen = -1;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    NET_API_STATUS nStatus;
    DWORD i;
    DWORD dwTotalCount = 0;
    LPBYTE pBufTmp = NULL;

    if ((Domain_Separator = _tcschr(UserName, _T('/'))) != NULL)
        *Domain_Separator = _T('\\');

    debug(_T("Valid_Local_Groups: checking group membership of '%s'.\n"), UserName);
#ifdef UNICODE
  _tcscpy_s(wszUserName, UserName);
#else
    /* Convert ANSI User Name and Group to Unicode */

    MultiByteToWideChar(CP_UTF8, 0, UserName,
                        strlen(UserName) + 1, wszUserName, SIZE_A(wszUserName));
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
                                    &pBufTmp,
                                    dwPrefMaxLen,
                                    &dwEntriesRead,
                                    &dwTotalEntries);
    pBuf = (LPLOCALGROUP_USERS_INFO_0) pBufTmp;
    /*
     * If the call succeeds,
     */
    if (nStatus == NERR_Success) {
        if ((pTmpBuf = pBuf) != NULL) {
            for (i = 0; i < dwEntriesRead; ++i) {
                assert(pTmpBuf != NULL);
                if (pTmpBuf == NULL) {
                    result = 0;
                    break;
                }
                if (wcstrcmparray(pTmpBuf->lgrui0_name, Groups) == 0) {
                    result = 1;
                    break;
                }
                ++pTmpBuf;
                ++dwTotalCount;
            }
        }
    } else {
        debug(_T("Valid_Local_Groups: ERROR NetUserGetLocalGroups returned: %s\n"), Get_WIN32_ErrorMessage(nStatus));
        result = 0;
    }
    /*
     * Free the allocated memory.
     */
    if (pBuf != NULL)
        NetApiBufferFree(pBuf);
    return result;
}
//-------------------------------------------------------------------
/* returns 1 on success, 0 on failure */
int
Valid_Global_Groups(TCHAR *UserName, const TCHAR **Groups)
{
    int result = 0;
    WCHAR wszUser[DNLEN + UNLEN + 2];   /* Unicode user name */
    TCHAR NTDomain[DNLEN + UNLEN + 2];

    TCHAR *domain_qualify = NULL;
    TCHAR User[DNLEN + UNLEN + 2];
    size_t j;

    wchar_t *User_DN, *User_LDAP_path, *User_PrimaryGroup;
    wchar_t **wszGroups, **tmp;
    IADs *pUser;
    HRESULT hr;

    _tcsncpy(NTDomain, UserName, SIZE_A(NTDomain));

    for (j = 0; j < _tcslen(NTV_VALID_DOMAIN_SEPARATOR); ++j) {
        if ((domain_qualify = _tcschr(NTDomain, NTV_VALID_DOMAIN_SEPARATOR[j])) != NULL)
            break;
    }
    if (domain_qualify == NULL) {
      _tcsncpy(User, DefaultDomain, DNLEN);
      _tcscat(User, _T("\\"));
      _tcsncat(User, UserName, UNLEN);
      _tcsncpy(NTDomain, DefaultDomain, DNLEN);
    } else {
        domain_qualify[0] = _T('\\');
        _tcsncpy(User, NTDomain, DNLEN + UNLEN + 2);
        domain_qualify[0] = _T('\0');
    }

    debug(_T("Valid_Global_Groups: checking group membership of '%s'.\n"), User);

#ifdef UNICODE
    _tcscpy_s(wszUser, User);
#else
    /* Convert ANSI User Name to Unicode */
    MultiByteToWideChar(CP_UTF8, 0, User,
                        strlen(User) + 1, wszUser,
                        sizeof(wszUser) / sizeof(wszUser[0]));
#endif
    /* Get CN of User */
    if ((User_DN = My_NameTranslate(wszUser, ADS_NAME_TYPE_NT4, ADS_NAME_TYPE_1779)) == NULL) {
        debug(_T("Valid_Global_Groups: cannot get DN for '%s'.\n"), User);
        return result;
    }
    wszGroups = build_groups_DN_array(Groups, NTDomain);

    User_LDAP_path = GetLDAPPath(User_DN, GC_MODE);

    hr = ADsGetObject(User_LDAP_path, IID_IADs, (void **) &pUser);
    if (SUCCEEDED(hr)) {
        wchar_t *User_PrimaryGroup_Path;
        IADs *pGrp;

        User_PrimaryGroup = Get_primaryGroup(pUser);
        if (User_PrimaryGroup == NULL)
            debug(_T("Valid_Global_Groups: cannot get Primary Group for '%s'.\n"), User);
        else {
            add_User_Group(User_PrimaryGroup);
            User_PrimaryGroup_Path = GetLDAPPath(User_PrimaryGroup, GC_MODE);
            hr = ADsGetObject(User_PrimaryGroup_Path, IID_IADs, (void **) &pGrp);
            if (SUCCEEDED(hr)) {
                hr = Recursive_Memberof(pGrp);
                pGrp->Release();
                safe_free(User_PrimaryGroup_Path);
                User_PrimaryGroup_Path = GetLDAPPath(User_PrimaryGroup, LDAP_MODE);
                hr = ADsGetObject(User_PrimaryGroup_Path, IID_IADs, (void **) &pGrp);
                if (SUCCEEDED(hr)) {
                    hr = Recursive_Memberof(pGrp);
                    pGrp->Release();
                } else
                    debug(_T("Valid_Global_Groups: ADsGetObject for %s failed, ERROR: %s\n"), User_PrimaryGroup_Path, Get_WIN32_ErrorMessage(hr));
            } else
                debug(_T("Valid_Global_Groups: ADsGetObject for %s failed, ERROR: %s\n"), User_PrimaryGroup_Path, Get_WIN32_ErrorMessage(hr));
            safe_free(User_PrimaryGroup_Path);
        }
        hr = Recursive_Memberof(pUser);
        pUser->Release();
        safe_free(User_LDAP_path);
        User_LDAP_path = GetLDAPPath(User_DN, LDAP_MODE);
        hr = ADsGetObject(User_LDAP_path, IID_IADs, (void **) &pUser);
        if (SUCCEEDED(hr)) {
            hr = Recursive_Memberof(pUser);
            pUser->Release();
        } else
            debug(_T("Valid_Global_Groups: ADsGetObject for %s failed, ERROR: %s\n"), User_LDAP_path, Get_WIN32_ErrorMessage(hr));

        tmp = User_Groups;
        while (*tmp) {
            if (wccmparray(*tmp, wszGroups) == 0) {
                result = 1;
                break;
            }
            ++tmp;
        }
    } else
        debug(_T("Valid_Global_Groups: ADsGetObject for %s failed, ERROR: %s\n"), User_LDAP_path, Get_WIN32_ErrorMessage(hr));

    safe_free(User_DN);
    safe_free(User_LDAP_path);
    safe_free(User_PrimaryGroup);
    tmp = wszGroups;
    while (*tmp) {
        safe_free(*tmp);
        ++tmp;
    }
    safe_free(wszGroups);

    tmp = User_Groups;
    while (*tmp) {
        safe_free(*tmp);
        ++tmp;
    }
    safe_free(User_Groups);
    User_Groups_Count = 0;

    return result;
}
//-------------------------------------------------------------------
static void
usage(const TCHAR *program)
{
  _ftprintf(stderr, _T("Usage: %s [-D domain][-G][-c][-d][-h]\n")
            _T(" -D    default user Domain\n")
            _T(" -G    enable Active Directory Global group mode\n")
            _T(" -c    use case insensitive compare (local mode only)\n")
            _T(" -d    enable debugging\n")
            _T(" -h    this message\n"),
            program);
}
//-------------------------------------------------------------------
int opterr = 1,         /* if error message should be printed */
    optind = 1,            /* index into parent argv vector */
    optopt,           /* character checked for validity */
    optreset;         /* reset getopt */
TCHAR *optarg;           /* argument associated with option */

#define BADCH   (int)_T('?')
#define BADARG  (int)_T(':')
#define EMSG    (TCHAR*)_T("")
//-------------------------------------------------------------------
/*
 * getopt --
 *      Parse argc/argv argument vector.
 */
int
getopt(int nargc, TCHAR *const *nargv, const TCHAR *ostr)
{
    static TCHAR *place = EMSG;  /* option letter processing */
    const TCHAR *oli;          /* option letter list index */

    if (optreset || !*place) {  /* update scanning pointer */
        optreset = 0;
        if (optind >= nargc || *(place = nargv[optind]) != '-') {
            place = EMSG;
            return (-1);
        }
        if (place[1] && *++place == '-') {  /* found "--" */
            ++optind;
            place = EMSG;
            return (-1);
        }
    }               /* option letter okay? */
    if ((optopt = (int) *place++) == (int) _T(':') ||
            !(oli = _tcschr(ostr, optopt))) {
        /*
         * if the user didn't specify '-' as an option,
         * assume it means -1.
         */
        if (optopt == (int) _T('-'))
            return (-1);
        if (!*place)
            ++optind;
        if (opterr && *ostr != _T(':'))
            (void) _ftprintf(stderr,
                           _T("%s: illegal option -- %c\n"), _T(__FILE__), optopt);
        return (BADCH);
    }
    if (*++oli != _T(':')) {    /* don't need argument */
        optarg = NULL;
        if (!*place)
            ++optind;
    } else {            /* need an argument */
        if (*place)     /* no white space */
            optarg = place;
        else if (nargc <= ++optind) {   /* no arg */
            place = EMSG;
            if (*ostr == _T(':'))
                return (BADARG);
            if (opterr)
                (void) _ftprintf(stderr,
                               _T("%s: option requires an argument -- %c\n"),
                               _T(__FILE__), optopt);
            return (BADCH);
        } else          /* white space */
            optarg = nargv[optind];
        place = EMSG;
        ++optind;
    }
    return (optopt);        /* dump back option letter */
}
//-------------------------------------------------------------------
void
process_options(int argc, TCHAR *argv[])
{
    int opt;

    opterr = 0;
    while (-1 != (opt = getopt(argc, argv, _T("D:Gcdh")))) {
        switch (opt) {
        case _T('D'):
            DefaultDomain = _tcsdup(optarg); //, DNLEN + 1);
            _tcslwr(DefaultDomain);
            break;
        case _T('G'):
            use_global = 1;
            break;
        case _T('c'):
            use_case_insensitive_compare = 1;
            break;
        case _T('d'):
            debug_enabled = 1;
            break;
        case _T('h'):
            usage(argv[0]);
            exit(0);
        case _T('?'):
            opt = optopt;
        /* fall thru to default */
        default:
          _ftprintf(stderr, _T("%s: FATAL: Unknown option: -%c. Exiting\n"), program_name, opt);
            usage(argv[0]);
            exit(1);
            break;      /* not reached */
        }
    }
    return;
}
//-------------------------------------------------------------------
static bool isUnicode;
bool onUnicode() { return isUnicode; }
//-------------------------------------------------------------------
// http://www.brescianet.com/appunti/vari/unicode.htm
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
//-------------------------------------------------------------------
bool isutf2(unsigned char c)
{
	return (c & 0x80) == 0x80;
}
//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
LPTSTR convert(LPCTSTR username)
{
#ifndef _UNICODE
	return username;
#else
	char p[1024];
	for (int i = 0; i < sizeof(p); ++i)
	{
		p[i] = (char)username[i];
		if (!p[i])
			break;
	}
    if(!isUnicode)
      	checkUnicode((LPCSTR)p);
    UINT cp = onUnicode() ? CP_UTF8 : CP_ACP;
	return makeW_char(p, cp);
#endif
}
//-------------------------------------------------------------------
void convertGrp(const TCHAR *groups[], int numberofgroups)
{
#ifndef _UNICODE
	return;
#else
	for (int i = 0; i < numberofgroups; ++i)
		groups[i] = convert(groups[i]);
#endif
}
//-------------------------------------------------------------------
int _tmain(int argc, TCHAR *argv[])
{
  TCHAR *p;
  TCHAR buf[HELPER_INPUT_BUFFER];
  TCHAR *username;
  TCHAR *group;
  const TCHAR *groups[512];
  int n;

    if (argc > 0) {     /* should always be true */
        program_name = _tcsrchr(argv[0], _T('/'));
        if (program_name == NULL)
            program_name = argv[0];
    } else {
        program_name = _T("(unknown)");
    }
    mypid = _getpid();

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    /* Check Command Line */
    process_options(argc, argv);

    if (use_global) {
        if ((machinedomain = GetDomainName()) == NULL) {
          _ftprintf(stderr, _T("%s: FATAL: Can't read machine domain\n"), program_name);
            exit(1);
        }
        _tcslwr(machinedomain);
        if (!DefaultDomain)
            DefaultDomain = _tcsdup(machinedomain);
    }
//    debug(_T("%s ") VERSION _T(" ") SQUID_BUILD_INFO _T(" starting up...\n"), argv[0]);
    debug(_T("%s Ver.3.5 starting up...\n"), argv[0]);
    if (use_global)
        debug(_T("Domain Global group mode enabled using '%s' as default domain.\n"), DefaultDomain);
    if (use_case_insensitive_compare)
        debug(_T("Warning: running in case insensitive mode !!!\n"));

    atexit(CloseCOM);

    /* Main Loop */
    while (_fgetts(buf, HELPER_INPUT_BUFFER, stdin)) {
        if (NULL == _tcschr(buf, _T('\n'))) {
            /* too large message received.. skip and deny */
          _ftprintf(stderr, _T("%s: ERROR: Too large: %s\n"), argv[0], buf);
            while (_fgetts(buf, HELPER_INPUT_BUFFER, stdin)) {
              _ftprintf(stderr, _T("%s: ERROR: Too large..: %s\n"), argv[0], buf);
                if (_tcschr(buf, '\n') != NULL)
                    break;
            }
            SEND_BH(HLP_MSG(_T("Invalid Request. Too Long.")));
            continue;
        }
        if ((p = _tcschr(buf, _T('\n'))) != NULL)
            *p = _T('\0');      /* strip \n */
        if ((p = _tcschr(buf, _T('\r'))) != NULL)
            *p = _T('\0');      /* strip \r */

        debug(_T("Got '%s' from Squid (length: %d).\n"), buf, _tcslen(buf));

        if (buf[0] == _T('\0')) {
            SEND_BH(HLP_MSG(_T("Invalid Request. No Input.")));
            continue;
        }
        username = _tcstok(buf, _T(" "));
        isUnicode = false;
        for (n = 0; (group = _tcstok(NULL, _T(" "))) != NULL; ++n) {
            rfc1738_unescape(group);
            groups[n] = group;
        }
        groups[n] = NULL;
        numberofgroups = n;

        if (NULL == username) {
            SEND_BH(HLP_MSG(_T("Invalid Request. No Username.")));
            continue;
        }
//        debug(_T("Username before = '%s'\n"), username);
        rfc1738_unescape(username);
//        debug(_T("Username after = '%s'\n"), username);
		LPTSTR user = convert(username);
		convertGrp(groups, numberofgroups);
        if ((use_global ? Valid_Global_Groups(user, groups) : Valid_Local_Groups(user, groups))) {
            SEND_OK(_T(""));
        } else {
            SEND_ERR(_T(""));
        }
        err = 0;
#ifdef _UNICODE
		delete[]user;
		for (n = 0; n < numberofgroups; ++n)
			delete[]groups[n];
#endif
	}
    return 0;
}


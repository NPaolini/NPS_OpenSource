/*
 * Copyright (C) 1996-2018 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */
#define _CRT_SECURE_NO_WARNINGS

//#include "squid.h"
#include "rfc1738.h"

//#if HAVE_STRING_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#endif

/*
 *  RFC 1738 defines that these characters should be escaped, as well
 *  any non-US-ASCII character or anything between 0x00 - 0x1F.
 */
static TCHAR rfc1738_unsafe_chars[] = {
    (TCHAR) 0x3C,        /* < */
    (TCHAR) 0x3E,        /* > */
    (TCHAR) 0x22,        /* " */
    (TCHAR) 0x23,        /* # */
#if 0               /* done in code */
    (TCHAR) 0x20,        /* space */
    (TCHAR) 0x25,        /* % */
#endif
    (TCHAR) 0x7B,        /* { */
    (TCHAR) 0x7D,        /* } */
    (TCHAR) 0x7C,        /* | */
    (TCHAR) 0x5C,        /* \ */
    (TCHAR) 0x5E,        /* ^ */
    (TCHAR) 0x7E,        /* ~ */
    (TCHAR) 0x5B,        /* [ */
    (TCHAR) 0x5D,        /* ] */
    (TCHAR) 0x60,        /* ` */
    (TCHAR) 0x27         /* ' */
};

static TCHAR rfc1738_reserved_chars[] = {
    (TCHAR) 0x3b,        /* ; */
    (TCHAR) 0x2f,        /* / */
    (TCHAR) 0x3f,        /* ? */
    (TCHAR) 0x3a,        /* : */
    (TCHAR) 0x40,        /* @ */
    (TCHAR) 0x3d,        /* = */
    (TCHAR) 0x26         /* & */
};

/*
 *  rfc1738_escape - Returns a static buffer contains the RFC 1738
 *  compliant, escaped version of the given url.
 */
TCHAR *
rfc1738_do_escape(const TCHAR *url, int flags)
{
    static TCHAR *buf;
    static size_t bufsize = 0;
    const TCHAR *src;
    TCHAR *dst;
    unsigned int i, do_escape;

    if (buf == NULL || _tcslen(url) * 3 > bufsize) {
        delete []buf;
        bufsize = _tcslen(url) * 3 + 1;
        buf = new TCHAR[bufsize];
    }
    for (src = url, dst = buf; *src != _T('\0') && dst < (buf + bufsize - 1); src++, dst++) {

        /* a-z, A-Z and 0-9 are SAFE. */
        if ((*src >= _T('a') && *src <= _T('z')) || (*src >= _T('A') && *src <= _T('Z')) || (*src >= _T('0') && *src <= _T('9'))) {
            *dst = *src;
            continue;
        }

        do_escape = 0;

        /* RFC 1738 defines these chars as unsafe */
        if ((flags & RFC1738_ESCAPE_UNSAFE)) {
            for (i = 0; i < sizeof(rfc1738_unsafe_chars); i++) {
                if (*src == rfc1738_unsafe_chars[i]) {
                    do_escape = 1;
                    break;
                }
            }
            /* Handle % separately */
            if (!(flags & RFC1738_ESCAPE_NOPERCENT) && *src == _T('%'))
                do_escape = 1;
            /* Handle space separately */
            else if (!(flags & RFC1738_ESCAPE_NOSPACE) && *src <= _T(' '))
                do_escape = 1;
        }
        /* RFC 1738 defines these chars as reserved */
        if ((flags & RFC1738_ESCAPE_RESERVED) && do_escape == 0) {
            for (i = 0; i < sizeof(rfc1738_reserved_chars); i++) {
                if (*src == rfc1738_reserved_chars[i]) {
                    do_escape = 1;
                    break;
                }
            }
        }
        if ((flags & RFC1738_ESCAPE_CTRLS) && do_escape == 0) {
            /* RFC 1738 says any control chars (0x00-0x1F) are encoded */
            if ((unsigned char) *src <= (unsigned char) 0x1F)
                do_escape = 1;
            /* RFC 1738 says 0x7f is encoded */
            else if (*src == (TCHAR) 0x7F)
                do_escape = 1;
            /* RFC 1738 says any non-US-ASCII are encoded */
            else if (((unsigned char) *src >= (unsigned char) 0x80))
                do_escape = 1;
        }
        /* Do the triplet encoding, or just copy the TCHAR */
        if (do_escape == 1) {
            (void)_sntprintf(dst, (bufsize-(dst-buf)), _T("%%%02X"), (unsigned) *src);
            dst += 2; //sizeof(TCHAR) * 2;
        } else {
            *dst = *src;
        }
    }
    *dst = _T('\0');
    return (buf);
}

/*
 * Converts a ascii hex code into a binary character.
 */
static int
fromhex(TCHAR ch)
{
    if (ch >= _T('0') && ch <= _T('9'))
        return ch - _T('0');
    if (ch >= _T('a') && ch <= _T('f'))
        return ch - _T('a') + 10;
    if (ch >= _T('A') && ch <= _T('F'))
        return ch - _T('A') + 10;
    return -1;
}

/*
 *  rfc1738_unescape() - Converts escaped characters (%xy numbers) in
 *  given the string.  %% is a %. %ab is the 8-bit hexadecimal number "ab"
 */
void
rfc1738_unescape(TCHAR *s)
{
    int i, j;           /* i is write, j is read */
    for (i = j = 0; s[j]; i++, j++) {
        s[i] = s[j];
        if (s[j] != _T('%')) {
            /* normal case, nothing more to do */
        } else if (s[j + 1] == _T('%')) {   /* %% case */
            j++;        /* Skip % */
        } else {
            /* decode */
            int v1, v2, x;
            v1 = fromhex(s[j + 1]);
            if (v1 < 0)
                continue;  /* non-hex or \0 */
            v2 = fromhex(s[j + 2]);
            if (v2 < 0)
                continue;  /* non-hex or \0 */
            x = v1 << 4 | v2;
            if (x > 0 && x <= 255) {
                s[i] = x;
                j += 2;
            }
        }
    }
    s[i] = _T('\0');
}


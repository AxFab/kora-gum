/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2021  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 */
#include <stddef.h>
#include <gum/xml.h>
// Multibyte/wide character conversions

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Defined in header <stdlib.h>

static int uclen_(const char *str, size_t lg)
{
    int len = 1;
    unsigned s = (unsigned char)(*str);
    if (s >= 0xfe)
        return -1;
    else if (s >= 0xfc)
        len = 6;
    else if (s >= 0xf8)
        len = 5;
    else if (s >= 0xf0)
        len = 4;
    else if (s >= 0xe0)
        len = 3;
    else if (s >= 0xc0)
        len = 2;
    else if (s >= 0x80)
        return -1;
    if (lg < (unsigned)len)
        return -1;
    return len;
}



// returns the number of bytes in the next multibyte character
int uclen(const char* str, size_t lg)
{
    int i, len = uclen_(str, lg);
    for (i = 1; i < len; ++i) {
        if ((str[i] & 0xc0) != 0x80)
            return -1;
    }
    return len;
}


// Converts the next multibyte character to wide character
int mbtouc(uchar_t* unicode, const char* str, size_t lg)
{
    // mbstate_t ps;
    // return mbrtowc(pwc, str, lg, &ps);
    int len = uclen(str, lg);
    if (len == 1) {
        *unicode = *str;
        return 1;
    }
    if (len < 1)
        return -1;
    int l = 0, i = len;
    int wc = 0;
    while (i-- > 1)
        wc |= (((unsigned char)str[i] & 0x3f) << (6 * l++));
    wc |= (((unsigned char)str[0]) & ((1 << (6 - l)) - 1)) << (6 * l);
    *unicode = wc;
    return len;
}

// converts a wide character to its multibyte representation
int uctomb(char* str, uchar_t unicode)
{
    if (unicode < 0)
        return -1;
    else if (unicode < 0x80) {
        str[0] = unicode & 0x7f;
        return 1;
    } else if (unicode < 0x800) {
        str[0] = 0xc0 | ((unicode >> 6) & 0x1f);
        str[1] = 0x80 | (unicode & 0x3f);
        return 2;
    } else if (unicode < 0x10000) {
        str[0] = 0xe0 | ((unicode >> 12) & 0x0f);
        str[1] = 0x80 | ((unicode >> 6) & 0x3f);
        str[2] = 0x80 | (unicode & 0x3f);
        return 3;
    } else if (unicode < 0x400000) {
        str[0] = 0xf0 | ((unicode >> 18) & 0x07);
        str[1] = 0x80 | ((unicode >> 12) & 0x3f);
        str[2] = 0x80 | ((unicode >> 6) & 0x3f);
        str[3] = 0x80 | (unicode & 0x3f);
        return 4;
    } else if (unicode < 0x4000000) {
        str[0] = 0xf8 | ((unicode >> 24) & 0x03);
        str[1] = 0x80 | ((unicode >> 18) & 0x3f);
        str[2] = 0x80 | ((unicode >> 12) & 0x3f);
        str[3] = 0x80 | ((unicode >> 6) & 0x3f);
        str[4] = 0x80 | (unicode & 0x3f);
        return 5;
    } else {
        str[0] = 0xfc | ((unicode >> 30) & 0x01);
        str[1] = 0x80 | ((unicode >> 24) & 0x3f);
        str[2] = 0x80 | ((unicode >> 18) & 0x3f);
        str[3] = 0x80 | ((unicode >> 12) & 0x3f);
        str[4] = 0x80 | ((unicode >> 6) & 0x3f);
        str[5] = 0x80 | (unicode & 0x3f);
        return 6;
    }
}

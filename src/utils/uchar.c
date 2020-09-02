#include <stddef.h>

// Multibyte/wide character conversions

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Defined in header <stdlib.h>

static int mblen_(const char* str, size_t lg)
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
int umblen(const char* str, size_t lg)
{
    int i, len = mblen_(str, lg);
    for (i = 1; i < len; ++i) {
        if ((str[i] & 0xc0) != 0x80)
            return -1;
    }
    return len;
}


// Converts the next multibyte character to wide character
int mbtouc(int* pwc, const char* str, size_t lg)
{
    // mbstate_t ps;
    // return mbrtowc(pwc, str, lg, &ps);
    int len = umblen(str, lg);
    if (len == 1) {
        *pwc = *str;
        return 1;
    }
    if (len < 1)
        return -1;
    int l = 0, i = len;
    int wc = 0;
    while (i-- > 1)
        wc |= (((unsigned char)str[i] & 0x3f) << (6 * l++));
    wc |= (((unsigned char)str[0]) & ((1 << (6 - l)) - 1)) << (6 * l);
    *pwc = wc;
    return len;
}

// converts a wide character to its multibyte representation
int uctomb(char* str, wchar_t wc)
{
    if (wc < 0)
        return -1;
    else if (wc < 0x80) {
        str[0] = wc & 0x7f;
        return 1;
    }
    else if (wc < 0x800) {
        str[0] = 0xc0 | ((wc >> 6) & 0x1f);
        str[1] = 0x80 | (wc & 0x3f);
        return 2;
    }
    else if (wc < 0x10000) {
        str[0] = 0xe0 | ((wc >> 12) & 0x0f);
        str[1] = 0x80 | ((wc >> 6) & 0x3f);
        str[2] = 0x80 | (wc & 0x3f);
        return 3;
    }
    else if (wc < 0x400000) {
        str[0] = 0xf0 | ((wc >> 18) & 0x07);
        str[1] = 0x80 | ((wc >> 12) & 0x3f);
        str[2] = 0x80 | ((wc >> 6) & 0x3f);
        str[3] = 0x80 | (wc & 0x3f);
        return 4;
    }
    else if (wc < 0x4000000) {
        str[0] = 0xf8 | ((wc >> 24) & 0x03);
        str[1] = 0x80 | ((wc >> 18) & 0x3f);
        str[2] = 0x80 | ((wc >> 12) & 0x3f);
        str[3] = 0x80 | ((wc >> 6) & 0x3f);
        str[4] = 0x80 | (wc & 0x3f);
        return 5;
    }
    else {
        str[0] = 0xfc | ((wc >> 30) & 0x01);
        str[1] = 0x80 | ((wc >> 24) & 0x3f);
        str[2] = 0x80 | ((wc >> 18) & 0x3f);
        str[3] = 0x80 | ((wc >> 12) & 0x3f);
        str[4] = 0x80 | ((wc >> 6) & 0x3f);
        str[5] = 0x80 | (wc & 0x3f);
        return 6;
    }
}


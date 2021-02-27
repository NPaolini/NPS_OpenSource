//----------- make_keydat.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "base64.h"
//----------------------------------------------------------------------------
static const char base64_chars[] =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";
//----------------------------------------------------------------------------
static char unbase64_chars[256];
//----------------------------------------------------------------------------
static void initializeDecode()
{
  for(uint i = 0; i < SIZE_A(base64_chars); ++i)
    unbase64_chars[base64_chars[i]] = i;
}
//----------------------------------------------------------------------------
static char find(char c)
{
#if 1
  return unbase64_chars[c];
#else
  for(uint i = 0; i < SIZE_A(base64_chars); ++i)
    if(c == base64_chars[i])
      return i;
  return 0;
#endif
}
//----------------------------------------------------------------------------
/*
la dimensione encodata è data da (n / 3 + 1) * 4;
al contrario è n / 4 * 3;

*/
//----------------------------------------------------------------------------
static inline int is_base64(unsigned char c)
{
  int res = (isalnum(c) || (c == '+') || (c == '/')) ? 1 : ((0xa == (c) || 0xd == (c)) ? -1 : 0);
  return res;
}
//----------------------------------------------------------------------------
//std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)
uint base64_encode(LPSTR target, LPCSTR bytes_to_encode, uint in_len)
{
  int i = 0, j = 0;
  unsigned char char_array_3[3], char_array_4[4];

  LPSTR ret = target;
  while (in_len--)
  {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3)
    {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++, ++ret)
        *ret = base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++, ++ret)
      *ret = base64_chars[char_array_4[j]];

    while((i++ < 3))
      *ret++ = '=';

  }
  *ret = 0;
  return uint(ret - target);
}
//----------------------------------------------------------------------------
//std::string base64_decode(std::string const& encoded_string)
uint base64_decode(LPSTR target, LPCSTR encoded_string, uint in_len)
{
  static bool inizialized = false;
  if(!inizialized) {
    inizialized = true;
    initializeDecode();
    }
  int i = 0, j = 0, in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  LPSTR ret = target;

  while (in_len-- && ( encoded_string[in_] != '='))
  {
    int res = is_base64(encoded_string[in_]);
    if(!res)
      break;
    if(-1 == res) {
      in_++;
      }
    else {
      char_array_4[i++] = encoded_string[in_]; in_++;
      if (i ==4) {
        for (i = 0; i <4; i++)
          char_array_4[i] = find(char_array_4[i]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (i = 0; (i < 3); i++, ++ret)
          *ret = char_array_3[i];
        i = 0;
      }
    }
  }

  if (i)
  {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++, ++ret)
      *ret = char_array_3[j];
  }
  *ret = 0;
  return uint(ret - target);
}
//----------------------------------------------------------------------------
void split_encoded(LPSTR target, LPCSTR source, uint lenRow)
{
  uint i = 0;
  while(*source) {
    if(i++ == lenRow) {
      *target++ = '\r';
      *target++ = '\n';
      i = 1;
      }
    *target++ = *source++;
    }
  *target = 0;
}

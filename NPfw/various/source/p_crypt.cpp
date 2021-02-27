//------ p_cript.cpp --------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_util.h"
#include "p_file.h"
#include "p_date.h"
//----------------------------------------------------------------------------
static WORD crypt_noZ_2(const BYTE* s, BYTE* d, int k, int l, int& pos)
{
  WORD crc = 0;
  for(int i = 0; i < l; ++i, ++pos) {
    crc += s[pos];
    d[pos] = (BYTE)(s[pos] ^ k ^ (pos + 1));
    }
 return crc;
}
//----------------------------------------------------------------------------
static WORD crypt_noZ_3(const BYTE* s, BYTE* d, int l, const BYTE* k, int dimKey)
{
  int currKey = 0;
  WORD crc = 0;
  for(int i = 0; i < l; ++i) {
    crc += s[i];
    d[i] = (BYTE)(s[i] ^ k[currKey] ^ (BYTE)(i + 1));
    ++currKey;
    currKey %= dimKey;
    }
 return crc;
}
//----------------------------------------------------------------------------
typedef unsigned __int64 uint_64;
//----------------------------------------------------------------------------
static uint_64 roll(uint_64 k)
{
  uint_64 t = toBool(k & 0x8000000000000000);
  k <<= 1;
  return k | t;
}
//----------------------------------------------------------------------------
WORD cryptBufferNoZ(LPBYTE target, LPCBYTE source, DWORD dim, LPCBYTE key, int dimKey, uint step)
{
  WORD crc = 0;
  if(step > 1) {
    int pos = 0;
    int currKey = 0;
    while(pos < (int)dim) {
      for(int j = 1; j <= (int)step && pos < (int)dim; ++j) {
        j = min(j, (int)dim - pos);
        if(j > 0) {
          crc += crypt_noZ_2(source, target, key[currKey], j, pos);
          ++currKey;
          currKey %= dimKey;
          }
        }
      }
    }
  else {
    step = 1;
    crc = crypt_noZ_3(source, target, dim, key, dimKey);
    }
  uint_64 k = dim * dimKey * step;
  #define MAX_SHIFT 59
  uint nCopy = (MAX_SHIFT / dimKey + 1) * dimKey;
  for(uint i = 0; i < nCopy; ++i) {
    int j = i % MAX_SHIFT;
    k += uint_64(key[i % dimKey]) << j;
    }
  for(int i = 0; i < (int)dim; ++i) {
    k = roll(k);
    target[i] ^= k;
    crc += target[i];
    }
  return crc;
}
//----------------------------------------------------------------------------
WORD cryptBuffer(LPBYTE target, LPCBYTE source, DWORD dim, LPCBYTE key, int dimKey, uint step)
{
  target[dim] = 0;
  return cryptBufferNoZ(target, source, dim, key, dimKey, step);
}
//----------------------------------------------------------------------------
WORD crypt_noZ(const BYTE* s, BYTE* d, int k, int l)
{
  return cryptBufferNoZ(d, s, l, (LPCBYTE)&k, sizeof(k), 1);
}
//----------------------------------------------------------------------------
WORD crypt(const BYTE* s, BYTE* d, int k, int l)
{
  WORD result = crypt_noZ(s, d, k, l);
  d[l] = 0;
  return result;
}
//----------------------------------------------------------------------------
bool decryptFile(LPCTSTR file, LPCBYTE key, int dimKey, uint step, infoFileCr& result)
{
  result.buff = 0;
  result.crc = (DWORD)-3;
  if(!dimKey || !step)
    return false;

  P_File pf(file, P_READ_ONLY);
  result.crc = (DWORD)-2;
  if(!pf.P_open())
    return false;

  int dim = (int)pf.get_len();
  result.crc = (DWORD)-1;
  if(!dim)
    return false;

  if(result.header && result.lenHeader) {
    dim -= result.lenHeader;

    LPBYTE header = new BYTE[result.lenHeader];
    pf.P_read(header, result.lenHeader);
    int mismatch = memcmp(header, result.header, result.lenHeader);
    delete []header;
    if(mismatch)
      return false;
    }

  LPBYTE p = new BYTE[dim + 2];
  pf.P_read(p, dim);

  result.crc = cryptBuffer(p, dim, key, dimKey, step);
  result.buff = p;
  result.dim = dim;
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool cryptFile(LPCTSTR file, LPCBYTE key, int dimKey, uint step, infoFileCr& source, const FILETIME* setData)
{
  source.crc = (DWORD)-3;
  if(!dimKey || !step)
    return false;

  P_File pf(file, P_CREAT);
  source.crc = (DWORD)-2;
  if(!pf.P_open())
    return false;

  LPBYTE p = source.buff;

  source.crc = cryptBuffer(p, source.dim, key, dimKey, step);

  if(source.header && source.lenHeader)
    pf.P_write(source.header, source.lenHeader);

  pf.P_write(p, source.dim);
  if(setData)
    pf.setData(*setData);

  return true;
}
//----------------------------------------------------------------------------
bool cryptFileCopy(LPCTSTR fileTarget, LPCTSTR fileSource, const infoFileCrCopy& info)
{
  P_File pf(fileSource, P_READ_ONLY);
  if(!pf.P_open())
    return false;

  infoFileCr result;
  result.header = info.header;
  result.lenHeader = info.lenHeader;

  result.dim = (int)pf.get_len();
  if(!result.dim)
    return false;


  result.buff = new BYTE[result.dim + 2];
  pf.P_read(result.buff, result.dim);
  FILETIME ft;
  pf.getData(ft);

  bool success = cryptFile(fileTarget, info.key, info.dimKey, info.step, result, &ft);
  delete []result.buff;
  return success;
}
//----------------------------------------------------------------------------
bool decryptFileCopy(LPCTSTR fileTarget, LPCTSTR fileSource, const infoFileCrCopy& info)
{
  infoFileCr result;
  result.header = info.header;
  result.lenHeader = info.lenHeader;

  if(!decryptFile(fileSource, info.key, info.dimKey, info.step, result))
    return false;

  P_File pf(fileTarget, P_CREAT);
  bool success = pf.P_open();
  if(success) {
    pf.P_write(result.buff, result.dim);
    FILETIME ft;
    do {
      P_File pf2(fileSource, P_READ_ONLY);
      pf2.P_open();
      pf2.getData(ft);
      } while(false);
    pf.setData(ft);
    }
  delete []result.buff;
  return success;
}
//----------------------------------------------------------------------------
bool cryptToFile(LPCTSTR fileTarget, const infoFileCrCopyOne& info)
{
  infoFileCr result;
  result.header = info.header;
  result.lenHeader = info.lenHeader;
  result.dim =  info.dim;
  result.buff = info.buff;

  FILETIME ft = getFileTimeCurr();
  bool success =  cryptFile(fileTarget, info.key, info.dimKey, info.step, result, &ft);
  info.crc = result.crc;
  return success;
}
//----------------------------------------------------------------------------
bool decryptFromFile(LPCTSTR fileSource, infoFileCrCopyOne& info)
{
  infoFileCr result;
  result.header = info.header;
  result.lenHeader = info.lenHeader;

  bool success = decryptFile(fileSource, info.key, info.dimKey, info.step, result);
  info.crc = result.crc;
  info.buff = result.buff;
  info.dim = result.dim;
  return true;
}

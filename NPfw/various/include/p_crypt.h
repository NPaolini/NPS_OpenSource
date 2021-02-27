//----- p_crypt.h -------------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef p_crypt_H
#define p_crypt_H
//----------------------------------------------------------------------------
#include "precHeader.h"

#include "pDef.h"
#include "Defin.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
WORD crypt(const BYTE* s, BYTE* dest, int k, int l);

inline
WORD crypt(BYTE* buff, int k, int l) { return crypt(buff, buff, k, l); }
//----------------------------------------------------------------------------
// non termina la stringa con uno zero
WORD crypt_noZ(const BYTE* s, BYTE* dest, int k, int l);

inline
WORD crypt_noZ(BYTE* buff, int k, int l) { return crypt_noZ(buff, buff, k, l); }
//----------------------------------------------------------------------------
struct infoFileCr
{
  // buff viene allocato quando legge da file, deve essere distrutto dal chiamante
  // mentre non viene toccato quando si scrive il file, deve essere allocato e
  // distrutto dal chiamante
  LPBYTE buff;
  int dim;
  DWORD crc;

  // se esiste un header per il file criptato deve corrispondere a questo
  LPCBYTE header;
  // dimensione dell'header
  uint lenHeader;
};
//----------------------------------------------------------------------------
WORD cryptBuffer(LPBYTE target, LPCBYTE source, DWORD dim, LPCBYTE key, int dimKey, uint step);
WORD cryptBufferNoZ(LPBYTE target, LPCBYTE source, DWORD dim, LPCBYTE key, int dimKey, uint step);
bool cryptFile(LPCTSTR file, LPCBYTE key, int dimKey, uint step, infoFileCr& source, const FILETIME* setData = 0);
bool decryptFile(LPCTSTR file, LPCBYTE key, int dimKey, uint step, infoFileCr& result);
//----------------------------------------------------------------------------
inline WORD cryptBuffer(LPBYTE buff, DWORD dim, LPCBYTE key, int dimKey, uint step)
{ return cryptBuffer(buff, buff, dim, key, dimKey, step); }
inline WORD cryptBufferNoZ(LPBYTE buff, DWORD dim, LPCBYTE key, int dimKey, uint step)
{ return cryptBufferNoZ(buff, buff, dim, key, dimKey, step); }
//----------------------------------------------------------------------------
struct infoFileCrCopy
{
  LPCBYTE key;
  int dimKey;
  uint step;
  LPCBYTE header;
  uint lenHeader;
};
//----------------------------------------------------------------------------
bool cryptFileCopy(LPCTSTR fileTarget, LPCTSTR fileSource, const infoFileCrCopy& info);
bool decryptFileCopy(LPCTSTR fileTarget, LPCTSTR fileSource, const infoFileCrCopy& info);
//----------------------------------------------------------------------------
struct infoFileCrCopyOne
{
  // stesso utilizzo di infoFileCr
  LPBYTE buff;
  int dim;
  mutable DWORD crc;

  LPCBYTE key;
  int dimKey;
  uint step;
  LPCBYTE header;
  uint lenHeader;
};
//----------------------------------------------------------------------------
bool cryptToFile(LPCTSTR fileTarget, const infoFileCrCopyOne& info);
bool decryptFromFile(LPCTSTR fileSource, infoFileCrCopyOne& info);
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

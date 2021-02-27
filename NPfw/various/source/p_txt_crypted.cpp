//------ P_Txt_Crypted.cpp ---------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_txt_crypted.h"
#include <stdio.h>
//----------------------------------------------------------------------------
static LPCTSTR decrypt(LPCTSTR filename, const infoFileCrCopy& info, LPTSTR result)
{
  bool success = false;
  do {
    TCHAR tmp[_MAX_PATH];
    if(_ttmpnam_s(tmp, _MAX_PATH))
      break;
    if(_T('/') == tmp[0] || _T('\\') == tmp[0]) {
      GetCurrentDirectory(_MAX_PATH, result);
      appendPath(result, tmp);
      }
    else
      _tcscpy_s(result, _MAX_PATH, tmp);
    if(!decryptFileCopy(result, filename, info))
      break;
    success = true;
    } while(false);
  if(!success) {
    *result = 0;
    return 0;
    }
  return result;
}
//----------------------------------------------------------------------------
setOfStringCrypted::setOfStringCrypted(LPCTSTR filename, const infoFileCrCopy& info) :
  baseClass(decrypt(filename, info, tempFile))
{
  if(*tempFile)
    DeleteFile(tempFile);
}
//----------------------------------------------------------------------------
void setOfStringCrypted::add(LPCTSTR filename, const infoFileCrCopy& info)
{
  tempFile[0] = 0;
  decrypt(filename, info, tempFile);
  if(*tempFile) {
    baseClass::add(tempFile);
    DeleteFile(tempFile);
    }
}

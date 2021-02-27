//------ P_Txt_Crypted.h -----------------------------------------------------
#ifndef P_Txt_Crypted_H_
#define P_Txt_Crypted_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "p_txt.h"
#include "p_util.h"
#include "setPack.h"
//----------------------------------------------------------------------------
class setOfStringCrypted : public setOfString
{
  private:
    typedef setOfString baseClass;
  public:
    setOfStringCrypted() : baseClass(0) {}

    setOfStringCrypted(LPCTSTR filename, const infoFileCrCopy& info);

    setOfStringCrypted(uint len, LPSTR buffer, bool autodelete = false) :
      baseClass(len, buffer, autodelete) {}
    setOfStringCrypted(uint len, LPWSTR buffer, bool autodelete = false) :
      baseClass(len, buffer, autodelete) {}

    void add(LPCTSTR filename, const infoFileCrCopy& info);
  protected:
    NO_COPY_COSTR_OPER(setOfStringCrypted)
    TCHAR tempFile[_MAX_PATH];
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

//----------- manageCryptPage.h ----------------------------------------------
#ifndef manageCryptPage_H_
#define manageCryptPage_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "pedit.h"
#include "p_file.h"
#include "file_lck.h"
#include "POwnBtnImage.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "popensave.h"
#include "pregistry.h"
#include "pCommonFilter.h"
//----------------------------------------------------------------------------
#include "resource.h"
#include "p_txt.h"
#include "headerMsg.h"
//----------------------------------------------------------------------------
#define MAX_CODE 8
#define MATCH "@#@#§§#@"
//-----------------------------------------------------------------
#define CRYPT_HEAD "CRYPNPS"
#define DIM_HEAD sizeof(CRYPT_HEAD)
//----------------------------------------------------------------------------
#define CRYPT_STEP    16
#define KEY_BASE_CRYPT 123
//----------------------------------------------------------------------------
class manageKey
{
  public:
   static int verifyKey();
   static bool evKeyDown(UINT& key);
   static bool evKeyUp(UINT& key);
   static bool evChar(UINT& key);
};
//----------------------------------------------------------------------------
class manageCryptPage
{
  public:
    manageCryptPage();
    ~manageCryptPage();

    bool isCrypted(LPCTSTR path);
    bool isCrypted();

    static void reset();
    bool makePathAndDecrypt(LPTSTR path);
    void releaseFileDecrypt(LPCTSTR path);

    bool makePathCrypt(LPTSTR path);
    void releaseFileCrypt(LPCTSTR path);

  private:
    // 0 non inizializzata, 1 è la key per tecnico, -1 chiave di cifratura
    static int loadedKey;
    // letta dal file .chi solo se valida, altrimenti viene chiesta
    static BYTE key[MAX_CODE + 2];

    void getUserKey();

};
//----------------------------------------------------------------------------
#endif

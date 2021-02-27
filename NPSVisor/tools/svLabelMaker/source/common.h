//----------- common.h -------------------------------------------------------
#ifndef COMMON_H_
#define COMMON_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "pmoddialog.h"
#include "pedit.h"
#include "p_file.h"
#include "file_lck.h"
#include "POwnBtnImage.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
#define cWHITE RGB(255, 255, 255)
#define cBLACK RGB(0, 0, 0)
#define cTRANSP ((DWORD)-1)
//----------------------------------------------------------------------------
#include "svmDataProject.h"
#define OLD_IMAGE_PATH  _T("OldSearchImagePath[%d]")
#define OLD_BASE_PATH  _T("OldSearchPath[%d]")
//----------------------------------------------------------------------------
#include "popensave.h"
#include "p_manage_ini.h"
#include "pCommonFilter.h"
//----------------------------------------------------------------------------
#include "resource.h"
#include "svmMainClient.h"
//----------------------------------------------------------------------------
//#define SUB_BASE_KEY _T("Software\\NPS_Programs\\svLabelMaker7")
#define INI_FILENAME _T("svLabelMaker.ini")
#define MAIN_PATH _T("\\Main")
//----------------------------------------------------------------------------
class myManageIni : public manageIni
{
  private:
    typedef manageIni baseClass;
  public:
    myManageIni(LPCTSTR filename);
};
//----------------------------------------------------------------------------
bool setKeyPath(LPCTSTR keyName, LPCTSTR keyBlock, LPCTSTR path);
bool getKeyPath(LPCTSTR keyName, LPCTSTR keyBlock, LPTSTR path);
bool setKeyParam(LPCTSTR keyName, LPCTSTR keyBlock, DWORD value);
bool getKeyParam(LPCTSTR keyName, LPCTSTR keyBlock, DWORD& value);
bool writeKey(LPCTSTR name, LPCTSTR keyBlock, LPCBYTE value, int len);
bool readKey(LPCTSTR name, LPCTSTR keyBlock, LPBYTE value, int len, bool writeIfNotFound);
//----------------------------------------------------------------------------
infoAdjusct loadAdjuct(infoAdjusct def);
//----------------------------------------------------------------------------
#define MAX_CODE 8
#define MATCH "@#@#§§#@"
//-----------------------------------------------------------------
#define CRYPT_HEAD "CRYPNPS"
#define DIM_HEAD sizeof(CRYPT_HEAD)
//----------------------------------------------------------------------------
#define CRYPT_STEP    16
#define KEY_BASE_CRYPT 123
#define CRIPTED_EMPTY { 0xA4, 0xB5, 0xBE, 0x97, 0x90, 0xF1, 0xEA, 0xD3 }
//----------------------------------------------------------------------------
extern bool getKeyCode(LPBYTE key);
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

    void checkKey();
    void getUserKey();

};
//----------------------------------------------------------------------------
class myOpenSave : public POpenSave
{
  public:
    myOpenSave(HWND owner, LPCTSTR keyBase, LPCTSTR keyString) : POpenSave(owner),
        KeyBase(str_newdup(keyBase)), KeyString(str_newdup(keyString)) {}
    ~myOpenSave() { delete []KeyString; delete []KeyBase; }
  protected:
    LPTSTR KeyBase;
    LPTSTR KeyString;
    virtual void setPathFromHistory();
    virtual void savePathToHistory();
};
//----------------------------------------------------------------------------
bool findFolder(LPCTSTR keyName, LPTSTR target, LPCTSTR title, bool onlySub = false);
//-----------------------------------------------------------
inline bool needSingleProperty()
{
  return toBool(GetKeyState(VK_MENU) & 0x8000);
}
//-----------------------------------------------------------
inline bool needFixedRatio()
{
  return toBool(GetKeyState(VK_MENU) & 0x8000);
}
//-----------------------------------------------------------
inline bool needMultipleAdd()
{
  return toBool(GetKeyState(VK_SHIFT) & 0x8000);
}
//-----------------------------------------------------------
inline bool needAddingObject()
{
  return toBool(GetKeyState(VK_SHIFT) & 0x8000);
}
//-----------------------------------------------------------
inline bool needAddingObject(int keyFlag)
{
  return toBool(keyFlag & MK_SHIFT);
}
//-----------------------------------------------------------
/*
inline bool needSelectingBkg()
{
  return GetKeyState(VK_CONTROL) & 0x8000;
}
//-----------------------------------------------------------
inline bool needSelectingBkg(int keyFlag)
{
  return toBool(keyFlag & MK_CONTROL);
}
*/
//----------------------------------------------------------------------------
void getKeyPath(LPCTSTR keyName, LPTSTR path);
//----------------------------------------------------------------------------
bool openFileImage(HWND owner, LPTSTR& file, bool multiple = false);
bool openFileImageWithCopy(HWND owner, LPTSTR& file, bool multiple = false);
bool openFileText(HWND owner, LPTSTR file, bool forSave, bool templ);
// se alsoSub controlla se in path ci sia un riferimento alla cartella image
// e torna il puntatore alla prima sub
LPCTSTR getOnlyName(LPCTSTR path, bool alsoSub = false);
// torna il percorso relativo sotto image, null se non coincide
// N.B. path deve essere il percorso completo
LPCTSTR getOnlySubImagePath(LPCTSTR path);
//------------------------------------------------------------------------------
void makeFileImagePath(LPTSTR path, size_t dim, LPCTSTR file);
void makeFileImagePath(LPTSTR path, size_t dim);
//------------------------------------------------------------------------------
// torna 1 se la riga è l'ultima della lista, -1 se errore
int remFromLb(HWND hwLb, LPTSTR buff = 0);
int moveOnLb(HWND hwLb, bool up);
//----------------------------------------------------------------------------
lUDimF writeStringChkUnicode(P_File& pf, LPCTSTR buff);
//------------------------------------------------------------------------------
inline
lUDimF writeStringForceUnicode(P_File& pf, LPCTSTR buff)
{
  return pf.P_writeToUnicode(buff);
}
//------------------------------------------------------------------------------
#endif

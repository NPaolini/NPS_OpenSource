//----------- common.h -------------------------------------------------------
#ifndef COMMON_H_
#define COMMON_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "pedit.h"
#include "p_file.h"
#include "file_lck.h"
#include "POwnBtnImage.h"
//----------------------------------------------------------------------------
#include "pModDialog.h"
//----------------------------------------------------------------------------
#include "popensave.h"
#include "p_manage_ini.h"
#include "pCommonFilter.h"
//----------------------------------------------------------------------------
#include "resource.h"
#include "p_txt.h"
#include "headerMsg.h"
#include "svmDataProject.h"
#include "clipboard_control.h"
//----------------------------------------------------------------------------
#define PAGE_EXT _T(".npt")
#define FIRST_PAGE _T("main_page") PAGE_EXT
//----------------------------------------------------------------------------
#define STD_MSG_NAME _T("base_data") PAGE_EXT
//----------------------------------------------------------------------------
extern int extent(HWND hWnd, LPCTSTR lpString);
extern void addStringToComboBox(HWND hList, LPCTSTR str, uint addSize);
extern void fillCBPerif(HWND hwnd, int select);
extern void fillCBTypeVal(HWND hwnd, int select);
P_File* makeFileTmpCustom(bool crypt);
//#include "svmMainClient.h"
//----------------------------------------------------------------------------
#define ID_SEND_VALUE_ON_OPEN  999996
#define ID_SEND_VALUE_ON_CLOSE 999997
#define ID_CODE_BTN_FZ_BEHA 999998
#define ID_CODE_PAGE_TYPE   999999
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
extern bool getKeyCode(LPBYTE key);
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

    static void setCanQuit() { canQuit = true; }

  private:
    // 0 non inizializzata, 1 è la key per tecnico, -1 chiave di cifratura
    static int loadedKey;
    // letta dal file .chi solo se valida, altrimenti viene chiesta
    static BYTE key[MAX_CODE + 2];

    void checkKey();
    void getUserKey();
    static bool canQuit;
};
//----------------------------------------------------------------------------
class PD_ActionPaste : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PD_ActionPaste(PWin* parent, uint& choose, uint resId = IDD_CHOOSE_PASTE_RCP_TYPE, HINSTANCE hinstance = 0)
        : baseClass(parent, resId, hinstance), Choose(choose) {  }
    virtual ~PD_ActionPaste() { destroy(); }
  private:
    uint& Choose;
  public:
    virtual bool create();
    void CmOk();
};
//-------------------------------------------------------------------
class myClipboard_ListBox : public Clipboard_ListBox
{
  private:
    typedef Clipboard_ListBox baseClass;
  public:
    myClipboard_ListBox(PWin* owner) : baseClass(owner) {}
  protected:
    virtual bool getChoosePaste(uint& choose);
};
//----------------------------------------------------------------------------
//#define SUB_BASE_KEY _T("Software\\NPS_Programs\\npsvMaker")
#define SVMAKER_INI_FILE _T("npsvMaker.ini")
#define TOOLS_PATH   _T("\\Tools")
#define ACTION_PATH  _T("\\Actions")
#define FILE_PATH    _T("\\Files")
#define WINTOOLS_COORD _T("WinCoords")

#define BASE_PATH       _T("BasePath")
#define REL_IMAGE_PATH  _T("ImagePath")
#define REL_SYSTEM_PATH _T("SystemPath")
#define REL_SYSTEM_DAT_PATH _T("SystemDatPath")

#define OLD_IMAGE_PATH  _T("OldSearchImagePath")

#define KEY_PARAM       _T("Param")
//----------------------------------------------------------------------------
#define CHG_PAGE_PATH _T("\\chgPage")
#define MOUSE_PATH _T("\\mousePos")

#define AD4_PATH _T("\\Ad4DialogPos")
#define AD4_COORD _T("Coords")

#define CHECKADDR_PATH  _T("\\CheckAddrDialogPos")

#define ADD_BORDER _T("BorderOffset")

#define OPEN_SOURCE_PATH   _T("\\open_Page_source")
#define SPLIT_PATH   _T("split")
#define OPEN_SOURCE_DIM   _T("dim")
#define OPEN_SOURCE_POS   _T("pos")

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
class openGenFile : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    openGenFile(PWin* parent, LPTSTR target, bool forSave, LPCTSTR pattern, uint resId = IDD_DIALOG_OPEN_ASSOC_FILE,
        HINSTANCE hinstance = 0, bool noSubSystem = false, bool alsoSub = true);
    virtual ~openGenFile();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    void LB_DblClick();
    void lbToEdit();

  private:
    void checkEnable();
    TCHAR subPath[_MAX_PATH];
    LPTSTR Target;
    bool forSave;
    LPCTSTR Pattern;
    bool noSubSystem;
    bool alsoSub;
};

//----------------------------------------------------------------------------
void setKeyPath(LPCTSTR keyName, LPCTSTR path);
void getKeyPath(LPCTSTR keyName, LPTSTR path);
//----------------------------------------------------------------------------
void setKeyParam(LPCTSTR keyName, DWORD value);
void getKeyParam(LPCTSTR keyName, LPDWORD value);
//----------------------------------------------------------------------------
void fillLBWithPageFile(HWND hlb, LPCTSTR sourcePath, LPCTSTR subPath, LPCTSTR pattern = _T("*")PAGE_EXT, bool onlyfile = false);
bool rePath(HWND hwlb, LPTSTR subPath, LPCTSTR basepath, LPTSTR target, LPCTSTR pattern = _T("*")PAGE_EXT);
void setTitlePath(HWND hw, LPCTSTR path);
//------------------------------------------------------------------------------
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
  return toBool(GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_CONTROL) & 0x8000);
}
//-----------------------------------------------------------
inline bool needAddingObject()
{
  return toBool(GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_CONTROL) & 0x8000);
}
//-----------------------------------------------------------
inline bool needAddingObject(int keyFlag)
{
  return toBool(keyFlag & MK_SHIFT) && !(GetKeyState(VK_CONTROL) & 0x8000);
}
//-----------------------------------------------------------
inline bool notUseGroup()
{
  return (GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000);
}
//-----------------------------------------------------------
bool openFileImage(HWND owner, LPTSTR& file, bool multiple = false);
bool openFileImageWithCopy(HWND owner, LPTSTR& file, bool multiple = false);
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
lUDimF writeStringChkUnicode(P_File& pfClear, P_File& pfCrypt, LPCTSTR buff);
//------------------------------------------------------------------------------
inline
lUDimF writeStringForceUnicode(P_File& pf, LPCTSTR buff)
{
  return pf.P_writeToUnicode(buff);
}
//------------------------------------------------------------------------------
lUDimF writeStringForceUnicode(P_File& pfClear, P_File& pfCrypt, LPCTSTR buff);
//------------------------------------------------------------------------------
enum actionChoice { acNo, acYes, acNoAll, acYesAll };
actionChoice getAction(PWin* owner, actionChoice prev, LPCTSTR title);
//------------------------------------------------------------------------------
#endif

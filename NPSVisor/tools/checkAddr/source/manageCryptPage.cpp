//----------- manageCryptPage.cpp --------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "manageCryptPage.h"
#include "macro_utils.h"
#include "PModDialog.h"
//-----------------------------------------------------------
bool manageKey::evKeyUp(UINT& key)
{
  if(VK_DOWN == key || VK_RETURN == key) {
    up_click(VK_TAB);
    return true;
    }
  if(VK_UP == key) {
    up_click(VK_TAB);
    up_click(VK_SHIFT);
    return true;
    }
  return false;
}
//-----------------------------------------------------------
bool manageKey::evKeyDown(UINT& key)
{
  if(VK_DOWN == key || VK_RETURN == key) {
    up_click(key);
    down_click(VK_TAB);
    return true;
    }

  if(VK_UP == key) {
    up_click(VK_UP);
    down_click(VK_SHIFT);
    down_click(VK_TAB);
    return true;
    }
  return false;
}
//-----------------------------------------------------------
bool manageKey::evChar(UINT& key)
{
  if(VK_RETURN == key) {
    click(VK_TAB);
    return true;
    }
  if(VK_ESCAPE == key) {
    return true;
    }
  return false;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int manageKey::verifyKey()
{
  if(GetKeyState(VK_RETURN)& 0x8000)
    return VK_RETURN;
  if(GetKeyState(VK_DOWN)& 0x8000)
    return VK_DOWN;
  if(GetKeyState(VK_UP)& 0x8000)
    return VK_UP;
  if(GetKeyState(VK_ESCAPE)& 0x8000)
    return VK_ESCAPE;
  return 0;
}
//-----------------------------------------------------------
//----------------------------------------------------------------------------
BYTE manageCryptPage::key[MAX_CODE + 2] = { 0 };
int manageCryptPage::loadedKey = 0;
//-----------------------------------------------------------
static infoFileCrCopy info_FileCrCopy =
{
  0, MAX_CODE, CRYPT_STEP, (LPCBYTE)CRYPT_HEAD, DIM_HEAD
};
//----------------------------------------------------------------------------
manageCryptPage::manageCryptPage() { }
//----------------------------------------------------------------------------
manageCryptPage::~manageCryptPage() {}
//----------------------------------------------------------------------------
void manageCryptPage::reset()
{
  ZeroMemory(key, sizeof(key));
  loadedKey = 0;
}
//----------------------------------------------------------------------------
class getUserKeyDlg : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    getUserKeyDlg(PWin* parent, LPBYTE key, LPCTSTR title, HINSTANCE hinstance = 0);
    virtual ~getUserKeyDlg();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk() {  unformatHex(Key, IDC_EDIT_HEX); baseClass::CmOk(); }
  private:
    LPBYTE Key;
    LPCTSTR title2;
    void checkEnable();
    void formatHex(LPBYTE svKey, uint idc);
    void unformatHex(LPBYTE svKey, uint idc);
    void updateEdit();
};
//----------------------------------------------------------------------------
extern void getPath(LPTSTR path);
extern PWin* getMain();
//----------------------------------------------------------------------------
bool loadKeyFromDll(LPBYTE key, LPCTSTR path);
//----------------------------------------------------------------------------
void manageCryptPage::getUserKey()
{
  TCHAR path[_MAX_PATH];
  path[0] = 0;
  getPath(path);
  if(!*path)
    _tcscpy_s(path, _T("???"));
//  if(IDOK != getUserKeyDlg(getMain(), key, path).modal())
  if(loadKeyFromDll(key, path))
    loadedKey = -2;
  else
    loadedKey = 1;
}
//----------------------------------------------------------------------------
bool manageCryptPage::isCrypted(LPCTSTR path)
{
  P_File pf(path, P_READ_ONLY);
  if(pf.P_open()) {
    BYTE t[DIM_HEAD];
    pf.P_read(t, sizeof(t));
//    603576
    bool crypted = !memcmp(t, CRYPT_HEAD, DIM_HEAD);
    if(crypted && !loadedKey && !key[0])
      getUserKey();
    return crypted;
    }
  return false;
}
//----------------------------------------------------------------------------
bool manageCryptPage::isCrypted()
{
  return loadedKey < 0 || toBool(key[0]);
}
//----------------------------------------------------------------------------
bool manageCryptPage::makePathAndDecrypt(LPTSTR path)
{
  TCHAR t[_MAX_PATH];
  _tcscpy_s(t, path);
  _tcscat_s(path, _MAX_PATH, _T(".~~~"));
  info_FileCrCopy.key = key;
  decryptFileCopy(path, t, info_FileCrCopy);
  return true;
}
//----------------------------------------------------------------------------
void manageCryptPage::releaseFileDecrypt(LPCTSTR path)
{
  DeleteFile(path);
}
//----------------------------------------------------------------------------
bool manageCryptPage::makePathCrypt(LPTSTR path)
{
  _tcscat_s(path, _MAX_PATH, _T(".~~~"));
  return true;
}
//----------------------------------------------------------------------------
void manageCryptPage::releaseFileCrypt(LPCTSTR path)
{
  TCHAR t[_MAX_PATH];
  _tcscpy_s(t, path);
  _tcscat_s(t, _MAX_PATH, _T(".~~~"));
  info_FileCrCopy.key = key;
  cryptFileCopy(path, t, info_FileCrCopy);
  DeleteFile(t);
}
//----------------------------------------------------------------------------
bool findMatch(uint& pos, LPBYTE buff, uint len)
{
  for(uint i = 0; i < len; ++i) {
    if(MATCH[0] == buff[i]) {
      if(!memcmp(MATCH, buff + i, sizeof(MATCH) - 1)) {
        pos = i + sizeof(MATCH) - 1;
        return true;
        }
      }
    }
  return false;
}
//----------------------------------------------------------------------------
int getCode(LPBYTE code, LPBYTE buff)
{
  WORD result = crypt(buff, code, KEY_BASE_CRYPT, MAX_CODE);
  for(uint i = 0; i < MAX_CODE; ++i)
    if(code[i] != 0x20)
      return 1;
  return 0;
}
//----------------------------------------------------------------------------
int hasCode(LPBYTE code, LPCTSTR path)
{
  P_File pf(path, P_READ_ONLY);
  if(pf.P_open()) {
    uint len = (uint)pf.get_len();
    LPBYTE buff = new BYTE[len];
    pf.P_read(buff, len);
    uint pos;
    bool found = findMatch(pos, buff, len);
    int result;
    if(!found)
      result = -1;
    else
      result = getCode(code, buff + pos);
    delete []buff;
    return result;
    }
  return -1;
}
//----------------------------------------------------------------------------
bool isSelfFile(LPCTSTR curr)
{
  TCHAR path[_MAX_PATH];
  TCHAR self[_MAX_PATH];
  _tfullpath(path, curr, SIZE_A(path));
  GetModuleFileName(0, self, SIZE_A(self));
  return !_tcsicmp(path, self);
}
//----------------------------------------------------------------------------
bool findKey(LPCTSTR base, LPBYTE code, LPCTSTR match = _T("npsvBase.dll"))
{
  TCHAR path[_MAX_PATH];
  GetCurrentDirectory(_MAX_PATH, path);
  SetCurrentDirectory(base);

  WIN32_FIND_DATA FindFileData;
  HANDLE hf = FindFirstFile(match, &FindFileData);
  int result = -2;
  if(hf != INVALID_HANDLE_VALUE) {
    do {
      if(FILE_ATTRIBUTE_DIRECTORY != FindFileData.dwFileAttributes) {
        TCHAR curr[_MAX_PATH];
        _tcscpy_s(curr, base);
        appendPath(curr, FindFileData.cFileName);
        if(!isSelfFile(curr))
          result = hasCode(code, curr);
        if(result > 0)
          break;
        }
      } while(FindNextFile(hf, &FindFileData));

    FindClose(hf);
    }
  SetCurrentDirectory(path);
  return result > 0;
}
//----------------------------------------------------------------------------
bool loadKeyFromDll(LPBYTE key, LPCTSTR path)
{
  return findKey(path, key);
}
//----------------------------------------------------------------------------
getUserKeyDlg::getUserKeyDlg(PWin* parent, LPBYTE key, LPCTSTR title, HINSTANCE hinstance) :
          baseClass(parent, IDD_GET_KEY_USER, hinstance), Key(key), title2(title)
{
}
//----------------------------------------------------------------------------
getUserKeyDlg::~getUserKeyDlg()
{
  destroy();
}
//----------------------------------------------------------------------------
bool getUserKeyDlg::create()
{
  if(!baseClass::create())
    return false;
  SET_TEXT(IDC_STATIC_PRG, title2);
  SET_CHECK(IDC_RADIO_ASCII);
  checkEnable();
  return true;
}
//----------------------------------------------------------------------------
LRESULT getUserKeyDlg::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RADIO_ASCII:
        case IDC_RADIO_HEX:
          checkEnable();
          break;
        case IDC_EDIT_HEX:
        case IDC_EDIT_ASCII:
          if(EN_CHANGE == HIWORD(wParam))
            updateEdit();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void getUserKeyDlg::checkEnable()
{
  bool enable = IS_CHECKED(IDC_RADIO_ASCII);
  ENABLE(IDC_EDIT_HEX, !enable);
  ENABLE(IDC_EDIT_ASCII, enable);
}
//----------------------------------------------------------------------------
void getUserKeyDlg::formatHex(LPBYTE svKey, uint idc)
{
  TCHAR t[32] = { 0 };
  LPTSTR p = t;
  for(uint i = 0; i < MAX_CODE; ++i) {
    wsprintf(p, _T("%02X "), svKey[i]);
    p += _tcslen(p);
    }
  p[_tcslen(p) - 1] = 0;
  SET_TEXT(idc, t);
}
//----------------------------------------------------------------------------
static
LPTSTR getValid(LPTSTR p)
{
  while(p && *p) {
    if(_istxdigit(*p))
      return p;
    ++p;
    }
  return 0;
}
//----------------------------------------------------------------------------
static
LPTSTR getNotValid(LPTSTR p)
{
  int i = 0;
  while(p && *p && i < 2) {
    if(!_istxdigit(*p))
      return p;
    ++p;
    ++i;
    }
  return p && *p ? p : 0;
}
//----------------------------------------------------------------------------
static
LPTSTR getHex(LPTSTR p, BYTE& b)
{
  b = 0;
  p = getValid(p);
  if(p) {
    int t = 0;
    _stscanf_s(p, _T("%02X"), &t);
    b = BYTE(t);
    p = getNotValid(p);
    }
  return p;
}
//----------------------------------------------------------------------------
void getUserKeyDlg::unformatHex(LPBYTE svKey, uint idc)
{
  TCHAR t[32] = { 0 };
  GET_TEXT(idc, t);
  LPTSTR p = t;
  for(uint i = 0; i < MAX_CODE; ++i)
    p = getHex(p, svKey[i]);
}
//----------------------------------------------------------------------------
void getUserKeyDlg::updateEdit()
{
  static bool inExec;
  if(inExec)
    return;
  inExec = true;
  TCHAR t[32];
  BYTE t2[32] = { 0 };
  if(IS_CHECKED(IDC_RADIO_ASCII)) {
    GetDlgItemText(*this, IDC_EDIT_ASCII, t, SIZE_A(t) - 1);
    copyStrZ(t2, t);
    formatHex(t2, IDC_EDIT_HEX);
    }
  else {
    unformatHex(t2, IDC_EDIT_HEX);
    copyStrZ(t, t2);
    SetDlgItemText(*this, IDC_EDIT_ASCII, t);
    }
  inExec = false;
}
//----------------------------------------------------------------------------

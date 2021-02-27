//----------- common.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "common.h"
#include "svmMainClient.h"
#include "macro_utils.h"
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
bool manageCryptPage::canQuit = false;
//-----------------------------------------------------------
static infoFileCrCopy info_FileCrCopy =
{
  0, MAX_CODE, CRYPT_STEP, (LPCBYTE)CRYPT_HEAD, DIM_HEAD
};
//----------------------------------------------------------------------------
manageCryptPage::manageCryptPage() { checkKey(); }
//----------------------------------------------------------------------------
manageCryptPage::~manageCryptPage() {}
//----------------------------------------------------------------------------
void manageCryptPage::reset()
{
  ZeroMemory(key, sizeof(key));
  loadedKey = 0;
}
//----------------------------------------------------------------------------
void manageCryptPage::checkKey()
{
  if(loadedKey)
    return;
  if(getKeyCode(key))
    loadedKey = key[0] ? -1 : 1;
  else if(canQuit)
    PostQuitMessage(0);
}
//------------------------------------------------------------------------------
bool PD_ActionPaste::create()
{
  if(!baseClass::create())
    return false;
  uint idc[] = { IDC_RADIO_RCP_PASTE_APPEND, IDC_RADIO_RCP_PASTE_INSERT, IDC_RADIO_RCP_PASTE_OVERWRITE, IDC_RADIO_RCP_PASTE_REPLACE_ALL };
  if(Choose >= SIZE_A(idc))
    Choose = 0;
  SET_CHECK(idc[Choose]);
  return true;
}
//------------------------------------------------------------------------------
void PD_ActionPaste::CmOk()
{
  uint idc[] = { IDC_RADIO_RCP_PASTE_APPEND, IDC_RADIO_RCP_PASTE_INSERT, IDC_RADIO_RCP_PASTE_OVERWRITE, IDC_RADIO_RCP_PASTE_REPLACE_ALL };
  for(uint i = 0; i < SIZE_A(idc); ++i) {
    if(IS_CHECKED(idc[i])) {
      Choose = i;
      break;
      }
    }
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
bool myClipboard_ListBox::getChoosePaste(uint& choose)
{
  return IDOK == PD_ActionPaste(getOwner(), choose).modal();
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
void manageCryptPage::getUserKey()
{
  dataProject& dp = getDataProject();
  if(IDOK != getUserKeyDlg(getAppl()->getMainWindow(), key, dp.newPath).modal())
    loadedKey = -2;
}
//----------------------------------------------------------------------------
bool manageCryptPage::isCrypted(LPCTSTR path)
{
  P_File pf(path, P_READ_ONLY);
  if(pf.P_open()) {
    BYTE t[DIM_HEAD];
    pf.P_read(t, sizeof(t));
    bool crypted = !memcmp(t, CRYPT_HEAD, DIM_HEAD);
    if(crypted && loadedKey > 0 && !key[0])
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
typedef PVect<LPCTSTR> setOfFiles;
//----------------------------------------------------------------------------
static
bool isFileReserved(LPCTSTR pFile, LPCTSTR pMatch, bool single)
{
  size_t len = _tcslen(pMatch);

  if(!_tcsnicmp(pMatch, pFile, len)) {
    LPCTSTR t = pFile + len;
    if(_istdigit((unsigned)t[0]) && !_tcsnicmp(PAGE_EXT, t + 1, 4))
      return true;
    if(single)
      return false;
    // c'è la doppia identificazione, filename_prph_language, quindi salta 'prph_'
    t += 2;
    // la lingua potrebbe essere 10, quindi controlla se inizia con un numero e poi se
    // segue un .npt subito dopo o al massimo un carattere dopo
    if(_istdigit((unsigned)t[0]) && (!_tcsnicmp(PAGE_EXT, t + 1, 4) || !_tcsnicmp(PAGE_EXT, t + 2, 4)))
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void addWithCheckExist(setOfFiles& set, LPCTSTR file)
{
  LPCTSTR pFile = _T('#') == *file ? file + 1 : file;
  LPCTSTR noInclude[] = {
    _T("config_9x") PAGE_EXT,
    _T("config_nt")PAGE_EXT,
    _T("config")PAGE_EXT,
    _T("infoAlarm")PAGE_EXT,
    _T("alarm")PAGE_EXT,
    STD_MSG_NAME,
    _T("globalPageText")PAGE_EXT,
    _T("npsvbase.dll"),
    };

  for(uint i = 0; i < SIZE_A(noInclude); ++i)
    if(!_tcsicmp(noInclude[i], pFile))
      return;
  if(isFileReserved(pFile, _T("alarm_"), true))
    return;
  if(isFileReserved(pFile, _T("infoAlarm_"), false))
    return;

  int nElem = set.getElem();

  TCHAR onlyName[_MAX_PATH];
  _tcscpy_s(onlyName, pFile);
  int len = _tcslen(onlyName);
  for(int i = len - 1; i >= 0; --i)
    if(_T('.') == onlyName[i]) {
      onlyName[i] = 0;
      break;
      }

  for(int i = nElem - 1; i >= 0; --i) {
    int cmp = _tcsicmp(set[i], onlyName);
    if(!cmp)
      return;
    if(cmp < 0) {
      set.insert(str_newdup(onlyName), i + 1);
      return;
      }
    }
  set.insert(str_newdup(onlyName), 0);
}
//----------------------------------------------------------------------------
void fillLBWithPageFile(HWND hlb, LPCTSTR sourcePath, LPCTSTR subPath, LPCTSTR pattern, bool onlyfile)
{
  SendMessage(hlb, LB_RESETCONTENT, 0, 0);
  TCHAR oldDir[_MAX_PATH];
  GetCurrentDirectory(SIZE_A(oldDir), oldDir);
  if(*sourcePath) {
    TCHAR tmp[_MAX_PATH];
    _tcscpy_s(tmp, sourcePath);
    appendPath(tmp, subPath);
    SetCurrentDirectory(tmp);
    }

  WIN32_FIND_DATA FindFileData;
  if(!onlyfile) {
    TCHAR files[] = _T("*.*");
    HANDLE hf = FindFirstFile(files, &FindFileData);
    if(hf != INVALID_HANDLE_VALUE) {
      bool sub = toBool(*subPath);
      do {
        if(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes) {
          bool addParent = sub && _T('.') == FindFileData.cFileName[1];
          if(addParent || _T('.') != *FindFileData.cFileName) {
            TCHAR buff[_MAX_PATH];
            wsprintf(buff, _T("<%s>"), FindFileData.cFileName);
            SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)buff);
            }
          }
        } while(FindNextFile(hf, &FindFileData));

      FindClose(hf);
      }
    }
  HANDLE hf = FindFirstFile(pattern, &FindFileData);
  setOfFiles set;
  if(hf != INVALID_HANDLE_VALUE) {
    do {
      if(!(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes))
        addWithCheckExist(set, FindFileData.cFileName);
      } while(FindNextFile(hf, &FindFileData));

    FindClose(hf);
    }
  int nElem = set.getElem();
  for(int i = 0; i < nElem; ++i)
    SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)set[i]);
  flushPAV(set);
  SetCurrentDirectory(oldDir);
}
//----------------------------------------------------------------------------
void setTitlePath(HWND hw, LPCTSTR path)
{
  TCHAR buff[_MAX_PATH];
  GetWindowText(hw, buff, SIZE_A(buff));
  int len = _tcslen(buff);
  for(int i = 0; i < len; ++i)
    if(_T('-') == buff[i]) {
      buff[i - 1] = 0;
      break;
      }
  if(path && *path) {
    TCHAR title[_MAX_PATH];
    wsprintf(title, _T("%s - %s"), buff, path);
    SetWindowText(hw, title);
    }
  else
    SetWindowText(hw, buff);
}
//----------------------------------------------------------------------------
bool rePath(HWND hwlb, LPTSTR subPath, LPCTSTR basepath, LPTSTR target, LPCTSTR pattern)
{
  int sel = SendMessage(hwlb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return true;
  SendMessage(hwlb, LB_GETTEXT, sel, (LPARAM) target);
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, subPath);

  if(_T('<') == *target) {
    if(_T('.') == target[1]) {
      int len = _tcslen(path);
      int i;
      for(i = len - 1; i >= 0; --i)
        if(_T('\\') == path[i]) {
          path[i] = 0;
          break;
          }
      if(i <= 0)
        path[0] = 0;
      }
    else {
      target[_tcslen(target) - 1] = 0;
      if(*path)
        appendPath(path, target + 1);
      else
        _tcscpy_s(path, target + 1);
      }
    _tcscpy_s(subPath, _MAX_PATH, path);

    fillLBWithPageFile(hwlb, basepath, subPath, pattern);
    HWND hw = GetParent(hwlb);
    setTitlePath(hw, subPath);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
#define MAX_BUFF 0xfffff
//----------------------------------------------------------------------------
void copySub(LPTSTR target, LPTSTR base, LPCTSTR full)
{
  appendPath(base);
  int len = _tcslen(base);
  _tcscpy_s(target, _MAX_PATH, full + len);
}
//----------------------------------------------------------------------------
static void makeModulePath(LPTSTR target, size_t dim,  LPCTSTR filename)
{
  LPCTSTR pf = filename + _tcslen(filename) - 1;
  while(pf > filename) {
    if(_T('\\') == *pf || _T('/') == *pf) {
      ++pf;
      break;
      }
    --pf;
    }
  GetModuleDirName(dim, target);
  appendPath(target, pf);
}
//----------------------------------------------------------------------------
myManageIni::myManageIni(LPCTSTR filename) : baseClass(filename)
{
  TCHAR t[_MAX_PATH];
  makeModulePath(t, SIZE_A(t), filename);
  delete []Filename;
  Filename = str_newdup(t);
}
//----------------------------------------------------------------------------
bool findFolder(LPCTSTR keyName, LPTSTR target, LPCTSTR title, bool onlySub)
{
  TCHAR path[_MAX_PATH] = _T("\0");
  TCHAR relPath[_MAX_PATH] = _T("\0");
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  do {
    _tcscpy_s(path, target);
    LPCTSTR str = ini.getValue(keyName, MAIN_PATH);
    if(str) {
      if(onlySub)
        _tcscpy_s(relPath, str);
      else
        _tcscpy_s(path, str);
      }
    } while(false);

  TCHAR oldPath[_MAX_PATH];
  _tcscpy_s(oldPath, path);

  if(PChooseFolder(path, title, onlySub, *relPath ? relPath : 0)) {
    if(onlySub)
      copySub(target, oldPath, path);
    else
      _tcscpy_s(target, _MAX_PATH, path);
    ini_param param(keyName, target);
    ini.addParam(param, MAIN_PATH);
    ini.save();
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void setKeyPath(LPCTSTR keyName, LPCTSTR path)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  ini_param param = { keyName, path };
  ini.addParam(param, MAIN_PATH);
  ini.save();
}
//----------------------------------------------------------------------------
void getKeyPath(LPCTSTR keyName, LPTSTR path)
{
  path[0] = 0;
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, MAIN_PATH);
  if(str)
    _tcscpy_s(path, _MAX_PATH, str);
}
//----------------------------------------------------------------------------
void setKeyParam(LPCTSTR keyName, DWORD value)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  TCHAR t[64];
  wsprintf(t, _T("%lu"), value);
  ini_param param = { keyName, t };
  ini.addParam(param, MAIN_PATH);
  ini.save();
}
//----------------------------------------------------------------------------
void getKeyParam(LPCTSTR keyName, LPDWORD value)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, MAIN_PATH);
  if(str) {
    __int64 t = _tcstoi64(str, 0, 10);
    *value = (DWORD)t;
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void myOpenSave::setPathFromHistory()
{
  TCHAR path[_MAX_PATH] = {};
  getKeyPath(KeyString, path);
  if(*path)
    SetCurrentDirectory(path);
}
//----------------------------------------------------------------------------
void myOpenSave::savePathToHistory()
{
  TCHAR oldPath[_MAX_PATH];
  GetCurrentDirectory(_MAX_PATH, oldPath);
  setKeyPath(KeyString, oldPath);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR filterExt[] = { _T(".bmp"),  _T(".jpg"),  _T(".jpeg"), _T(".gif"), _T(".png"), _T(".tif"),
    _T(".emf"),  _T(".wmf"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpen =
  _T("File immagini Compatibili (bmp, jfif, gif, png, tif, emf, wmf)\0")
  _T("*.bmp;*.jpg;*.jpeg;*.gif;*.png;*.tif;*.emf;*.wmf\0");
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static DWORD lastIx;
//----------------------------------------------------------------------------
static int calcDimMultipleFile(LPCTSTR files)
{
  int tot = 0;
  do {
    int len = _tcslen(files) + 1;
    if(!len)
      break;
    tot += len;
    files += len;
    } while(*files);
  return tot + 1;
}
//----------------------------------------------------------------------------
bool openFileImage(HWND owner, LPTSTR& file, bool multiple)
{
  LPTSTR p_file = multiple ?  0 : file;
  infoOpenSave Info(filterExt, filterOpen,
        multiple ? infoOpenSave::OPEN_F_MULTIPLE : infoOpenSave::OPEN_F, lastIx, p_file);
  myOpenSave open(owner, SVMAKER_INI_FILE, OLD_IMAGE_PATH);

  if(open.run(Info)) {
    lastIx = Info.ixFilter;
    if(multiple) {
      int len = calcDimMultipleFile(open.getFile());
      file = new TCHAR[len + 1];
      memcpy(file, open.getFile(), len * sizeof(TCHAR));
      }
    else
      _tcscpy_s(file, _MAX_PATH, open.getFile());
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
// verifica solo fino al path di base, considera una subDirectory come valida
bool isSameFolder(LPCTSTR fName, LPCTSTR fPath, bool multiple)
{
  TCHAR bF1[_MAX_PATH];
  TCHAR bF2[_MAX_PATH];
  _tcscpy_s(bF1, fName);
  _tcscpy_s(bF2, fPath);

  if(!multiple) {
    int len = _tcslen(bF1);
    for(int i = len - 1; i > 0; --i)
      if(_T('\\') == bF1[i]) {
        bF1[i] = 0;
        break;
        }
    }
  appendPath(bF1, 0);
  appendPath(bF2, 0);
  uint len = _tcslen(bF2);
  if(len > _tcslen(bF1))
    return false;
  if(_T('\\') == bF1[len - 1])
    bF1[len] = 0;
  return !_tcsicmp(bF1, bF2);
}
//----------------------------------------------------------------------------
#define MSG_COPY _T("Vuoi copiare i file\r\n") \
                 _T("nella cartella predefinita?\r\n\r\n") \
                 _T("Se non vengono copiati, le immagini non potranno essere visualizzate")
//----------------------------------------------------------------------------
void copyFile(HWND owner, LPCTSTR toPath, LPCTSTR fromPath, LPCTSTR name)
{
  TCHAR toP[_MAX_PATH];
  _tcscpy_s(toP, toPath);

  appendPath(toP, name);
  bool okToCopy = true;
  if(P_File::P_exist(toP)) {
    if(IDYES != MessageBox(owner, _T("Il file esiste,\r\nvuoi sovrascriverlo?"),
              _T("Attenzione"), MB_YESNO | MB_ICONINFORMATION))
      okToCopy = false;
    }
  if(okToCopy) {
    TCHAR fromP[_MAX_PATH];
    _tcscpy_s(fromP, fromPath);
    appendPath(fromP, name);
    CopyFile(fromP, toP, FALSE);
    }
}
//----------------------------------------------------------------------------
bool openFileImageWithCopy(HWND owner, LPTSTR& file, bool multiple)
{
  if(openFileImage(owner, file, multiple)) {
    TCHAR path[_MAX_PATH];
    const dataProject& dP = getDataProject();
    _tcscpy_s(path, dP.newPath);
    appendPath(path, dP.imagePath);
    if(!isSameFolder(file, path, multiple)) {
      if(IDYES == MessageBox(owner, MSG_COPY, _T("Devi usare la cartella predefinita"),
              MB_YESNO | MB_ICONINFORMATION)) {
        if(multiple) {
          LPCTSTR p = file;
          LPCTSTR f = p + _tcslen(p) + 1;
          while(*f) {
            copyFile(owner, path, p, f);
            f += _tcslen(f) + 1;
            }
          }
        else {
          LPTSTR p = const_cast<LPTSTR>(getOnlyName(file));
          *(p - 1) = 0;
          copyFile(owner, path, file, p);
          int len = _tcslen(p) + 1;
          for(int i = 0; i < len; ++i)
            file[i] = p[i];
          }
        }
      }
    else if(!multiple) {
      LPCTSTR p = getOnlyName(file, true);
      int len = _tcslen(p) + 1;
      for(int i = 0; i < len; ++i)
        file[i] = p[i];
      }
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
LPCTSTR getOnlyName(LPCTSTR path, bool alsoSub)
{
  if(!path)
    return 0;
  if(alsoSub) {
    // N.B. non verifica il caso di [.\]
    const dataProject& dP = getDataProject();
    TCHAR ipath[_MAX_PATH];
    // verifica se ha un path completo
    if(_T('\\') == path[0] && _T('\\') == path[1] || _T(':') == path[1]) {
      _tcscpy_s(ipath, dP.newPath);
      appendPath(ipath, dP.imagePath);
      appendPath(ipath, 0);
      }
    // ha un percorso relativo
    else {
      _tcscpy_s(ipath, dP.imagePath);
      appendPath(ipath, 0);
      }
    int len = _tcslen(ipath);
    bool matchPath = true;
    for(int i = 0; i < len; ++i)
      if(ipath[i] != path[i]) {
        matchPath = false;
        break;
        }
    if(matchPath)
      return path + len;
    }
  int len = _tcslen(path);
  for(int i = len - 1; i > 0; --i)
    if(_T('\\') == path[i])
      return path + i + 1;
  return path;
}
//----------------------------------------------------------------------------
void makeFileImagePath(LPTSTR path, size_t dim)
{
  TCHAR tmp[_MAX_PATH];
  _tcscpy_s(tmp, dim, path);
  makeFileImagePath(path, dim, tmp);
}
//----------------------------------------------------------------------------
void makeFileImagePath(LPTSTR path, size_t dim, LPCTSTR file)
{
  const dataProject& dP = getDataProject();
  _tcscpy_s(path, dim, dP.newPath);
  appendPath(path, dP.imagePath);
  appendPath(path, file);
}
//----------------------------------------------------------------------------
LPCTSTR getOnlySubImagePath(LPCTSTR path)
{
  const dataProject& dP = getDataProject();
  TCHAR ipath[_MAX_PATH];
  _tcscpy_s(ipath, dP.newPath);
  appendPath(ipath, dP.imagePath);
  appendPath(ipath, 0);
  int len = _tcslen(ipath);
  bool matchPath = true;
  for(int i = 0; i < len; ++i)
    if(ipath[i] != path[i]) {
      matchPath = false;
      break;
      }
  if(matchPath)
    return path + len;
  return 0;
}
//----------------------------------------------------------------------------
extern bool saveAsUnicode();
extern bool isGlobalPageString(LPCTSTR p);
//----------------------------------------------------------------------------
lUDimF writeStringChkUnicode(P_File& pf, LPCTSTR buff)
{
  if(saveAsUnicode())
    return pf.P_writeToUnicode(buff);
  return pf.P_writeToAnsi(buff);
}
//----------------------------------------------------------------------------
lUDimF writeStringChkUnicode(P_File& pfClear, P_File& pfCrypt, LPCTSTR buff)
{
  LPCTSTR p = findNextParamTrim(buff);
  if(isGlobalPageString(p))
    return writeStringChkUnicode(pfCrypt, buff);
  return writeStringChkUnicode(pfClear, buff);
}
//----------------------------------------------------------------------------
lUDimF writeStringForceUnicode(P_File& pfClear, P_File& pfCrypt, LPCTSTR buff)
{
  LPCTSTR p = findNextParamTrim(buff);
  if(isGlobalPageString(p))
    return pfCrypt.P_writeToUnicode(buff);
  return pfClear.P_writeToUnicode(buff);
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
openGenFile::openGenFile(PWin* parent,  LPTSTR target, bool forSave, LPCTSTR pattern, uint resId, HINSTANCE hinstance, bool noSubSystem, bool alsoSub)
  :  baseClass(parent, resId, hinstance), Target(target), forSave(forSave), Pattern(pattern), noSubSystem(noSubSystem), alsoSub(alsoSub)
{
  ZeroMemory(subPath, sizeof(subPath));
}
//----------------------------------------------------------------------------
openGenFile::~openGenFile()
{
  destroy();
}
//----------------------------------------------------------------------------
bool openGenFile::create()
{
  if(!baseClass::create())
    return false;

  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_PAGEFILES);
  TCHAR path[_MAX_PATH];
  const dataProject& dP = getDataProject();

  _tcscpy_s(path, dP.newPath);
  if(!noSubSystem)
    appendPath(path, dP.systemPath);
  fillLBWithPageFile(hlb, path, subPath, Pattern, !alsoSub);

  setTitlePath(*this, subPath);
  if(*Target) {
    TCHAR onlyName[_MAX_PATH];
    _tcscpy_s(onlyName, Target);
    int len = _tcslen(onlyName);
    for(int i = len - 1; i >= 0; --i)
      if(_T('.') == onlyName[i]) {
        onlyName[i] = 0;
        break;
        }
    int count = SendMessage(hlb, LB_GETCOUNT, 0, 0);
    if(count > 0) {
      for(int i = 0; i < count; ++i) {
        SendMessage(hlb, LB_GETTEXT, i, (LPARAM)path);
        if(!_tcsicmp(path, onlyName)) {
          SendMessage(hlb, LB_SETCURSEL, i, 0);
          break;
          }
        }
      }
    }
  else
    SendMessage(hlb, LB_SETCURSEL, 0, 0);
  if(forSave) {
    HWND hw = GetDlgItem(*this, IDC_EDIT_PAGEFILE);
    SetWindowText(hw, Target);
    ShowWindow(hw, SW_SHOW);
    setCaption(_T("Salva file"));
    }
  else
    setCaption(_T("Scelta file"));
  return true;
}
//----------------------------------------------------------------------------
LRESULT openGenFile::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case LBN_SELCHANGE:
          if(forSave)
            lbToEdit();
          break;
        case LBN_DBLCLK:
          CmOk();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void openGenFile::lbToEdit()
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_PAGEFILES);
  int sel = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[_MAX_PATH];
  SendMessage(hlb, LB_GETTEXT, sel, (LPARAM) buff);
  if(_T('<') == *buff)
    return;
  SET_TEXT(IDC_EDIT_PAGEFILE, buff);
}
//----------------------------------------------------------------------------
void openGenFile::CmOk()
{
  if(forSave) {
    TCHAR buff[_MAX_PATH];
    GET_TEXT(IDC_EDIT_PAGEFILE, buff);
    if(!buff)
      return;
    TCHAR path[_MAX_PATH];
    const dataProject& dP = getDataProject();
    _tcscpy_s(path, dP.newPath);
    appendPath(path, dP.systemPath);
    appendPath(path, subPath);
    appendPath(path, buff);
    _tcscat_s(path, Pattern + 1);

    if(P_File::P_exist(path)) {
      int result = MessageBox(*this, _T("Il file esiste,\r\nvuoi sovrascriverlo?"),
          _T("Warning"), MB_YESNOCANCEL | MB_ICONINFORMATION);
      switch(result) {
        case IDYES:
          _tcscpy_s(Target, _MAX_PATH, buff);
          break;

        case IDNO:
          CmCancel();
          // fall throught

        case IDCANCEL:
          return;
        }
      }
    else
      _tcscpy_s(Target, _MAX_PATH, buff);
    }
  else {
    HWND hlb = GetDlgItem(*this, IDC_LISTBOX_PAGEFILES);

    TCHAR path[_MAX_PATH];
    const dataProject& dP = getDataProject();
    _tcscpy_s(path, dP.newPath);
    appendPath(path, dP.systemPath);

    TCHAR buff[_MAX_PATH];
    if(rePath(hlb, subPath, path, buff, Pattern))
      return;
    if(*subPath) {
      _tcscpy_s(Target, _MAX_PATH, subPath);
      appendPath(Target, buff);
      }
    else
      _tcscpy_s(Target, _MAX_PATH, buff);
    }
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
class PDialogAction : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDialogAction(PWin* parent, actionChoice& prev, LPCTSTR title, HINSTANCE hinstance = 0) :
      baseClass(parent, IDD_DIALOG_YES_NO_ALL, hinstance), title(title), prev(prev) {}
    virtual ~PDialogAction() { destroy(); }

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk() {  }
    void CmCancel() {  }
  private:
    actionChoice& prev;
    LPCTSTR title;
};
//----------------------------------------------------------------------------
bool PDialogAction::create()
{
  if(!baseClass::create())
    return false;
  setCaption(title);
  return true;
}
//----------------------------------------------------------------------------
LRESULT PDialogAction::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_NO:
          prev = acNo;
          baseClass::CmOk();
          break;
        case IDC_BUTTON_NO_ALL:
          prev = acNoAll;
          baseClass::CmOk();
          break;
        case IDC_BUTTON_YES:
          prev = acYes;
          baseClass::CmOk();
          break;
        case IDC_BUTTON_YES_ALL:
          prev = acYesAll;
          baseClass::CmOk();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
actionChoice getAction(PWin* owner, actionChoice prev, LPCTSTR title)
{
  PDialogAction(owner, prev, title).modal();
  return prev;
}

//----------------------------------------------------------------------------

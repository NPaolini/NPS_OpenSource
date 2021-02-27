//----------- common.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "common.h"
#include "macro_utils.h"
//----------------------------------------------------------------------------
BYTE manageCryptPage::key[MAX_CODE + 2] = { 0 };
int manageCryptPage::loadedKey = 0;
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
/*
  if(loadedKey)
    return;
  if(getKeyCode(key))
    loadedKey = key[0] ? -1 : 1;
  else
    PostQuitMessage(0);
*/
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
  if(IDOK != getUserKeyDlg(getAppl()->getMainWindow(), key, dp.getcurrPath()).modal())
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
/*
    if(!crypted && isCrypted()) {
      MessageBox(0, _T("Pagina non abilitata"), _T("Errore - Chiusura applicazione"), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL);
      PostQuitMessage(0);
      }
*/
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
bool setKeyPath(LPCTSTR keyName, LPCTSTR keyBlock, LPCTSTR path)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  ini_param param = { keyName, path };
  ini.addParam(param, keyBlock);
  return ini.save();
}
//----------------------------------------------------------------------------
bool getKeyPath(LPCTSTR keyName, LPCTSTR keyBlock, LPTSTR path)
{
  path[0] = 0;
  myManageIni ini(INI_FILENAME);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, keyBlock);
  if(str) {
    _tcscpy_s(path, _MAX_PATH, str);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool setKeyParam(LPCTSTR keyName, LPCTSTR keyBlock, DWORD value)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  TCHAR t[64];
  wsprintf(t, _T("%lu"), value);
  ini_param param = { keyName, t };
  ini.addParam(param, keyBlock);
  return ini.save();
}
//----------------------------------------------------------------------------
bool getKeyParam(LPCTSTR keyName, LPCTSTR keyBlock, DWORD& value)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, keyBlock);
  if(str) {
    __int64 t = _tcstoi64(str, 0, 10);
    value = (DWORD)t;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static int getDec(TCHAR car)
{
  if(car >= _T('0') && car <= _T('9'))
    return car - _T('0');
  if(car >= _T('a') && car <= _T('f'))
    return car - _T('a') + 10;
  if(car >= _T('A') && car <= _T('F'))
    return car - _T('A') + 10;
  return 0;
}
//----------------------------------------------------------------------------
static int hexToInt(LPCTSTR buff)
{
  return (getDec(buff[0]) << 4) + getDec(buff[1]);
}
//----------------------------------------------------------------------------
bool writeKey(LPCTSTR name, LPCTSTR keyBlock, LPCBYTE value, int len)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  TCHAR t[256];
  for (int i = 0, j = 0; i < len; ++i, j += 2) {
    wsprintf(t + j, _T("%02X"), value[i]);
  }
  ini_param param = { name, t };
  ini.addParam(param, MAIN_PATH);
  return ini.save();
}
//----------------------------------------------------------------------------
bool readKey(LPCTSTR name, LPCTSTR keyBlock, LPBYTE value, int len, bool writeIfNotFound)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  LPCTSTR str = ini.getValue(name, MAIN_PATH);
  if(!str) {
    if(writeIfNotFound)
      return writeKey(name, keyBlock, value, len);
    return false;
    }
  int l = _tcslen(str);
  l = min(l, len * 2);
  int j = 0;
  for(int i = 0; i < l; i += 2, ++j) {
    TCHAR t[3] = {};
    t[0] = str[i];
    t[1] = str[i + 1];
    value[j] = (BYTE) hexToInt(t);
    }
  for(; j < len; ++j)
    value[j] = 0;
  return true;
}
//----------------------------------------------------------------------------
extern bool existFilePath(LPCTSTR t);
//----------------------------------------------------------------------------
bool findFolder(LPCTSTR keyName, LPTSTR target, LPCTSTR title, bool onlySub)
{
  TCHAR path[_MAX_PATH] = _T("\0");
  TCHAR relPath[_MAX_PATH] = _T("\0");
  if(existFilePath(_T("*.ptp"))) {
    GetCurrentDirectory(SIZE_A(path), path);
    setKeyPath(keyName, MAIN_PATH, path);
    path[0] = 0;
    }
  do {
    _tcscpy_s(path, target);
    TCHAR t[_MAX_PATH] = _T("");
    if(getKeyPath(keyName, MAIN_PATH, t)) {
      if(onlySub)
        _tcscpy_s(relPath, t);
      else
        _tcscpy_s(path, t);
      }
    } while(false);
  TCHAR oldPath[_MAX_PATH];
  _tcscpy_s(oldPath, path);

  if(PChooseFolder(path, title, onlySub, *relPath ? relPath : 0)) {
    if(onlySub)
      copySub(target, oldPath, path);
    else
      _tcscpy_s(target, _MAX_PATH, path);
    return setKeyPath(keyName, MAIN_PATH, target);
    }
  return false;
}
//----------------------------------------------------------------------------
void setKeyPath(LPCTSTR keyName, LPCTSTR path)
{
  setKeyPath(keyName, MAIN_PATH, path);
}
//----------------------------------------------------------------------------
void getKeyPath(LPCTSTR keyName, LPTSTR path)
{
  path[0] = 0;
  getKeyPath(keyName, MAIN_PATH, path);
}
//----------------------------------------------------------------------------
void setKeyParam(LPCTSTR keyName, DWORD value)
{
  setKeyParam(keyName, MAIN_PATH, value);
}
//----------------------------------------------------------------------------
void getKeyParam(LPCTSTR keyName, LPDWORD value)
{
  getKeyParam(keyName, MAIN_PATH, *value);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void myOpenSave::setPathFromHistory()
{
}
//----------------------------------------------------------------------------
void myOpenSave::savePathToHistory()
{
}
//----------------------------------------------------------------------------
#if 1
//----------------------------------------------------------------------------
LPCTSTR filterExt[] = { _T(".bmp"),  _T(".jpg"),  _T(".jpeg"), _T(".gif"), _T(".png"), _T(".tif"),
    _T(".emf"),  _T(".wmf"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpen =
  _T("File immagini Compatibili (bmp, jfif, gif, png, tif, emf, wmf)\0")
  _T("*.bmp;*.jpg;*.jpeg;*.gif;*.png;*.tif;*.emf;*.wmf\0");
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
LPCTSTR filterExt[] = { _T(".bmp"),  _T(".jpg"),  _T(".jpeg"),  _T(".gif"),
    _T(".emf"),  _T(".wmf"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpen =
  _T("File immagini Compatibili (bmp, jfif, gif, emf, wmf)\0")
  _T("*.bmp;*.jpg;*.jpeg;*.gif;*.emf;*.wmf\0");
//----------------------------------------------------------------------------
#endif
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
struct info4Search
{
  HWND owner;
  LPTSTR* file;
  bool multiple;
  LPCTSTR* fltExt;
  LPCTSTR fltOpen;
  LPCTSTR key;
  DWORD lastIx;
  bool forSave;
};
//----------------------------------------------------------------------------
bool openFileGeneric(info4Search& i4s)
{
  LPTSTR p_file = i4s.multiple ?  0 : *i4s.file;
  infoOpenSave Info(i4s.fltExt, i4s.fltOpen, i4s.forSave ? infoOpenSave::SAVE_AS :
        i4s.multiple ? infoOpenSave::OPEN_F_MULTIPLE : infoOpenSave::OPEN_F, i4s.lastIx, p_file);
  TCHAR t[_MAX_PATH];
  wsprintf(t, i4s.key, getDataProject().currPage);
  myOpenSave open(i4s.owner, MAIN_PATH, t);

  bool success = false;
  if(open.run(Info)) {
    i4s.lastIx = Info.ixFilter;
    if(i4s.multiple) {
      int len = calcDimMultipleFile(open.getFile());
      *i4s.file = new TCHAR[len + 1];
      memcpy(*i4s.file, open.getFile(), len * sizeof(TCHAR));
      }
    else
      _tcscpy_s(*i4s.file, _MAX_PATH, open.getFile());
    success = true;
    }
  return success;
}
//----------------------------------------------------------------------------
bool openFileImage(HWND owner, LPTSTR& file, bool multiple)
{
  static DWORD lastIx;
  info4Search i4s = {
    owner, &file, multiple, filterExt, filterOpen, OLD_IMAGE_PATH, lastIx, false
    };

  TCHAR imgPath[_MAX_PATH];
  _tcscpy_s(imgPath, getDataProject().getcurrPath());
  appendPath(imgPath, _T("image"));
  createDirectoryRecurse(imgPath);

  TCHAR oldPath[_MAX_PATH];
  GetCurrentDirectory(_MAX_PATH, oldPath);
  SetCurrentDirectory(imgPath);

  bool success = false;
  if(openFileGeneric(i4s)) {
    lastIx = i4s.lastIx;
    success = true;
    }
  SetCurrentDirectory(oldPath);
  return success;
}
//----------------------------------------------------------------------------
LPCTSTR filterExtT[] = { _T(".ptp"),  _T(".txt"),  _T(".*"),  0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpenT =
  _T("File Template (ptp)\0")
  _T("*.ptp\0")
  _T("File Testo (txt)\0")
  _T("*.txt\0")
  _T("Tutti i File (*.*)\0")
  _T("*.*\0");
//----------------------------------------------------------------------------
LPCTSTR filterExtTD[] = { _T(".ptd"),  _T(".txt"),  _T(".*"),  0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpenTD =
  _T("File Dati (ptd)\0")
  _T("*.ptd\0")
  _T("File Testo (txt)\0")
  _T("*.txt\0")
  _T("Tutti i File (*.*)\0")
  _T("*.*\0");
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool openFileText(HWND owner, LPTSTR file, bool forSave, bool templ)
{
  static DWORD lastIx;
  info4Search i4s = {
    owner, &file, false, templ ? filterExtT : filterExtTD, templ ? filterOpenT : filterOpenTD, OLD_BASE_PATH, lastIx, forSave
    };

  if(openFileGeneric(i4s)) {
    lastIx = i4s.lastIx;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
#if 1
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
#else
bool isSameFolder(LPCTSTR fName, LPCTSTR fPath, bool multiple)
{
  TCHAR bF1[_MAX_PATH];
  TCHAR bF2[_MAX_PATH];
  _tcscpy(bF1, fName);
  _tcscpy(bF2, fPath);

  if(!multiple) {
    int len = _tcslen(bF1);
    for(int i = len - 1; i > 0; --i)
      if(_T('\\') == bF1[i]) {
        bF1[i] = 0;
        break;
        }
    }
  appendPath(bF1, _T("@"));
  appendPath(bF2, _T("@"));
  return !_tcsicmp(bF1, bF2);
}
#endif
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
extern void newProject(HWND owner);
//----------------------------------------------------------------------------
bool openFileImageWithCopy(HWND owner, LPTSTR& file, bool multiple)
{
  if(openFileImage(owner, file, multiple)) {
    TCHAR path[_MAX_PATH];
    dataProject& dP = getDataProject();
    while(!*dP.getcurrPath()) {
      newProject(owner);
      }
    _tcscpy_s(path, dP.getcurrPath());
    appendPath(path, dP.getimagePath());
    createDirectoryRecurse(path);
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
      _tcscpy_s(ipath, dP.getcurrPath());
      appendPath(ipath, dP.getimagePath());
      appendPath(ipath, 0);
      }
    // ha un percorso relativo
    else {
      _tcscpy_s(ipath, dP.getimagePath());
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
  _tcscpy_s(path, dim, dP.getcurrPath());
  appendPath(path, dP.getimagePath());
  appendPath(path, file);
}
//----------------------------------------------------------------------------
LPCTSTR getOnlySubImagePath(LPCTSTR path)
{
  const dataProject& dP = getDataProject();
  TCHAR ipath[_MAX_PATH];
  _tcscpy_s(ipath, dP.getcurrPath());
  appendPath(ipath, dP.getimagePath());
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
//----------------------------------------------------------------------------
lUDimF writeStringChkUnicode(P_File& pf, LPCTSTR buff)
{
  if(saveAsUnicode())
    return pf.P_writeToUnicode(buff);
  return pf.P_writeToAnsi(buff);
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

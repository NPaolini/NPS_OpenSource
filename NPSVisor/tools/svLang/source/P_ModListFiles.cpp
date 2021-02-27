//-------- P_ModListFiles.cpp ---------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include <commctrl.h>
#include "mainwin.h"
#include "P_ModListFiles.h"
#include "P_ModListCodes.h"
#include "P_ModEdit.h"
#include "p_Util.h"
#include "resource.h"
#include "P_file.h"
#include "P_txt.h"
//-------------------------------------------------------------------
#define IDC_LIST1 101
//-------------------------------------------------------------------
P_ModListFiles::~P_ModListFiles()
{
  destroy();
  flush();
}
//-------------------------------------------------------------------
void P_ModListFiles::init()
{
  Path[0] = 0;
  currSel = -1;
}
//-------------------------------------------------------------------
bool P_ModListFiles::isDirty() const
{
  uint nElem = InfoFile.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(InfoFile[i]->isDirty())
      return true;

  P_ModListCodes* mlc = getListCode((PWin*)this);
  if(currSel >= 0 && mlc->isDirty())
    return true;
  P_ModEdit* mec = getEditCode((PWin*)this);
  if(mec->isDirtyNameLang())
    return true;
  return false;
}
//-------------------------------------------------------------------
bool P_ModListFiles::save(bool onUnicode)
{
  getCurrent();
  uint nElem = InfoFile.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(InfoFile[i]->isDirty()) {
      if(!InfoFile[i]->save(onUnicode))
        return false;
      }
    }
  P_ModEdit* mec = getEditCode(this);
  if(mec->isDirtyNameLang()) {
    PVect<LPTSTR> buff;
    mec->getTitle(buff);
    uint sel = mec->getSelLang();
    if(!saveStdLang(sel, buff, onUnicode))
      return false;
    mec->resetDirtyNameLang();
    }
  return true;
}
//-------------------------------------------------------------------
#define ID_LANGUAGE 24
//-----------------------------------------------------------------
#define CRYPT_HEAD "CRYPNPS"
#define DIM_HEAD sizeof(CRYPT_HEAD)
//-----------------------------------------------------------------
#define CRYPT_STEP    16
#define DIM_KEY_CRYPT  8
#define DEF_CRYPT_STD_MSG _T("#") STD_MSG_NAME
//----------------------------------------------------------------------------
#define MAX_CODE 8
#define MATCH "@#@#§§#@"
//-----------------------------------------------------------------
#define CRYPT_STEP    16
#define KEY_BASE_CRYPT 123
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
DWORD load_CriptedTxt(LPCBYTE key, setOfString& set, LPCTSTR path)
{
  infoFileCr result;
  result.header = (LPCBYTE)CRYPT_HEAD;
  result.lenHeader = DIM_HEAD;

  if(decryptFile(path, key, DIM_KEY_CRYPT, CRYPT_STEP, result)) {
    uint len = result.dim;
    LPTSTR buff = autoConvert(result.buff, len);
    set.add(len, buff, true);
    }
  return result.crc;
}
//-------------------------------------------------------------------
int loadFileSharp(setOfString& set, LPCTSTR path, LPBYTE key)
{
  bool crypted = false;
  do {
    P_File pf(path, P_READ_ONLY);
    if(!pf.P_open())
      return -1;
    BYTE head[DIM_HEAD];
    if(sizeof(head) != pf.P_read(head, sizeof(head)))
      return -1;
    if(!memcmp(head, CRYPT_HEAD, sizeof(head)))
      crypted = true;
    } while(false);
  if(crypted) {
    TCHAR base[_MAX_PATH];
    _tcscpy_s(base, path);
    checkSystem(base);

    if(!findKey(base, key))
      return -1;
    load_CriptedTxt(key, set, path);
    }
  else
    set.add(path);
  return crypted;
}
//-------------------------------------------------------------------
bool saveFileSharp(setOfString& set, LPCTSTR path, bool crypted, LPBYTE key, bool onUnicode)
{
  if(crypted) {
    TCHAR path2[] = _T("~tmp.tmp");
    do {
      P_File pf(path2, P_CREAT);
      if(!pf.P_open())
        return false;
      if(!set.setFirst())
        return false;
      do {
        if(!set.writeCurrent(pf, onUnicode))
          return false;
        } while(set.setNext());
      } while(false);
    infoFileCrCopy ifc;
    ifc.key = key;
    ifc.dimKey = DIM_KEY_CRYPT;
    ifc.step = CRYPT_STEP;
    ifc.header = (LPCBYTE)CRYPT_HEAD;
    ifc.lenHeader = DIM_HEAD;
    bool success = cryptFileCopy(path, path2, ifc);
    if(success)
      DeleteFile(path2);
    return success;
    }
  P_File pf(path, P_CREAT);
  if(!pf.P_open())
    return false;
  if(!set.setFirst())
    return false;
  do {
    if(!set.writeCurrent(pf, onUnicode))
      return false;
    } while(set.setNext());
  return true;
}
//-------------------------------------------------------------------
bool P_ModListFiles::saveStdLang(uint sel, PVect<LPTSTR>& lang, bool onUnicode)
{
  TCHAR buff[4096];
  wsprintf(buff, _T("%d"), sel);
  uint nElem = lang.getElem();
  for(uint i = 0; i < nElem; ++i) {
    _tcscat_s(buff, _T(","));
    if(*lang[i])
      _tcscat_s(buff, lang[i]);
    }
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, Path);
  appendPath(path, DEF_CRYPT_STD_MSG);
  setOfString set;
  BYTE key[DIM_KEY_CRYPT + 2] = { 0 };
  int crypted = loadFileSharp(set, path, key);
  if(crypted < 0)
    return false;
  set.replaceString(ID_LANGUAGE, str_newdup(buff), true);
  return saveFileSharp(set, path, toBool(crypted), key, onUnicode);
}
//-------------------------------------------------------------------
bool P_ModListFiles::create()
{
  List = new PListBox(this, IDC_LIST1, PRect(0, 0, 100, 100));
  int tabs[] = { _MAX_PATH };
  List->SetTabStop(SIZE_A(tabs), tabs);
  if(!baseClass::create())
    return false;
  List->setFont(D_FONT(18, 0, fBOLD, _T("arial")), true);
  return true;
}
//-------------------------------------------------------------------
void P_ModListFiles::resize()
{
  baseClass::resize();
  List->setIntegralHeight();
}
//----------------------------------------------------------------------------
typedef PVect<LPCTSTR> setOfFiles;
//----------------------------------------------------------------------------
static bool findSharp(LPCTSTR pFile)
{
  TCHAR path[_MAX_PATH] = _T("#");
  _tcscat_s(path, pFile);
  return P_File::P_exist(path);
}
//----------------------------------------------------------------------------
static
bool isFileReserved(LPCTSTR pFile, LPCTSTR pMatch, bool single)
{
  size_t len = _tcslen(pMatch);

  if(!_tcsnicmp(pMatch, pFile, len)) {
    LPCTSTR t = pFile + len;
    if(_istdigit((unsigned)t[0]) && !_tcsnicmp(PAGE_EXT, t + 1, 4))
      return !findSharp(pFile);
    if(single)
      return false;
    // c'è la doppia identificazione, filename_prph_language, quindi salta 'prph_'
    t += 2;
    // la lingua potrebbe essere 10, quindi controlla se inizia con un numero e poi se
    // segue un .txt subito dopo o al massimo un carattere dopo.
    // In questi file non esiste quello con # iniziale
    if(_istdigit((unsigned)t[0]) && (!_tcsnicmp(PAGE_EXT, t + 1, 4) || !_tcsnicmp(PAGE_EXT, t + 2, 4)))
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void addWithCheckExist(setOfFiles& set, LPCTSTR file, LPCTSTR folder)
{
  if(_T('#') == *file)
    return;
  LPCTSTR pFile = file;
  LPCTSTR noInclude[] = {
    _T("config_9x") PAGE_EXT,
    _T("config_nt") PAGE_EXT,
    _T("config") PAGE_EXT,
    _T("alarm") PAGE_EXT,
    STD_MSG_NAME,
    _T("infoAlarm") PAGE_EXT,
    };
  for(uint i = 0; i < SIZE_A(noInclude); ++i)
    if(!_tcsicmp(noInclude[i], pFile))
      return;

  if(!folder && isFileReserved(pFile, _T("alarm_"), true))
    return;

  if(!folder && isFileReserved(pFile, _T("infoAlarm_"), false))
    return;
  int nElem = set.getElem();

  TCHAR onlyName[_MAX_PATH];
  if(folder) {
    _tcscpy_s(onlyName, folder);
    appendPath(onlyName, pFile);
    }
  else
    _tcscpy_s(onlyName, pFile);

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
//-------------------------------------------------------------------
void addFiles(setOfFiles& Set, LPCTSTR folder)
{
  TCHAR oldDir[_MAX_PATH];
  GetCurrentDirectory(SIZE_A(oldDir), oldDir);
  if(folder && *folder) {
    LPCTSTR p = _tcsrchr(folder, _T('\\'));
    if(p)
      SetCurrentDirectory(p + 1);
    else
      SetCurrentDirectory(folder);
    }
  WIN32_FIND_DATA FindFileData;

  TCHAR files[] = _T("*")PAGE_EXT;
  HANDLE hf = FindFirstFile(files, &FindFileData);
  setOfFiles set;
  if(hf != INVALID_HANDLE_VALUE) {
    do {
      if(!(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes))
        addWithCheckExist(set, FindFileData.cFileName, folder);
      } while(FindNextFile(hf, &FindFileData));
    FindClose(hf);
    }
  int nElem = set.getElem();
  int j = Set.getElem();
  for(int i = 0; i < nElem; ++i, ++j) {
    Set[j] = set[i];
    set[i] = 0;
    }
  TCHAR fold[] = _T("*.*");
  hf = FindFirstFile(fold, &FindFileData);
  if(hf != INVALID_HANDLE_VALUE) {
    do {
      if((FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes) && _T('.') != *FindFileData.cFileName) {
        if(folder && *folder) {
          TCHAR path[_MAX_PATH];
          _tcscpy_s(path, folder);
          appendPath(path, FindFileData.cFileName);
          addFiles(Set, path);
          }
        else
          addFiles(Set, FindFileData.cFileName);
        }
      } while(FindNextFile(hf, &FindFileData));
    FindClose(hf);
    }
  SetCurrentDirectory(oldDir);
}
//-------------------------------------------------------------------
void P_ModListFiles::refresh(LPCTSTR path)
{
  currSel = -1;
  flush();
  SendMessage(*List, LB_RESETCONTENT, 0, 0);
  if(!*path)
    return;
  _tcscpy_s(Path, path);
  PVect<LPCTSTR> set;
  TCHAR oldDir[_MAX_PATH];
  GetCurrentDirectory(SIZE_A(oldDir), oldDir);
  SetCurrentDirectory(path);
  addFiles(set, 0);
  SetCurrentDirectory(oldDir);

  SendMessage(*List, LB_ADDSTRING, 0, (LPARAM)(DEF_CRYPT_STD_MSG + 1));
  uint nElem = set.getElem();

  InfoFile.setDim(nElem + 1);
  InfoFile[0] = new infoFile(DEF_CRYPT_STD_MSG + 1, Path);
  for(uint i = 0; i < nElem; ++i) {
    SendMessage(*List, LB_ADDSTRING, 0, (LPARAM)set[i]);
    InfoFile[i + 1] = new infoFile(set[i], Path);
    }

  flushPAV(set);
  BYTE key[DIM_KEY_CRYPT + 2];
  setOfString sos;
  TCHAR file[_MAX_PATH];
  _tcscpy_s(file, path);
  appendPath(file, DEF_CRYPT_STD_MSG);
  int crypt = loadFileSharp(sos, file, key);
  if(crypt < 0)
    return;
  LPCTSTR p = sos.getString(ID_LANGUAGE);
  if(!p)
    p = _T("0,Italiano,,,,,,,,,");

  pvvChar target;
  uint nEl = splitParam(target, p);
  uint sel = _ttoi(target[0].getVect());
  freeList& SetFree = getSetFree();
  PVect<LPTSTR> names;
  for(uint i = 1; i < nEl; ++i) {
    LPTSTR tmp = SetFree.get();
    _tcscpy_s(tmp, SIZE_SINGLE_BUFF, target[i].getVect());
    names[i - 1] = tmp;
    }
  for(uint i = nEl; i < MAX_LANG + 1; ++i) {
    LPTSTR tmp = SetFree.get();
    *tmp = 0;
    names[i - 1] = tmp;
    }
  P_ModEdit* mec = getEditCode(this);
  mec->setTitle(names);
  mec->setSelLang(sel);
  P_ModListCodes* mlc = getListCode(this);

  for(uint i = 1; i < nEl; ++i) {
    LPTSTR tmp = SetFree.get();
    _tcscpy_s(tmp, SIZE_SINGLE_BUFF, target[i].getVect());
    names[i - 1] = tmp;
    }
  for(uint i = nEl; i < MAX_LANG + 1; ++i) {
    LPTSTR tmp = SetFree.get();
    *tmp = 0;
    names[i - 1] = tmp;
    }
  mlc->setTitle(names);

  currSel = -1;
  SendMessage(*List, LB_SETCURSEL, 0, 0);
  selChange();
}
//-------------------------------------------------------------------
void P_ModListFiles::fillListFile(PVect<LPCTSTR>& filename)
{
  uint nEl = InfoFile.getElem();
  freeList& SetFree = getSetFree();
  for(uint i = 0; i < nEl; ++i) {
    LPTSTR tmp = SetFree.get();
    _tcscpy_s(tmp, SIZE_SINGLE_BUFF, InfoFile[i]->getFilename());
    filename[i] = tmp;
    }
}
//-------------------------------------------------------------------
void P_ModListFiles::getCurrent()
{
  P_ModListCodes* mlc = getListCode(this);
  if(currSel >= 0 && mlc->isDirty())
    InfoFile[currSel]->updateFromLV(mlc->getHWLv());
  mlc->resetDirty();
}
//-------------------------------------------------------------------
void P_ModListFiles::selChange()
{
  int curr = SendMessage(*List, LB_GETCURSEL, 0, 0);
  if(curr < 0 || curr == currSel)
    return;

  P_ModListCodes* mlc = getListCode(this);
  if(currSel >= 0 && mlc->isDirty())
    InfoFile[currSel]->updateFromLV(mlc->getHWLv());

  currSel = curr;
  InfoFile[currSel]->fillLV(mlc->getHWLv());
  mlc->refresh();
  mlc->resetDirty();
}
//-------------------------------------------------------------------
int P_ModListFiles::getCurrSel()
{
  return currSel;
}
//-------------------------------------------------------------------
void P_ModListFiles::setCurrSel(int sel)
{
  SendMessage(*List, LB_SETCURSEL, sel, 0);
  selChange();
}
//-------------------------------------------------------------------
bool P_ModListFiles::setFirst()
{
  int count = SendMessage(*List, LB_GETCOUNT, 0, 0);
  if(count > 0) {
    setCurrSel(0);
    return true;
    }
  return false;
}
//-------------------------------------------------------------------
bool P_ModListFiles::setNext()
{
  int curr = SendMessage(*List, LB_GETCURSEL, 0, 0);
  int count = SendMessage(*List, LB_GETCOUNT, 0, 0);
  if(curr < count - 1) {
    setCurrSel(curr + 1);
    return true;
    }
  return false;
}
//-------------------------------------------------------------------
LRESULT P_ModListFiles::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SIZE:
      List->setIntegralHeight();
      break;
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case LBN_SELCHANGE:
          selChange();
          break;
        }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
infoFile::infoFile(LPCTSTR file, LPCTSTR basePath) : Dirty(false), BasePath(basePath),
  Filename(str_newdup(file)) {}
//----------------------------------------------------------------------------
infoFile::~infoFile()
{
  delete []Filename;
  flushRow();
}
//----------------------------------------------------------------------------
void infoFile::flushRow()
{
  uint nElem = Rows.getElem();
  for(uint i = 0; i < nElem; ++i)
    delete []Rows[i].text;
  Rows.reset();
}
//----------------------------------------------------------------------------
#define DEF_SEP_LANG _T('§')
//----------------------------------------------------------------------------
void infoFile::addItem(HWND hwLv, uint id, LPCTSTR text)
{
  TCHAR buff[64];
  wsprintf(buff, _T("%d"), id);
  LVITEM lvi;
  ZeroMemory(&lvi, sizeof(lvi));
  lvi.mask = LVIF_TEXT;
  lvi.iItem = 65535;
  lvi.iSubItem = 0;
  lvi.pszText = buff;

  int pos = ListView_InsertItem(hwLv, &lvi);
  if(-1 == pos)
    return;

  lvi.iItem = pos;

  if(DEF_SEP_LANG == *text)
    ++text;

  pvvChar target;
  uint nEl = splitParam(target, text, DEF_SEP_LANG);
  freeList& SetFree = getSetFree();
  LPTSTR tmp = SetFree.get();

  for(uint i = 0; i < nEl; ++i) {
    lvi.iSubItem = i + 1;
    translateFromCRNL(tmp, target[i].getVect());
    lvi.pszText = tmp;

    ListView_SetItem(hwLv, &lvi);
    }
  SetFree.release(tmp);
}
//----------------------------------------------------------------------------
void infoFile::fillLV(HWND hwLv)
{
  ListView_DeleteAllItems(hwLv);
  if(!Rows.getElem())
    refresh();
  uint nElem = Rows.getElem();
  if(!nElem)
    return;
  for(uint i = 0; i < nElem; ++i)
    addItem(hwLv, Rows[i].id, Rows[i].text);
}
//----------------------------------------------------------------------------
void infoFile::addRow(HWND hwLv, int pos, LPTSTR buff)
{
  LVITEM item;
  item.mask = LVIF_TEXT;
  item.pszText = buff;
  item.cchTextMax = 0xfff0 / 10;
  item.iItem = pos;
  item.iSubItem = 0;
  ListView_GetItem(hwLv, &item);
  DWORD id = _ttoi(buff);
  LPTSTR p = buff;
  uint nTxt = 0;
  freeList& SetFree = getSetFree();
  LPTSTR tmp = SetFree.get();
  for(int i = 0; i < MAX_LANG; ++i) {
    *p++ = DEF_SEP_LANG;
    *p = 0;
    item.iSubItem = i + 1;
    item.pszText = tmp;
    ListView_GetItem(hwLv, &item);
    if(*tmp) {
      translateToCRNL(p, tmp);
      p += _tcslen(p);
      ++nTxt;
      }
    }
  SetFree.release(tmp);
  // se c'è un solo testo, elimina i separatori, inutile appesantire
  if(nTxt <= 1) {
    int i = 0;
    while(true) {
      buff[i] = buff[i + 1];
      if(DEF_SEP_LANG == buff[i] || !buff[i]) {
        buff[i] = 0;
        break;
        }
      ++i;
      }
    }
  int len = Rows.getElem();
  Rows[len].id = id;
  Rows[len].text = str_newdup(buff);
}
//----------------------------------------------------------------------------
void infoFile::updateFromLV(HWND hwLv)
{
  flushRow();
  Dirty = true;
  LPTSTR buff = new TCHAR[0xfff0];
  buff[0] = 0;
  int nEl = ListView_GetItemCount(hwLv);
  for(int i = 0; i < nEl; ++i)
    addRow(hwLv, i, buff);

  delete []buff;
}
//----------------------------------------------------------------------------
bool infoFile::refresh()
{
  flushRow();
  Dirty = false;
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, BasePath);
  appendPath(path, Filename);
  setOfString set(path);
  if(!set.setFirst())
    return false;
  uint i = 0;
  do {
    Rows[i].id = set.getCurrId();
    Rows[i].text = str_newdup(set.getCurrString());
    ++i;
    } while(set.setNext());
  return true;
}
//----------------------------------------------------------------------------
bool infoFile::save(bool onUnicode)
{
  if(!Dirty)
    return true;
  Dirty = false;
  uint nElem = Rows.getElem();
  if(!nElem)
    return true;

  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, BasePath);
  appendPath(path, Filename);
  setOfString set(path);
  P_File pf(path, P_CREAT);
  if(!pf.P_open())
    return false;

  for(uint i = 0; i < nElem; ++i)
    set.replaceString(Rows[i].id, str_newdup(Rows[i].text), true);

  if(!set.setFirst())
    return true;

  do {
    if(!set.writeCurrent(pf, onUnicode))
      return false;
    } while(set.setNext());
  return true;
}
//----------------------------------------------------------------------------
//-------------------------------------------------------------------

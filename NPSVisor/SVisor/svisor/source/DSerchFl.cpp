//--------- dserchfl.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "1.h"
#include "dserchfl.h"
#include "def_dir.h"
#include "p_avl.h"
#include "p_date.h"
#include "macro_utils.h"
#include "language.h"
#include "p_base.h"
//----------------------------------------------------------------------------
class TD_ChooseParam : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    TD_ChooseParam(DWORD& bit, PWin* parent, bool noHistory, uint resId = IDD_CHOOSE_BIT_SEARCH_FILE, HINSTANCE hinst = 0) :
      baseClass(parent, resId, hinst), Bit(bit), noHistory(noHistory)
      {}
    virtual ~TD_ChooseParam() { destroy(); }

    virtual bool create();

  private:
    DWORD& Bit;
    bool noHistory;

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
};
//----------------------------------------------------------------------------
bool TD_ChooseParam::create()
{
  if(!baseClass::create())
    return false;
  SET_CHECK_SET(IDC_CHECK_DATE, toBool(Bit & 1));
  SET_CHECK_SET(IDC_CHECK_DATE_ORD, toBool(Bit & 2));
  SET_CHECK_SET(IDC_CHECK_REV_ORD, toBool(Bit & 4));
  SET_CHECK_SET(IDC_CHECK_HISTORY, toBool(Bit & 8));
  if(noHistory)
    ShowWindow(GetDlgItem(*this, IDC_CHECK_HISTORY), SW_HIDE);
/**/
  setWindowTextByLangGlob(*this, ID_CHOOSE_FILE_TITLE, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDOK), ID_DIALOG_OK, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDCANCEL), ID_DIALOG_CANC, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_CHECK_DATE), ID_CHOOSE_FILE_DATE, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_CHECK_DATE_ORD), ID_CHOOSE_FILE_DATE_ORD, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_CHECK_REV_ORD), ID_CHOOSE_FILE_REV_ORD, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_CHECK_HISTORY), ID_CHOOSE_FILE_HIST, false);
/**/
  return true;
}
//----------------------------------------------------------------------------
LRESULT TD_ChooseParam::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
          CmOk();
          break;
        case IDC_BUTTON_F2:
          CmCancel();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void TD_ChooseParam::CmOk()
{
  Bit = 0;
  if(IS_CHECKED(IDC_CHECK_DATE))
    Bit |= 1;
  if(IS_CHECKED(IDC_CHECK_DATE_ORD))
    Bit |= 2;
  if(IS_CHECKED(IDC_CHECK_REV_ORD))
    Bit |= 4;
  if(!noHistory && IS_CHECKED(IDC_CHECK_HISTORY))
    Bit |= 8;
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
bool infoSearch::chooseParam(PWin* parent, bool noHistory)
{
  DWORD Bit = 0;
  if(withDate)
    Bit |= 1;
  if(orderByDate)
    Bit |= 2;
  if(reverse)
    Bit |= 4;
  if(!noHistory && history)
    Bit |= 8;
  if(IDOK != TD_ChooseParam(Bit, parent, noHistory).modal())
    return false;
  withDate = toBool(Bit & 1);
  orderByDate = toBool(Bit & 2);
  reverse = toBool(Bit & 4);
  history = !noHistory && toBool(Bit & 8);
  return true;
}
//-----------------------------------------------------------------
class set_String : public genericSet
{
  public:
    set_String() : msg(0) { }
    set_String(LPTSTR msg) : msg(msg) { }
    ~set_String() { delete []msg; }

    LPTSTR msg;
};
//-----------------------------------------------------------------
typedef set_String* pA_String;
typedef const set_String* pcA_String;
//----------------------------------------------------------------------------
class SetOfFile : public P_Avl
{
  public:
    SetOfFile(bool withDate, bool orderByDate) : P_Avl(true), withDate(withDate), orderByDate(orderByDate) { }
  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const;
    int CmpDate(LPCTSTR pChk, LPCTSTR pCur) const;

    bool withDate;
    bool orderByDate;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class TD_SearchFile : public svDialog
{
  public:
    TD_SearchFile(const infoSearch& info, PWin* parent, uint resId = IDD_SEARCH_FILE, HINSTANCE hinst = 0);
    virtual ~TD_SearchFile();

    virtual bool create();

    // ritorna il file scelto (senza il percorso né l'estensione)
    LPCTSTR getFile() { return SelFile; }
  private:
  protected:
    infoSearch Info;
    TCHAR SelFile[_MAX_PATH];

    virtual void fill_Set(LPCTSTR path);
    virtual void fill_Lb();
    virtual void addToLB(LPCTSTR name);

  protected:
    PListBox* LB_Choose;

    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    typedef svDialog baseClass;
    SetOfFile sof;
};
//-------------------------------------------------------------------
// deve essere passato solo il path per i dati correnti
// i file appartenenti alla History iniziano con "H/"
class TD_SearchCurrAndHistory : public TD_SearchFile
{
  public:
    TD_SearchCurrAndHistory(const infoSearch& curr, PWin* parent, int resId = IDD_SEARCH_FILE, HINSTANCE hinst = 0);

    virtual bool create();

  protected:
    virtual void addToLB(LPCTSTR name);
  private:
    bool useBaseAdd;
};
//----------------------------------------------------------------------------
//#define SUB_BASE_KEY _T("Software\\NPS_Programs\\SVisor")
//----------------------------------------------------------------------------
void setKeyInfoSearch(LPCTSTR keyName, DWORD value)
{
  myManageIni ini(SVISOR_INI_FILE);
  ini.parse();
  TCHAR t[256];
  wsprintf(t, _T("%lu"), value);
  ini_param param = { keyName, t };
  ini.addParam(param, MAIN_PATH);
  ini.save();
}
//----------------------------------------------------------------------------
void getKeyInfoSearch(LPCTSTR keyName, DWORD& value)
{
  myManageIni ini(SVISOR_INI_FILE);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, MAIN_PATH);
  if(str) {
    __int64 t = _tcstoi64(str, 0, 10);
    value = (DWORD)t;
    }
}
//----------------------------------------------------------------------------
bool gSearchFile(const infoSearch& info, PWin* parent, LPTSTR target)
{
  TD_SearchFile* search = 0;
  if(info.history)
    search = new TD_SearchCurrAndHistory(info, parent);
  else
    search = new TD_SearchFile(info, parent);
  bool success = IDOK == search->modal();
  if(success) {
    success = false;
    if(*search->getFile()) {
      _tcscpy_s(target, _MAX_PATH, search->getFile());
      success = true;
      }
    }
  delete search;
  return success;

}
//----------------------------------------------------------------------------
TD_SearchFile::TD_SearchFile(const infoSearch& info, PWin* parent, uint resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), Info(info), sof(info.withDate, info.orderByDate)
{
  LB_Choose = new PListBox(this, IDC_LISTBOX_CHOOSE_FILE);

  fillStr(SelFile, 0, SIZE_A(SelFile));
}
//----------------------------------------------------------------------------
TD_SearchFile::~TD_SearchFile()
{
  destroy();
}
//----------------------------------------------------------------------------
bool TD_SearchFile::create()
{
  if(!baseClass::create())
    return false;

  if(Info.withDate) {
    PRect rW;
    GetWindowRect(*this, rW);
    PRect rL;
    GetWindowRect(*LB_Choose, rL);

    int w = rW.Width() * 3 / 2;
    SetWindowPos(*this, 0, 0, 0, w, rW.Height(), SWP_NOMOVE | SWP_NOZORDER);
    int diff = rW.Width() - rL.Width();
    SetWindowPos(*LB_Choose, 0, 0, 0, w - diff, rL.Height(), SWP_NOMOVE | SWP_NOZORDER);
    }
  LB_Choose->setIntegralHeight();
  setWindowTextByLangGlob(GetDlgItem(*this, IDOK), ID_DIALOG_OK);
  setWindowTextByLangGlob(GetDlgItem(*this, IDCANCEL), ID_DIALOG_CANC);

  fill_Set(Info.getPath());
  fill_Lb();

  SetWindowText(*this, Info.getTitle());

  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void TD_SearchFile::fill_Lb()
{
  int len = 0;
  if(Info.reverse) {
    if(sof.setLast()) {
      do {
        set_String *p = reinterpret_cast<set_String*>(sof.getCurr());
        if(p) {
          int l = _tcslen(p->msg);
          if(l > len)
            len = l;
          SendMessage(*LB_Choose, LB_INSERTSTRING, -1, (LPARAM)p->msg);
          }
        } while(sof.setPrev());
      }
    }
  else {
    if(sof.setFirst()) {
      do {
        set_String *p = reinterpret_cast<set_String*>(sof.getCurr());
        if(p) {
          int l = _tcslen(p->msg);
          if(l > len)
            len = l;
          SendMessage(*LB_Choose, LB_INSERTSTRING, -1, (LPARAM)p->msg);
          }
        } while(sof.setNext());
      }
    }
  if(len > 20)
    len -= 4;
  LB_Choose->SetTabStop(1, &len);
  LB_Choose->recalcWidth();
  SendMessage(*LB_Choose, LB_SETCURSEL, 0, 0);
}
//----------------------------------------------------------------------------
void TD_SearchFile::addToLB(LPCTSTR name)
{
  LPTSTR pt = 0;
  if(!Info.withDate && Info.orderByDate) {
    int l1 = _tcslen(name);
    int l2 = _tcslen(name + l1 + 1);
    pt = new TCHAR[l1 + l2 + 2];
    copyStr(pt, name, l1 + l2 + 2);
    }

  else
    pt = str_newdup(name);

  set_String *set = new set_String(pt);
  sof.Add(set);
}
//----------------------------------------------------------------------------
#define OFFS_DATE (10 + 8 + 3 + 3)
//----------------------------------------------------------------------------
void TD_SearchFile::fill_Set(LPCTSTR path)
{
  sof.Flush();
  WIN32_FIND_DATA fff;
  HANDLE hf = FindFirstFile(path, &fff);
  if(INVALID_HANDLE_VALUE != hf) {
    do {
      if(FILE_ATTRIBUTE_DIRECTORY != fff.dwFileAttributes) {
        int len = _tcslen(fff.cFileName);
        for(--len; len > 0; --len)
          if(_T('.') == fff.cFileName[len])
            break;
        TCHAR path[_MAX_PATH] = _T("\0");
        LPTSTR p = path;
        if(Info.withDate) {
          FILETIME ft;
          FileTimeToLocalFileTime(&fff.ftLastWriteTime, &ft);
          set_format_data(path, SIZE_A(path), ft, whichData(), _T(" - "));
          p += _tcslen(path);
          _tcscpy_s(p, SIZE_A(path) - (p - path), _T(" - "));
          p += 3;
          }
        _tcsncpy_s(p, SIZE_A(path) - (p - path), fff.cFileName, len);
        p[len] = 0;
        if(!Info.withDate && Info.orderByDate) {
          p += len + 1;
          set_format_data(p, SIZE_A(path) - (p - path), fff.ftLastWriteTime, whichData(), _T("-"));
          }

        addToLB(path);
        }
      } while(FindNextFile(hf, &fff));
    FindClose(hf);
    }
}
//----------------------------------------------------------------------------
LRESULT TD_SearchFile::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_LISTBOX_CHOOSE_FILE:
          switch(HIWORD(wParam)) {
            case LBN_DBLCLK:
              PostMessage(*this, WM_COMMAND, IDOK, 0);
              break;
            }
          break;
        case IDC_BUTTON_F1:
        case IDOK:
          do {
            int sel = SendMessage(*LB_Choose, LB_GETCURSEL, 0, 0);
            if(sel < 0)
              return baseClass::windowProc(hwnd, message, wParam, lParam);
            if(Info.withDate) {
              TCHAR path[_MAX_PATH];
              SendMessage(*LB_Choose, LB_GETTEXT, sel, (LPARAM)path);
              uint offs = 0;
              if(_T('H') == path[0] && _T('/') == path[1]) {
                SelFile[0] = _T('H');
                SelFile[1] = _T('/');
                offs = 2;
                }
              _tcscpy_s(SelFile + offs, SIZE_A(SelFile) - offs, path + OFFS_DATE + offs);
              }
            else
              SendMessage(*LB_Choose, LB_GETTEXT, sel, (LPARAM)SelFile);
            } while(false);
          EndDialog(hwnd, IDOK);
          break;
        case IDC_BUTTON_F2:
        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
TD_SearchCurrAndHistory::TD_SearchCurrAndHistory(const infoSearch& curr, PWin* parent, int resId, HINSTANCE hinst)
:
    TD_SearchFile(curr, parent, resId, hinst), useBaseAdd(true) {  }
//---------------------------------------------------------
bool TD_SearchCurrAndHistory::create()
{
  if(!TD_SearchFile::create())
    return false;

  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, SIZE_A(path), Info.getPath());
  int len = _tcslen(path);
  int i;
  bool first = true;
  for(i = len - 1; i > 0; --i) {
    if(_T('\\') == path[i]) {
      if(!first)
        break;        // secondo backslash
      first = false;  // primo backslash
      }
    }
  if(i) {
    TCHAR tmp[_MAX_PATH] = _T("\0");
    getRelPath(tmp, dHistory);
    _tcscat_s(tmp, SIZE_A(tmp), path + i + 1);
    _tcscpy_s(path + i + 1, SIZE_A(path) - (i + 1), tmp);
    }
  else
    getPath(path, dHistory);
  useBaseAdd = false;
  fill_Set(path);
  fill_Lb();
  return true;
}
//---------------------------------------------------------
void TD_SearchCurrAndHistory::addToLB(LPCTSTR name)
{
  if(useBaseAdd)
    TD_SearchFile::addToLB(name);
  else {
    TCHAR path[_MAX_PATH] = _T("H/");
    _tcscat_s(path, SIZE_A(path), name);
    TD_SearchFile::addToLB(path);
    }
}
//----------------------------------------------------------------------------
//-----------------------------------------------------------------
bool operator >(const set_String& a, const set_String& b)
{

  return _tcscmp(a.msg, b.msg) > 0;
}
//-----------------------------------------------------------------
bool operator <(const set_String& a, const set_String& b)
{
  return _tcscmp(a.msg, b.msg) < 0;
}
//-----------------------------------------------------------------
/*
bool operator ==(const set_String& a, const set_String& b)
{
  return _tcscmp(a.msg, b.msg) == 0;
}
*/
//----------------------------------------------------------------------------
int SetOfFile::CmpDate(LPCTSTR pChk, LPCTSTR pCur) const
{
  while(*pChk && !_istdigit((unsigned)*pChk))
    ++pChk;
  while(*pCur && !_istdigit((unsigned)*pCur))
    ++pCur;
  if(!pChk && pCur)
    return -1;
  if(!pCur)
    return 1;
  FILETIME ftChk;
  unformat_data(pChk, ftChk, whichData());

  FILETIME ftCur;
  unformat_data(pCur, ftCur, whichData());

  if(ftChk < ftCur)
    return -1;
  if(ftChk > ftCur)
    return 1;
  return 0;
}
//----------------------------------------------------------------------------
int SetOfFile::Cmp(const TreeData toCheck, const TreeData current) const
{
  pcA_String check = reinterpret_cast<pcA_String>(toCheck);
  pcA_String curr = reinterpret_cast<pcA_String>(current);
  if(withDate) {
    LPCTSTR pChk = check->msg;
    LPCTSTR pCur = curr->msg;
    if(!orderByDate) {
      int offs = 0;
      if(_T('H') == pChk[0] && _T('/') == pChk[1])
        offs = 2;
      int result = _tcsicmp(pChk + offs + OFFS_DATE, pCur + offs + OFFS_DATE);
      if(result)
        return result;
      }
    int result = CmpDate(pChk, pCur);
    if(result)
      return result;
    }
  else if(!withDate && orderByDate) {
    LPCTSTR pChk = check->msg + _tcslen(check->msg) + 1;
    LPCTSTR pCur = curr->msg + _tcslen(curr->msg) + 1;
    int result = CmpDate(pChk, pCur);
    if(result)
      return result;
    }
  if(*check < *curr)
    return -1;
  if(*check > *curr)
    return 1;
  return 0;
}

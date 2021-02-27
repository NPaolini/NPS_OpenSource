//--------- saveData.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"

#include <stdlib.h>
#include <stdio.h>

#include "lnk_body.h"
#include "gestdata.h"
#include "id_btn.h"
#include "sizer.h"
#include "p_Util.h"
#include "1.h"
#include "saveData.h"
#include "file_lck.h"
#include "p_date.h"
#include "def_dir.h"
#include "pvaredit.h"
#include "pVarListBox.h"
#include "dserchfl.h"

#define NAME_STR _T("save_data") PAGE_EXT
//----------------------------------------------------------------------------
P_Body *get_saveData_Body(int idPar, PWin *parent)
{
  return new saveData(idPar, parent);
}
//----------------------------------------------------------------------------
// Se occorresse impostare o eseguire cose particolari questo è un esempio
// di derivazione dalla classe base
//----------------------------------------------------------------------------
class saveDataLBox : public PVarListBox
{
  public:
    saveDataLBox(int vkBtn, P_BaseBody* parent, uint style, uint id, const PRect& rect, int len = 255,
              HINSTANCE hinst = 0);
    ~saveDataLBox() {  }
    void dblClick() {  click(VkBtn); }
    bool preProcessMsg(MSG& msg);
  protected:
    // deve tornare l'HPEN restituito dalla prima SelectObject(hdc, hPen)
    // per essere reimpostato al termine. hPen verrà distrutto dalla classe base.
//    virtual HPEN setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo);
//    void paintCustom(HDC hdc, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo);
  private:
    int VkBtn;
    typedef PVarListBox baseClass;
};
//------------------------------------------------------------------
saveDataLBox::saveDataLBox(int vkBtn, P_BaseBody* parent, uint style, uint id, const PRect& rect,
        int len, HINSTANCE hinst):
       PVarListBox(parent, style, id, rect, len, hinst), baseActive(parent, id), VkBtn(vkBtn)
{

}
//----------------------------------------------------------------------------
bool saveDataLBox::preProcessMsg(MSG& msg)
{
  if(WM_KEYDOWN == msg.message)
    if(VK_RETURN == msg.wParam)
      click(VkBtn);
  return PListBox::preProcessMsg(msg);
}
//----------------------------------------------------------------------------
// pulsante associato alla scelta nella listbox
#define ID_SAVE_LB_BTN VK_F5
#define ID_UNSAVE_LB_BTN VK_F6
//----------------------------------------------------------------------------
// metodo che permette di allocare una listBox custom
PVarListBox* saveData::allocOverrideLBox(P_BaseBody* parent, uint style, uint id, const PRect& rect,
              int textlen, HINSTANCE hinst)
{
  uint fBtn = ID_INIT_VAR_LBOX == id ? ID_SAVE_LB_BTN : ID_UNSAVE_LB_BTN;
  return new saveDataLBox(fBtn, parent, style, id, rect, textlen, hinst);
}
//----------------------------------------------------------------------------
#define MAX_LEN_CODE (sizeof(FILETIME) * 2)
#define MAX_DATE    (10 + 1 + 8)
#define MAX_LEN_1   20
#define MAX_LEN_2   20
#define MAX_LEN_3   20
#define MAX_LEN_4   20
#define MAX_LEN_5   20
#define MAX_LEN_6   20
#define MAX_LEN_7   20
#define MAX_LEN_8   20
#define MAX_LEN_9   20
#define MAX_LEN_10  20
//----------------------------------------------------------------------------
static int IdParent;
//----------------------------------------------------------------------------
saveData::saveData(int idPar, PWin* parent, uint resId, HINSTANCE hinst)
:
    baseClass(idPar, parent, resId, hinst)
{
  if(idPar)
    IdParent = idPar;

}
//----------------------------------------------------------------------------
saveData::~saveData()
{
  destroy();
}
//----------------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
static
LPTSTR get_line(LPTSTR buff, int len)
{
  if(len < 0)
    return 0;
  int i;
  LPTSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if(U_(*p) != _T('\t') && U_(*p) <= _T('\r'))
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if(++i == len)
    return 0;
  if(U_(*p) != _T('\t') && U_(*p) <= _T('\r') && *p != c) {
    ++p;
    ++i;
    }
  return(i < len ? p : 0);
}
//----------------------------------------------------------------------------
void saveData::fillLb(LPCTSTR file)
{
  if(LBox.getElem() < 2)
    return;

  ::SendMessage(*LBox[0], LB_RESETCONTENT, 0 ,0);
  ::SendMessage(*LBox[1], LB_RESETCONTENT, 0 ,0);

  P_File pf(file, P_READ_ONLY);
  if(!pf.P_open())
    return;
  int len = (int)pf.get_len();
  LPTSTR buff = new TCHAR[len + 2];
  pf.P_read(buff, len);
  buff[len] = 0;
  buff[len + 1] = 0;
  LPTSTR p = buff;
  while(p && *p) {
    LPTSTR p2 = get_line(p, len);
    addRow(p);
    if(p2)
      len -= p2 - p;
    else
      len = 0;
    p = p2;
    }
  delete []buff;
  handleSelChange(*LBox[1]);
}
//----------------------------------------------------------------------------
#define MAX_LEN_ROW (MAX_LEN_CODE + MAX_DATE + MAX_DATE +\
                     MAX_LEN_1 + MAX_LEN_2 + MAX_LEN_3 + MAX_LEN_4 + \
                     MAX_LEN_5 + MAX_LEN_6 + MAX_LEN_7 + MAX_LEN_8 + \
                     MAX_LEN_9 + MAX_LEN_10 + 13 + 8)
//----------------------------------------------------------------------------
/*
static
void copy_Str(LPTSTR buff, LPCTSTR name, int maxLen)
{
  for(int i = 0; i < maxLen; ++i) {
    if(!name[i])
      break;
    buff[i] = name[i];
    }
}
*/
//----------------------------------------------------------------------------
static
void formatRow(LPTSTR target, LPTSTR row)
{
  FILETIME ft;
  unformat_data(row, ft, whichData());
  __int64 i64 = MK_I64(ft);
  wsprintf(target, _T("%016I64x\t%s\t"), i64, row);
}
//----------------------------------------------------------------------------
void saveData::addRow(LPTSTR row)
{
  int id = findNextParam(row, 3, _T('\t')) ? 1 : 0;
  TCHAR buff[MAX_LEN_ROW];
  formatRow(buff, row);
  HWND hwnd = *LBox[id];
  int sel = SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)buff);
  SendMessage(hwnd, LB_SETCURSEL, sel, 0);
}
//--------------------------------------------------------------------
bool saveData::create()
{
  if(!baseClass::create())
    return false;
  Attr.id = IDD_SAVE_DATA;

  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void saveData::setReady(bool first)
{
  baseClass::setReady(true);
  if(LBox.getElem()) {
    int tabs[] = { MAX_LEN_CODE, MAX_DATE, MAX_DATE,
          MAX_LEN_1, MAX_LEN_2, MAX_LEN_3, MAX_LEN_4,
          MAX_LEN_5, MAX_LEN_6, MAX_LEN_7, MAX_LEN_8,
          MAX_LEN_9, MAX_LEN_10
          };
    int shows[] = { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    LBox[0]->SetTabStop(3, tabs, shows);
    LBox[1]->SetTabStop(SIZE_A(tabs), tabs, shows);
    LBox[0]->recalcWidth();
    LBox[0]->setIntegralHeight();
    LBox[1]->recalcWidth();
    LBox[1]->setIntegralHeight();
    fillLb();
    }
  refresh();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
P_Body* saveData::pushedBtn(int idBtn)
{
  extern void runPerif(PWin* main);

  switch(idBtn) {
    case ID_F1:
      if(IdParent)
        return getBody(IdParent, Par, 0);
      break;

    case ID_F4:
      actionByListboxSave();
      break;
    case ID_F5:
      actionByListboxUnsave();
      break;
    case ID_F7:
      if(!chooseFile())
        break;
    case ID_F9:
      fillLb();
      break;

    default:
      break;
    }
  return baseClass::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
LPTSTR saveData::addData(LPTSTR buff, uint ix)
{
  LPCTSTR p = sStr.getString(ix + ID_INIT_VAR_EDI + ADD_INIT_VAR);

  if(p) {
    int idprf;
    int addr;
    int type;
    int norm;
    int dec;
    _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &idprf, &addr, &type, &norm, &dec);

    genericPerif *prf = Par->getGenPerif(idprf);
    if(!prf)
      return 0;

    prfData data(dec);
    data.lAddr = addr;
    prf->get(data);
    fillStr(buff, _T(' '), dec);
    int len = strlen((LPSTR)data.U.str.buff);
    copyStr(buff, data.U.str.buff, len);
    buff += dec;
    *buff++ = _T('\t');
    return buff;
    }
  return 0;
}
//----------------------------------------------------------------------------
LPTSTR saveData::remData(LPTSTR buff, uint ix)
{
  LPCTSTR p = sStr.getString(ix + ID_INIT_VAR_EDI + ADD_INIT_VAR);

  if(p) {
    int idprf;
    int addr;
    int type;
    int norm;
    int dec;
    _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &idprf, &addr, &type, &norm, &dec);

    genericPerif *prf = Par->getGenPerif(idprf);
    if(!prf)
      return 0;

    buff[dec] = 0;
    trim(buff);
    prfData data(dec, (LPBYTE)buff);
    data.lAddr = addr;
    prf->set(data);
    buff += dec + 1;
    return buff;
    }
  return 0;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MAX_EDIT_SAVE 10
//----------------------------------------------------------------------------
void saveData::actionByListboxSave()
{
  if(LBox.getElem() < 2)
    return;

  int sel = ::SendMessage(*LBox[0], LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[MAX_LEN_ROW + 2];
  ::SendMessage(*LBox[0], LB_GETTEXT, sel, (LPARAM)buff);
  LPTSTR p = const_cast<LPTSTR>(findNextParam(buff, 2, _T('\t')));
  if(!p)
    return;
  p += _tcslen(p);
  if(*(p - 1) != _T('\t'))
    *p++ = _T('\t');

  for(int i = 0; i < MAX_EDIT_SAVE; ++i)
    p = addData(p, i);

  SendMessage(*LBox[0], LB_DELETESTRING, sel, 0);
  if(sel >= SendMessage(*LBox[0], LB_GETCOUNT, 0, 0))
    --sel;
  SendMessage(*LBox[0], LB_SETCURSEL, sel, 0);

  sel = SendMessage(*LBox[1], LB_ADDSTRING, 0, (LPARAM)buff);
  ::SendMessage(*LBox[1], LB_SETCURSEL, sel, 0);
  saveAll();
}
//----------------------------------------------------------------------------
void saveData::actionByListboxUnsave()
{
  if(LBox.getElem() < 2)
    return;

  int sel = ::SendMessage(*LBox[1], LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[MAX_LEN_ROW + 2];
  ::SendMessage(*LBox[1], LB_GETTEXT, sel, (LPARAM)buff);
  LPTSTR p = const_cast<LPTSTR>(findNextParam(buff, 3, _T('\t')));
  if(!p)
    return;
  LPTSTR p2 = p;
  for(int i = 0; i < MAX_EDIT_SAVE; ++i)
    p2 = remData(p2, i);

  --p;
  *p = 0;
//  while(*p && *p == _T(' ')) {
//    *p = 0;
//    --p;
//    }


  SendMessage(*LBox[1], LB_DELETESTRING, sel, 0);
  if(sel >= SendMessage(*LBox[1], LB_GETCOUNT, 0, 0))
    --sel;
  SendMessage(*LBox[1], LB_SETCURSEL, sel, 0);

  sel = SendMessage(*LBox[0], LB_ADDSTRING, 0, (LPARAM)buff);
  ::SendMessage(*LBox[0], LB_SETCURSEL, sel, 0);
  saveAll();
}
//----------------------------------------------------------------------------
void saveData::getFileStr(LPTSTR path)
{
  getFileStrCheckRemote(path, NAME_STR);
}
//----------------------------------------------------------------------------
LRESULT saveData::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {

    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case LBN_DBLCLK:
          handleLBDblClick((HWND)lParam);
          break;
        case LBN_SELCHANGE:
          handleSelChange((HWND)lParam);
          break;
        case LBN_SETFOCUS:
          handleSelChange((HWND)lParam);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void saveData::handleSelChange(HWND hwnd)
{
  if(hwnd != *LBox[1])
    return;
  int sel = ::SendMessage(*LBox[1], LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR buff[MAX_LEN_ROW + 2];
  ::SendMessage(*LBox[1], LB_GETTEXT, sel, (LPARAM)buff);
  LPTSTR p = const_cast<LPTSTR>(findNextParam(buff, 3, _T('\t')));
  if(!p)
    return;
  for(int i = 0; i < MAX_EDIT_SAVE; ++i)
    p = remData(p, i);
}
//----------------------------------------------------------------------------
void saveData::handleLBDblClick(HWND hwnd)
{
  PWin* w = getWindowPtr(hwnd);
  saveDataLBox* lb = dynamic_cast<saveDataLBox*>(w);
  if(lb)
    lb->dblClick();
}
//----------------------------------------------------------------------------
#define PREFIX_FILE _T("job")
#define EXT_FILE _T(".txt")
#define TEMPLATE_FILE _T("????????") PREFIX_FILE EXT_FILE
//----------------------------------------------------------------------------
#define ADDR_FILENAME 80
//----------------------------------------------------------------------------
bool saveData::chooseFile()
{
  TCHAR path[_MAX_PATH] = _T("");
  getPathExt(path);
  _tcscat_s(path, SIZE_A(path), TEMPLATE_FILE);
  TCHAR tit[] = _T("Search");

  static infoSearch stInfo;
  infoSearch info(tit, path, stInfo);

  if(!info.chooseParam(this, true))
    return false;

  stInfo.copyData(info);

  if(!gSearchFile(info, this, path))
    return false;
  prfData data(20, (LPBYTE)path);

  data.lAddr = ADDR_FILENAME;

  genericPerif *prf = Par->getGenPerif(PRF_MEMORY);

  prf->set(data);
  return true;
}
//----------------------------------------------------------------------------
void saveData::fillLb()
{
  if(LBox.getElem() < 2)
    return;
  prfData data(20);
  data.lAddr = ADDR_FILENAME;

  genericPerif *prf = Par->getGenPerif(PRF_MEMORY);

  prf->get(data);

  TCHAR path[_MAX_PATH] = _T("");
  copyStrZ(path, (LPCSTR)data.U.str.buff);
  getPathExt(path);
  _tcscat_s(path, SIZE_A(path), EXT_FILE);
  if(*path)
    fillLb(path);
}
//----------------------------------------------------------------------------
void saveData::saveAll()
{
  if(LBox.getElem() < 2)
    return;
  prfData data(20);
  data.lAddr = ADDR_FILENAME;

  genericPerif *prf = Par->getGenPerif(PRF_MEMORY);

  prf->get(data);

  TCHAR path[_MAX_PATH] = _T("");
  copyStrZ(path, (LPCSTR)data.U.str.buff);
  getPathExt(path);
  _tcscat_s(path, SIZE_A(path), EXT_FILE);
  if(!*path)
    return;
  if(P_File::P_exist(path))
    if(!showMsgFileExist(this))
      return;
  P_File pf(path, P_CREAT);
  if(!pf.P_open())
    return;
  TCHAR buff[MAX_LEN_ROW + 10];
  for(int j = 0; j < 2; ++j) {
    int nElem = SendMessage(*LBox[j], LB_GETCOUNT, 0, 0);
    for(int i = 0; i < nElem; ++i) {
      SendMessage(*LBox[j], LB_GETTEXT, i, (LPARAM)buff);
      LPTSTR p = buff + MAX_LEN_CODE + 1;
      pf.P_writeString(p);
      pf.P_writeString(_T("\r\n"));
      }
    }
}
//----------------------------------------------------------------------------
// questa la scrivi in gestData, fuori dalla funzione, prima di richiamarla
//extern bool saveJobTime(const FILETIME& init, const FILETIME& end);
//----------------------------------------------------------------------------
bool saveJobTime(const FILETIME& init, const FILETIME& end)
{
  TCHAR name[_MAX_PATH];
  SYSTEMTIME st;
  FileTimeToSystemTime(&init, &st);
  makeYearMonthAndDay(name, st);
  _tcscat_s(name, SIZE_A(name), PREFIX_FILE EXT_FILE);
  getPathExt(name);
  P_File pf(name);
  if(!pf.P_open())
    return false;
  pf.P_seek(0, SEEK_END_);
  TCHAR date1[30];
  set_format_data(date1, SIZE_A(date1), init, whichData(), _T("-"));
  TCHAR date2[30];
  set_format_data(date2, SIZE_A(date2), end, whichData(), _T("-"));
  TCHAR buff[100];
  _stprintf_s(buff, SIZE_A(buff), _T("%s\t%s\r\n"), date1, date2);
  pf.P_writeString(buff);
  return true;
}
//----------------------------------------------------------------------------

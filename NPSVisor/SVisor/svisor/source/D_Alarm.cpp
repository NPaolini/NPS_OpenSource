//--------- d_alarm.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include "p_body.h"
#include "d_alarm.h"
#include "config.h"
#include "sizer.h"
#include "macro_utils.h"
#include "P_BarProgr.h"

#include "dserchfl.h"
#include "svEdit.h"
#include "p_date.h"
#include "def_dir.h"
#include "currAlrm.h"

#include "password.h"
#include "p_name.h"

#include "1.h"

#include "printAlarm.h"
#include "pCommonFilter.h"
#include "pListView.h"
#include "POwnBtnImageStd.h"
#include "pVisualTextEdit.h"

#define TParent mainClient
#define CAST(a) const_cast<TParent*>(dynamic_cast<const TParent*>(a))
//----------------------------------------------------------------------------
DWORD findFirstPos(P_File& pFile, const FILETIME& ft, int lenRec, LPBYTE buff, int sizeHeader, bool prev, int offset=0);
//----------------------------------------------------------------------------
#define D_ID_ACTIVE 2
#define D_ID_PERIF  2
#define D_ID_AL_GRP 2
#define D_ID_COD_ALARM 14
//#define D_ID_COD_ALARM 12
#define D_DESCR_AL_GRP 20
#define D_COD_REPORT 6
#define D_TIME 9
//#define D_DESCR_ALARM 30
//#define D_DESCR_ALARM 80
#define D_DESCR_ALARM 200

#define D_START_STOP 8
#define D_DATE 11
//----------------------------------------------------------------------------
//static
//inline
int getIdAlarmByRow(LPCTSTR row)
{
  int id = _ttoi(row + D_ID_ACTIVE + D_ID_PERIF + D_ID_AL_GRP + 3);
  int idPrph = _ttoi(row + D_ID_ACTIVE + 1);
  return makeIdAlarmByPrph(id, idPrph);
}
//----------------------------------------------------------------------------
int getIdAlarmByRowReport(LPCTSTR row)
{
  int id = _ttoi(row + D_START_STOP + 1);
  int idPrph = _ttoi(row + D_START_STOP + D_COD_REPORT + 2);
  return makeIdAlarmByPrph(id, idPrph);
}
//----------------------------------------------------------------------------
static PWin* gAlarmWin = 0;
//----------------------------------------------------------------------------
void forceCloseAlarm(bool alsoModal)
{
  if(gAlarmWin)
    SendMessage(*gAlarmWin, WM_COMMAND, MAKEWPARAM(IDC_BUTTON_F1, 0), 0);
}
//----------------------------------------------------------------------------
void showAlarm(PWin* par, perifsSet *perif)
{
  static bool inExec;
  if(inExec)
    return;
  inExec = true;
  HWND hwnd = GetFocus();
  TD_Alarm dAlarm(par, perif);
  gAlarmWin = &dAlarm;
  dAlarm.modal();
  gAlarmWin = 0;
  FilterAlarm* fAl = getFilterAlarm();
  fAl->clearAll();
//  setFilterAlarm(0);
  SetFocus(hwnd);
  inExec = false;
}
//----------------------------------------------------------------------------
class myListGen : public PListBox
{
  public:
    myListGen(PWin* parent, uint id, int len = 255, HINSTANCE hinst = 0):
       PListBox(parent, id, len, hinst) { }
  protected:
    HPEN set_Color(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo, int ixColor);
};
//----------------------------------------------------------------------------
class myListAlarm : public myListGen
{
  public:
    myListAlarm(PWin* parent, uint id, int len=255, HINSTANCE hinst = 0):
       myListGen(parent, id, len, hinst), drawCod(false) { }
  protected:
    virtual int verifyKey();
    virtual bool evChar(WPARAM& key);
    virtual HPEN setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo);
    virtual void paintTxt(HDC hDC, int x, int y, LPCTSTR buff, int ix, DRAWITEMSTRUCT FAR& drawInfo);
    bool drawCod;
};
//----------------------------------------------------------------------------
class myListReport : public myListGen
{
  public:
    myListReport(PWin* parent, uint id, int len=255, HINSTANCE hinst = 0):
       myListGen(parent, id, len, hinst)
    {
      fillStr(init, ' ', D_START_STOP);
      init[D_START_STOP] = 0;
      fillStr(end, ' ', D_START_STOP);
      end[D_START_STOP] = 0;
      do {
        smartPointerConstString p(getStringOrIdByLangGlob(ID_DATA_TYPE_INIT));
        int l = _tcslen(p);
        if(l > D_START_STOP)
          l = D_START_STOP;
        _tcsncpy_s(init, SIZE_A(init), p, l);
      } while(false);
      smartPointerConstString p(getStringOrIdByLangGlob(ID_DATA_TYPE_END));
      int l = _tcslen(p);
      if(l > D_START_STOP)
        l = D_START_STOP;
      _tcsncpy_s(end, SIZE_A(end), p, l);
    }
  protected:
    virtual HPEN setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo);
    TCHAR init[D_START_STOP + 1];
    TCHAR end[D_START_STOP + 1];
};
//----------------------------------------------------------------------------
#define H_FONT_AL 16
#define DEF_NAME_INFO_ALARM _T("infoAlarm") PAGE_EXT
//----------------------------------------------------------------------------
TCHAR TD_Alarm::FilterText[512];
//----------------------------------------------------------------------------
TD_Alarm::TD_Alarm(PWin* parent, perifsSet *perif, uint resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), checkBtn(false), Perif(perif),
    currRep(0), currShow(SHOW_ALARM), idTimer(0), //InfoAlarm(DEF_NAME_INFO_ALARM),
    waitRefresh(1)
{
  allocInfoAlarm(InfoAlarm);

  LPCTSTR p = getString(ID_SIZE_FONT_ALARM);
  int h_font = H_FONT_AL;
  if(p)
    h_font = _ttoi(p);
  if(!h_font)
    h_font = H_FONT_AL;

  p = getString(ID_NAME_FONT_ALARM);
  if(!p)
    p = _T("arial");
  SIZE szOld;
  sizer::eSizer oldRes = sizer::getDefault(szOld);
  sizer::setDefault(sizer::s800x600);
  Font = D_FONT(R__Y(h_font), 0, 0, p);

  double fix = 0;

  p = getString(ID_FIXED_WIDTH_FONT_ALARM);
  if(p)
    fix = _tstof(p);

  LB_Alarm = new myListAlarm(this, IDC_LISTBOX_ALARM);
  LB_Alarm->setFont(Font, false);

  if(fix > 0)
    LB_Alarm->setPixelWidthChar(R__Xf(fix));

  int showType = 1;
  p = getString(ID_SHOW_COLUMN_TYPE_ALARM);
  if(p && _ttoi(p))
    showType = 0;
  p = findNextParamTrim(p);
  int showDate = 0;
  int hidePrphAlarm = 0;
  int hidePrphReport = 0;
  if(p) {
    bool order = toBool(_ttoi(p));
    currAlarm *Al = getGestAlarm();
    Al->setOrder(order);

    p = findNextParamTrim(p);
    if(p) {
      waitRefresh = _ttoi(p);
      p = findNextParamTrim(p);
      if(p && _ttoi(p))
        showDate = 1;
      p = findNextParamTrim(p);
      if(p && _ttoi(p))
        hidePrphAlarm = 1;
      p = findNextParamTrim(p);
      if(p && _ttoi(p))
        hidePrphReport = 1;
      }
    }
  int t[] = {  D_ID_ACTIVE, D_ID_PERIF, D_ID_AL_GRP, D_ID_COD_ALARM, D_DESCR_AL_GRP, D_DATE, D_TIME, D_DESCR_ALARM };
  int show_alarm[] = { 0, !hidePrphAlarm, showType, showType, showType, showDate, 1, 1 };

  for(uint i = 0; i < SIZE_A(showAlarm); ++i)
    showAlarm[i] = show_alarm[i + 1];

  LB_Alarm->SetTabStop(SIZE_A(t), t, show_alarm);
  LB_Alarm->setAlign(1, PListBox::aCenter);
  LB_Alarm->setAlign(2, PListBox::aCenter);
  LB_Alarm->setAlign(3, PListBox::aCenter);

  LB_Report = new myListReport(this, IDC_LISTBOX_REPORT);
  LB_Report->setFont(Font, false);
  if(fix > 0)
    LB_Report->setPixelWidthChar(R__Xf(fix));
  int tRep[] = { D_START_STOP, D_COD_REPORT, D_ID_PERIF, D_DESCR_AL_GRP, D_DATE, D_TIME, D_DESCR_ALARM };
  int show_report[] = { 1, 1, !hidePrphReport, showType, 1, 1, 1 };
  for(uint i = 0; i < SIZE_A(showReport); ++i)
    showReport[i] = show_report[i];

  LB_Report->SetTabStop(SIZE_A(tRep), tRep, show_report);
  LB_Report->setAlign(0, PListBox::aCenter);
  LB_Report->setAlign(1, PListBox::aCenter);
  LB_Report->setAlign(2, PListBox::aCenter);
  LB_Report->setAlign(4, PListBox::aCenter);

  Frame1 = new PStatic(this, IDC_STATICFRAME_AL_F1);
  HeadLabel = new PStatic(this, IDC_STATIC_HEAD_ALARM);
  HeadLabel->setFont(Font, false);
  Exit = new PStatic(this, IDC_STATICTEXT_AL_EXIT);
  Reset = new PStatic(this, IDC_STATICTEXT_RESET_AL);
  Show = new PStatic(this, IDC_STATICTEXT_SHOW_REP);

  int idRB[] = {
      IDC_RADIOBUTTON_NO_REP,
      IDC_RADIOBUTTON_ONLY_ALARM,
      IDC_RADIOBUTTON_ALL_REPORT
      };
  for(int i = 0; i < SIZE_A(idRB); ++i)
    RB_TypeRep[i] = new PRadioButton(this, idRB[i]);
  sizer::setDefault(oldRes, &szOld);
}
//----------------------------------------------------------------------------
TD_Alarm::~TD_Alarm()
{
  destroy();
  DeleteObject(Font);
  flushPV(InfoAlarm);
}
//----------------------------------------------------------------------------
void TD_Alarm::makeHeader(bool report)
{
  TCHAR head[4000] = _T("");

  if(report) {
    for(uint i = 0; i < SIZE_A(showReport); ++i) {
      if(showReport[i]) {
        smartPointerConstString p(getStringOrIdByLangGlob(ID_HEAD_REPORT_EVENT + i));
        _tcscat_s(head, p);
        _tcscat_s(head, _T(" | "));
        }
      }
    }
  else {
    for(uint i = 0; i < SIZE_A(showAlarm); ++i) {
      if(showAlarm[i]) {
        smartPointerConstString p(getStringOrIdByLangGlob(ID_HEAD_ALARM_PRPH + i));
        _tcscat_s(head, p);
        _tcscat_s(head, _T(" | "));
        }
      }
    }
  head[_tcslen(head) - 3] = 0;
  HeadLabel->setCaption(head);
}
//----------------------------------------------------------------------------
#define SEP TAB
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void TD_Alarm::print_Alarm(bool preview, bool setup)
{
  currAlarm *Al = getGestAlarm();
  if(!Al->setFirst())
    return;
  svPrinter* Printer = getPrinter();
  if(!Printer)
    return;
  LPCTSTR fn = _T("~rows.dat");
  do {
    P_File pf(fn, P_CREAT);
    if(!pf.P_open())
      return;

    rowAlarmFileHeader Header;
    _tcscpy_s(Header.title, SIZE_A(Header.title), _T("Allarmi correnti"));
    Header.type = 1;
    pf.P_write(&Header, sizeof(Header));

    SYSTEMTIME st;
    currAlarm::alarmStat stat;
    rowAlarmFile raf;
    do {
      fullInfoAlarm fia;
      Al->get(&fia, &st, stat);
      SystemTimeToFileTime(&st, &raf.ft);
      raf.stat = stat;
      raf.alarm = fia.alarm;
      raf.prph = fia.getTruePrph();
      makeAlarmDescr(raf.descr, D_DESCR_ALARM, fia);
      pf.P_write(&raf, sizeof(raf));
      } while(Al->setNext());
    } while(false);

  printAlarm pa(this, Printer, fn);

  if(preview)
    pa.Preview(setup);
  else
    pa.Print(setup);
}
//----------------------------------------------------------------------------
int TD_Alarm::checkExist(const statAlarm& sa)
{
  int nElem = Status.getElem();
  for(int i = 0; i < nElem; ++i) {
    // se esiste
    if(sa.id == Status[i].id) {
      // se lo stato è lo stesso
      if(sa.stat == Status[i].stat) {
        // annulla l'id per indicare nessuna modifica
        Status[i].id = 0;
        return true;
        }
      else {
        // lo segna per aggiornare e far ridisegnare la riga
        Status[i].id = -Status[i].id;
        Status[i].stat = sa.stat;
        // se è un allarme riattivato deve riscriverlo da capo
        if(currAlarm::asActive == sa.stat)
          return false;
        return true;
        }
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool TD_Alarm::updateStatus(int& oldSel, int row, int ix)
{
  TCHAR buff[500];
  SendMessage(*LB_Alarm, LB_GETTEXT, row, (LPARAM)buff);
  int id = getIdAlarmByRow(buff);
  int oldRow = row;
  if(id != -Status[ix].id) {
    while(SendMessage(*LB_Alarm, LB_GETTEXT, ++row, (LPARAM)buff) >= 0) {
      id = getIdAlarmByRow(buff);
      if(id == -Status[ix].id)
        break;
      }
    }
  if(id != -Status[ix].id) {
    row = oldRow;
    while(--row >= 0) {
      SendMessage(*LB_Alarm, LB_GETTEXT, row, (LPARAM)buff);
      id = getIdAlarmByRow(buff);
      if(id == -Status[ix].id)
        break;
      }
    }
  if(id != -Status[ix].id)
    return false;
  SendMessage(*LB_Alarm, LB_DELETESTRING , row, 0);
  // se non si tratta di una riattivazione deve aggiornare quello esistente
  switch(Status[ix].stat) {
    case currAlarm::asActive:
      if(oldSel >= row)
        --oldSel;
      return true;
    case currAlarm::asInactive:
      buff[0] = _T('1');
      break;
    default:
      buff[0] = _T('2');
      break;
    }
  SendMessage(*LB_Alarm, LB_INSERTSTRING, row, (LPARAM)buff);
  return false;
}
//----------------------------------------------------------------------------
void TD_Alarm::removeItem(int& oldSel, int row, int ix)
{
  TCHAR buff[500];
  SendMessage(*LB_Alarm, LB_GETTEXT, row, (LPARAM)buff);
  int id = getIdAlarmByRow(buff);
  int oldRow = row;
  if(id != Status[ix].id) {
    while(SendMessage(*LB_Alarm, LB_GETTEXT, ++row, (LPARAM)buff) >= 0) {
      id = getIdAlarmByRow(buff);
      if(id == Status[ix].id)
        break;
      }
    }
  if(id != Status[ix].id) {
    row = oldRow;
    while(--row >= 0) {
      SendMessage(*LB_Alarm, LB_GETTEXT, row, (LPARAM)buff);
      id = getIdAlarmByRow(buff);
      if(id == Status[ix].id)
        break;
      }
    }
  if(id != Status[ix].id)
    return;
  SendMessage(*LB_Alarm, LB_DELETESTRING , row, 0);
  if(oldSel >= row)
    --oldSel;
}
//----------------------------------------------------------------------------
void TD_Alarm::update_LB()
{
  currAlarm *Al = getGestAlarm();
  Al->commit();
  bool oldOrder = Al->isReverseOrder();
  Al->setOrder(!oldOrder);
  if(!Al->setFirst()) {
    SendMessage(*LB_Alarm, LB_RESETCONTENT, 0, 0);
    Status.reset();
    Al->setOrder(oldOrder);
    return;
    }
  int oldSel = SendMessage(*LB_Alarm, LB_GETCURSEL, 0, 0);
  FILETIME ft;
  int id;
  currAlarm::alarmStat stat;
  int added = 0;
  do {
    fullInfoAlarm fia;
    Al->get(&fia, &ft, stat);
    id = fia.makeFullIdAlarm();
    statAlarm sa(id, stat);
    if(!checkExist(sa)) {
      SYSTEMTIME st;
      FileTimeToSystemTime(&ft, &st);
      if(formatAlarm(fia, st, stat, oldOrder))
        ++added;
      }
    } while(Al->setNext());
  Al->setOrder(oldOrder);

  int nElem = Status.getElem();

  // se l'ordine non è rovesciato le aggiunte sono in coda
  if(!oldOrder)
    added = 0;
  oldSel += added;
  int removed = 0;
  for(int i = 0; i < nElem; ++i) {
    if(0 != Status[i].id) {
      // è cambiato solo lo stato
      if(0 > Status[i].id) {
        if(updateStatus(oldSel, i + added - removed, i))
          ++removed;
        }
      else {
        removeItem(oldSel, i + added - removed, i);
        ++removed;
        }
      }
    }
  Status.reset();
  Al->setFirst();
  int ix = 0;
  do {
    fullInfoAlarm fia;
    Al->get(&fia, &ft, stat);
    id = fia.makeFullIdAlarm();
    statAlarm sa(id, stat);
    Status[ix] = sa;
    ++ix;
    } while(Al->setNext());

  int count = SendMessage(*LB_Alarm, LB_GETCOUNT, 0, 0);
  if(count)
    ::SetDlgItemInt(*this, IDC_STATICTEXT_NUM_ON_LB, count, false);
  else
    ::SetDlgItemText(*this, IDC_STATICTEXT_NUM_ON_LB, _T(""));

  if(oldSel < 0)
    oldSel = 0;
  SendMessage(*LB_Alarm, LB_SETCURSEL, oldSel, 0);

}
//--------------------------------------------------------------------------
#define KEY_REP_ALARM _T("Report Alarm Files")
//----------------------------------------------------------------------------
void TD_Alarm::fill_LB(int action)
{
  int oldCount = -1;
  int oldSel;

  PListBox *lb;
  if(SHOW_ALARM == currShow) {
    currAlarm *Al = getGestAlarm();
    bool changed = Al->isDirty() || action;
    if(!changed)
      return;
    if(!action) {
      update_LB();
      return;
      }
    oldCount = SendMessage(*LB_Alarm, LB_GETCOUNT, 0, 0);
    oldSel = SendMessage(*LB_Alarm, LB_GETCURSEL, 0, 0);
    SendMessage(*LB_Alarm, LB_RESETCONTENT, 0, 0);
    Al->commit();
    if(!Al->setFirst())
      return;
    SYSTEMTIME st;
    int id;
    currAlarm::alarmStat stat;
    Status.reset();
    int ix = 0;
    do {
      fullInfoAlarm fia;
      Al->get(&fia, &st, stat);
      if(formatAlarm(fia, st, stat)) {
        id = fia.makeFullIdAlarm();
        statAlarm sa(id, stat);
        Status[ix] = sa;
        ++ix;
        }
      } while(Al->setNext());
    lb = LB_Alarm;
    }
  else {
    if(!action)
      return;
    _tcscpy_s(filenameReport, NAME_FILE_REP_ALARM2);
    _tcscat_s(filenameReport, _T("*"));
    makePath(filenameReport, dRepAlarm, false);

    smartPointerConstString sp(getTitleRepAlarm());
    static infoSearchParam isp;
    static TCHAR path[_MAX_PATH] = {0};
    bool success = true;
    bool history = false;
    if(2 != action) {
      getKeyInfoSearch(KEY_REP_ALARM, isp);
      infoSearch info(sp, filenameReport, isp);

      do {
        cfg _cfg = config().getAll();
        if(_cfg.noAdvancedReport) {
          info.history = true;
          info.reverse = true;
          info.withDate = true;
          info.orderByDate = true;
          }
        else
          success = info.chooseParam(this, false);
        } while(false);
      if(success) {
        info.copyDataTo(isp);
        setKeyInfoSearch(KEY_REP_ALARM, isp);
        if(gSearchFile(info, this, path)) {
          if(!_tcsncmp(path, _T("H/"), 2))
            history = true;
          }
        else
          success = false;
        }
      }
    if(success) {
      makePath(filenameReport, history ? path + 2 : path, dRepAlarm, history);
      fill_Report(filenameReport, 2 != action);
      }

    lb = LB_Report;
    }
  int count = SendMessage(*lb, LB_GETCOUNT, 0, 0);
  if(count)
    ::SetDlgItemInt(*this, IDC_STATICTEXT_NUM_ON_LB, count, false);
  else
    ::SetDlgItemText(*this, IDC_STATICTEXT_NUM_ON_LB, _T(""));

  if(-1 != oldCount) {
    oldSel += count - oldCount;
    if(oldSel < 0)
      oldSel = 0;
    SendMessage(*lb, LB_SETCURSEL, oldSel, 0);
    }
  else
    SendMessage(*lb, LB_SETCURSEL, 0, 0);
  if(getHandle() == GetForegroundWindow())
    SetFocus(*lb);
}
//----------------------------------------------------------------------------
#define FILL__(a, b) fillStr(a, b, SIZE_A(a)-1); a[SIZE_A(a)-1] = 0
#define SPACE__(a) FILL__(a, _T(' '))
#define ALL_DIM (D_ID_ACTIVE + D_ID_PERIF + D_ID_AL_GRP + D_ID_COD_ALARM + D_DESCR_AL_GRP + D_DATE + D_TIME + D_DESCR_ALARM + 9)
//----------------------------------------------------------------------------
bool TD_Alarm::acceptRowAlarm(const fullInfoAlarm& fia)
{
  if(SendMessage(GetDlgItem(*this, IDC_EDIT_FILTER_TEXT_ALARM), EM_GETMODIFY, 0, 0))
    return true;
  GET_TEXT(IDC_EDIT_FILTER_TEXT_ALARM, FilterText);
  trim(lTrim(FilterText));
  if(*FilterText) {
    TCHAR descr[D_DESCR_ALARM];
    makeAlarmDescr(descr, D_DESCR_ALARM, fia);
    _tcslwr_s(descr);
    _tcslwr_s(FilterText);
    return toBool(_tcsstr(descr, FilterText));
    }
  return true;
}
//----------------------------------------------------------------------------
bool TD_Alarm::formatAlarm(const fullInfoAlarm& fia, const SYSTEMTIME &st, int stat, bool atTop)
{
  if(!acceptRowAlarm(fia))
    return false;

  TCHAR buff[ALL_DIM + 4];
  TCHAR aIAct[D_ID_ACTIVE +1];
  TCHAR aIdPrf[D_ID_PERIF +1];
  TCHAR aId[D_ID_AL_GRP +1];
  TCHAR aIdCod[D_ID_COD_ALARM + 10];
  TCHAR aCod[D_DESCR_AL_GRP +1];
  TCHAR aDescr[D_DESCR_ALARM +1];

  SPACE__(aIAct);
  SPACE__(aIdPrf);
  SPACE__(aId);
  SPACE__(aCod);
  SPACE__(aIdCod);
  SPACE__(aDescr);

  switch(stat) {
    case currAlarm::asActive:
      aIAct[0] = _T('0');
      break;
    case currAlarm::asInactive:
      aIAct[0] = _T('1');
      break;
    default:
      aIAct[0] = _T('2');
      break;
    }
  aIdPrf[0] = fia.getTruePrph() + _T('0');
  aId[0] = fia.idGrp + _T('0');
  int nBitAlarm = 32;
  mainClient* par = getMain();
  genericPerif *prf = par->getGenPerif(fia.getTruePrph());
//  gestPerif* prf = par->getPerif(fia.getTruePrph());
  if(prf)
    nBitAlarm = prf->getNBitAlarm();

  int addr = (fia.alarm - 1) / nBitAlarm;
  if(prf)
    addr += prf->getAddrAlarm();

  TCHAR descr[D_DESCR_ALARM];
  wsprintf(descr, _T("%d(%d,%d)"), fia.alarm, addr, (fia.alarm - 1) % nBitAlarm);
  int len = _tcslen(descr);
  if(len > D_ID_COD_ALARM)
    len = D_ID_COD_ALARM;
  copyStr(aIdCod, descr, len);

  smartPointerConstString cod(getStringOrIdByLangGlob(fia.idGrp + INIT_COD_ALARM));
  len = _tcslen(cod);
  if(len > D_DESCR_AL_GRP)
    len = D_DESCR_AL_GRP;
  copyStr(aCod, (LPCTSTR)cod, len);

  makeAlarmDescr(descr, D_DESCR_ALARM, fia);
#if 1
  TCHAR date[64];
  set_format_data(date, SIZE_A(date), st, whichData(), _T("\t"));
  _stprintf_s(buff, SIZE_A(buff), _T("%s%c%s%c%s%c%s%c%s%c")
                 _T("%s %c")
                 _T("%s%c"),
                aIAct, SEP, aIdPrf, SEP, aId, SEP, aIdCod, SEP, aCod, SEP,
                date, SEP,
                descr, SEP);
#else
  _stprintf_s(buff, SIZE_A(buff), _T("%s%c%s%c%s%c%s%c%s%c")
                 _T("%02d:%02d:%02d %c")
                 _T("%s%c"),
                aIAct, SEP, aIdPrf, SEP, aId, SEP, aIdCod, SEP, aCod, SEP,
                st.wHour, st.wMinute, st.wSecond, SEP,
                descr, SEP);
#endif
  if(atTop)
    SendMessage(*LB_Alarm, LB_INSERTSTRING, 0, (LPARAM)buff);
  else
    SendMessage(*LB_Alarm, LB_ADDSTRING, 0, (LPARAM)buff);
  return true;
}
//----------------------------------------------------------------------------
#define SZ1 sizeof(int)
#define SZ2 sizeof(FILETIME)
#define SZ3 sizeof(DWORD)
#define SZ4 sizeof(uint)
#define SZ  (SZ1 + SZ2 + SZ3 + SZ4)
#define MAX_RECORD 2000
#define MAX_LEN_BUFF (MAX_RECORD * SZ)
//----------------------------------------------------------------------------
class tFILETIME : public FILETIME
{
  public:
    tFILETIME(__int64 v = 0) : F(I64_TO_FT(v)) {}
    tFILETIME(const FILETIME& f) : F(f) {}
    tFILETIME(const tFILETIME& f) : F(f.F) {}
    operator FILETIME() { return F; }
    const tFILETIME& operator=(const tFILETIME& other) { F = other.F; return *this; }
    const tFILETIME& operator=(const FILETIME& other) { F = other; return *this; }
  private:
    FILETIME F;
};
//----------------------------------------------------------------------------
template <class T>
struct rangeValue
{
  T from;
  T to;
  rangeValue() : from(0), to(0) {}

  bool isInside(T id) const {
    if(id < from)
      return false;
    if(to < id)
      return false;
    return true;
    }
};
//----------------------------------------------------------------------------
typedef rangeValue<tFILETIME> rangeTime;
typedef rangeValue<DWORD> rangeNumber;
typedef PVect<rangeNumber> vRangeNumber;
//----------------------------------------------------------------------------
struct infoRange
{
  LPBYTE buff;
  int len;
  vRangeNumber rIdAlarm;
  vRangeNumber rGrpAlarm;
  vRangeNumber rSectAlarm;
  vRangeNumber rPrph;
  rangeTime rTime;
  bool ascOrder;

  infoRange() : buff(0), len(0), ascOrder(true) {}
  ~infoRange() { delete []buff; }

  void freeBuff() { delete []buff; buff = 0; len = 0; }

};
//----------------------------------------------------------------------------
enum resultRange { rrIgnore, rrInside, rrOutside };
//----------------------------------------------------------------------------
resultRange isInRange(DWORD id, const PVect<rangeNumber>& range)
{
  uint nElem = range.getElem();
  if(!nElem)
    return rrIgnore;
  for(uint i = 0; i < nElem; ++i)
    if(range[i].isInside(id))
      return rrInside;
  return rrOutside;
}
//----------------------------------------------------------------------------
#define SZ_REPORT (D_START_STOP + D_COD_REPORT + D_ID_PERIF + D_DESCR_AL_GRP + D_DATE + D_TIME + D_DESCR_ALARM + 10)
//----------------------------------------------------------------------------
bool getFilter(PWin* parent, LPCTSTR path, infoRange& info);
//----------------------------------------------------------------------------
bool isInsideAlarm(const infoRange& info, const fullInfoAlarm& fia)
{
  resultRange insideId = isInRange(fia.alarm, info.rIdAlarm);
  resultRange insidePrph = rrIgnore;
  resultRange insideSect = rrIgnore;
  resultRange insideGrp = rrIgnore;

  if(useMultipleAlarmFile())
    insidePrph = isInRange(fia.getTruePrph(), info.rPrph);

  if(useAlarmSection())
    insideSect = isInRange(fia.idFlt, info.rSectAlarm);
  insideGrp = isInRange(fia.idGrp, info.rGrpAlarm);

  if(insideId <= rrInside && insidePrph <= rrInside && insideSect <= rrInside && insideGrp <= rrInside)
    return true;

  return false;
}
//----------------------------------------------------------------------------
static infoRange gInfo_;
//----------------------------------------------------------------------------
class adjuctRecordToView : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    adjuctRecordToView(PWin* parent, DWORD& records, int& first, int szRecord) :
      baseClass(parent, IDD_RECORD_TO_VIEW), Records(records), First(first), szRec(szRecord) {}
    ~adjuctRecordToView() { destroy(); }

    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    void CmCancel() {}
  private:
    DWORD& Records;
    int& First;
    int szRec;

    static DWORD oldRec;
    static int oldFirst;
};
//----------------------------------------------------------------------------
DWORD adjuctRecordToView::oldRec;
int adjuctRecordToView::oldFirst;
//----------------------------------------------------------------------------
bool adjuctRecordToView::create()
{
  new svEdit(this, IDC_EDIT_SHOW_REC_FROM);
  new svEdit(this, IDC_EDIT_SHOW_REC_NUM);
  if(!baseClass::create())
    return false;
  TCHAR buff[64];
  wsprintf(buff, _T("%d"), Records);
  TCHAR point[8];
  getLocaleThousandSep(point, SIZE_A(point));
  TCHAR t[128] = { 0 };
  pVisualTextEditSep::addSep(t, buff, point[0], 3);
  SET_TEXT(IDC_STATIC_INFO_TOT_REC, t);

  SET_INT(IDC_EDIT_SHOW_REC_FROM, oldFirst);
  if(!oldRec)
    oldRec = Records;
  SET_INT(IDC_EDIT_SHOW_REC_NUM, oldRec);

  setWindowTextByLangGlob(*this, ID_RP_LIMIT_REC_TITLE, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F1), ID_DIALOG_OK, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_LABEL_TOT_REC), ID_RP_LIMIT_REC_TOT, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_LABEL_SHOW_REC), ID_RP_LIMIT_REC_SHOW_START, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_LABEL_SHOW_NUMREC), ID_RP_LIMIT_REC_SHOW, false);
  return true;
}
//----------------------------------------------------------------------------
LRESULT adjuctRecordToView::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
          CmOk();
          break;
        }
      break;
    }
  return baseClass::windowProc( hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void adjuctRecordToView::CmOk()
{
  int num = 0;
  GET_INT(IDC_EDIT_SHOW_REC_NUM, num);
  if(!num)
    return;
  int start = 0;
  GET_INT(IDC_EDIT_SHOW_REC_FROM, start);
  if(start + num > (int)Records) {
    if(start >= (int)Records)
      SET_INT(IDC_EDIT_SHOW_REC_FROM, start = 0);
    if(start + num > (int)Records)
      SET_INT(IDC_EDIT_SHOW_REC_NUM, Records - start);
    return;
    }
  oldFirst = start;
  oldRec = num;
  Records = num;
  First = start * szRec;
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void TD_Alarm::fill_Report(LPCTSTR path, bool reset)
{
  if(reset) {
    gInfo_.freeBuff();
    if(!getFilter(this, path, gInfo_))
      return;
    }
  if(!gInfo_.buff)
    return;

  SendMessage(*LB_Report, LB_RESETCONTENT, 0, 0);

  TCHAR row[SZ_REPORT + 2];

  TCHAR init_[D_START_STOP + 1];
  TCHAR end_[D_START_STOP + 1];
  TCHAR ack_[D_START_STOP + 1];

  fillStr(init_, _T(' '), D_START_STOP);
  fillStr(end_, _T(' '), D_START_STOP);
  fillStr(ack_, _T(' '), D_START_STOP);

  init_[D_START_STOP] = 0;
  end_[D_START_STOP] = 0;
  ack_[D_START_STOP] = 0;

  int l;
  do {
    smartPointerConstString p(getStringOrIdByLangGlob(ID_DATA_TYPE_INIT));
    l = _tcslen(p);
    if(l > D_START_STOP)
      l = D_START_STOP;
    copyStr(init_, &p, l);
    } while(false);

  do {
    smartPointerConstString p(getStringOrIdByLangGlob(ID_DATA_TYPE_END));
    l = _tcslen(p);
    if(l > D_START_STOP)
      l = D_START_STOP;
    copyStr(end_, &p, l);
    } while(false);
  do {
    smartPointerConstString p(getStringOrIdByLangGlob(ID_DATA_TYPE_ACK));
    l = _tcslen(p);
    if(l > D_START_STOP)
      l = D_START_STOP;
    copyStr(ack_, &p, l);
    } while(false);

  int first = 0;
  int end = gInfo_.len;
  int step = SZ;

#if 1
//  const DWORD MAX_COUNT_LB = 1000;
  const DWORD MAX_COUNT_LB = 20000;
  DWORD records = (end - first) / step;

  if(records >= MAX_COUNT_LB)
    adjuctRecordToView(this, records, first, SZ).modal();
#else
  const DWORD MAX_COUNT_LB = 32767;
  DWORD records = (end - first) / step;

  if(records >= MAX_COUNT_LB && (isWin95() || isWin98orLater()))
    records = MAX_COUNT_LB;
#endif
  SendMessage(*LB_Report, LB_INITSTORAGE, records, SZ_REPORT);

  if(!gInfo_.ascOrder) {
    first = gInfo_.len - first - SZ;
    step = -(int)SZ;
    }
//  LockWindowUpdate(*LB_Report);

  BarProgr bar(this, records);
  bar.create();

  int pos = first;
  for(uint i = 0; i < records; ++i, pos += step) {
    FILETIME ft = *(reinterpret_cast<FILETIME*>(gInfo_.buff + SZ1 + pos));
    if(!(i & 0xf))
      if(!bar.setPos(i))
        break;
    fillStr(row, _T(' '), SZ_REPORT);
    long al = *(reinterpret_cast<int*>(gInfo_.buff + pos));

    if(al > 0) {
      if(isAckAlarm(al, true))
        copyStr(row, ack_, D_START_STOP);
      else
        copyStr(row, init_, D_START_STOP);
      }
    else {
      copyStr(row, end_, D_START_STOP);
      al = -al;
      }

    clearUnusedBitAlarm(al);

/*
    if(al > 0)
      copyStr(row, init_, D_START_STOP);
    else {
      copyStr(row, end_, D_START_STOP);
       al = -al;
      }
*/
    infoAssocAlarm info;
    info.dw = *(reinterpret_cast<LPDWORD>(gInfo_.buff + SZ1 + SZ2 + pos));
    info.type = *(reinterpret_cast<uint*>(gInfo_.buff + SZ1 + SZ2 + SZ3 + pos));
    fullInfoAlarm fia;
    if(!getTextReport(al, fia, info))
      continue;

    bool inside = isInsideAlarm(gInfo_, fia);

    if(!inside)
      continue;
    if(!acceptRowAlarm(fia))
      continue;
    row[D_START_STOP] = TAB;
    int j = D_START_STOP + 1;
    TCHAR t[20];

    _itot_s(fia.alarm, t, SIZE_A(t), 10);
    copyStr(row + j, t, _tcslen(t));
    j += D_COD_REPORT;
    row[j++] = TAB;

    _itot_s(fia.getTruePrph(), t, SIZE_A(t), 10);
    copyStr(row + j, t, _tcslen(t));
    j += D_ID_PERIF;
    row[j++] = TAB;

    smartPointerConstString cod(getStringOrIdByLangGlob(fia.idGrp + INIT_COD_ALARM));
    int len = _tcslen(cod);
    if(len > D_DESCR_AL_GRP)
      len = D_DESCR_AL_GRP;
    copyStr(row + j, (LPCTSTR)cod, len);
    j += D_DESCR_AL_GRP;
    row[j++] = TAB;

    TCHAR date[D_DATE + D_TIME + 4];
    set_format_data(date, SIZE_A(date), ft, whichData(), _T("\t"));
    copyStr(row + j, date, _tcslen(date));
    j += D_DATE + D_TIME;
    row[j++] = TAB;

    TCHAR descr[D_DESCR_ALARM];
    makeAlarmDescr(descr, D_DESCR_ALARM, fia);
    l = _tcslen(descr);
    copyStr(row + j, descr, l);
    row[j + l] = 0;
    j += D_DESCR_ALARM;
    row[j++] = TAB;
    row[j] = 0;
    if(SendMessage(*LB_Report, LB_ADDSTRING, 0, (LPARAM)row) < 0)
      break;
    }
//  LockWindowUpdate(0);
}
//----------------------------------------------------------------------------
void TD_Alarm::print_Setup()
{
  svPrinter* Printer = getPrinter();
  if(!Printer)
    return;
  printGeneric::Setup(Printer);
}
//----------------------------------------------------------------------------
void TD_Alarm::print_Report(bool preview, bool setup)
{
  svPrinter* Printer = getPrinter();
  if(!Printer)
    return;

  gInfo_.freeBuff();

  if(!getFilter(this, filenameReport, gInfo_))
    return;
  if(!gInfo_.buff)
    return;


  int first = 0;
  int end = gInfo_.len;
  int step = SZ;

  if(!gInfo_.ascOrder) {
    first = gInfo_.len - SZ;
    end = -(int)SZ;
    step = -(int)SZ;
    }

  LPCTSTR fn = _T("~rows.dat");
  do {
    P_File pf(fn, P_CREAT);
    if(!pf.P_open())
      return;

    rowAlarmFileHeader Header;
    _tcscpy_s(Header.title, SIZE_A(Header.title), _T("Report Allarmi"));
    Header.type = 2;
    pf.P_write(&Header, sizeof(Header));

    rowAlarmFile raf;

    for(int i = first; i != end; i += step) {
      raf.ft = *(reinterpret_cast<FILETIME*>(gInfo_.buff + SZ1 + i));
      long al = *(reinterpret_cast<int*>(gInfo_.buff + i));

      if(al > 0) {
        if(isAckAlarm(al, true))
          raf.stat = 2;
        else
          raf.stat = 1;
        }
      else {
        al = -al;
        raf.stat = 0;
        }
      clearUnusedBitAlarm(al);
/*
      if(al < 0) {
//        raf.alarm = -al;
        raf.stat = 0;
        al = -al;
        }
      else {
//        raf.alarm = al;
        raf.stat = 1;
        }
*/
      infoAssocAlarm info;
      info.dw = *(reinterpret_cast<LPDWORD>(gInfo_.buff + SZ1 + SZ2 + i));
      info.type = *(reinterpret_cast<uint*>(gInfo_.buff + SZ1 + SZ2 + SZ3 + i));
      fullInfoAlarm fia;
      if(!getTextReport(al, fia, info))
        continue;

      bool inside = isInsideAlarm(gInfo_, fia);
      if(!inside)
        continue;
      if(!acceptRowAlarm(fia))
        continue;
      raf.prph = fia.getTruePrph();
      raf.alarm = fia.alarm;
      makeAlarmDescr(raf.descr, D_DESCR_ALARM, fia);
      pf.P_write(&raf, sizeof(raf));
      }
    } while(false);
  printAlarm pa(this, Printer, fn);

  if(preview)
    pa.Preview(setup);
  else
    pa.Print(setup);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool TD_Alarm::create()
{
  new svEdit(this, IDC_EDIT_FILTER_TEXT_ALARM);
  if(!baseClass::create())
    return false;
#if 1
  setInfoTitle();
#else
  setWindowTextByLangGlob(*this, ID_TITLE_ALARM);
#endif
  PRect r(0, 0, sizer::getWidth(), sizer::getHeight());
  r.Inflate(R__X(-20), R__Y(-20));
//  setWindowPos(0, r, SWP_NOZORDER);
  do {
    int x = r.left;
    int y = r.top;
    alignWithMonitor(this, x, y);
    SetWindowPos(*this, 0, x, y, r.Width(), r.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);

    } while(false);

  r.Inflate(-20, -(30 + GetSystemMetrics(SM_CYCAPTION) / 2));
  r.MoveTo(20 - GetSystemMetrics(SM_CXEDGE), 30);

  PRect rf;
  GetWindowRect(*HeadLabel, rf);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(PRECT)rf, 2);
  r.top = rf.bottom + 10;

  rf.left = r.left;
  rf.right = r.right;
  HeadLabel->setWindowPos(0, rf, SWP_NOZORDER);

//  LB_Alarm->rescale(1, 2, 0.85);
  LB_Alarm->setWindowPos(0, r, SWP_NOZORDER);
  LB_Alarm->setIntegralHeight();

  GetWindowRect(*LB_Alarm, r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(PRECT)r, 2);
  LB_Report->setWindowPos(0, r, SWP_NOZORDER);
  LB_Report->setIntegralHeight();

  SET_TEXT(IDC_EDIT_FILTER_TEXT_ALARM, FilterText);
  fill_LB(1);
  cfg _cfg = config().getAll();
  currRep = _cfg.logAllAlarm;
  if(currRep > 2)
    currRep = 2;
  SendMessage(*RB_TypeRep[currRep], BM_SETCHECK, BST_CHECKED, 0);
  for(int i = 0; i < SIZE_A(RB_TypeRep); ++i)
    setWindowTextByLangGlob(*RB_TypeRep[i], ID_TXT_NO_ACTIVE_REP + i);

  setWindowTextByLangGlob(*Exit, ID_TXT_EXIT);
  setWindowTextByLangGlob(*Reset, ID_TXT_RESET);
  setWindowTextByLangGlob(*Show, ID_TXT_SHOW_REPORT);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_TEXT_FILTER_AL), ID_TXT_FILTER_TEXT, false);
  if(getString(ID_TXT_ADVANCED_REPORT))
    setWindowTextByLangGlob(GetDlgItem(*this, IDC_CHECK_ADVANCED_REPORT), ID_TXT_ADVANCED_REPORT);
  if(!_cfg.noAdvancedReport)
    SET_CHECK(IDC_CHECK_ADVANCED_REPORT);
  makeHeader(false);
  return true;
}
//----------------------------------------------------------------------------
void TD_Alarm::checkWaitRefresh()
{
  waitRefresh = -2;
  smartPointerConstString msg(getStringByLangGlob(ID_ALARM_REFRESH_MSG));
  if(!msg)
    msg = smartPointerConstString(_T("YES = Refresh automatico\r\nNO = Nessun refresh\r\nCANC = aggiorna e riproponi\r\n\r\n")
             _T("N.B. col Tasto F11 si aggiorna e si riazzera la scelta"), false);

  smartPointerConstString title(getStringByLangGlob(ID_ALARM_REFRESH_TITLE));
  if(!title)
    title = smartPointerConstString(_T("Gestione refresh"), false);
  int result = MessageBox(*this, msg, title, MB_YESNOCANCEL | MB_ICONINFORMATION);
  switch(result) {
    case IDYES:
      waitRefresh = 1;
      break;
    case IDNO:
      waitRefresh = -1;
      break;
    default:
      waitRefresh = 0;
      break;
    }
}
//-----------------------------------------------------------
bool TD_Alarm::preProcessMsg(MSG& msg)
{
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
LRESULT TD_Alarm::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
          EndDialog(hwnd, IDOK);
          break;
        case IDC_BUTTON_F2:
          BNClicked_F2();
          break;
        case IDC_BUTTON_F3:
          BNClicked_F3();
          break;
        case IDC_BUTTON_F4:
          BNClicked_F4();
          break;

        case IDC_BUTTON_F5:
          if(SHOW_ALARM == currShow)
            print_Alarm(true, true);
          else
            print_Report(true, true);
          break;

        case IDC_BUTTON_F6:
          print_Setup();
          break;

        case IDC_BUTTON_F7:
          showInfoAlarm();
          break;

        case IDC_BUTTON_F11:
          if(SHOW_ALARM != currShow)
            break;
          waitRefresh = 0;
          fill_LB(1);
          break;

        case IDC_RADIOBUTTON_NO_REP:
          BNClicked_RB(0);
          break;
        case IDC_RADIOBUTTON_ONLY_ALARM:
          BNClicked_RB(1);
          break;
        case IDC_RADIOBUTTON_ALL_REPORT:
          BNClicked_RB(2);
          break;
        case IDC_CHECK_ADVANCED_REPORT:
          setAdvancedReport();
          break;
        case IDC_BUTTON_GO_FILTER:
          if(SendMessage(GetDlgItem(*this, IDC_EDIT_FILTER_TEXT_ALARM), EM_GETMODIFY, 0, 0)) {
            SendMessage(GetDlgItem(*this, IDC_EDIT_FILTER_TEXT_ALARM), EM_SETMODIFY, 0, 0);
            }
          fill_LB(2);
          break;
        }
      switch(HIWORD(wParam)) {
        case LBN_DBLCLK:
          handleLBDblClick((HWND)lParam);
          break;
        }
      break;
    case WM_TIMER:
      if(idTimer == (int)wParam) {
        if(SHOW_ALARM == currShow) {
          if(-2 == waitRefresh)
            break;
          currAlarm *Al = getGestAlarm();
          if(!Al->isDirty())
            break;
          if(!waitRefresh)
            checkWaitRefresh();
          if(-1 == waitRefresh)
            break;
          }
        fill_LB();
        }
      break;

    case WM_INITDIALOG:
      SetTimer(hwnd, idTimer = 2, 1000, 0);
      return true;

    case WM_DESTROY:
      if(idTimer) {
        KillTimer(hwnd, idTimer);
        idTimer = 0;
        }
      break;
      }
  return baseClass::windowProc( hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void TD_Alarm::setAdvancedReport()
{
  cfg _cfg = config().getAll();
  _cfg.noAdvancedReport = !IS_CHECKED(IDC_CHECK_ADVANCED_REPORT);
  config().setup(_cfg);
}
//----------------------------------------------------------------------------
class PDlgInfoAlarm : public PModDialog
{
  public:
    PDlgInfoAlarm(PWin* parent, LPCTSTR info, HINSTANCE hInst = 0) :
      PModDialog(parent, IDD_SHOW_INFO_ALARM, hInst), Info(info) {}
    ~PDlgInfoAlarm() { destroy(); }
    bool create();
  private:
    LPCTSTR Info;
};
//----------------------------------------------------------------------------
bool PDlgInfoAlarm::create()
{
  if(!PModDialog::create())
    return false;
  SetDlgItemText(*this, IDC_EDIT_INFO_ALARM, Info);
  return true;
}
//----------------------------------------------------------------------------
#ifndef WM_PLC
  #define WM_PLC 2
#endif
//----------------------------------------------------------------------------
void TD_Alarm::showInfoAlarm()
{
  TCHAR buff[500];
  int id;
  if(SHOW_ALARM != currShow) {
    int sel = SendMessage(*LB_Report, LB_GETCURSEL, 0, 0);
    if(sel < 0)
      return;
    SendMessage(*LB_Report, LB_GETTEXT, sel, (LPARAM)buff);
    id = getIdAlarmByRowReport(buff);
    }
  else {
    int sel = SendMessage(*LB_Alarm, LB_GETCURSEL, 0, 0);
    if(sel < 0)
      return;
    SendMessage(*LB_Alarm, LB_GETTEXT, sel, (LPARAM)buff);
    id = getIdAlarmByRow(buff);
    }
  uint al;
  uint prph = getPrphFromAlarm(id, al) - FIRST_PRPH_ALARM;
  LPCTSTR p = InfoAlarm[prph]->getString(al);
  if(p)
    PDlgInfoAlarm(this, p).modal();
}
//----------------------------------------------------------------------------
void TD_Alarm::handleLBDblClick(HWND hwnd)
{
  if(hwnd == *LB_Alarm) {
    TCHAR buff[500];
    int sel = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
    if(sel >= 0) {
      SendMessage(hwnd, LB_GETTEXT, sel, (LPARAM)buff);
      int id = getIdAlarmByRow(buff);
      currAlarm *Al = getGestAlarm();
      Al->remove(id);
      }
    }
}
//----------------------------------------------------------------------------
void TD_Alarm::BNClicked_F2()
{
  mainClient* par = getMain();
  genericPerif *prf = par->getGenPerif(1);
  if(prf)
    prf->sendResetAlarm();

  Perif->resetAlarm();
  currAlarm *Al = getGestAlarm();
   Al->remove(-1);
  SetFocus(*this);
  if(getHandle() == GetForegroundWindow())
    if(SHOW_REPORT == currShow)
      SetFocus(*LB_Report);
    else
      SetFocus(*LB_Alarm);
}
//----------------------------------------------------------------------------
void TD_Alarm::BNClicked_F3()
{
  int t = currRep + 1;
  if(t > 2)
    t = 0;
    // attiva flag di attivazione tramite F3
  checkBtn = true;
  if(BNClicked_RB(t)) {
    if(getHandle() == GetForegroundWindow())
      if(SHOW_REPORT == currShow)
        SetFocus(*LB_Report);
      else
        SetFocus(*LB_Alarm);
    }
}
//----------------------------------------------------------------------------
#define PSW_LEVEL 1
//----------------------------------------------------------------------------
#define INIT_PSW(level) \
  PassWord psw; \
  if(level == psw.getPsW(level, this)) { \
    success = true;
//----------------------------------------------------------------------------
#define END_PSW(level) \
    psw.restartTime(level); \
  }
//----------------------------------------------------------------------------
bool TD_Alarm::BNClicked_RB(int id)
{
  if(!checkBtn && BST_CHECKED != SendMessage(*RB_TypeRep[id], BM_GETCHECK, 0, 0))
    return false;
  bool success = false;
INIT_PSW(PSW_LEVEL)
  if(checkBtn)
    SendMessage(*RB_TypeRep[currRep], BM_SETCHECK, BST_UNCHECKED, 0);
//    SendMessage(*RB_TypeRep[id], BM_SETCHECK, BST_UNCHECKED, 0);
  currRep = id;
  setReport();
END_PSW(PSW_LEVEL)
  if(!success && !checkBtn) {
    SendMessage(*RB_TypeRep[id], BM_SETCHECK, BST_UNCHECKED, 0);
    SendMessage(*RB_TypeRep[currRep], BM_SETCHECK, BST_CHECKED, 0);
    }
  return success;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void TD_Alarm::setReport()
{
  if(checkBtn) {
    checkBtn = false;
    SendMessage(*RB_TypeRep[currRep], BM_SETCHECK, BST_CHECKED, 0);
    }
  cfg _cfg = config().getAll();
  _cfg.logAllAlarm = BYTE(currRep);
  TParent* par;
  const PWin* w = getParent();
  do {
    par = CAST(w);
    w = w->getParent();
    } while(!par && w);
  if(par)
    par->RepAllAlarm(toBool(_cfg.logAllAlarm));
  config().setup(_cfg);
  if(getHandle() == GetForegroundWindow())
    if(SHOW_REPORT == currShow)
      SetFocus(*LB_Report);
    else
      SetFocus(*LB_Alarm);
}
//----------------------------------------------------------------------------
void TD_Alarm::BNClicked_F4()
{
  if(SHOW_ALARM == currShow) {
    setWindowTextByLangGlob(*Show, ID_TXT_SHOW_ALARM);
    ShowWindow(*LB_Alarm, SW_HIDE);
    ShowWindow(*LB_Report, SW_SHOWNORMAL);
    currShow = SHOW_REPORT;
    makeHeader(true);
    }
  else {
    setWindowTextByLangGlob(*Show, ID_TXT_SHOW_REPORT);
    ShowWindow(*LB_Report, SW_HIDE);
    ShowWindow(*LB_Alarm, SW_SHOWNORMAL);
    currShow = SHOW_ALARM;
    makeHeader(false);
    }
  setInfoTitle();
  fill_LB(1);
}
//----------------------------------------------------------------------------
void TD_Alarm::setInfoTitle()
{
  if(SHOW_ALARM == currShow)
    setWindowTextByLangGlob(*this, ID_TITLE_ALARM_CURR);
  else
    setWindowTextByLangGlob(*this, ID_TITLE_ALARM_HIST);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// i codici entro questi valori sono allarmi e vengono visualizzati in rosso
// gli altri sono eventi e sono visualizzati in giallo
#define CHAR_INIT_ALARM _T('1')
#define CHAR_END_ALARM  _T('6')
// colore per lo sfondo degli allarmi
#define BKCOLOR_ALARM RGB(0xff, 0xaf, 0xaf)
#define BKCOLOR_ALARM_E RGB(0xff, 0x80, 0x80)
//#define BKCOLOR_ALARM_E RGB(0x8f, 0, 0)
#define COLORT_ALARM      RGB(0, 0, 0)
#define COLORT_ALARM_E    COLORT_ALARM
//#define COLORT_ALARM_E    RGB(0xff, 0xff, 0xff)

// colore per lo sfondo degli allarmi non più attivi
#define BKCOLOR_ALARM_E_NA RGB(0x7f, 0x7f, 0x7f)
#define BKCOLOR_ALARM_NA RGB(0xcf, 0xcf, 0xcf)
#define COLORT_ALARM_NA      RGB(0, 0, 0)
//#define COLORT_ALARM      BKCOLOR_ALARM

// colore per lo sfondo degli allarmi attivi ma riconosciuti
#define BKCOLOR_ALARM_E_NA_REC RGB(0x3f, 0x3f, 0xcf)
#define BKCOLOR_ALARM_NA_REC RGB(0xcf, 0xcf, 0xff)
#define COLORT_ALARM_NA_REC      RGB(0, 0, 0x1f)

#define BKCOLOR     C_Bkg
#define BKCOLOR_E   C_BkgSel
#define COLORT      C_Text
#define COLORT_E    C_TextSel
//----------------------------------------------------------------------------
HPEN myListGen::set_Color(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo, int ixColor)
{
  COLORREF cText;
  COLORREF cBkg;

  if (drawInfo.itemState & ODS_SELECTED)  {
    switch(ixColor) {
      case 0:
        cText = COLORT_E;
        cBkg = BKCOLOR_E;
        break;
      case 1:
        cText = COLORT_ALARM_E;
        cBkg = BKCOLOR_ALARM_E;
        break;
      case 2:
        cText = COLORT_ALARM_NA_REC;
        cBkg = BKCOLOR_ALARM_E_NA_REC;
        break;
      }
    }
  else {
    switch(ixColor) {
      case 0:
        cBkg = BKCOLOR;
        cText = COLORT;
        break;
      case 1:
        cBkg = BKCOLOR_ALARM;
        cText = COLORT_ALARM;
        break;
      case 2:
        cBkg = BKCOLOR_ALARM_NA_REC;
        cText = COLORT;
        break;
      }
    }

  PRect Rect(drawInfo.rcItem);

  SetBkColor(hDC, cBkg);
  ExtTextOut(hDC, 0, 0, ETO_OPAQUE, Rect, 0, 0, 0);

  SetTextColor(hDC, cText);
  HPEN pen = CreatePen(PS_SOLID, 1, cText);
  HPEN oldPen = (HPEN) SelectObject(hDC, pen);
  return oldPen;
}
//----------------------------------------------------------------------------
extern  int getWarningLevelFilterAlarm();
//----------------------------------------------------------------------------
HPEN myListAlarm::setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo)
{
  drawCod = false;
  const uint offs = D_ID_ACTIVE + D_ID_PERIF + 2;
  if(*(Buff + offs) >= (CHAR_INIT_ALARM + getWarningLevelFilterAlarm()) && *(Buff + offs) <= CHAR_END_ALARM)
    drawCod = true;
  if(_T('0') == *Buff)
    return myListGen::set_Color(hDC, drawInfo, drawCod);

  COLORREF cText;
  COLORREF cBkg;

  if(_T('1') == *Buff) {
    if (drawInfo.itemState & ODS_SELECTED)  {
      cText = COLORT_ALARM_NA;
      cBkg = BKCOLOR_ALARM_E_NA;
      }
    else {
      cBkg = BKCOLOR_ALARM_NA;
      cText = COLORT;
      }
    }
  else {
    if (drawInfo.itemState & ODS_SELECTED)  {
      cText = COLORT_ALARM_NA_REC;
      cBkg = BKCOLOR_ALARM_E_NA_REC;
      }
    else {
      cBkg = BKCOLOR_ALARM_NA_REC;
      cText = COLORT;
      }
    }
  PRect Rect(drawInfo.rcItem);

  SetBkColor(hDC, cBkg);
  ExtTextOut(hDC, 0, 0, ETO_OPAQUE, Rect, 0, 0, 0);

  SetTextColor(hDC, cText);
  HPEN pen = CreatePen(PS_SOLID, 1, cText);
  HPEN oldPen = (HPEN) SelectObject(hDC, pen);
  return oldPen;
}
//----------------------------------------------------------------------------
void myListAlarm::paintTxt(HDC hDC, int x, int y, LPCTSTR buff, int ix, DRAWITEMSTRUCT FAR& drawInfo)
{
  if(drawCod || ix > 1)
    myListGen::paintTxt(hDC, x, y, buff, ix, drawInfo);
//    TextOut(hDC, x, y, buff, _tcslen(buff));
}
//----------------------------------------------------------------------------
int myListAlarm::verifyKey()
{
  if(GetKeyState(VK_RETURN)& 0x8000)
    return VK_RETURN;
  return 0;
}
//----------------------------------------------------------------------------
bool myListAlarm::evChar(WPARAM& key)
{
  if(VK_RETURN == key)
    PostMessage(*getParent(), WM_COMMAND, MAKEWPARAM(Attr.id, LBN_DBLCLK), (LPARAM)getHandle());
  return myListGen::evChar(key);
}
//----------------------------------------------------------------------------
HPEN myListReport::setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo)
{
  int ixColor = 2;
  TCHAR t[D_START_STOP + 1];
  copyStrZ(t, Buff, D_START_STOP);
  trim(t);
  if(!_tcsicmp(t, init))
    ixColor = 1;
  else if(!_tcsicmp(t, end))
    ixColor = 0;

  return myListGen::set_Color(hDC, drawInfo, ixColor);
}
//----------------------------------------------------------------------------
class PFilterReport : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    PFilterReport(PWin* parent, LPCTSTR path, infoRange& info, uint resId = IDD_FILTER_REPORT) :
      baseClass(parent, resId), Info(info), Path(path) {}

    virtual bool create();
    bool makeOnly();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();

  private:
    infoRange& Info;
    LPCTSTR Path;
    void replaceLangText();
    void verifyDate(UINT idc);
    void verifyTime(UINT idc);
    void dialogFilter(bool isType);
};
//----------------------------------------------------------------------------
class myListViewFilter : public PListView
{
  private:
    typedef PListView baseClass;
  public:
    myListViewFilter(PWin * parent, uint resid, HINSTANCE hinst = 0) :
        baseClass(parent, resid, hinst) {}
    ~myListViewFilter() { destroy(); }
  protected:
    virtual int verifyKey() {
      if(GetKeyState(VK_RETURN)& 0x8000)
        return VK_RETURN;
      return 0;
      }
};
//----------------------------------------------------------------------------
#define MAX_DIM_ID 6
#define MAX_DIM_DESCR 64
//----------------------------------------------------------------------------
#define MAX_DIM_FILTER_ALL_ID (MAX_DIM_ID * 64)
//----------------------------------------------------------------------------
struct lvRow
{
  TCHAR id[MAX_DIM_ID];
  TCHAR descr[MAX_DIM_DESCR];
  lvRow()  { id[0] = 0; descr[0] = 0; }
};
//----------------------------------------------------------------------------
class PFilteID_Choose : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    PFilteID_Choose(PWin* parent, LPTSTR filter, uint resId = IDD_FILTER_CHOOSE);
    ~PFilteID_Choose() { destroy(); }
    virtual bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    virtual void fillList() = 0;
    LPTSTR Filter;

    PListView* lbAllDisp;
    PListView* lbToFilter;

    void replaceLangText();
    void addAll(PListView* target, PListView* source);
    void addLB(PListView* target, PListView* source);
    int beginDrag;
    void enableWindow(bool enable);
    int makeLvRow(const lvRow& source, PListView* target, int item = 65535);
    bool unmakeLvRow(lvRow& target, PListView* source, int item);
};
//----------------------------------------------------------------------------
class PFilteID_ChooseType : public PFilteID_Choose
{
  private:
    typedef PFilteID_Choose baseClass;
  public:
    PFilteID_ChooseType(PWin* parent, LPTSTR filter) :
      baseClass(parent, filter) {}
    ~PFilteID_ChooseType() { destroy(); }
  protected:
    virtual void fillList();
};
//----------------------------------------------------------------------------
class PFilteID_ChooseSection : public PFilteID_Choose
{
  private:
    typedef PFilteID_Choose baseClass;
  public:
    PFilteID_ChooseSection(PWin* parent, LPTSTR filter) :
      baseClass(parent, filter) {}
    ~PFilteID_ChooseSection() { destroy(); }
  protected:
    virtual void fillList();
};
//----------------------------------------------------------------------------
bool getFilter(PWin* parent, LPCTSTR path, infoRange& info)
{
  cfg _cfg = config().getAll();
  if(_cfg.noAdvancedReport)
    return PFilterReport(parent, path, info).makeOnly();
  return IDOK == PFilterReport(parent, path, info).modal();
}
//----------------------------------------------------------------------------
void fillStrDateFilter(const FILETIME& ft, HWND date, HWND time)
{
  TCHAR buff[64] = _T("");
  set_format_data(buff, SIZE_A(buff), ft, whichData(), _T("|"));
  buff[10] = 0;
  SetWindowText(date, buff);
  SetWindowText(time, buff + 11);
}
//----------------------------------------------------------------------------
void fillStrRowFilter(const vRangeNumber& r, HWND edit)
{
  TCHAR buff[1024] = _T("");
  uint nElem = r.getElem();
  LPTSTR p = buff;
  for(uint i = 0; i < nElem; ++i) {
    if(r[i].from == r[i].to)
      wsprintf(p, _T("%d,"), r[i].from);
    else
      wsprintf(p, _T("%d-%d,"), r[i].from, r[i].to);
    p += _tcslen(p);
    if(p - buff >= SIZE_A(buff) - 10)
      break;
    }
  if(p > buff)
    *(p - 1) = 0;
  SetWindowText(edit, buff);
}
//----------------------------------------------------------------------------
void PFilterReport::replaceLangText()
{
  setWindowTextByLangGlob(*this, ID_REPORT_FILTER_TITLE, false);

  setWindowTextByLangGlob(GetDlgItem(*this, IDOK), ID_DIALOG_OK, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDCANCEL), ID_DIALOG_CANC, false);

  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATICTEXT_DATE_FROM_TREE), ID_INP_DATE_FROM, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATICTEXT_DATE_TO_TREE), ID_INP_DATE_TO, false);

  setWindowTextByLangGlob(GetDlgItem(*this, IDC_GROUPBOX_DATE_TREE), ID_REPORT_FILTER_GBOX_DATE, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_GROUPBOX_F_ALARM), ID_REPORT_FILTER_GBOX_ALARM, false);

  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_F_BY_ID_ALARM), ID_REPORT_FILTER_ID_ALARM, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_F_BY_ID_GROUP), ID_REPORT_FILTER_ID_GROUP, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_F_BY_ID_SECT), ID_REPORT_FILTER_ID_SECT, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_F_BY_ID_PRPH), ID_REPORT_FILTER_ID_PRPH, false);

  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_TOT_REC_LABEL), ID_REPORT_FILTER_INFO_REC, false);

  setWindowTextByLangGlob(GetDlgItem(*this, IDC_CHECK_ORDER), ID_REPORT_FILTER_ORDER, false);
}
//----------------------------------------------------------------------------
bool PFilterReport::makeOnly()
{
  P_File f(Path, P_READ_ONLY);
  if(!f.P_open())
    return false;
  int len = (int)f.get_len();

  BYTE buff[SZ] = { 0 };
  f.P_read(buff, SZ);
  FILETIME ft = *(reinterpret_cast<FILETIME*>(buff + SZ1));
  Info.rTime.from = ft;

  f.P_seek(-(long)SZ, SEEK_END_);
  f.P_read(buff, SZ);
  ft = *(reinterpret_cast<FILETIME*>(buff + SZ1));
  Info.rTime.to = ft;

  Info.len = len;
  Info.buff = new BYTE[Info.len];
  f.P_seek(0);
  f.P_read(Info.buff, Info.len);
  Info.ascOrder = false;
  return true;
}
//----------------------------------------------------------------------------
bool PFilterReport::create()
{
  svEdit* ed = new svEdit(this, IDC_EDIT_INIT_DATE);
  ed->setFilter(new PDateFilter);

  ed = new svEdit(this, IDC_EDIT_INIT_TIME);
  ed->setFilter(new PTimeFilter);

  ed = new svEdit(this, IDC_EDIT_END_DATE);
  ed->setFilter(new PDateFilter);

  ed = new svEdit(this, IDC_EDIT_END_TIME);
  ed->setFilter(new PTimeFilter);

  new svEdit(this, IDC_EDIT_F_ALARM);
  new svEdit(this, IDC_EDIT_F_GRP);
  new svEdit(this, IDC_EDIT_F_SECTION);
  new svEdit(this, IDC_EDIT_F_PRPH);

  if(!baseClass::create())
    return false;

  replaceLangText();

  HWND hwnd = GetDlgItem(*this, IDC_EDIT_F_SECTION);
  if(!useAlarmSection())
    EnableWindow(hwnd, false);
  else
    fillStrRowFilter(Info.rSectAlarm, hwnd);

  hwnd = GetDlgItem(*this, IDC_EDIT_F_PRPH);
  if(!useMultipleAlarmFile())
    EnableWindow(hwnd, false);
  else
    fillStrRowFilter(Info.rPrph, hwnd);

  fillStrRowFilter(Info.rGrpAlarm, GetDlgItem(*this, IDC_EDIT_F_GRP));
  fillStrRowFilter(Info.rIdAlarm, GetDlgItem(*this, IDC_EDIT_F_ALARM));

  SET_CHECK_SET(IDC_CHECK_ORDER, Info.ascOrder);

  FILETIME ftInit = I64_TO_FT(0);
  FILETIME ftEnd = I64_TO_FT(0);

  P_File f(Path, P_READ_ONLY);
  if(!f.P_open())
    return false;
  int len = (int)(f.get_len() / SZ);
  SET_INT(IDC_STATIC_TOT_REC_INFO, len);

  BYTE buff[SZ] = { 0 };
  f.P_read(buff, SZ);
  FILETIME ft = *(reinterpret_cast<FILETIME*>(buff + SZ1));
  fillStrDateFilter(ft, GetDlgItem(*this, IDC_STATIC_INIT_DATE), GetDlgItem(*this, IDC_STATIC_INIT_TIME));
  if(MK_I64(Info.rTime.from))
    ft = Info.rTime.from;
  fillStrDateFilter(ft, GetDlgItem(*this, IDC_EDIT_INIT_DATE), GetDlgItem(*this, IDC_EDIT_INIT_TIME));


  f.P_seek(-(long)SZ, SEEK_END_);

  f.P_read(buff, SZ);
  ft = *(reinterpret_cast<FILETIME*>(buff + SZ1));
  fillStrDateFilter(ft, GetDlgItem(*this, IDC_STATIC_END_DATE), GetDlgItem(*this, IDC_STATIC_END_TIME));

  if(MK_I64(Info.rTime.to))
    ft = Info.rTime.to;
  fillStrDateFilter(ft, GetDlgItem(*this, IDC_EDIT_END_DATE), GetDlgItem(*this, IDC_EDIT_END_TIME));

  return true;
}
//----------------------------------------------------------------------------
static
uint ctrl_[] = { IDC_EDIT_INIT_DATE, IDC_EDIT_INIT_TIME, IDC_EDIT_END_DATE, IDC_EDIT_END_TIME };
//----------------------------------------------------------------------------
LRESULT PFilterReport::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        case IDC_BUTTON_AL_TYPE_FILTER:
        case IDC_BUTTON_AL_SECTION_FILTER:
          dialogFilter(IDC_BUTTON_AL_TYPE_FILTER == LOWORD(wParam));
          break;
        }
      switch(HIWORD(wParam)) {
        case EN_KILLFOCUS:
          switch(LOWORD(wParam)) {
            case IDC_EDIT_INIT_DATE:
            case IDC_EDIT_END_DATE:
              verifyDate(LOWORD(wParam));
              break;
            case IDC_EDIT_INIT_TIME:
            case IDC_EDIT_END_TIME:
              verifyTime(LOWORD(wParam));
              break;
            }
          break;
        }
      break;
    }
  return baseClass::windowProc( hwnd, message, wParam, lParam);
}
//--------------------------------------------------------------------
void PFilterReport::dialogFilter(bool isType)
{
  TCHAR buff[MAX_DIM_FILTER_ALL_ID];
  if(isType) {
    GET_TEXT(IDC_EDIT_F_GRP, buff);
    if(IDOK == PFilteID_ChooseType(this, buff).modal())
      SET_TEXT(IDC_EDIT_F_GRP, buff);
    }
  else {
    GET_TEXT(IDC_EDIT_F_SECTION, buff);
    if(IDOK == PFilteID_ChooseSection(this, buff).modal())
      SET_TEXT(IDC_EDIT_F_SECTION, buff);
    }
}
//----------------------------------------------------------------------------
void unfillStrDateFilter(FILETIME& ft, HWND date, HWND time)
{
  TCHAR buff[24];
  GetWindowText(date, buff, SIZE_A(buff));
  FILETIME ftt;
  unformat_data(buff, ftt, whichData());
  GetWindowText(time, buff, SIZE_A(buff));
  unformat_time(buff, ft);
  ft += ftt;
}
//----------------------------------------------------------------------------
void unfillStrRowFilter(vRangeNumber& r, HWND edit)
{
  TCHAR buff[1024] = _T("");
  GetWindowText(edit, buff, SIZE_A(buff));
  r.reset();
  if(!*buff)
    return;
  pvvChar pvv;
  uint nElem = splitParam(pvv, buff);
  r.setDim(nElem);
  for(uint i = 0; i < nElem; ++i) {
    LPCTSTR p = pvv[i].getVect();
    r[i].from = _ttoi(p);
    r[i].to = r[i].from;
    while(*p) {
      if(_T('-') == *p) {
        r[i].to = _ttoi(p + 1);
        if(r[i].to < r[i].from) {
          DWORD t = r[i].to ;
          r[i].to = r[i].from;
          r[i].from = t;
          }
        break;
        }
      ++p;
      }
    }
}
//--------------------------------------------------------------------
int findPos(P_File& f, const FILETIME& ft, bool prev)
{
  BYTE buff[SZ] = { 0 };
  FILETIME ftt = ft;
  if(!prev) {
    MK_I64(ftt) /= SECOND_TO_I64;
    MK_I64(ftt) *= SECOND_TO_I64;
    MK_I64(ftt) += SECOND_TO_I64 - 1;
    }
  DWORD pos = findFirstPos(f, ftt, SZ, buff, 0, prev, SZ1);

  return pos * SZ;
}
//--------------------------------------------------------------------
void PFilterReport::CmOk()
{
  P_File f(Path, P_READ_ONLY);
  if(!f.P_open())
    return;

  HWND hwnd = GetDlgItem(*this, IDC_EDIT_F_SECTION);
  unfillStrRowFilter(Info.rSectAlarm, hwnd);

  hwnd = GetDlgItem(*this, IDC_EDIT_F_PRPH);
  unfillStrRowFilter(Info.rPrph, hwnd);

  unfillStrRowFilter(Info.rGrpAlarm, GetDlgItem(*this, IDC_EDIT_F_GRP));
  unfillStrRowFilter(Info.rIdAlarm, GetDlgItem(*this, IDC_EDIT_F_ALARM));

  Info.ascOrder = IS_CHECKED(IDC_CHECK_ORDER);

  int len = (int)f.get_len();
  BYTE buff[SZ] = { 0 };

  FILETIME ft;
  unfillStrDateFilter(ft, GetDlgItem(*this, IDC_EDIT_INIT_DATE), GetDlgItem(*this, IDC_EDIT_INIT_TIME));
  Info.rTime.from = ft;
  int initPos = findPos(f, ft, true);

  unfillStrDateFilter(ft, GetDlgItem(*this, IDC_EDIT_END_DATE), GetDlgItem(*this, IDC_EDIT_END_TIME));
  Info.rTime.to = ft;
  int endPos = findPos(f, ft, false);

  Info.len = endPos - initPos;
  Info.buff = new BYTE[Info.len];
  f.P_seek(initPos);
  f.P_read(Info.buff, Info.len);

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void PFilterReport::verifyDate(UINT idc)
{
  bool from = IDC_EDIT_INIT_DATE == idc;
  HWND hT = GetDlgItem(*this, from ? IDC_EDIT_INIT_TIME : IDC_EDIT_END_TIME);
  HWND hD = GetDlgItem(*this, idc);
  FILETIME ft;
  unfillStrDateFilter(ft, hD, hT);

  FILETIME ftFrom;
  HWND hTt = GetDlgItem(*this, IDC_STATIC_INIT_TIME);
  HWND hDt = GetDlgItem(*this, IDC_STATIC_INIT_DATE);
  unfillStrDateFilter(ftFrom, hDt, hTt);

  FILETIME ftTo;
  hTt = GetDlgItem(*this, IDC_STATIC_END_TIME);
  hDt = GetDlgItem(*this, IDC_STATIC_END_DATE);
  unfillStrDateFilter(ftTo, hDt, hTt);

  if(ft < ftFrom)
    ft = ftFrom;
  if(ft > ftTo)
    ft = ftTo;

  fillStrDateFilter(ft, hD, hT);
}
//----------------------------------------------------------------------------
void PFilterReport::verifyTime(UINT idc)
{
  HWND hD = GetDlgItem(*this, IDC_EDIT_INIT_TIME == idc ? IDC_EDIT_INIT_DATE : IDC_EDIT_END_DATE);
  HWND hT = GetDlgItem(*this, idc);
  FILETIME ft;
  unfillStrDateFilter(ft, hD, hT);
  fillStrDateFilter(ft, hD, hT);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PFilteID_Choose::PFilteID_Choose(PWin* parent, LPTSTR filter, uint resId) :
      baseClass(parent, resId), beginDrag(0), Filter(filter)
{
  lbAllDisp = new myListViewFilter(this, IDC_LIST_EXIST);
  lbToFilter = new myListViewFilter(this, IDC_LIST_TO_REMOVE);

  uint idBtnMove[] = {
       IDC_BUTTON_ADD_ALL,
       IDC_BUTTON_ADD,
       IDC_BUTTON_REM,
       IDC_BUTTON_REM_ALL
     };

  for(int i = 0; i < SIZE_A(idBtnMove); ++i)
    new POwnBtnImageStd(this, idBtnMove[i], (PBitmap*)0);
  int idBmp[] = { IDB_OK, IDB_DONE };
  int idBtn[] = { IDC_BUTTON_F1, IDC_BUTTON_F2 };

  for(int i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* Bmp = new PBitmap(idBmp[i], getHInstance());
    new POwnBtnImageStd(this, idBtn[i], Bmp, POwnBtnImageStd::wLeft, true);
    }
}
//----------------------------------------------------------------------------
static void setDefStyle(HWND obj)
{
  SendMessage (obj, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
  DWORD dwStyle = GetWindowLong (obj, GWL_STYLE);
  SetWindowLong (obj, GWL_STYLE, dwStyle | LVS_SHOWSELALWAYS);
}
//----------------------------------------------------------------------------
static void setLvColor(HWND obj, COLORREF fg, COLORREF bg)
{
  ListView_SetBkColor(obj, bg);
  ListView_SetTextBkColor(obj, bg);
  ListView_SetTextColor(obj, fg);
}
//----------------------------------------------------------------------------
void PFilteID_Choose::replaceLangText()
{
  setWindowTextByLangGlob(*this, ID_RP_TYPE_SECT_FLT_TITLE, false);
  int idTxt[] = { ID_DIALOG_OK, ID_DIALOG_CANC };
  int idBtn[] = { IDC_BUTTON_F1, IDC_BUTTON_F2 };

  for(uint i = 0; i < SIZE_A(idTxt); ++i)  {
    smartPointerConstString t = getStringByLangGlob(idTxt[i]);
    if(t) {
      PWin* btn = PWin::getWindowPtr(GetDlgItem(*this, idBtn[i]));
      if(btn)
        btn->setCaption(t);
      }
    }

  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_1), ID_RP_TYPE_SECT_FLT_AVAIL, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_2), ID_RP_TYPE_SECT_FLT_ACTIVED, false);
}
//----------------------------------------------------------------------------
bool PFilteID_Choose::create()
{
  if(!baseClass::create())
    return false;
  replaceLangText();
  setDefStyle(*lbAllDisp);
  setDefStyle(*lbToFilter);
  setLvColor(*lbAllDisp, RGB(0, 0, 100), RGB(255, 255, 200));
  setLvColor(*lbToFilter, RGB(100, 0, 0), RGB(255, 240, 220));

  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  lvc.fmt = LVCFMT_RIGHT;
  int sz[] = { 270, 50 };

  LPCTSTR tit[] = { _T("Description"), _T("Code")  };
  smartPointerConstString tDescr = getStringByLangGlob(ID_RP_TYPE_SECT_FLT_DESCR);
  if(tDescr)
    tit[0] = tDescr;
  smartPointerConstString tCode = getStringByLangGlob(ID_RP_TYPE_SECT_FLT_CODE);
  if(tCode)
    tit[1] = tCode;
  for(uint i = 0; i < SIZE_A(tit); ++i) {
    lvc.pszText = (LPTSTR)tit[i];
    lvc.cx = sz[i];
    if(ListView_InsertColumn (*lbAllDisp, i, &lvc) < 0)
      return false;
    if(ListView_InsertColumn (*lbToFilter, i, &lvc) < 0)
      return false;
    }
  fillList();
  SetFocus(*lbAllDisp);
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT PFilteID_Choose::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_ADD_ALL:
          addAll(lbToFilter, lbAllDisp);
          break;

        case IDC_BUTTON_ADD:
          addLB(lbToFilter, lbAllDisp);
          break;

        case IDC_BUTTON_REM:
          addLB(lbAllDisp, lbToFilter);
          break;

        case IDC_BUTTON_REM_ALL:
          addAll(lbAllDisp, lbToFilter);
          break;
        case IDC_BUTTON_F1:
          CmOk();
          break;
        case IDC_BUTTON_F2:
          CmCancel();
          break;
        }
      break;
    case WM_MOUSEMOVE:
      if(beginDrag) {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        HWND hw = ChildWindowFromPoint(*this, pt);
        if(2 == beginDrag) {
          if(*lbAllDisp == hw)
            SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN)));
          else
            SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN_MOVE)));
          }
        else if(1 == beginDrag) {
          if(*lbToFilter == hw)
            SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN)));
          else
            SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN_MOVE)));
          }
        }
      break;
    case WM_LBUTTONUP:
      if(beginDrag) {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        HWND hw = ChildWindowFromPoint(*this, pt);
        if(2 == beginDrag) {
          if(*lbAllDisp == hw)
            addLB(lbAllDisp, lbToFilter);
          }
        else if(1 == beginDrag) {
          if(*lbToFilter == hw)
            addLB(lbToFilter, lbAllDisp);
          }
        beginDrag = 0;
        ReleaseCapture();
        SetCursor(LoadCursor(0, IDC_ARROW));
        }
      break;
    case WM_NOTIFY:
      switch(((LPNMHDR)lParam)->code) {
        case LVN_KEYDOWN:
          if(VK_RETURN != ((LPNMLVKEYDOWN)lParam)->wVKey)
            break;
          // fall through
        case NM_DBLCLK:
          switch(((LPNMHDR)lParam)->idFrom) {
            case IDC_LIST_EXIST:
              addLB(lbToFilter, lbAllDisp);
              break;
            case IDC_LIST_TO_REMOVE:
              addLB(lbAllDisp, lbToFilter);
              break;
            }
          break;
        case LVN_BEGINDRAG:
          switch(((LPNMHDR)lParam)->idFrom) {
            case IDC_LIST_EXIST:
              beginDrag = 1;
              SetCapture(*this);
              SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN)));
              break;
            case IDC_LIST_TO_REMOVE:
              beginDrag = 2;
              SetCapture(*this);
              SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN)));
              break;
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
int PFilteID_Choose::makeLvRow(const lvRow& source, PListView* target, int item)
{
  LVITEM lvI;
  lvI.mask = LVIF_TEXT;
  lvI.iItem = item;
  lvI.iSubItem = 0;
  lvI.pszText = (LPTSTR)source.descr;

  lvI.iItem = ListView_InsertItem(*target, &lvI);
  if(lvI.iItem < 0) {
//    DisplayErrorString(GetLastError());
    return -1;
    }
  lvI.iSubItem = 1;
  lvI.pszText = (LPTSTR)source.id;
  ListView_SetItem(*target, &lvI);

  return lvI.iItem;
}
//----------------------------------------------------------------------------
bool PFilteID_Choose::unmakeLvRow(lvRow& target, PListView* source, int item)
{
  TCHAR buff[_MAX_PATH];
  LVITEM lvI;
  lvI.mask = LVIF_TEXT;
  lvI.iItem = item;
  lvI.cchTextMax = SIZE_A(buff);
  lvI.pszText = buff;
  lvI.iSubItem = 1;

  if(!ListView_GetItem(*source, &lvI))
    return false;

  _tcscpy_s(target.id, buff);
  lvI.iSubItem = 0;
  if(!ListView_GetItem(*source, &lvI))
    return false;
  _tcscpy_s(target.descr, buff);
  return true;
}
//------------------------------------------------------------------------------
void PFilteID_Choose::addAll(PListView* target, PListView* source)
{
  int count = ListView_GetItemCount(*source);
  if(0 >= count)
    return;
  PVect<lvRow> lvr;
  lvr.setDim(count);
  for(int i = 0; i < count; ++i)
    unmakeLvRow(lvr[i], source, i);

  for(int i = 0; i < count; ++i)
    makeLvRow(lvr[i], target);

  ListView_DeleteAllItems(*source);
}
//----------------------------------------------------------------------------
void PFilteID_Choose::addLB(PListView* target, PListView* source)
{
  int sel = ListView_GetNextItem(*source, -1, LVNI_SELECTED);
  if(-1 == sel)
    return;
  PVect<int> selected;
  selected[0] = sel;
  while(true) {
    sel = ListView_GetNextItem(*source, sel, LVNI_SELECTED);
    if(-1 == sel)
      break;
    selected[selected.getElem()] = sel;
    }
  int count = selected.getElem();

  PVect<lvRow> lvr;
  lvr.setDim(count);
  for(int i = count - 1; i >= 0; --i) {
    unmakeLvRow(lvr[i], source, selected[i]);
    ListView_DeleteItem(*source, selected[i]);
    }
  for(int i = 0; i < count; ++i)
    makeLvRow(lvr[i], target);
}
//----------------------------------------------------------------------------
void PFilteID_Choose::CmOk()
{
  *Filter = 0;
  int count = ListView_GetItemCount(*lbToFilter);
  if(0 >= count)
    return;
  lvRow lvr;
  unmakeLvRow(lvr, lbToFilter, 0);
  _tcscat_s(Filter, MAX_DIM_FILTER_ALL_ID, lvr.id);
  for(int i = 1; i < count; ++i) {
    unmakeLvRow(lvr, lbToFilter, i);
    _tcscat_s(Filter, MAX_DIM_FILTER_ALL_ID, _T(","));
    _tcscat_s(Filter, MAX_DIM_FILTER_ALL_ID, lvr.id);
    }
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
static void fillActived(PVect<int>& actived, LPCTSTR p)
{
  if(!*p)
    return;
  uint add = 0;
  while(p) {
    int t = _ttoi(p);
    actived[add++] = t;
    p = findNextParamTrim(p);
    }
}
//---------------------------------------------------------------------
static bool isInList(PVect<int>& actived, uint id)
{
  uint nElem = actived.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(id == actived[i]) {
      actived.remove(i);
      return true;
      }
    }
  return false;
}
//---------------------------------------------------------------------
void PFilteID_ChooseType::fillList()
{
  PVect<int> actived;
  fillActived(actived, Filter);
  for(uint i = 0, j = INIT_COD_ALARM; i < 8; ++i, ++j) {
    lvRow lvr;
    wsprintf(lvr.id, _T("%d"), i);
    bool needDelete = false;
    LPCTSTR p = getStringByLangGlob(j, needDelete);
    if(p) {
      copyStrZ(lvr.descr, p, MAX_DIM_DESCR - 1);
      if(needDelete)
        delete []p;
      }
    if(isInList(actived, i))
      makeLvRow(lvr, lbToFilter);
    else
      makeLvRow(lvr, lbAllDisp);
    }
}
//---------------------------------------------------------------------
extern void getFileStrCheckRemote(LPTSTR target, LPCTSTR file);
#define FILE_SECTION_ALARM_TXT _T("alarmSectionId.txs")
//---------------------------------------------------------------------
void PFilteID_ChooseSection::fillList()
{
  PVect<int> actived;
  fillActived(actived, Filter);

  TCHAR path[_MAX_PATH];
  getFileStrCheckRemote(path, FILE_SECTION_ALARM_TXT);
  setOfString sos(path);
  if(sos.setFirst()) {
    do {
      uint id = sos.getCurrId();
      smartPointerConstString str = getStringByLang(sos, id);
      lvRow lvr;
      wsprintf(lvr.id, _T("%d"), id);
      if(str) {
        LPCTSTR p = str;
        copyStrZ(lvr.descr, p, (size_t)MAX_DIM_DESCR - 1);
        }
      if(isInList(actived, id))
        makeLvRow(lvr, lbToFilter);
      else
        makeLvRow(lvr, lbAllDisp);
      } while(sos.setNext());
    }
  else {
    uint nElem = actived.getElem();
    for(uint i = 0; i < nElem; ++i) {
      lvRow lvr;
      wsprintf(lvr.id, _T("%d"), actived[i]);
      makeLvRow(lvr, lbToFilter);
      }
    }
}
//----------------------------------------------------------------------------

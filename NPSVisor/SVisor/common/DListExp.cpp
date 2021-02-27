//----- dlistexp.cpp ---------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
#include <commctrl.h>
//----------------------------------------------------------------------------
#include "dlistExp.h"
#include "P_BarProgr.h"
#include "dserchfl.h"
#include "p_body.h"
//#include "log_oper.h"
#include "log_ord.h"
#include "val_ord.h"
#include "def_dir.h"
#include "gestdata.h"
#include "p_date.h"
#include "d_chooseDate.h"
#include "dlistChoose.h"

#include "DInputDt.h"
#include "Commonlgraph.h"

#include "1.h"
#include "5500.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int TD_ListExp::choose;
//--------------------------------------------------------------------------
static void incrExpCounter(P_File& pf, bool first)
{
  TCHAR disk[_MAX_DRIVE];
  TCHAR dir[_MAX_DIR];
  TCHAR file[_MAX_FNAME];
  TCHAR ext[_MAX_EXT];

  _tsplitpath_s(pf.get_name(), disk, SIZE_A(disk), dir, SIZE_A(dir), file, SIZE_A(file), ext, SIZE_A(ext));
  if(first)
    _tcscat_s(file, _T("_a`"));
  int len = _tcslen(file);
#if 0
  if(len < 3 || file[len-3] != _T('_')) {
    switch(file[len - 1]) {
      case _T('-'):
      case _T('_'):
      case _T('.'):
        file[len-1] = _T('_');
        _tcscat_s(file, _T("aa"));
        break;
      default:
        _tcscat_s(file, _T("_aa"));
        break;
      }
    len = _tcslen(file);
    }
#endif
  if(file[len - 1] + 1 > _T('z')) {
    ++file[len - 2];
    file[len - 1] = _T('a');
    }
  else
    ++file[len - 1];

  TCHAR filename[_MAX_PATH];
  _tmakepath_s(filename, SIZE_A(filename), disk, dir, file, ext);

  pf.P_chgName(filename);
}
//--------------------------------------------------------------------------
TD_ListExp::TD_ListExp(PWin* parent, int resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst)
{
  LB = new PListBox(this, IDC_LISTBOX_CHOOSE_EXP);
}
//--------------------------------------------------------------------------
TD_ListExp::~TD_ListExp()
{
  destroy();
}
//--------------------------------------------------------------------------
bool TD_ListExp::create()
{
  if(!baseClass::create())
    return false;

  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F1), ID_DIALOG_OK);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F2), ID_DIALOG_CANC);
  int j = 0;
  for(int i = ID_EXP_TITLE_INIT; i <= ID_EXP_TITLE_END; ++i) {
    bool needDelete;
    LPCTSTR p = getStringByLangGlob(i, needDelete);
    smartPointerConstString sp(p, needDelete);

    if(p) {
      SendMessage(*LB, LB_ADDSTRING, 0, (LPARAM)p);
      ++j;
      }
    }
  SendMessage(*LB, LB_SETCURSEL, choose, 0);
  if(choose < 0 || choose >= j)
    choose = 0;

  return true;
}
//--------------------------------------------------------------------------
class TD_ChooseMatch : public TD_ListExp
{
  public:
    TD_ChooseMatch::TD_ChooseMatch(PWin* parent, PVect<LPCTSTR>& list) :
      TD_ListExp(parent), List(list) {}
    virtual bool create();
  private:
    PVect<LPCTSTR>& List;

};
//--------------------------------------------------------------------------
bool TD_ChooseMatch::create()
{
  if(!svDialog::create())
    return false;
  int nElem = List.getElem();
  for(int i = 0; i < nElem; ++i)
    SendMessage(*LB, LB_ADDSTRING, 0, (LPARAM)List[i]);

  if(choose < 0 || choose >= nElem)
    choose = 0;
  SendMessage(*LB, LB_SETCURSEL, choose, 0);
  return true;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
LRESULT TD_ListExp::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_LISTBOX_CHOOSE_EXP:
          if(LBN_DBLCLK != HIWORD(wParam))
            break;
        case IDC_BUTTON_F1:
        case IDOK:
          choose = SendMessage(*LB, LB_GETCURSEL, 0, 0);
          EndDialog(*this, IDOK);
          break;
        case IDC_BUTTON_F2:
        case IDCANCEL:
          EndDialog(*this, IDCANCEL);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
#define STD_SEP _T('.')
void replaceSep(LPTSTR buff, LPCTSTR sep)
{
  if(!sep)
    return;
  int len = _tcslen(buff);
  for(int i = 0; i < len; ++i)
    if(STD_SEP == buff[i])
      buff[i] = *sep;
}
//--------------------------------------------------------------------------
static LPTSTR makeStrHistory(LPTSTR buff, size_t lenBuff, int id)
{
  smartPointerConstString sp = getStringOrIdByLangGlob(id);
  _tcscpy_s(buff, lenBuff, &sp);

  smartPointerConstString sp2 = getStringOrIdByLangGlob(ID_TITLE_HISTORY);
  _tcscat_s(buff, lenBuff, &sp2);

  return buff;
}
//--------------------------------------------------------------------------
typedef void (*fPath)(LPTSTR);

#define EXP_TO_HISTORY

//--------------------------------------------------------------------------
#define KEY_EXP_ALARM _T("Exp_Alarm")
//--------------------------------------------------------------------------
static returnExp exp_alarm(PWin* par, bool rep, bool history, int id)
{
  TCHAR path[_MAX_PATH];
  eDir which = rep ? dRepAlarm : dAlarm;
  makePath(path, _T("*"), which, history);

  LPCTSTR p;
  TCHAR tit[100];
  if(history)
    p = makeStrHistory(tit, SIZE_A(tit), id);
  else
    p = getStringOrId(id);

#if 0
  static infoSearch stInfo;
  infoSearch info(p, path, stInfo);
#else
  infoSearchParam isp;
  getKeyInfoSearch(KEY_EXP_ALARM, isp);
  infoSearch info(p, path, isp);
#endif
  TCHAR tFile[_MAX_PATH];
  if(!info.chooseParam(par, true))
    return rAbort;

#if 0
  stInfo.copyData(info);
#else
  info.copyDataTo(isp);
  setKeyInfoSearch(KEY_EXP_ALARM, isp);
#endif

  if(gSearchFile(info, par, tFile)) {
  #ifdef EXP_TO_HISTORY
    path[0] = 0;
    makeExportPath(path, _T(""), true, dHistory);
    createDirectoryRecurse(path);
  #endif
    makePath(path, tFile, which, history);

    TCHAR tabFile[_MAX_PATH];
  #ifdef EXP_TO_HISTORY
    makeExportPath(tabFile, tFile, true, dExport);
  #else
    makePath(tabFile, tFile, which, history, dExport);
  #endif

    P_File f(tabFile, P_CREAT);
    if(f.P_open()) {
      if(rep) {
        rep_alarm stat;
        if(stat.exportData(path, f))
          return rSuccess;
        }
      else {
        stat_alarm stat;
        if(stat.exportData(path, f))
          return rSuccess;
        }
      }
    return rFailed;
    }
  return rAbort;
}
//--------------------------------------------------------------------------
static void getDateInitEnd(P_File& source, FILETIME& ftInit, FILETIME& ftEnd, int len, LPBYTE buff)
{
  source.P_seek(0);
  source.P_read(buff, len);
  ftInit = *reinterpret_cast<FILETIME*>(buff);
  source.P_seek(-len, SEEK_END_);
  source.P_read(buff, len);
  ftEnd = *reinterpret_cast<FILETIME*>(buff);
  source.P_seek(0);
  if(ftEnd < ftInit)
    ftEnd = getFileTimeCurr();
}
//--------------------------------------------------------------------------
static LPBYTE performInit(PWin* par, P_File& source, rangeTime& rt, int lenRec, int type)
{
  LPBYTE buff = new BYTE[lenRec + 2];
  getDateInitEnd(source, rt.from, rt.to, lenRec, buff);
  // 100 è l'indicazione di gestione storico
  if(type >= 100)
    type -= 100;
  // se type == 0 il trend è giornaliero, non ha senso scegliere la data per giorno
  bool success = true;
  switch(type) {
    case 0:
      success = IDOK == TD_InputTime(rt.from, rt.to, par).modal();
      break;
    case 1:
      success = IDOK == TD_InputDateOnlyDay(rt.from, rt.to, par).modal();
      break;
    default:
      success = IDOK == TD_InputDate(rt.from, rt.to, par).modal();
      break;
    }
 if(!success) {
   delete []buff;
   return 0;
   }
  return buff;
}
//--------------------------------------------------------------------------
static void clearDirectory(LPCTSTR targetPath)
{
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, SIZE_A(path), targetPath);
  _tcscat_s(path, SIZE_A(path), _T("*.*"));
  WIN32_FIND_DATA fd;
  HANDLE hfff = FindFirstFile(path, &fd);
  if(INVALID_HANDLE_VALUE != hfff) {
    do {
      if(FILE_ATTRIBUTE_DIRECTORY == fd.dwFileAttributes)
        continue;
      _tcscpy_s(path, SIZE_A(path), targetPath);
      _tcscat_s(path, SIZE_A(path), fd.cFileName);
      DeleteFile(path);
      } while(FindNextFile(hfff, &fd));
    FindClose(hfff);
    }
}
//--------------------------------------------------------------------------
static bool exportBin(PWin* par, P_File& target, P_File& source, const rangeTime& rt, int lenRec, LPBYTE buff)
{
  source.P_seek(0, SEEK_END_);
  DWORD records = (DWORD)source.get_len() / lenRec;
  BarProgr bar(par, records);
  bar.create();
  DWORD ix = findFirstPos(source, rt.from, lenRec, buff, 0, true);

  for(; ix < records;) {
    if(!(++ix & 0xf))
      if(!bar.setPos(ix))
        break;
    if(lenRec != source.P_read(buff, lenRec))
      return true;
    FILETIME& ft = *reinterpret_cast<FILETIME*>(buff);
    if(cMK_I64(rt.from) && rt.from > ft)
      continue;

    if(cMK_I64(rt.to) && rt.to < ft)
      break;

    if(lenRec != target.P_write(buff, lenRec))
      return false;
    }
  return true;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
static returnExp ExpBinary(PWin* par, LPCTSTR source, bool history, int ix, LPCTSTR ext)
{
  TCHAR sourcePath[_MAX_PATH] = _T("\0");
  makePath(sourcePath, dExRep, true, _T(""));

  TCHAR targetPath[_MAX_PATH];
  _tcscpy_s(targetPath, SIZE_A(targetPath), sourcePath);

  appendPath(targetPath, _T("Bin\\"));
  CreateDirectory(targetPath, 0);
  clearDirectory(targetPath);

  _tcscat_s(targetPath, SIZE_A(targetPath), source);
  _tcscat_s(targetPath, SIZE_A(targetPath), ext);

  makePath(sourcePath, source, dExRep, history, ext);
  P_File fS(sourcePath, P_READ_ONLY);
  if(!fS.P_open())
    return rFailed;
  P_File fT(targetPath);
  if(!fT.P_open())
      return rFailed;

  mainClient* wMain = getMain();
  if(!wMain)
    return rFailed;
  ImplJobData* job = wMain->getIData();
  const ImplJobMemBase::save_trend& sTrend = job->getSaveTrend(ix);
  int lenRec = sTrend.getLenRec();

  rangeTime rt;
  int type = sTrend.getType();
  LPBYTE buff = performInit(par, fS, rt, lenRec, type);
  if(!buff)
    return rAbort;

  bool success = exportBin(par, fT, fS, rt, lenRec, buff);
  delete []buff;
  return success ? rSuccess : rFailed;

}





//--------------------------------------------------------------------------
#define MAX_REC_EXP 65530
//--------------------------------------------------------------------------
//returnExp exportExByGraph(PWin* par, P_File& target, P_File& source, setOfString& set)
returnExp exportExByGraph(PWin* par, P_File& target, P_File& source, uint idTrend)
{
  mainClient* wMain = getMain();
  if(!wMain)
    return rFailed;

  criticalSect cSect;
  ImplJobMemBase::save_trend sTrend(cSect, idTrend, 0);
  if(!sTrend.init(0))
    return rFailed;

  do {
    LPCTSTR p = getString(idTrend);
    p = findNextParam(p, 1);
    uint id2 = _ttoi(p);
    p = getString(id2);
    int v1 = 0;
    int v2 = 0;
    int v3 = 0;
    DWORD resolution = 0;
     _stscanf_s(p, _T("%d,%d,%d,%d"), &v1, &v2, &v3, &resolution);
    if(!resolution) {
      int success = 0;
      while(!success)
        success = sTrend.checkTimeRes(source, resolution);
      if(resolution) {
        TCHAR buff[100];
        _stprintf_s(buff, SIZE_A(buff), _T("%d,%d,%d,%d"), v1, v2, v3, resolution);
        replaceString(id2, str_newdup(buff), true);
        }
      source.P_seek(0);
      }
    } while(false);

  int lenRec = sTrend.getLenRec();
  rangeTime rt;
  int type = sTrend.getType();
  LPBYTE buff = performInit(wMain, source, rt, lenRec, type);
  if(!buff)
    return rAbort;

  DWORD records = (DWORD)source.get_len() / lenRec;
  BarProgr bar(par, records);
  bar.create();

  sTrend.exportHeader(target);
  DWORD count = 0;
  DWORD maxRecExp = MAX_REC_EXP;
  DWORD i = findFirstPos(source, rt.from, lenRec, buff, 0, true);
  bool first = true;
  for(; i < records; ++i) {
    if(!(i & 0xf))
      if(!bar.setPos(i))
        break;
    if(lenRec != source.P_read(buff, lenRec))
      break;
    FILETIME& ft = *reinterpret_cast<FILETIME*>(buff);
    if(cMK_I64(rt.from) && rt.from > ft)
      continue;

    if(cMK_I64(rt.to) && rt.to < ft)
      break;
    sTrend.exportData(target, buff);
    if(++count >= maxRecExp) {
      incrExpCounter(target, first);
      first = false;
      sTrend.exportHeader(target);
      count = 0;
      }
    }
  delete []buff;
  return rSuccess;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
static returnExp exportEx(PWin* par, P_File& target, P_File& source, int ix)
{
  mainClient* wMain = getMain();
  if(!wMain)
    return rFailed;
  ImplJobData* job = wMain->getIData();
  const ImplJobMemBase::save_trend& sTrend = job->getSaveTrend(ix);
  int lenRec = sTrend.getLenRec();
  rangeTime rt;
  int type = sTrend.getType();
  LPBYTE buff = performInit(wMain, source, rt, lenRec, type);
  if(!buff)
    return rAbort;

  DWORD records = (DWORD)source.get_len() / lenRec;
  BarProgr bar(par, records);
  bar.create();

  sTrend.exportHeader(target);

  DWORD count = 0;
  DWORD maxRecExp = MAX_REC_EXP;
  DWORD i = findFirstPos(source, rt.from, lenRec, buff, 0, true);
  bool first = true;
  for(; i < records; ++i) {
    if(!(i & 0xf))
      if(!bar.setPos(i))
        break;
    if(lenRec != source.P_read(buff, lenRec))
      break;
    FILETIME& ft = *reinterpret_cast<FILETIME*>(buff);
    if(cMK_I64(rt.from) && rt.from > ft)
      continue;

    if(cMK_I64(rt.to) && rt.to < ft)
      break;
    sTrend.exportData(target, buff);
    if(++count >= maxRecExp) {
      incrExpCounter(target, first);
      first = false;
      sTrend.exportHeader(target);
      count = 0;
      }
    }
  delete []buff;
  return rSuccess;
}
//--------------------------------------------------------------------------
#define KEY_EXP_DATA _T("Exp_Data")
//--------------------------------------------------------------------------
returnExp exportData(idExport choose, PWin* par, int id, bool history)
{
  mainClient* wMain = getMain();
  if(!wMain)
    return rFailed;
  ImplJobData* job = wMain->getIData();

  int nTrend = job->getNumOfTrend();
  if(nTrend <= 0)
    return rAbort;

  bool binaryExp;
  do {
    TD_ListChoose which(par);
    if(IDOK != which.modal())
      return rAbort;
    binaryExp = which.getChoose() > 0;
    } while(false);

  PVect<LPCTSTR> matchList;
  matchList.setDim(nTrend);
  int j = 0;
  for(int i = 0; i < nTrend; ++i) {
    const ImplJobMemBase::save_trend& sTrend = job->getSaveTrend(i);
    TCHAR path[_MAX_PATH];
    bool useHistory;
    if(sTrend.makeFindableName(path, useHistory))
      if(!(useHistory ^ history))
        matchList[j++] = str_newdup(path);
    }
  if(!j)
    return rAbort;

  TD_ChooseMatch which(par, matchList);
  if(IDOK != which.modal()) {
    flushPAV(matchList);
    return rAbort;
    }
  int ix = which.getChoose();

  TCHAR path[_MAX_PATH];
  makePath(path, matchList[ix], dExRep, history, _T(""));
  TCHAR ext[10];
  _tsplitpath_s(matchList[ix], 0, 0, 0, 0, 0, 0, ext, SIZE_A(ext));
  flushPAV(matchList);
  LPCTSTR p;
  TCHAR tit[100];
  if(history)
    p = makeStrHistory(tit, SIZE_A(tit), id);
  else
    p = getStringOrId(id);

#if 0
  static infoSearch stInfo;
  infoSearch info(p, path, stInfo);
#else
  infoSearchParam isp;
  getKeyInfoSearch(KEY_EXP_DATA, isp);
  infoSearch info(p, path, isp);
#endif
  TCHAR tFile[_MAX_PATH];
  if(!info.chooseParam(par, true))
    return rAbort;

#if 0
  stInfo.copyData(info);
#else
  info.copyDataTo(isp);
  setKeyInfoSearch(KEY_EXP_DATA, isp);
#endif

  if(gSearchFile(info, par, tFile)) { {
    if(binaryExp)
      return ExpBinary(par, tFile, history, ix, ext);
  #ifdef EXP_TO_HISTORY
    makeExportPath(path, _T(""), true, dHistory);
    createDirectoryRecurse(path);
  #endif
      _tcscpy_s(path, SIZE_A(path), tFile);

      TCHAR tabFile[_MAX_PATH];
#ifdef EXP_TO_HISTORY
      makeExportPath(tabFile, path, true, dHistory);
//      _tcscat_s(tabFile, SIZE_A(tabFile), _T("_aa"));
      _tcscat_s(tabFile, SIZE_A(tabFile), _T(".tab"));

      makePath(path, dExRep, history, ext);
#else
      _tcscpy_s(tabFile, SIZE_A(tabFile), path);
      makePath(path, dExRep, history, ext);
//      _tcscat_s(tabFile, SIZE_A(tabFile), _T("_aa"));
      makePath(tabFile, dExport, history);
#endif

      P_File f(path, P_READ_ONLY);
      P_File t(tabFile, P_CREAT);

      if(f.P_open() && t.P_open()) {
        return exportEx(par, t, f, ix);
        }
      }
    return rFailed;
    }
  return rAbort;
}
//------------------------------------------------------------
/**/
//------------------------------------------------------------
void exportData(PWin* par, bool history)
{
  TD_ListExp lb(par);
  if(IDOK == lb.modal()) {
    idExport choose = static_cast<idExport>(lb.getChoose());
    int id = choose + ID_EXP_TITLE_INIT;
    returnExp success;// = rAbort;
    switch(choose) {
      case Glob: //  globale
        success = exp_alarm(par, false, history, id);
        break;
      case Rep: //  tutti gli allarmi
        success = exp_alarm(par, true, history, id);
        break;
      default:
        // nessuno dei precedenti, passa alla routine esterna
        success = exportData(choose, par, id, history);
        break;
      }
    if(rAbort != success)
      msgBoxByLangGlob(par, ID_EXP_FAILED + success, ID_EXP_SUCCESS_TITLE, MB_ICONINFORMATION);

    }
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
smartPointerConstString getTitleRepAlarm()
{
  return getStringOrIdByLangGlob(ID_EXP_TITLE_REP_ALARM);
}



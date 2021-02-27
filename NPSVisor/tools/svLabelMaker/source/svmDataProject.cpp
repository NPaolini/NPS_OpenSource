//------------------ svmDataProject.cpp ----------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "POwnBtnImageStd.h"
#include "svmDataProject.h"
#include "macro_utils.h"
#include "svmproperty.h"
#include "pPrintPage.h"
//-----------------------------------------------------------
class PInputDataVars : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PInputDataVars(PWin* parent, uint* idVars, uint id = IDD_GET_DATA_ID,  HINSTANCE hInstance = 0) :
        baseClass(parent, id, hInstance), idVars(idVars)
    {
    }
    virtual ~PInputDataVars() { destroy(); }

  protected:
    void CmOk();
    uint* idVars;
};
//-----------------------------------------------------------
void PInputDataVars::CmOk()
{
  TCHAR t[_MAX_PATH];
  GET_TEXT(IDC_EDIT_IDS, t);
  LPCTSTR p = t;
  for(uint i = 0; i < MAX_ID_DATA_VARS; ++i) {
    if(p)
      idVars[i] = _ttoi(p);
    else
      idVars[i] = 0;
    p = findNextParamTrim(p);
    }
  baseClass::CmOk();
}
//-----------------------------------------------------------
bool PDataProject::create()
{
  int idBmp[] = { IDB_OK, IDB_CANC };
  int idBtn[] = { IDOK, IDCANCEL };

  if(useForSave == whichUse)
    idBmp[0] = IDB_SAVE;

  for(int i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* Bmp = new PBitmap(idBmp[i], getHInstance());
    new POwnBtnImageStd(this, idBtn[i], Bmp, POwnBtnImageStd::wLeft, true);
    }

  if(!baseClass::create())
    return false;
  dataProject& dp = getDataProject();
//  if(useForNew != whichUse) {
    SET_TEXT(IDC_EDIT_BASEPATH, dp.getcurrPath());
    SET_TEXT(IDC_EDIT_TEMPLATE, dp.gettemplateName());
    SET_TEXT(IDC_EDIT_TEMPL_DATA, dp.getvarFileName());
    enableChoose();
//    }

  SET_INT(IDC_EDIT_WIDTH, dp.getPageWidth());
  SET_INT(IDC_EDIT_HEIGHT, -dp.getPageHeight());

  if(useForNew == whichUse) {
    SET_CHECK(IDC_CHECK_MODIFY);
    ShowWindow(GetDlgItem(*this, IDC_CHECK_MODIFY), SW_HIDE);
    }
  else if(useForOPen == whichUse)
    setDim(dp.gettemplateName());
  enableDim();
  return true;
}
//-----------------------------------------------------------
void PDataProject::CmOk()
{
  dataProject& dp = getDataProject();
  TCHAR t[_MAX_PATH];
//  if(useForNew != whichUse) {
    GET_TEXT(IDC_EDIT_BASEPATH, t);
    dp.setcurrPath(t);
    GET_TEXT(IDC_EDIT_TEMPLATE, t);
    dp.settemplateName(t);
    GET_TEXT(IDC_EDIT_TEMPL_DATA, t);
    dp.setvarFileName(t);
//    }
  int v;
  GET_INT(IDC_EDIT_WIDTH, v);
  dp.setPageWidth(v);
  GET_INT(IDC_EDIT_HEIGHT, v);
  dp.setPageHeight(-v);

  if(idVars && useForOPen == whichUse) {
    GET_TEXT(IDC_EDIT_BASEPATH, t);
    appendPath(t, dp.gettemplateName());
    setOfString set(t);
    LPCTSTR p = set.getString(ID_FILE_DATA_VARS);
    if(!p)
      PInputDataVars(this, idVars).modal();
    }
  baseClass::CmOk();
}
//-----------------------------------------------------------
void PDataProject::enableDim()
{
//  if(useForNew != whichUse) {
    bool enable = IS_CHECKED(IDC_CHECK_MODIFY);
    ENABLE(IDC_EDIT_WIDTH, enable);
    ENABLE(IDC_EDIT_HEIGHT, enable);
//    }
}
//-----------------------------------------------------------
void PDataProject::enableChoose()
{
  TCHAR t[_MAX_PATH];
  GET_TEXT(IDC_EDIT_BASEPATH, t);
  bool enable = *t != 0;
  ENABLE(IDC_BUTTON_CHOOSE_TEMPLATE, enable);
  ENABLE(IDC_BUTTON_CHOOSE_TEMPL_DATA, enable);
}
//-----------------------------------------------------------
bool existFilePath(LPCTSTR t)
{
  WIN32_FIND_DATA FindFileData;
  HANDLE h = FindFirstFile(t, &FindFileData);
  if(h == INVALID_HANDLE_VALUE)
    return false;
  FindClose(h);
  return true;
}
//-----------------------------------------------------------
bool checkExistPath(PWin* owner, LPTSTR t)
{
  if(!existFilePath(t)) {
    if(IDYES == MessageBox(*owner, _T("La cartella non esiste, vuoi crearla?"), _T("Attenzione"), MB_YESNO | MB_ICONSTOP)) {
      createDirectoryRecurse(t);
      return true;
      }
    return false;
    }
  return true;
}
//-----------------------------------------------------------
void PDataProject::choosePath()
{
  TCHAR t[_MAX_PATH];
  GET_TEXT(IDC_EDIT_BASEPATH, t);
  TCHAR key[_MAX_PATH];
  wsprintf(key, OLD_BASE_PATH, getDataProject().currPage);

  if(findFolder(key, t, _T("Scelta percorso base"), false)) {
    if(checkExistPath(this, t)) {
      SET_TEXT(IDC_EDIT_BASEPATH, t);
      enableChoose();
      }
    }
}
//-----------------------------------------------------------
static bool isSamePath(LPTSTR path, LPCTSTR file)
{
  int len = _tcslen(path);
  if(!cmpStr(path, file, len)) {
    if(_T('\\') == file[len] || _T('/') == file[len])
      ++len;
    _tcscpy_s(path, _MAX_PATH, file + len);
    return true;
    }
  return false;
}
//-----------------------------------------------------------
void PDataProject::setDim(LPCTSTR file)
{
  TCHAR path[_MAX_PATH];
  GET_TEXT(IDC_EDIT_BASEPATH, path);
  appendPath(path, file);
  setOfString set(path);
  LPCTSTR p = set.getString(ID_FILE_PAGE_DIM);
  if(p) {
    SET_INT(IDC_EDIT_WIDTH, _ttoi(p));
    p = findNextParamTrim(p);
    if(p)
      SET_INT(IDC_EDIT_HEIGHT, _ttoi(p));
    }
  p = set.getString(ID_FILE_DATA);
  bool enable = true;
  if(p) {
    TCHAR file2[_MAX_PATH];
    TCHAR ext[_MAX_FNAME];
    _tsplitpath_s(p, 0, 0, 0, 0, file2, SIZE_A(file2), ext, SIZE_A(ext));
    _tcscat_s(file2, ext);
    SET_TEXT(IDC_EDIT_TEMPL_DATA, file2);
    enable = false;
    }
  ENABLE(IDC_BUTTON_CHOOSE_TEMPL_DATA, enable);
  ENABLE(IDC_EDIT_TEMPL_DATA, enable);
}
//-----------------------------------------------------------
void PDataProject::chooseFile(uint idc_edit, bool templ)
{
  TCHAR currPath[_MAX_PATH];
  GetCurrentDirectory(_MAX_PATH, currPath);

  TCHAR t[_MAX_PATH];
  GET_TEXT(IDC_EDIT_BASEPATH, t);
  SetCurrentDirectory(t);

  TCHAR t2[_MAX_PATH];
  GET_TEXT(idc_edit, t2);
  if(*t2)
    appendPath(t, t2);
  else
    t[0] = 0;
  if(openFileText(*this, t, useForOPen != whichUse, templ)) {
    GET_TEXT(IDC_EDIT_BASEPATH, t2);
    if(isSamePath(t2, t)) {
      SET_TEXT(idc_edit, t2);
      if(templ)
        setDim(t2);
      }
    else
      MessageBox(*this, _T("Il file deve essere nel percorso base"), _T("Attenzione"), MB_OK | MB_ICONSTOP);
    }
  SetCurrentDirectory(currPath);
}
//-----------------------------------------------------------
LRESULT PDataProject::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_PATH:
          choosePath();
          break;
        case IDC_BUTTON_CHOOSE_TEMPLATE:
          chooseFile(IDC_EDIT_TEMPLATE, true);
          break;
        case IDC_BUTTON_CHOOSE_TEMPL_DATA:
          chooseFile(IDC_EDIT_TEMPL_DATA, false);
          break;

        case IDC_CHECK_MODIFY:
          enableDim();
          break;
        case IDC_EDIT_BASEPATH:
          switch(HIWORD(wParam)) {
            case EN_KILLFOCUS:
              do {
                if(SendMessage(GetDlgItem(*this, IDC_EDIT_BASEPATH), EM_GETMODIFY, 0, 0)) {
                  TCHAR t[_MAX_PATH];
                  GET_TEXT(IDC_EDIT_BASEPATH, t);
                  if(!checkExistPath(this, t))
                    SET_TEXT(IDC_EDIT_BASEPATH, _T(""));
                  enableChoose();
                  }
                } while(false);
              break;
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//---------------------------------------------------------
void dataProject::initPath()
{
/*
  TCHAR t[_MAX_PATH];
  for(currPage = 0; currPage < MAX_PAGES; ++currPage) {
    wsprintf(t, OLD_BASE_PATH, currPage);
    TCHAR path[_MAX_PATH];
    getKeyPath(t, path);
    setcurrPath(path);
    }
  currPage = 0;
*/
}
//---------------------------------------------------------
#define GetRGB(c) GetRValue(c), GetGValue(c), GetBValue(c)
//---------------------------------------------------------
bool infoState::savePen(P_File& pf, uint id)
{
  if(idPen != id) {
    idPen = id;
    TCHAR t[64];
    dualPen* dp = getDualPen();
    infoPen* ip = dp->getIt(idPen);
    wsprintf(t, _T("%d,%d,%d,%d,%d,%d\r\n"), PRN_SET_PEN, GetRGB(ip->color), ip->tickness, ip->style);
    return writeStringChkUnicode(pf, t) > 0;
    }
  return true;
}
//---------------------------------------------------------
bool infoState::saveBrush(P_File& pf, uint id)
{
  if(idBrush != id) {
    idBrush = id;
    TCHAR t[64];
    dualBrush* dp = getDualBrush();
    infoBrush* ib = dp->getIt(idBrush);
    if(cTRANSP == ib->color)
      wsprintf(t, _T("%d,0\r\n"), PRN_SET_NULL_BRUSH);
    else if(-1 == ib->style)
      wsprintf(t, _T("%d,%d,%d,%d\r\n"), PRN_SET_BRUSH1, GetRGB(ib->color));
    else
      wsprintf(t, _T("%d,%d,%d,%d,%d\r\n"), PRN_SET_BRUSH2, GetRGB(ib->color), ib->style);

    return writeStringChkUnicode(pf, t) > 0;
    }
  return true;
}
//---------------------------------------------------------
bool infoState::saveFont(P_File& pf, uint id, int angle)
{
  if(idFont != id || Angle != angle) {
    idFont = id;
    Angle = angle;
    TCHAR t[64];
    svmFont& fnt = getFontObj();
    const typeFont* tpf = fnt.getType(idFont);
    uint flag = tpf->getFlag();

    if(Angle)
      wsprintf(t, _T("%d,%d,%d,%d,%d,"), PRN_SET_FONT2, tpf->h, tpf->w, Angle, flag);
    else
      wsprintf(t, _T("%d,%d,%d,%d,"), PRN_SET_FONT, tpf->h, tpf->w, flag);

    if(writeStringChkUnicode(pf, t))
      if(writeStringChkUnicode(pf, tpf->name))
        return writeStringChkUnicode(pf, _T("\r\n")) > 0;
    }
  return true;
}
//---------------------------------------------------------
bool infoState::saveForeground(P_File& pf, COLORREF c)
{
  if(foreground != c) {
    foreground = c;
    TCHAR t[64];
    wsprintf(t, _T("%d,%d,%d,%d\r\n"), PRN_SET_COLOR_FG, GetRGB(foreground));
    return writeStringChkUnicode(pf, t) > 0;
    }
  return true;
}
//---------------------------------------------------------
bool infoState::saveBackground(P_File& pf, COLORREF c)
{
  if(background != c) {
    background = c;
    TCHAR t[64];
    wsprintf(t, _T("%d,%d,%d,%d\r\n"), PRN_SET_COLOR_BKG, GetRGB(background));
    return writeStringChkUnicode(pf, t) > 0;
    }
  return true;
}
//---------------------------------------------------------
bool infoState::saveBkMode(P_File& pf, uint mode)
{
  if(bkMode != mode) {
    bkMode = mode;
    TCHAR t[64];
    wsprintf(t, _T("%d,%d\r\n"), PRN_SET_BKMODE, bkMode);
    return writeStringChkUnicode(pf, t) > 0;
    }
  return true;
}
//---------------------------------------------------------
bool infoState::saveAlign(P_File& pf, uint align)
{
  if(Align != align) {
    Align = align;
    TCHAR t[64];
    wsprintf(t, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, Align, TA_TOP);
    return writeStringChkUnicode(pf, t) > 0;
    }
  return true;
}
//----------------------------------------------------------
//----------------------------------------------------------
static LPCTSTR getRgb(LPCTSTR p, COLORREF& c)
{
  int r = 0;
  int g = 0;
  int b = 0;
  _stscanf_s(p, _T("%d,%d,%d"),  &r, &g, &b);
  c = RGB(r, g, b);
  return findNextParamTrim(p, 3);
}
//---------------------------------------------------------
bool infoState::loadPen(LPCTSTR row)
{
  int id = _ttoi(row);
  if(PRN_SET_PEN != id)
    return false;
  row = findNextParamTrim(row);
  bool success = false;
  while(row) {
    infoPen ipen;
    row = getRgb(row, ipen.color);
    if(!row)
      break;
    ipen.tickness = _ttoi(row);
    row = findNextParamTrim(row);
    if(!row)
      break;
    ipen.style = _ttoi(row);
    dualPen* dp = getDualPen();
    idPen = dp->addAndGetId(&ipen);
    success = true;
    break;
    }
  return success;
}
//---------------------------------------------------------
bool infoState::loadBrush(LPCTSTR row)
{
  int id = _ttoi(row);
  infoBrush iBr;
  row = findNextParamTrim(row);
  switch(id) {
    case PRN_SET_NULL_BRUSH:
      iBr.color = cTRANSP;
      break;
    case PRN_SET_BRUSH1:
      if(!row)
        return false;
      getRgb(row, iBr.color);
      iBr.style = -1;
      break;
    case PRN_SET_BRUSH2:
      if(!row)
        return false;
      row = getRgb(row, iBr.color);
      if(!row)
        return false;
      iBr.style = _ttoi(row);
      break;
    default:
      return false;
    }

  dualBrush* dp = getDualBrush();
  idBrush = dp->addAndGetId(&iBr);
  return true;
}
//---------------------------------------------------------
bool infoState::loadFont(LPCTSTR row, bool linked)
{
  int id = _ttoi(row);
  row = findNextParamTrim(row);
  typeFont* tpf = new typeFont;
  uint flag = 0;
  Angle = 0;
  switch(id) {
    case PRN_SET_FONT:
      _stscanf_s(row, _T("%hd,%hd,%d"),  &tpf->h, &tpf->w, &flag);
      tpf->setFlag(flag);
      tpf->name = str_newdup(findNextParamTrim(row, 3));
      break;
    case PRN_SET_FONT2:
      _stscanf_s(row, _T("%hd,%hd,%d,%d"),  &tpf->h, &tpf->w, &Angle, &flag);
      tpf->setFlag(flag);
      tpf->name = str_newdup(findNextParamTrim(row, 4));
      break;
    default:
      delete tpf;
      return false;
    }
  if(linked) {
    svmFont& fnt = getFontObjLinked();
    idFont = fnt.addFont(tpf);
    }
  else {
    svmFont& fnt = getFontObj();
    idFont = fnt.addFont(tpf);
    }
  return true;
}
//---------------------------------------------------------
bool infoState::loadForeground(LPCTSTR row)
{
  int id = _ttoi(row);
  if(PRN_SET_COLOR_FG != id)
    return false;
  row = findNextParamTrim(row);
  getRgb(row, foreground);
  return true;
}
//---------------------------------------------------------
bool infoState::loadBackground(LPCTSTR row)
{
  int id = _ttoi(row);
  if(PRN_SET_COLOR_BKG != id)
    return false;
  row = findNextParamTrim(row);
  getRgb(row, background);
  return true;
}
//---------------------------------------------------------
bool infoState::loadBkMode(LPCTSTR row)
{
  int id = _ttoi(row);
  if(PRN_SET_BKMODE != id)
    return false;
  row = findNextParamTrim(row);
  bkMode = _ttoi(row);
  return true;
}
//---------------------------------------------------------
bool infoState::loadAlign(LPCTSTR row)
{
  int id = _ttoi(row);
  if(PRN_SET_ALIGN != id)
    return false;
  row = findNextParamTrim(row);
  Align = _ttoi(row);
  return true;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
class PDialogLinkedFile : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDialogLinkedFile(PWin* parent, uint id, HINSTANCE hInstance = 0) :
        baseClass(parent, id, hInstance)
    {
    }
    virtual ~PDialogLinkedFile() { destroy(); }

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    void chooseFile();
};
//-----------------------------------------------------------
bool openLinked(PWin* parent)
{
  return IDOK == PDialogLinkedFile(parent, IDD_DIALOG_LINKED).modal();
}
//-----------------------------------------------------------
bool PDialogLinkedFile::create()
{
  int idBmp[] = { IDB_OK, IDB_CANC };
  int idBtn[] = { IDOK, IDCANCEL };

  for(int i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* Bmp = new PBitmap(idBmp[i], getHInstance());
    new POwnBtnImageStd(this, idBtn[i], Bmp, POwnBtnImageStd::wLeft, true);
    }

  if(!baseClass::create())
    return false;
  dataProject& dp = getDataProject();

  SET_TEXT(IDC_EDIT_LINKED, dp.getlinkedFileName());

  return true;
}
//-----------------------------------------------------------
void PDialogLinkedFile::CmOk()
{
  dataProject& dp = getDataProject();
  TCHAR t[_MAX_PATH];
  GET_TEXT(IDC_EDIT_LINKED, t);
  if(!_tcsicmp(dp.getlinkedFileName(), t))
    baseClass::CmCancel();
  else {
    dp.setlinkedFileName(t);
    baseClass::CmOk();
    }
}
//-----------------------------------------------------------
LRESULT PDialogLinkedFile::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CHOOSE_LINKED:
          chooseFile();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void PDialogLinkedFile::chooseFile()
{
  TCHAR currPath[_MAX_PATH];
  GetCurrentDirectory(_MAX_PATH, currPath);

  TCHAR t[_MAX_PATH];
  dataProject& dp = getDataProject();
  _tcscpy_s(t, dp.getcurrPath());
  SetCurrentDirectory(t);

  TCHAR t2[_MAX_PATH];
  GET_TEXT(IDC_EDIT_LINKED, t2);
  if(*t2)
    appendPath(t, t2);
  else
    t[0] = 0;
  if(openFileText(*this, t, false, true)) {
    _tcscpy_s(t2, dp.getcurrPath());
    if(isSamePath(t2, t)) {
      SET_TEXT(IDC_EDIT_LINKED, t2);
      }
    else
      MessageBox(*this, _T("Il file deve essere nel percorso base"), _T("Attenzione"), MB_OK | MB_ICONSTOP);
    }
  SetCurrentDirectory(currPath);
}

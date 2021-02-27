//--------------- svmPropertyTrend.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyTrend.h"
#include "macro_utils.h"
#include "PListBox.h"
#include "POwnBtn.h"
#include "PStatic.h"
#include "svmPropertyTrendColor.h"
//-----------------------------------------------------------
oTrendColors::oTrendColors() : gridType(3)
{
  baseColor[cLineShow]  = RGB(0xff, 0xff, 0xff);
  baseColor[cGrid]  = RGB(0x3f, 0x50, 0x9f);
//  baseColor[cText]   = RGB(0x00, 0x00, 0x80);
  baseColor[cBkg]    = RGB(0x9f, 0x9f, 0x9f);
}
//-----------------------------------------------------------
void oTrendColors::clone(const oTrendColors& other)
{
  if(this == &other)
    return;
  for(uint i = 0; i < SIZE_A(baseColor); ++i)
    CLONE(baseColor[i]);
  CLONE(gridType);
//  CLONE(bkgType);
  CLONE(linesColor);
}
//-----------------------------------------------------------
PropertyTrend::~PropertyTrend()
{
  delete []trendName;
  delete []trendMinMaxFile;
}
//-----------------------------------------------------------
class paint_SampleTrend
{
  public:
    paint_SampleTrend(const oTrendColors& refColors) : RefColors(refColors) {}
    // usata per disegnare l'oggetto
    void paint(HDC hdc, const PRect& rect, uint nRow, uint nCol, bool inside = false);
  private:
    void drawLines(HDC hdc, const PRect& rect);
    void drawAxis(HDC hdc, const PRect& rect, uint nRow, uint nCol, bool inside);
    const oTrendColors& RefColors;
};
//-----------------------------------------------------------
class sampleTrend : public PStatic
{
  private:
    typedef PStatic baseClass;
  public:
    sampleTrend(PWin* parent, uint id) :
      baseClass(parent, id) {}

    void setColors(oTrendColors& refColors) { RefColors = refColors; }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void evPaint(HDC hdc);
  private:
    oTrendColors RefColors;
};
//-----------------------------------------------------------
LRESULT sampleTrend::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_PAINT:
      do {
        PAINTSTRUCT Paint;
        HDC hdc = BeginPaint(hwnd, &Paint);

        evPaint(hdc);

        EndPaint(hwnd, &Paint);
        } while(false);
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
extern void arrowUp(HDC hdc, int x, int y);
extern void arrowRight(HDC hdc, int x, int y);
//-----------------------------------------------------------
void sampleTrend::evPaint(HDC hdc)
{
  PRect r;
  GetClientRect(*this, r);
  paint_SampleTrend pst(RefColors);
  pst.paint(hdc, r, 4, 4);
}
//----------------------------------------------------------------------------
static void makeInside(PRect& r)
{
  r.left += r.Width() / 8;
  r.bottom -= r.Height() / 10;
}
//----------------------------------------------------------------------------
void paint_SampleTrend::paint(HDC hdc, const PRect& rect, uint nRow, uint nCol, bool inside)
{
  drawAxis (hdc, rect, nRow, nCol, inside);
  if(inside)  {
    PRect r(rect);
    makeInside(r);
    drawLines(hdc, r);
    }
  else
    drawLines(hdc, rect);
}
//----------------------------------------------------------------------------
#define ADD_RECT 5
//----------------------------------------------------------------------------
void paint_SampleTrend::drawAxis(HDC hdc, const PRect& rect, uint nRow, uint nCol, bool inside)
{
  COLORREF oldColor;

  HBRUSH br = CreateSolidBrush(RefColors.baseColor[oTrendColors::cBkg]);
  FillRect(hdc, rect, br);
  DeleteObject(HGDIOBJ(br));
  oldColor = SetBkColor(hdc, RefColors.baseColor[oTrendColors::cBkg]);

  uint penType = PS_DOT;
  bool full = true;
  switch(RefColors.gridType) {
    case 0:
      full = false;
    case 1:
      penType = PS_SOLID;
      break;
    case 2:
      penType = PS_DASH;
      break;
    case 3:
    default:
      break;
    }
  PRect r(rect);
  if(inside)
    makeInside(r);

  int left = r.left - ADD_RECT;
  int right = r.right;
  int top = r.bottom + ADD_RECT;
  int bottom = r.top;
  if(!full) {
    right = r.left;
    bottom = r.bottom;
    }
  HPEN Pen2 = CreatePen(penType, 1, RefColors.baseColor[oTrendColors::cGrid]);
  HGDIOBJ oldPen = SelectObject(hdc, Pen2);

  double stepx = r.Width() / (double)nCol;
  double j = stepx;
  for(uint i = 1; i < nCol; ++i, j += stepx) {
    long x = (long)j + r.left;
    MoveToEx(hdc, x, top, 0);
    LineTo(hdc, x, bottom);
    }

  double stepy = r.Height() / (double)nRow;
  j = stepy;
  for(uint i = 1; i < nRow; ++i, j += stepy) {
    long y = (LONG)j + r.top;
    MoveToEx(hdc, left, y, 0);
    LineTo(hdc, right, y);
    }

  HPEN Pen = CreatePen(PS_SOLID, 2, RefColors.baseColor[oTrendColors::cGrid]);
  DeleteObject(SelectObject(hdc, Pen));

  int y =  r.bottom;
  MoveToEx(hdc, rect.left, y, 0);
  LineTo(hdc, rect.right, y);

  MoveToEx(hdc, r.left, r.top, 0);
  LineTo(hdc, r.left, rect.bottom);

  arrowUp(hdc, r.left, r.top);
  arrowRight(hdc, r.right, y);

  SelectObject(hdc, oldPen);
  DeleteObject(Pen);
  SetBkColor(hdc, oldColor);
}
//-----------------------------------------------------------
void paint_SampleTrend::drawLines(HDC hdc, const PRect& r)
{
  uint nElem = RefColors.linesColor.getElem();
  if(!nElem)
    return;

  int width = r.Width();
  srand(width);
  uint step = 10;
  uint nl = width / step;
  if(nl < 10) {
    nl = 10;
    step = 1;
    }
  for(uint i = 0; i < nElem; ++i) {
    HPEN Pen = CreatePen(PS_SOLID, RefColors.linesColor[i].tickness, RefColors.linesColor[i].Color);
    HGDIOBJ oldPen = SelectObject(hdc, Pen);

    int y = (rand() % r.Height());
    int mod = r.Height() / 10;
    if(!mod)
      mod = 2;
    MoveToEx(hdc, r.left, y + r.top, 0);
    for(int i = 4; i < width; i += step) {
      int ty = (rand() % mod) - mod / 2;
      y += ty;
      if(y >= r.Height() - 5)
        y = r.Height() - 6;
      else if(y < 2)
        y = 2;
      LineTo(hdc, i + r.left, y + r.top);
      }
    SelectObject(hdc, oldPen);
    DeleteObject(Pen);
    }
  HPEN Pen = CreatePen(PS_SOLID, 1, RefColors.baseColor[oTrendColors::cLineShow]);
  HGDIOBJ oldPen = SelectObject(hdc, Pen);
  int x = r.left + width / 3;
  MoveToEx(hdc, x, r.top, 0);
  LineTo(hdc, x, r.bottom);
  x = r.left + (width / 3) * 2;
  MoveToEx(hdc, x, r.top, 0);
  LineTo(hdc, x, r.bottom);
  SelectObject(hdc, oldPen);
  DeleteObject(Pen);
}
//-----------------------------------------------------------
void paintSampleTrend(HDC hdc, const PRect& rect, const oTrendColors& info, uint nRow, uint nCol, bool inside)
{
  paint_SampleTrend pst(info);
  pst.paint(hdc, rect, nRow, nCol, inside);
}
//-----------------------------------------------------------
void PropertyTrend::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyTrend* po = dynamic_cast<const PropertyTrend*>(&other);
  if(po && po != this) {

    oTrendColor = po->getColors();

    for(uint i = 0; i < SIZE_A(DataPrf); ++i)
      DataPrf[i] = po->DataPrf[i];

    delete []trendName;
    if(po->trendName)
      trendName = str_newdup(po->trendName);
    else
      trendName = 0;

    delete []trendMinMaxFile;
    if(po->trendMinMaxFile)
      trendMinMaxFile = str_newdup(po->trendMinMaxFile);
    else
      trendMinMaxFile = 0;

    autoStepTen = po->autoStepTen;
    autoRows = po->autoRows;
    }
}
//-----------------------------------------------------------
void PropertyTrend::cloneMinusProperty(const Property& other)
{
  clone(other);
}
//-----------------------------------------------------------
struct ids
{
  uint idPrph;
  uint idAddr;
};
//-----------------------------------------------------------
static
ids Ids[] = {
  { IDC_COMBOBOX_PERIFS2, IDC_EDIT_ADDR2 },
  { IDC_COMBOBOX_PERIFS3, IDC_EDIT_ADDR3 },
  { IDC_COMBOBOX_PERIFS4, IDC_EDIT_ADDR4 },
  { IDC_COMBOBOX_PERIFS5, IDC_EDIT_ADDR5 },
  };
//-----------------------------------------------------------
static void invalidateSample(PWin* win)
{
  PWin* parent = win->getParent();
  PRect r;
  GetWindowRect(*win, r);
  r.Inflate(5, 5);
  MapWindowPoints(HWND_DESKTOP, *parent, (LPPOINT)(LPRECT)r, 2);
  InvalidateRect(*parent, r, 1);
}
//-----------------------------------------------------------
bool svmDialogTrend::create()
{
  sxs = new sampleTrend(this, IDC_SAMPLE_TREND);
  bkgText = CreateSolidBrush(Prop->background);

  if(!baseClass::create())
    return false;

  firstCheckTrend();

  PropertyTrend* pt = dynamic_cast<PropertyTrend*>(tmpProp);
  tmpProp->background = pt->getColors().baseColor[0];

  sxs->setColors(pt->getColors());
  invalidateSample(sxs);

  for(uint i = 0; i < SIZE_A(Ids); ++i)
    fillDataShort(Ids[i], pt->DataPrf[i]);

  SET_INT(IDC_EDIT_NUM_COL_TREND, Prop->type1);
  SET_INT(IDC_EDIT_NUM_ROW_TREND, Prop->type2);

  fillCbType(pt->getColors().gridType);

  SET_TEXT(IDC_STATIC_TREND_NAME, pt->trendName);
  if(pt->trendMinMaxFile && *pt->trendMinMaxFile) {
    SET_TEXT(IDC_EDIT_USE_FILE_MIN_MAX, pt->trendMinMaxFile);
    SET_CHECK(IDC_CHECK_USE_FILE_MIN_MAX);
    }
  if(pt->autoStepTen)
    SET_CHECK(IDC_CHECK_AUTO_STEP);
  if(pt->autoRows)
    SET_CHECK(IDC_CHECK_AUTO_ROWS);
  checkAutoStep();
  checkUseMinMax();
  return true;
}
//-----------------------------------------------------------
void svmDialogTrend::checkUseMinMax()
{
  bool useFile = IS_CHECKED(IDC_CHECK_USE_FILE_MIN_MAX);
  uint idcs1[] = { IDC_COMBOBOX_PERIFS, IDC_EDIT_ADDR, IDC_EDIT_NORMALIZ, IDC_BUTTON_NORMALIZ };
  uint idcs2[] = { IDC_EDIT_USE_FILE_MIN_MAX };
  for(uint i = 0; i < SIZE_A(idcs1); ++i)
    ENABLE(idcs1[i], !useFile);
  for(uint i = 0; i < SIZE_A(idcs2); ++i)
    ENABLE(idcs2[i], useFile);
}
//-----------------------------------------------------------
void svmDialogTrend::checkAutoStep()
{
  bool enabled = IS_CHECKED(IDC_CHECK_AUTO_STEP);
  if(enabled) {
    ENABLE(IDC_CHECK_AUTO_ROWS, true);
    bool enableEdit = !IS_CHECKED(IDC_CHECK_AUTO_ROWS);
    ENABLE(IDC_EDIT_NUM_ROW_TREND, enableEdit);
    }
  else {
    ENABLE(IDC_EDIT_NUM_ROW_TREND, true);
    ENABLE(IDC_CHECK_AUTO_ROWS, false);
    }
}
//-----------------------------------------------------------
void svmDialogTrend::fillCbType(int sel)
{
  HWND hwnd = ::GetDlgItem(*this, IDC_COMBOBOX_GRID_TYPE);
  static LPCTSTR info[] = {
    _T("solo tacche esterne"),
    _T("continua"),
    _T("lineette"),
    _T("punti")
    };
  for(uint i = 0; i < SIZE_A(info); ++i)
    addStringToComboBox(hwnd, info[i]);
  SendMessage(hwnd, CB_SETCURSEL, sel, 0);
}
//-----------------------------------------------------------
void svmDialogTrend::fillData(const ids& Ids, const dataPrf& DataPrf)
{
  HWND hwnd = ::GetDlgItem(*this, Ids.idPrph);
  fillCBPerif(hwnd, DataPrf.perif);

  SET_INT(Ids.idAddr, DataPrf.addr);

//  hwnd = ::GetDlgItem(*this, Ids.idType);
//  fillCBTypeVal(hwnd, DataPrf.typeVal);

//  SET_INT(Ids.idNorm, DataPrf.normaliz);
}
//-----------------------------------------------------------
void svmDialogTrend::fillDataShort(const ids& Ids, const dataPrf& DataPrf)
{
  HWND hwnd = ::GetDlgItem(*this, Ids.idPrph);
  fillCBPerif(hwnd, DataPrf.perif);

  SET_INT(Ids.idAddr, DataPrf.addr);
}
//-----------------------------------------------------------
void getAllTrendName(PVect<infoTrendName*>& trendName, setOfString& sosStdMsg)
{
  LPCTSTR p = sosStdMsg.getString(INIT_TREND_DATA);
  while(p) {
    uint code = _ttol(p);
    if(!code)
      break;
    LPCTSTR p2 = sosStdMsg.getString(code);
    LPCTSTR p3 = findNextParamTrim(p2, 6);

    if(!p3)
      continue;

    p2 = findNextParam(p2);
    uint dataCod = _ttoi(p2);
    p2 = findNextParam(p2);
    bool history = _ttoi(p2) >= 100;
    p2 = findNextParam(p2);

    pvvChar target;
    splitParam(target, p2);
    TCHAR match[_MAX_PATH] =  { 0 };
    LPCTSTR p4 = sosStdMsg.getString(ID_CREATE_FOLDER + 9);
    if(p4) {
      _tcscpy_s(match, p4);
      appendPath(match, &target[0]);
      }
    else
      _tcscpy_s(match, &target[0]);
    _tcscat_s(match, _T("*"));
    _tcscat_s(match, &target[1]);

    p2 = sosStdMsg.getString(dataCod);
    if(!p2)
      continue;
    infoTrendName* itn = new infoTrendName;
    itn->name = str_newdup(p3);
    itn->match = str_newdup(match);
    itn->nField = _ttoi(p2);
    itn->history = history;
    trendName[trendName.getElem()] = itn;
    p = findNextParamTrim(p);
    }
}
//-----------------------------------------------------------
class svmChooseTrendName : public PModDialog
{  private:
    typedef PModDialog baseClass;
  public:
    svmChooseTrendName(const PVect<infoTrendName*>& nameSet, uint& choose, PWin* parent,
        uint id = IDD_CHOOSE_TREND_NAME, HINSTANCE hInst = 0) : baseClass(parent, id, hInst),
          NameSet(nameSet), Choose(choose) {}
    ~svmChooseTrendName() { destroy(); }

    virtual bool create();
  protected:
    const PVect<infoTrendName*>& NameSet;
    uint& Choose;
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
};
//-----------------------------------------------------------
bool svmChooseTrendName::create()
{
  if(!baseClass::create())
    return false;
  uint nElem = NameSet.getElem();
  HWND hwlb = GetDlgItem(*this, IDC_LISTBOX_TREND);
  for(uint i = 0; i < nElem; ++i)
    SendMessage(hwlb, LB_ADDSTRING, 0, (LPARAM)NameSet[i]->name);
  SendMessage(hwlb, LB_SETCURSEL, Choose, 0);
  return true;
}
//-----------------------------------------------------------
LRESULT svmChooseTrendName::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_LISTBOX_TREND:
          switch(HIWORD(wParam)) {
            case LBN_DBLCLK:
              CmOk();
              break;
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmChooseTrendName::CmOk()
{
  int sel = SendMessage(GetDlgItem(*this, IDC_LISTBOX_TREND), LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  Choose = sel;
  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmDialogTrend::chooseTrend()
{
  PVect<infoTrendName*> nameSet;
  do {
    setOfString set;
    makeStdMsgSet(set);
    getAllTrendName(nameSet, set);
    } while(false);
  uint nElem = nameSet.getElem();
  if(!nElem)
    return;
  uint ix = 0;
  if(1 < nElem) {
    if(IDOK != svmChooseTrendName(nameSet, ix, this).modal())
      return;
    }
  PropertyTrend* pt = dynamic_cast<PropertyTrend*>(tmpProp);
  if(pt->trendName && _tcsicmp(pt->trendName, nameSet[ix]->name)) {
    delete []pt->trendName;
    pt->trendName = nameSet[ix]->name;
    nameSet[ix]->name = 0;
    SET_TEXT(IDC_STATIC_TREND_NAME, pt->trendName);
    oTrendColors& cl = pt->getColors();
    cl.linesColor.setDim(nameSet[ix]->nField);
    cl.linesColor.setElem(nameSet[ix]->nField);
    drawColors();
    }
  flushPV(nameSet);
}
//-----------------------------------------------------------
void svmDialogTrend::firstCheckTrend()
{
  PVect<infoTrendName*> nameSet;
  do {
    setOfString set;
    makeStdMsgSet(set);
    getAllTrendName(nameSet, set);
    } while(false);
  uint nElem = nameSet.getElem();
  if(!nElem)
    return;
  uint ix = 0;
  PropertyTrend* pt = dynamic_cast<PropertyTrend*>(tmpProp);
  if(pt->trendName) {
    ix = nElem;
    for(uint i = 0; i < nElem; ++i) {
      if(!_tcsicmp(pt->trendName, nameSet[i]->name)) {
        ix = i;
        break;
        }
      }
    if(ix == nElem) {
      delete []pt->trendName;
      ix = 0;
      pt->trendName = nameSet[ix]->name;
      nameSet[ix]->name = 0;
      }
    }
  else {
    pt->trendName = nameSet[ix]->name;
    nameSet[ix]->name = 0;
    }
  oTrendColors& cl = pt->getColors();
  cl.linesColor.setDim(nameSet[ix]->nField);
  cl.linesColor.setElem(nameSet[ix]->nField);
  flushPV(nameSet);
}
//-----------------------------------------------------------
LRESULT svmDialogTrend::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_TREND_BKG:
        case IDC_BUTTON_TREND_GRID:
        case IDC_BUTTON_TREND_LINE_BAR:
          if(chooseColor(LOWORD(wParam)))
            drawColors();
          break;
        case IDC_BUTTON_TREND_LINE_COLOR:
          if(chooseLineColor())
            drawColors();
          break;
        case IDC_BUTTON_TREND_CHOOSE:
          chooseTrend();
          break;
        case IDC_BUTTON_CHOOSE_TEXT_COLOR:
          chooseTextColor();
          break;
        case IDC_COMBOBOX_GRID_TYPE:
          if(CBN_SELCHANGE == HIWORD(wParam)) {
            PropertyTrend* pt = dynamic_cast<PropertyTrend*>(tmpProp);
            if(pt) {
              uint type = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_GRID_TYPE), CB_GETCURSEL, 0, 0);
              pt->getColors().gridType = type;
              drawColors();
              }
            }
          break;
        case IDC_CHECK_USE_FILE_MIN_MAX:
          checkUseMinMax();
          break;
        case IDC_CHECK_AUTO_STEP:
        case IDC_CHECK_AUTO_ROWS:
          checkAutoStep();
          break;
        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
HBRUSH svmDialogTrend::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(bkgText && GetDlgItem(*this, IDC_STATICTEXT_FONT) == hWndChild) {
    SetTextColor(hdc, tmpProp->foreground);
    SetBkColor(hdc, tmpProp->background);
    return bkgText;
    }
  return baseClass::evCtlColor(hdc, hWndChild, ctlType);
}
//-----------------------------------------------------------
bool svmDialogTrend::chooseLineColor()
{
  PropertyTrend* pt = dynamic_cast<PropertyTrend*>(tmpProp);
  oTrendColors& colors = pt->getColors();
  PVect<colorLine>& colorset =  colors.linesColor;
  return IDOK == svmPropertyTrendColor(this, colorset).modal();
}
//-----------------------------------------------------------
void svmDialogTrend::chooseTextColor()
{
  if(choose_Color(*this, tmpProp->foreground))
    InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_FONT), 0, 1);
}
//-----------------------------------------------------------
bool svmDialogTrend::chooseColor(uint idc)
{
  PropertyTrend* pt = dynamic_cast<PropertyTrend*>(tmpProp);
  oTrendColors& colors = pt->getColors();
  uint ix = 0;
  switch(idc) {
    case IDC_BUTTON_TREND_BKG:
      break;
    case IDC_BUTTON_TREND_GRID:
      ix = 1;
      break;
    case IDC_BUTTON_TREND_LINE_BAR:
      ix = 2;
      break;
    default:
      return false;
    }
  bool changed = choose_Color(*this, colors.baseColor[ix]);
  if(changed && !ix) {
    DeleteObject(bkgText);
    tmpProp->background = colors.baseColor[0];
    bkgText = CreateSolidBrush(tmpProp->background);
    InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_FONT), 0, 1);
    }
  return changed;
}
//-----------------------------------------------------------
void svmDialogTrend::drawColors()
{
  PropertyTrend* pt = dynamic_cast<PropertyTrend*>(tmpProp);
  sxs->setColors(pt->getColors());
  invalidateSample(sxs);
}
//-----------------------------------------------------------
void svmDialogTrend::unfillData(const ids& Ids, dataPrf& DataPrf)
{
  HWND hwnd = ::GetDlgItem(*this, Ids.idPrph);
  DataPrf.perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

  GET_INT(Ids.idAddr, DataPrf.addr);

  DataPrf.typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

//  GET_INT(Ids.idNorm, DataPrf.normaliz);
}
//-----------------------------------------------------------
void svmDialogTrend::unfillDataShort(const ids& Ids, dataPrf& DataPrf)
{
  HWND hwnd = ::GetDlgItem(*this, Ids.idPrph);
  DataPrf.perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);

  GET_INT(Ids.idAddr, DataPrf.addr);
  DataPrf.typeVal = 4;
}
//-----------------------------------------------------------
void svmDialogTrend::CmOk()
{
  PropertyTrend* pt = dynamic_cast<PropertyTrend*>(tmpProp);
  if(pt) {
    for(uint i = 0; i < SIZE_A(Ids); ++i)
      unfillDataShort(Ids[i], pt->DataPrf[i]);
    }
  GET_INT(IDC_EDIT_NUM_COL_TREND, tmpProp->type1);
  GET_INT(IDC_EDIT_NUM_ROW_TREND, tmpProp->type2);

  uint type = SendMessage(GetDlgItem(*this, IDC_COMBOBOX_GRID_TYPE), CB_GETCURSEL, 0, 0);
  pt->getColors().gridType = type;

  delete []pt->trendMinMaxFile;
  pt->trendMinMaxFile = 0;
  if(IS_CHECKED(IDC_CHECK_USE_FILE_MIN_MAX)) {
    TCHAR t[_MAX_PATH];
    GET_TEXT(IDC_EDIT_USE_FILE_MIN_MAX, t);
    pt->trendMinMaxFile = str_newdup(t);
    }
  pt->autoStepTen = IS_CHECKED(IDC_CHECK_AUTO_STEP);
  pt->autoRows = IS_CHECKED(IDC_CHECK_AUTO_ROWS);
  baseClass::CmOk();
}
//-----------------------------------------------------------

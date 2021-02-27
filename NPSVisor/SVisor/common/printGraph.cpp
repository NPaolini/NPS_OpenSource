//--------------- printGraph.cpp ---------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include "printGraph.h"
#include "p_file.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_date.h"
//----------------------------------------------------------------------------
#define NAME_PRINT_GRAPH _T("~printGraph.tmp")
//----------------------------------------------------------------------------
#define NAME_INTERNAL_PRINT_GRAPH _T("printGraphStyle.cfg")
//----------------------------------------------------------------------------
printGraph::printGraph(PWin* par, svPrinter* printer, const infoPrintGraph& ipg) :
    baseClass(par, printer, NAME_PRINT_GRAPH), IPG(ipg)
{
}
//----------------------------------------------------------------------------
#define HEIGHT_FONT_HEADER 20
#define HEIGHT_FONT_STD    14
#define HEIGHT_FONT_ROWS   heightFont
//----------------------------------------------------------------------------
#define OFFS_Y_ROWS heightRow
//----------------------------------------------------------------------------
#define FORM_CENTER ((pageForm.right + pageForm.left) / 2)
//----------------------------------------------------------------------------
#define DIM_LOCAL_BUFF 512
//----------------------------------------------------------------------------
void makeHorzLines(P_File* pf, LPTSTR buff, int x1, int x2, int y, int nLine, int offs)
{
  int result;
  for(int i = 0; i < nLine; ++i) {
    PRINT5_S(pf, buff, DIM_LOCAL_BUFF, _T("%d,%d,%d,%d,%d\r\n"), PRN_LINE, x1, y, x2, y);
    y += offs;
    }
}
//----------------------------------------------------------------------------
#define TWO_LINE(y) \
    makeHorzLines(File, buff, pageForm.left, pageForm.right, y, 2, 5)
//----------------------------------------------------------------------------
void readColor(COLORREF& color, setOfString& set, uint id)
{
  LPCTSTR p = set.getString(id);
  if(p) {
    int r;
    int g;
    int b;
    _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
    color = RGB(r, g, b);
    }
}
//----------------------------------------------------------------------------
LPCTSTR readMargin(LPCTSTR p, int& m)
{
  if(p) {
    int v = _ttoi(p);
    if(v >= 0)
      m = v;
    p = findNextParam(p, 1);
    }
  return p;
}
//----------------------------------------------------------------------------
void readStyle(uint& style, setOfString& set, uint id)
{
  LPCTSTR p = set.getString(id);
  if(p) {
    uint t = _ttoi(p);
    switch(t) {
      case 0:
        style = PS_SOLID;
        break;
      case 1:
        style = PS_DASH;
        break;
      case 2:
        style = PS_DOT;
        break;
      case 3:
        style = PS_DASHDOT;
        break;
      case 4:
        style = PS_DASHDOTDOT;
        break;
      }
    }
}
//----------------------------------------------------------------------------
void readVal(uint& val, setOfString& set, uint id)
{
  LPCTSTR p = set.getString(id);
  if(p)
    val = _ttoi(p);
}
//----------------------------------------------------------------------------
void readFlags(uint& val, setOfString& set, uint id)
{
  LPCTSTR p = set.getString(id);
  if(p) {
    int alwaysYAxes;
    int drawHorzLines;
    int drawVertLines;
    int drawLabels;
    _stscanf_s(p, _T("%d,%d,%d,%d"), &alwaysYAxes,
          &drawHorzLines, &drawVertLines, &drawLabels);

    union {
      uint flags;
      struct {
        uint alwaysYAxes    : 1;
        uint drawHorzLines  : 1;
        uint drawVertLines  : 1;
        uint drawLabels     : 1;
        } f;
      } U;
    U.f.alwaysYAxes = 1;
//    U.f.alwaysYAxes = alwaysYAxes;
    U.f.drawHorzLines = drawHorzLines;
    U.f.drawVertLines = drawVertLines;
    U.f.drawLabels = drawLabels;
    val = U.flags;
    }
}
//----------------------------------------------------------------------------
#define ID_MARGINS 2
#define ID_COLOR_AXES 3
#define ID_TICK_AXES  4
#define ID_COLOR_HORZ 5
#define ID_STYLE_HORZ 6
#define ID_COLOR_VERT 7
#define ID_STYLE_VERT 8
#define ID_ADD_TOP_MARGIN 9
#define ID_FLAGS 10
//----------------------------------------------------------------------------
#define DEF_VERT_SPACE_X_LABELS 80
//----------------------------------------------------------------------------
bool printGraph::beginDoc(bool showSetup)
{
  if(!baseClass::beginDoc(showSetup))
    return false;
  setOfString set(NAME_INTERNAL_PRINT_GRAPH);

  LPCTSTR p = set.getString(ID_MARGINS);
  if(p) {
    PRect r = getMargin();
    p = readMargin(p, r.left);
    p = readMargin(p, r.top);
    p = readMargin(p, r.right);
    p = readMargin(p, r.bottom);
    setMargin(r);
    }
  readColor(IIP.cAxes, set, ID_COLOR_AXES);
  readColor(IIP.cHorzLine, set, ID_COLOR_HORZ);
  readColor(IIP.cVertLine, set, ID_COLOR_VERT);

  readStyle(IIP.horzStyle, set, ID_STYLE_HORZ);
  readStyle(IIP.vertStyle, set, ID_STYLE_VERT);

  readVal(IIP.topGraph, set, ID_ADD_TOP_MARGIN);
  readVal(IIP.tickness, set, ID_TICK_AXES);
  readFlags(IIP.U.flags, set, ID_FLAGS);
  if(IPG.U.f.graphOver && IPG.numGraph > 1)
    IIP.U.f.drawLabels = 0;

  calcOffset();
  return true;
}
//----------------------------------------------------------------------------
inline int getSpace4Labels()
{
  return 200;
}
//----------------------------------------------------------------------------
void printGraph::calcOffset()
{
  for(int i = 0; i < SIZE_A(lenPage); ++i)
    lenPage[i] = 0;

  ratioX = (float)pageForm.Width();

#if 0
  // se si differenzia la prima pagina (disegno asse sulla Y) rispetto alle altre
  // occorre implementare diversamente il codice, incluso ratioX che non
  // avrebbe più senso (ce ne vorrebbe uno per pagina).
#else
  if(IIP.U.f.drawLabels && IIP.U.f.alwaysYAxes || IPG.U.f.forceLabel)
    ratioX -= getSpace4Labels();

  ratioX /= IPG.pixWidth;

  const lgCoord* ax = IPG.XCoord;

  if(IPG.numPage <= 1) {
    lenPage[0] = IPG.nElem;
    return;
    }

  ratioX *= IPG.numPage;

  float range;
  if(IPG.trendType < 0)
    range = (float)(IPG.to - IPG.from);
  else
    range = *(float*)&IPG.to - *(float*)&IPG.from;

  float ratio = range;
  ratio /= IPG.pixWidth;

  range /= IPG.numPage;

  int nElem = IPG.nElem;
  int curr = 0;
  for(int i = 0; i < SIZE_A(lenPage) && i < IPG.numPage; ++i) {
    float val = range * (i + 1);
    int j;
    for(j = curr; j < nElem; ++j) {
      float t = (float)ax[j];
      t *= ratio;
      if(t > val) {
        lenPage[i] = j - curr;
        curr = j;
        break;
        }
      }
    if(j == nElem) {
      lenPage[i] = j - curr;
      curr = j;
      }
    }
#endif
}
//----------------------------------------------------------------------------
bool printGraph::makeHeader()
{
  if(!baseClass::makeHeader())
    return false;

  TCHAR buff[DIM_LOCAL_BUFF];
  int result;
  if(IPG.U.f.forceLabel) {
    LPCTSTR p = IPG.FieldSet[IPG.U.f.activeLabel].info;
    if(p) {
      PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, TA_LEFT, TA_TOP);
      PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, pageForm.left + 10, Y + 40, p);
      }
    }

  PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, TA_CENTER, TA_TOP);

  PRINT5_A_S(File, buff, _T("%d,%d,%d,%d,%s\r\n"), PRN_SET_FONT, HEIGHT_FONT_HEADER, 0, 0, _T("arial"));

  PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, FORM_CENTER, Y, IPG.title);

  if(IPG.numPage > 1) {
    PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, TA_RIGHT, TA_TOP);
    PRINT5_A_S(File, buff, _T("%d,%d,%d,%d,%s\r\n"), PRN_SET_FONT, HEIGHT_FONT_STD, 0, 0, _T("arial"));
    PRINT5_A_S(File, buff, _T("%d,%d,%d,Pag. %d di %d\r\n"),
          PRN_TXT, pageForm.right, Y - 50, currPage + 1, IPG.numPage);
    }

  if(IIP.topGraph)
    Y += IIP.topGraph;
  else
    Y += 60;
  TWO_LINE(Y);
  Y += 20;

  int x1 = pageForm.left;

  if(IIP.U.f.drawLabels && IIP.U.f.alwaysYAxes || IPG.U.f.forceLabel)
    x1 += getSpace4Labels();

  int x2 = pageForm.right;
  int y1 = getInit_Y_Axes();

  int y2 = pageForm.bottom - DEF_VERT_SPACE_X_LABELS;
  if(!IPG.U.f.graphOver) {
    int offsY = y2 - getInit_Y_Axes();
    offsY /= IPG.numGraph;
    y2 = y1 + offsY;
    }
  currDrawing = PRect(x1, y1, x2, y2);

  return true;
}
//----------------------------------------------------------------------------
int printGraph::getInit_Y_Axes()
{
  int y = pageForm.top;
  if(IIP.topGraph)
    y += IIP.topGraph;
  else
    y += 60;
  y += 20 + 10;
  return y;
}
//----------------------------------------------------------------------------
int printGraph::addRow()
{
  if(!drawBottomLabels())
    return 0;
  if(IPG.U.f.graphOver) {
    if(!drawAxes(true, IPG.U.f.activeLabel))
      return 0;
    for(int i = 0; i < IPG.numGraph; ++i)
      if(!drawGraph(i))
        return 0;
    }
  else {
    for(int i = 0; i < IPG.numGraph; ++i) {
      if(!drawAxes(false, i))
        return 0;
      if(!drawGraph(i))
        return 0;
      currDrawing.Offset(0, currDrawing.Height());
      }
    }

  Y = MAX_Y;
  return (IPG.numPage > currPage + 1) ? 1 : -1;
}
//----------------------------------------------------------------------------
bool printGraph::drawGraph(int ix)
{
  TCHAR buff[DIM_LOCAL_BUFF];
  int result;
  int x = currDrawing.left;
  int y = currDrawing.top;
//  int y = currDrawing.bottom;

  const infoPrintGraph::field& fld = IPG.FieldSet[ix];
  if(!IPG.U.f.graphOver || 1 == IPG.FieldSet.getElem()) {
    PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, TA_LEFT, TA_TOP);
    PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, x + 20, y + 20, fld.info);
    }
  float ratioY = (float)currDrawing.Height();
#if 1
  ratioY /= MAX_H_DATA;
#else
  ratioY /= fld.vMax - fld.vMin;
#endif
  const lgCoord* set = IPG.yGraph[ix];

  int offs = 0;
  for(int i = 0; i < currPage; ++i)
    offs += lenPage[i];

  PRINT4_A_S(File, buff, _T("%d,%d,%d,%d\r\n"), PRN_SET_PEN, fld.color, fld.tickness, PS_SOLID);
  if(!result)
    return false;

#if 1
  int vY = (int)(y + set[offs] * ratioY + 0.5);

  const lgCoord* ax = IPG.XCoord;

  x -= currDrawing.Width() * currPage;
  int vX = (int)(x + ax[offs] * ratioX + 0.5);

  int tOffs = offs + 1;
  int totData = 1;
  for(uint i = 1; i < lenPage[currPage]; ++i, ++tOffs) {
    int tY = (int)(y + set[tOffs] * ratioY + 0.5);
    int tX = (int)(x + ax[tOffs] * ratioX + 0.5);
    if(tY == vY && tX == vX)
      continue;
    vX = tX;
    vY = tY;
    ++totData;
    }

  vY = (int)(y + set[offs] * ratioY + 0.5);
  vX = (int)(x + ax[offs] * ratioX + 0.5);

  ++offs;

  PRINT4_A_S(File, buff, _T("%d,%d,%d,%d"), PRN_POLYLINE, totData - 1, vX, vY);
  if(!result)
    return false;

  for(int i = 1; i < totData; ++i, ++offs) {
    int tY = (int)(y + set[offs] * ratioY + 0.5);
    int tX = (int)(x + ax[offs] * ratioX + 0.5);
    if(tY == vY && tX == vX) {
      --i;
      continue;
      }
    if(tX < 0)
      tX = vX;
    vX = tX;
    vY = tY;
    PRINT2_A_S(File, buff, _T(",%d,%d"), vX, vY);
    if(!result)
      return false;
    }
#else
  int vY = set[offs] * ratioY + 0.5;
  ++offs;

  PRINT4_A_S(File, buff,"%d,%d,%d,%d", PRN_POLYLINE, lenPage[currPage] - 1, x, y + vY);
  if(!result)
    return false;

  const lgCoord* ax = IPG.XCoord;

  x -= currDrawing.Width() * currPage;

  for(int i = 1; i < lenPage[currPage]; ++i, ++offs) {
    vY = y + set[offs] * ratioY + 0.5;
    int vX = x + ax[offs] * ratioX + 0.5;
    PRINT2_A_S(File, buff, _T(",%d,%d"), vX, vY);
    if(!result)
      return false;
    }
#endif
  PRINT_CR_NL(File, buff);
  if(!result)
    return false;

  return true;
}
//----------------------------------------------------------------------------
bool printGraph::drawBottomLabels()
{
  TCHAR buff[DIM_LOCAL_BUFF];
  bool result;
  int y = pageForm.bottom - DEF_VERT_SPACE_X_LABELS + 5;
  int x1 = currDrawing.left;
  int x2 = currDrawing.right;

  float range;
  if(IPG.trendType < 0)
    range = (float)(IPG.to - IPG.from);
  else
    range = *(float*)&IPG.to - *(float*)&IPG.from;

  range /= IPG.numPage;
  __int64 v1X = (__int64)(range * currPage);
  __int64 v2X = (__int64)(v1X + range);

  double deltaPix = currDrawing.Width();
  deltaPix /= BASE_VERT_LINE;

  PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, TA_LEFT, TA_TOP);
  if(!result)
    return false;
  PRINT5_A_S(File, buff, _T("%d,%d,%d,%d,%s\r\n"), PRN_SET_FONT, HEIGHT_FONT_STD, 0, 0, _T("arial"));
  if(!result)
    return false;

  colCoord[0] = x1;
  if(IPG.trendType < 0) {
    vertLineInside = VERT_LINE_TIME;
    double rX = (double)(IPG.to - IPG.from);
    rX /= IPG.pixWidth;

    __int64 t1 = v1X;
    __int64 t2 = v2X;

    double deltaV = (double)(t2 - t1);
    deltaV /= BASE_VERT_LINE;

    t1 += IPG.from;

    TCHAR time[50];
    FILETIME ft = I64_TO_FT(t1);
    set_format_data(time, SIZE_A(time), ft, whichData(), _T("|"));
    time[10] = 0;

#define D_Y_TIME 40

    PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, x1, y, time);
    PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, x1, y + D_Y_TIME, time + 11);

    PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, TA_CENTER, TA_TOP);

    for(int i = 0; i < BASE_VERT_LINE - 1;++i) {
      ft = I64_TO_FT(t1 + (__int64)(deltaV * (i + 1)));
      set_format_data(time, SIZE_A(time), ft, whichData(), _T("|"));
      time[10] = 0;
      int x = (int)(x1 + deltaPix * (i + 1));
      PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, x, y, time);
      PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, x, y + D_Y_TIME, time + 11);
      colCoord[i + 1] = x;
      }
    PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, TA_RIGHT, TA_TOP);
    ft = I64_TO_FT(t2 + IPG.from);
    set_format_data(time, SIZE_A(time), ft, whichData(), _T("|"));
    time[10] = 0;
    PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, x2, y, time);
    PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, x2, y + D_Y_TIME, time + 11);
    }
  else {
    vertLineInside = VERT_LINE_OTHER;

    double rX = *(float*)&IPG.to - *(float*)&IPG.from;
    rX /= IPG.pixWidth;

    double t1 = (double)v1X;
    t1 *= rX;
    double t2 = (double)v2X;
    t2 *= rX;
    double deltaV = t2 - t1;
    deltaV /= BASE_VERT_LINE;

    t1 += *(float*)&IPG.to;
    t2 += *(float*)&IPG.to;
    TCHAR val[50];
    makeFixedString(t1, val, SIZE_A(val), IPG.nDec);

    PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, x1, y, val);

    PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, TA_CENTER, TA_TOP);

    for(int i = 0; i < BASE_VERT_LINE - 1;++i) {
      t1 += deltaV;
      makeFixedString(t1, val, SIZE_A(val), IPG.nDec);
      int x = (int)(x1 + deltaPix * (i + 1));
      PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, x, y, val);
      colCoord[i] = x;
      }
    PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, TA_RIGHT, TA_TOP);
    makeFixedString(t2, val, SIZE_A(val), IPG.nDec);
    PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, x2, y, val);
    }
  colCoord[BASE_VERT_LINE] = x2;

  return true;
}
//----------------------------------------------------------------------------
bool printGraph::drawHorzGrid_LeftLabels(int x1, int x2, int y1, int y2, int ix)
{
  TCHAR buff[DIM_LOCAL_BUFF];
  bool result;
  PRINT3_A_S(File, buff, _T("%d,%d,%d\r\n"), PRN_SET_ALIGN, TA_RIGHT, TA_BOTTOM);

  PRINT4_A_S(File, buff, _T("%d,%d,%d,%d\r\n"), PRN_SET_PEN, IIP.cHorzLine, 1, IIP.horzStyle);
  if(!result)
    return false;

  const infoPrintGraph::field& fld = IPG.FieldSet[ix];
  double delta = y2 - y1;
  delta /= IPG.numRow;
  int nRow = IPG.numRow;
  if(delta < IPG.dimMinRow) {
    delta = IPG.dimMinRow;
    nRow = (int)((y2 - y1) / delta);
    }

  double dV = fld.vMax - fld.vMin;
  dV /= nRow;

  bool needLabel = IIP.U.f.drawLabels && (IIP.U.f.alwaysYAxes || !currPage);

  for(int i = 0; i < nRow; ++i) {
    int y = (int)(y2 - i * delta);
    if(needLabel || (IPG.U.f.forceLabel && IPG.U.f.activeLabel == ix)) {
      TCHAR t[50];
      makeFixedString(fld.vMin + i * dV, t, SIZE_A(t), fld.nDec);
      PRINT4_A_S(File, buff, _T("%d,%d,%d,%s\r\n"), PRN_TXT, x1 - 10, y, t);
      if(!result)
        return false;
      }
    PRINT5_A_S(File, buff, _T("%d,%d,%d,%d,%d\r\n"), PRN_LINE, x1, y, x2, y);
    if(!result)
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
bool printGraph::drawVertGrid(bool fullArea)
{
  bool result;
  TCHAR buff[DIM_LOCAL_BUFF];

  PRINT4_A_S(File, buff, _T("%d,%d,%d,%d\r\n"), PRN_SET_PEN, IIP.cVertLine, 1, IIP.vertStyle);
  if(!result)
    return false;

  int y1 = currDrawing.top;
  if(!fullArea)
    y1 += 20;
  int y2 = currDrawing.bottom;

  for(int i = 0; i < BASE_VERT_LINE; ++i) {
    int x = colCoord[i];
    double step = colCoord[i + 1] - x;
    step /= vertLineInside;
    for(int j = 0; j < vertLineInside; ++j) {
      if(!i && !j)
        continue;
      int tx = (int)(x + step * j);
      if(!j) {
        PRINT4_A_S(File, buff, _T("%d,%d,%d,%d\r\n"), PRN_SET_PEN, 0, 1, PS_SOLID);
        if(!result)
          return false;
        }
      PRINT5_A_S(File, buff, _T("%d,%d,%d,%d,%d\r\n"), PRN_LINE, tx, y1, tx, y2);
      if(!result)
        return false;
      if(!j) {
        PRINT4_A_S(File, buff, _T("%d,%d,%d,%d\r\n"), PRN_SET_PEN, IIP.cVertLine, 1, IIP.vertStyle);
        if(!result)
          return false;
        }
      }
    }
  return true;
}
//----------------------------------------------------------------------------
#define H_ARROW 30
#define W_ARROW 10
//----------------------------------------------------------------------------
bool printGraph::drawAxes(bool fullArea, int ix)
{
  if(!drawVertGrid(fullArea))
    return false;

  int x1 = currDrawing.left;

  int x2 = currDrawing.right;
  int y1 = currDrawing.top;
  if(!fullArea)
    y1 += 20;

  int y2 = currDrawing.bottom;

  bool result = drawHorzGrid_LeftLabels(x1, x2, y1, y2, ix);
  if(!result)
    return false;

  TCHAR buff[DIM_LOCAL_BUFF];
  PRINT4_A_S(File, buff, _T("%d,%d,%d,%d\r\n"), PRN_SET_PEN, IIP.cAxes, IIP.tickness, PS_SOLID);
  if(!result)
    return false;

  PRINT5_A_S(File, buff, _T("%d,%d,%d,%d,%d\r\n"), PRN_LINE, x1, y2, x2 - H_ARROW, y2);
  if(!result)
    return false;

  PRINT5_A_S(File, buff, _T("%d,%d,%d,%d,%d\r\n"), PRN_LINE, x1, y1 + H_ARROW, x1, y2);
  if(!result)
    return false;

  PRINT4_A_S(File, buff, _T("%d,%d,%d,%d"), PRN_POLYLINE, 3, x1, y1);
  if(!result)
    return false;
  PRINT6_A_S(File, buff, _T(",%d,%d,%d,%d,%d,%d\r\n"),
      x1 - W_ARROW, y1 + H_ARROW,
      x1 + W_ARROW, y1 + H_ARROW,
      x1, y1
      );
  if(!result)
    return false;

  PRINT4_A_S(File, buff, _T("%d,%d,%d,%d"), PRN_POLYLINE, 3, x2, y2);
  if(!result)
    return false;

  PRINT6_A_S(File, buff, _T(",%d,%d,%d,%d,%d,%d\r\n"),
      x2 - H_ARROW, y2 - W_ARROW,
      x2 - H_ARROW, y2 + W_ARROW,
      x2, y2
      );
  if(!result)
    return false;

  return true;
}
//----------------------------------------------------------------------------
LPCTSTR printGraph::getTitle()
{
  return IPG.title;
}

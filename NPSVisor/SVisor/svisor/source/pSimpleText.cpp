//------ PSimpleText.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "p_basebody.h"
#include "p_util.h"
#include "PSimpleText.h"
//----------------------------------------------------------------------------
void loadBorder(LPCTSTR p, PPanelBorder& Border);
//----------------------------------------------------------------------------
PSimpleText::PSimpleText(P_BaseBody* owner, uint id) :  baseSimple(owner, id),
    Text(0) { }
//----------------------------------------------------------------------------
bool PSimpleText::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  uint idfont = 0;
  int Rfg = 0;
  int Gfg = 0;
  int Bfg = 0;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int no_up_dn = 0;
  int align = 0;
  int id_msg = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idfont,
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &no_up_dn, &align, &id_msg);

  p = findNextParamTrim(p, 14);
  PPanelBorder border;
  loadBorder(p, border);

  idfont -= ID_INIT_FONT;

  PPanel::bStyle type = no_up_dn == 0 ? PPanel::NO  :
                        no_up_dn == 1 ? PPanel::UP_FILL :
                        no_up_dn == 2 ? PPanel::DN_FILL :
                        no_up_dn == 3 ? PPanel:: FILL   :

                        no_up_dn == 4 ? PPanel::UP :
                        no_up_dn == 5 ? PPanel::DN :
                        no_up_dn == 6 ? PPanel::BORDER_FILL :
                                        PPanel::BORDER;

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  setRect(rect);

  HFONT font = getOwner()->getFont(idfont);

  smartPointerConstString sp = getOwner()->getPageStringLang(id_msg);

  Text = new PTextFixedPanel(sp, rect, font, RGB(Rfg, Gfg, Bfg), type, RGB(Rbk, Gbk, Bbk), PTextPanel::NO3D);
  Text->setAllBorder(border);

  enum { aCenter, aLeft, aRight, aTop = 4, aMid = 8, aBottom = 16 };

  UINT vAlign = align & ~3;
  align &= 3;

  vAlign = aBottom == (vAlign & aBottom) ? DT_BOTTOM :
         aMid == (vAlign & aMid) ? DT_VCENTER :
                                     DT_TOP;

  align = aLeft == (align & aLeft) ? TA_LEFT :
         aRight == (align & aRight) ? TA_RIGHT :
                                     TA_CENTER;

  Text->setAlign(align);
  Text->setVAlign(vAlign);

  uint id2 = getSecondExtendId();
  p = getOwner()->getPageString(id2);
  if(p) {
    DWORD tick = _ttoi(p);
    if(tick)
      allocBlink(tick);
    }
  allocVisibility(p);

  return true;
}
//----------------------------------------------------------------------------
bool PSimpleText::update(bool force)
{
  if(baseSimple::update(force)) {
    invalidate();
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------

//--------------- svmPropertyTableInfo.cpp ------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertyTableInfo.h"
#include "svmPropertyTblHeader.h"
#include "svmPropertyTblFirstCol.h"
#include "svmPropertyTblRows.h"
#include "macro_utils.h"
//-----------------------------------------------------------
PropertyTableInfo::PropertyTableInfo() : offsX(0), offsY(0), HeightHeader(20), //useBmp(false),
    nRow(NROW_FULL), styleShow(0), heightRows(20), useSwap(false), timeCycle(0)
{
  textHeader.setDim(NCOL_FULL);
  for(uint i = 0; i < NCOL_FULL; ++i) {
    Width[i] = 50 * (i + 1);
    textHeader[i] = 0;
    }

  First.setDim(NSTATE_FIRST);
  for(uint i = 0; i < NSTATE_FIRST; ++i)
    First[i] = 0;

  Second.setDim(NROW_FULL);
  Third.setDim(NROW_FULL);
  for(uint i = 0; i < NROW_FULL; ++i) {
    Second[i] = 0;
    Third[i] = 0;
    }
  type2 = RGB(0xff, 0xff, 0xff);
  type1 = UP_FILL;
}
//-----------------------------------------------------------
PropertyTableInfo::~PropertyTableInfo()
{
  flushPAV(First);
  flushPAV(Second);
  flushPAV(Third);
  flushPAV(textHeader);
}
//-----------------------------------------------------------
static void cloneText(PVect<LPCTSTR>& target, const PVect<LPCTSTR>& source)
{
  flushPAV(target);
  uint nElem = source.getElem();
  target.setDim(nElem);
  for(uint i = 0; i < nElem; ++i)
    target[i] = source[i] ? str_newdup(source[i]) : 0;
}
//-----------------------------------------------------------
#define CLONE_TXT(a) cloneText(a, po->a)
//-----------------------------------------------------------
#define CLONE_V(a) a = po->a
//-----------------------------------------------------------
void PropertyTableInfo::clone(const Property& other)
{
  baseClass::clone(other);
  const PropertyTableInfo* po = dynamic_cast<const PropertyTableInfo*>(&other);
  if(po && po != this) {

    CLONE_TXT(First);
    CLONE_TXT(Second);
    CLONE_TXT(Third);
    CLONE_TXT(textHeader);

    CLONE_V(offsX);
    CLONE_V(offsY);
    CLONE_V(HeightHeader);
//    CLONE_V(useBmp);
    CLONE_V(nRow);
    CLONE_V(styleShow);
    CLONE_V(infoRows);
    CLONE_V(heightRows);
    CLONE_V(useSwap);
    CLONE_V(timeCycle);

    for(uint i = 0; i < SIZE_A(Width); ++i)
      Width[i] = po->Width[i];

    for(uint i = 0; i < SIZE_A(infoFirst); ++i)
      infoFirst[i] = po->infoFirst[i];
    }
}
//-----------------------------------------------------------
void PropertyTableInfo::cloneMinusProperty(const Property& other)
{
  clone(other);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
bool svmDialogTableInfo::create()
{
  if(!baseClass::create())
    return false;
  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(Prop);
  SET_INT(IDC_EDIT_NDEC, pt->nRow);
  SET_INT(IDC_EDIT_X2, REV__X(pt->offsX));
  SET_INT(IDC_EDIT_Y2, REV__X(pt->offsY));
  SET_INT(IDC_TIMECYCLE, pt->timeCycle);
  if(pt->useSwap)
    SET_CHECK(IDC_CHECK_SWAP);
  return true;
}
//-----------------------------------------------------------
LRESULT svmDialogTableInfo::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_TBL_INFO_HEADER:
          svmDialogTblHeader(Owner, dynamic_cast<PropertyTableInfo*>(tmpProp), this).modal();
          break;
        case IDC_TBL_INFO_FIRST_COL:
          svmDialogTblFirstCol(Owner, dynamic_cast<PropertyTableInfo*>(tmpProp), this).modal();
          break;
        case IDC_TBL_INFO_ROWS:
          svmDialogTblRows(Owner, dynamic_cast<PropertyTableInfo*>(tmpProp), this).modal();
          break;
        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
void svmDialogTableInfo::CmOk()
{
  PropertyTableInfo* pt = dynamic_cast<PropertyTableInfo*>(tmpProp);
  GET_INT(IDC_EDIT_NDEC, pt->nRow); // numero di righe
  pt->nDec = 0;
  int t;
  GET_INT(IDC_EDIT_X2, t);
  pt->offsX = R__X(t);
  GET_INT(IDC_EDIT_Y2, t);
  pt->offsY = R__Y(t);
  GET_INT(IDC_TIMECYCLE, pt->timeCycle);
  pt->useSwap = IS_CHECKED(IDC_CHECK_SWAP);
  uint align = pt->alignText;
  baseClass::CmOk();
  Prop->alignText = align;
}

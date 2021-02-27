//--------------- svmPropertySpin.cpp -----------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "svmPropertySpin.h"
#include "macro_utils.h"
//-----------------------------------------------------------
svmDialogSpin::svmDialogSpin(svmObject* owner, Property* prop, PWin* parent, uint id,
                            HINSTANCE hInst) :
      baseClass(owner, prop, parent, id, hInst)
{
}
//-----------------------------------------------------------
svmDialogSpin::~svmDialogSpin()
{
  destroy();
}
//-----------------------------------------------------------
union U__
{
  int iv;
  float fv;
};
//-----------------------------------------------------------
/*
  per la corrispondenza tra nomi reali e alias (significato reale)
  vedere nel file 'svmObjSpin.cpp'
*/
//-----------------------------------------------------------
bool svmDialogSpin::create()
{
  uint style = Prop->style;
  uint type1 = Prop->type1;
  uint type2 = Prop->type2;
  U__ idFont;
  idFont.iv = Prop->idFont;
  uint alignText = Prop->alignText;
  U__ foreground;
  foreground.iv = Prop->foreground;

  if(!baseClass::create())
    return false;

  SET_INT(IDC_EDIT_ID_SPIN, style);
  SET_INT(IDC_EDIT_SPIN_MEM_ADDR, type1);
  SET_INT(IDC_EDIT_SPIN_DELAY, type2);
  TCHAR buff[64];
  if(alignText) {
    _stprintf_s(buff, _T("%f"), idFont.fv);
    zeroTrim(buff);
    SET_TEXT(IDC_EDIT_SPIN_STEP, buff);
    _stprintf_s(buff, _T("%f"), foreground.fv);
    zeroTrim(buff);
    SET_TEXT(IDC_EDIT_SPIN_STEP_DECR, buff);
    }
  else {
    wsprintf(buff, _T("%d"), idFont.iv);
    SET_TEXT(IDC_EDIT_SPIN_STEP, buff);
    wsprintf(buff, _T("%d"), foreground.iv);
    SET_TEXT(IDC_EDIT_SPIN_STEP_DECR, buff);
    }
  return true;
}
//-----------------------------------------------------------
static bool isReal(LPCTSTR p)
{
  int len = _tcslen(p);
  for(int i = 0; i < len; ++i)
    if(_T('.') == p[i])
      return true;
    else if(_T(',') == p[i])
      return false;
  return false;
}
//-----------------------------------------------------------
void svmDialogSpin::CmOk()
{
  uint style;
  uint type1;
  uint type2;
  uint alignText = 0;
  GET_INT(IDC_EDIT_ID_SPIN, style);
  GET_INT(IDC_EDIT_SPIN_MEM_ADDR, type1);
  GET_INT(IDC_EDIT_SPIN_DELAY, type2);

  U__ idFont = { 0 };
  U__ foreground = { 0 };

  TCHAR buff[64];
  TCHAR buff2[64];
  GET_TEXT(IDC_EDIT_SPIN_STEP, buff);
  GET_TEXT(IDC_EDIT_SPIN_STEP_DECR, buff2);
  if(isReal(buff) || isReal(buff2)) {
    idFont.fv = (float)_tstof(buff);
    foreground.fv = (float)_tstof(buff2);
    if(!foreground.fv)
      foreground.fv = idFont.fv;
    alignText = 1;
    }
  else {
    idFont.iv = _ttoi(buff);
    foreground.iv = _ttoi(buff2);
    if(!foreground.iv)
      foreground.iv = idFont.iv;
    }
  baseClass::CmOk();
  Prop->style = style;
  Prop->type1 = type1;
  Prop->type2 = type2;
  Prop->idFont = idFont.iv;
  Prop->foreground = foreground.iv;
  Prop->alignText = alignText;
  Prop->Rect.bottom = Prop->Rect.top + DEF_EDGE_BMP;
  Prop->Rect.right = Prop->Rect.left + DEF_EDGE_BMP;
}
//-----------------------------------------------------------

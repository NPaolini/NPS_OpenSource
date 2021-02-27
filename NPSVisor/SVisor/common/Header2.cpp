//-------- header2.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "header2.h"
#include "pVarText.h"
#include "1.h"
//----------------------------------------------------------------------------
Header *allocHeader(PWin* parent)
{
  Header* hd = new Header2(parent);
  return hd;
}
//-----------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
#define NAME_STR_HEADER _T("header2") PAGE_EXT
//--------------------------------------------------------------
Header2::Header2(PWin* parent, int resId, HINSTANCE hinst)
:
  baseClass(parent, resId, hinst), idRecipeInfo(0),
          idRecipeInMem(0)
{
  TCHAR path[_MAX_PATH];
  getFileStr(path);

  fillPageStr(path);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
Header2::~Header2()
{
  destroy();
}
//--------------------------------------------------------------
//--------------------------------------------------------------
void Header2::getFileStr(LPTSTR path)
{
  LPCTSTR p = ::getString(ID_HEADER_TEXT_FILE);
  if(!p)
    p = NAME_STR_HEADER;
  getFileStrCheckRemote(path, p);
//  _tcscpy(path, NAME_STR_HEADER);
//  getPath(path, dSystem);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
#define ID_RECIPE_INFO        1000001
#define ID_RECIPE_ROW_IN_MEM  1000002
#define ID_FILE_PAGE_RECIPE   1000003
extern bool getRecipeTextType(LPTSTR target, uint row, LPCTSTR pageFile);
//--------------------------------------------------------------
void Header2::setReady(bool first)
{
  baseClass::setReady(true);

  LPCTSTR p = sStr.getString(ID_RECIPE_INFO);
  if(!p)
    return;
  idRecipeInfo = _ttoi(p);
  p = sStr.getString(ID_RECIPE_ROW_IN_MEM);
  if(!p || !_ttoi(p)) {
    idRecipeInfo = 0;
    return;
    }
  idRecipeInMem = _ttoi(p);
  idRecipePrph = 1;
  p = findNextParam(p, 1);
  if(p)
    idRecipePrph = _ttoi(p);
}
//--------------------------------------------------------------
void Header2::refresh()
{
  baseClass::refresh();
  needRefresh = 0;
  if(!idRecipeInfo)
    return;

  prfData dataJob;
  dataJob.typeVar = prfData::tDWData;
  dataJob.lAddr = idRecipeInMem;

  genericPerif* prf = Par->getGenPerif(idRecipePrph);
  if(!prf) {
    idRecipeInfo = 0;
    return;
    }

  prfData::tResultData result = prf->get(dataJob);

  if(prfData::isChanged != result)
    return;

  PVarText* recipeText = getVarFromId(idRecipeInfo);
  if(!recipeText)
    return;
  TCHAR target[200];

  LPCTSTR p = sStr.getString(ID_FILE_PAGE_RECIPE);
  if(!p)
    return;
  if(!getRecipeTextType(target, dataJob.U.dw, p))
    return;
  recipeText->set_Text(target);
  recipeText->invalidate();
}
//--------------------------------------------------------------

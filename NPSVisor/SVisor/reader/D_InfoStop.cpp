//------ D_InfoStop.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "d_infoStop.h"
#include "p_date.h"
//#include "2000.h"
//--------------------------------------------------------------------------
#include "commonInfo.h"
//--------------------------------------------------------------------------
//----------------------------------------------------------------------------
void TD_Info_Stop::fill()
{
#if 1
  setWindowTextByLangGlob(*this, ID_TREE_INFO_TITLE_CAUSE, false);
#else
  setCaption(getStringOrId(ID_TREE_INFO_TITLE_CAUSE));
#endif
  baseClass::fill(InfoCause, InfoOper, I_LOGS);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool TD_Info_Stop::fill4print()
{
  return baseClass::fill4print(InfoCause, InfoOper, false);
}
//----------------------------------------------------------------------------
void TD_Info_Stop::fillSub1(HTREEITEM parent)
{
  TCHAR buff[500];

  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.hItem = parent;
  tvi.pszText = buff;
  tvi.cchTextMax = SIZE_A(buff);
  if(!TreeView_GetItem(*Tree, &tvi))
    return;

  codeType cause = getCodeCauseByBuff(buff);

  const infoList& listCause = InfoCause->getInfo();
  if(!listCause.find(cause))
    return;

  const infoList& listOper = InfoOper->getInfo();

  baseClass::fillSub1(parent, listCause, listOper, I_OPER);
}
//----------------------------------------------------------------------------
void TD_Info_Stop::fillSub2(HTREEITEM parent)
{
  baseClass::fillSub2(parent, InfoCause, InfoOper);
}
//----------------------------------------------------------------------------
void TD_Info_Stop::getCodeMasterAndSlave(HTREEITEM parent, codeType& master, codeType& slave)
{
  TCHAR buff[500];

  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.hItem = parent;
  tvi.pszText = buff;
  tvi.cchTextMax = SIZE_A(buff);
  if(!TreeView_GetItem(*Tree, &tvi))
    return;

  slave = getCodeOperByBuff(buff);
  master = 0;

  HTREEITEM parentOfParent = TreeView_GetParent(*Tree, parent);
  if(parentOfParent) {
    tvi.hItem = parentOfParent;
    if(TreeView_GetItem(*Tree, &tvi))
      master = getCodeCauseByBuff(buff);
    }
}
//----------------------------------------------------------------------------
void TD_Info_Stop::makeNameFromCodeMaster(LPTSTR target, size_t len, const codeType& code)
{
  TD_Info::makeNameCause(target, len, code);
}
//----------------------------------------------------------------------------
codeType TD_Info_Stop::useCodeOrCountMaster(const codeType& code, int /*count*/)
{
  return code;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void TD_Info_Stop::makeNameFromCodeSlave(LPTSTR target, size_t len, const codeType& code)
{
  TD_Info::makeNameOper(target, len, code);
}
//----------------------------------------------------------------------------
codeType TD_Info_Stop::useCodeOrCountSlave(const codeType& /*code*/, int count)
{
  return count;
}


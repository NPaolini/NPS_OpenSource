//------ D_InfoOper.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "d_infoOper.h"
#include "p_date.h"
//#include "2000.h"
//----------------------------------------------------------------------------
#include "commonInfo.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void TD_Info_Oper::fill()
{
#if 1
  setWindowTextByLangGlob(*this, ID_TREE_INFO_TITLE_OPER, false);
#else
  setCaption(getStringOrId(ID_TREE_INFO_TITLE_OPER));
#endif
  baseClass::fill(InfoOper, InfoCause, I_OPER);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool TD_Info_Oper::fill4print()
{
  return baseClass::fill4print(InfoOper, InfoCause, true);
}
//----------------------------------------------------------------------------
void TD_Info_Oper::fillSub1(HTREEITEM parent)
{
  TCHAR buff[500];

  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.hItem = parent;
  tvi.pszText = buff;
  tvi.cchTextMax = SIZE_A(buff);
  if(!TreeView_GetItem(*Tree, &tvi))
    return;

  codeType oper = getCodeOperByBuff(buff);

  const infoList& listOper = InfoOper->getInfo();
  if(!listOper.find(oper))
    return;

  const infoList& listCause = InfoCause->getInfo();

  baseClass::fillSub1(parent, listOper, listCause, I_LOGS);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void TD_Info_Oper::fillSub2(HTREEITEM parent)
{
  baseClass::fillSub2(parent, InfoOper, InfoCause);
}
//----------------------------------------------------------------------------
void TD_Info_Oper::getCodeMasterAndSlave(HTREEITEM parent, codeType& master, codeType& slave)
{
  TCHAR buff[500];

  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.hItem = parent;
  tvi.pszText = buff;
  tvi.cchTextMax = SIZE_A(buff);
  if(!TreeView_GetItem(*Tree, &tvi))
    return;

  slave = getCodeCauseByBuff(buff);
  master = 0;

  HTREEITEM parentOfParent = TreeView_GetParent(*Tree, parent);
  if(parentOfParent) {
    tvi.hItem = parentOfParent;
    if(TreeView_GetItem(*Tree, &tvi))
      master = getCodeOperByBuff(buff);
    }
}
//----------------------------------------------------------------------------
void TD_Info_Oper::makeNameFromCodeMaster(LPTSTR target, size_t len, const codeType& code)
{
  TD_Info::makeNameOper(target, len, code);
}
//--------------------------------------------------------------------------
codeType TD_Info_Oper::useCodeOrCountMaster(const codeType& /*code*/, int count)
{
  return count;
}
//----------------------------------------------------------------------------
void TD_Info_Oper::makeNameFromCodeSlave(LPTSTR target, size_t len, const codeType& code)
{
  TD_Info::makeNameCause(target, len, code);
}
//----------------------------------------------------------------------------
codeType TD_Info_Oper::useCodeOrCountSlave(const codeType& code, int /*count*/)
{
  return code;
}


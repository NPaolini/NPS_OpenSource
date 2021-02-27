//---------- d_empty.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "d_empty.h"
#include "id_btn.h"
#include "lnk_body.h"
//----------------------------------------------------------------------------
static int IdParent;
//----------------------------------------------------------------------------
P_Body *getEmpty(int idPar, PWin *parent)
{
  P_Body *bd = new TD_EMPTY(idPar, parent);
  return bd;
}

//{{TD_EMPTY Implementation}}


TD_EMPTY::TD_EMPTY(int idPar, PWin* parent, uint resId, HINSTANCE hinst)
:
    P_DefBody(idPar, parent, resId, hinst)
{
//  if(idPar)
    IdParent = idPar;
}


TD_EMPTY::~TD_EMPTY()
{
  destroy();
}


bool TD_EMPTY::create()
{
  if(!P_DefBody::create())
    return false;
  PostMessage(*getMain(), WM_CUSTOM, MAKEWPARAM(MSG_PREV_PAGE, 0), 0);
//  PostMessage(*Par, WM_MY_HOTKEY, IDC_BUTTON_F1, 0);
  return true;
}

P_Body* TD_EMPTY::pushedBtn(int idBtn)
{
/*
  switch(idBtn) {
    case ID_F1:
      if(IdParent)
        return getBody(IdParent, Par, 0);
      break;
    }
*/
  return P_DefBody::pushedBtn(idBtn);
}
void TD_EMPTY::getFileStr(LPTSTR path)
{
  path[0] = 0;
}
/*
const long *TD_EMPTY::getInfo()
{
  return 0;
}

long TD_EMPTY::getTitle()
{
  return 0;
}
*/

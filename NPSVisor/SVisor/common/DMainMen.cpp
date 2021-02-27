//---------- dmainmen.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "dmainmen.h"
#include "1.h"
//----------------------------------------------------------------------------
// prima creazione
P_Body *allocMainMenu(PWin *parent)
{
  TD_MainMenu *mm = new TD_MainMenu(0, parent);
  return mm;
}
//---------------------------------------------------
P_Body *getMainMenu(int idPar, PWin *parent)
{
  P_Body *bd = new TD_MainMenu(idPar, parent);
  return bd;
}
//----------------------------------------------------------------------------
bool isMainPage(LPCTSTR page)
{
  return page ? !_tcsicmp(page, FIRST_PAGE) : true;
}
//----------------------------------------------------------------------------
TD_MainMenu::TD_MainMenu(int idPar, PWin* parent, int resId, HINSTANCE hinst)
:
    baseClass(idPar, parent, resId, hinst)
{
  LPCTSTR p = getString(ID_READ_ONLY_PROGRAM);
  bool read_only = toBool(p && _ttoi(p));
  setReadOnly(read_only);
}
//----------------------------------------------------------------------------
TD_MainMenu::~TD_MainMenu()
{
  destroy();
}
//----------------------------------------------------------------------------
void TD_MainMenu::getFileStr(LPTSTR path)
{
  getFileStrCheckRemote(path, FIRST_PAGE);
}
//----------------------------------------------------------------------------

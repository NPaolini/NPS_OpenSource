//------------- svDialog.cpp ---------------------------------------------
//------------------------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------------
#include "svDialog.h"
//----------------------------------------------------------------------------
#include "mainClient.h"
//--------------------------------------------------------------
/*
mainClient* getMain(PWin* win)
{
  mainClient* main = 0;
  do {
    main = dynamic_cast<mainClient*>(win);
    if(main)
      break;
    win = win->getParent();
    } while(win);
  return main;
}
*/
//------------------------------------------------------------------------
svDialog::svDialog(PWin* parent, uint id, HINSTANCE hInst) :
    baseClass(parent, id, hInst)
{
  mainClient* mcl = getMain();
  if(mcl)
    mcl->enableDialogFunctionKey();
}
//------------------------------------------------------------------------
svDialog::~svDialog()
{
  mainClient* mcl = getMain();
  if(mcl)
    mcl->disableDialogFunctionKey();
  destroy();
}
//-----------------------------------------------------------
//------------------------------------------------------------------------
svModDialog::svModDialog(PWin* parent, uint id, HINSTANCE hInst) :
    baseClass(parent, id, hInst)
{
  mainClient* mcl = getMain();
  if(mcl)
    mcl->enableDialogFunctionKey();
}
//------------------------------------------------------------------------
svModDialog::~svModDialog()
{
  mainClient* mcl = getMain();
  if(mcl)
    mcl->disableDialogFunctionKey();
  destroy();
}
//-----------------------------------------------------------

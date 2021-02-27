//-------------------- svDialog.H ---------------------------
#ifndef SVDIALOG_H_
#define SVDIALOG_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PMODDIALOG_H_
  #include "PModDialog.h"
#endif
#include "resource.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT svDialog : public PDialog
{
  public:
    svDialog(PWin* parent, uint id, HINSTANCE hInst = 0);
    ~svDialog();
  private:
    typedef PDialog baseClass;
};
//-----------------------------------------------------------
class IMPORT_EXPORT svModDialog : public PModDialog
{
  public:
    svModDialog(PWin* parent, uint id, HINSTANCE hInst = 0);
    ~svModDialog();
  private:
    typedef PModDialog baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

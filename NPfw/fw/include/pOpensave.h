//------ pOpensave.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef POPENSAVE_H_
#define POPENSAVE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class infoOpenSave
{
  public:
    enum whichAction { OPEN_F, SAVE_AS, OPEN_F_MULTIPLE, OPEN_NO_EXIST };
    // non vengono duplicati i dati passati,
    // sono copiati solo i puntatori
    infoOpenSave(LPCTSTR* ext,  LPCTSTR filter, whichAction action, DWORD ix = 0, LPCTSTR file = 0);

    LPCTSTR* getExt() const;
    LPCTSTR getFilter() const;
    LPCTSTR getFile() const;
    whichAction getAction() const;

    // input/output
    // per mantenere l'ultima scelta va salvato e reimpostato
    DWORD ixFilter;

  private:
    LPCTSTR* Ext;
    LPCTSTR Filter;
    LPCTSTR File;
    whichAction Action;
    NO_COPY_COSTR_OPER(infoOpenSave)
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class POpenSave
{
  public:
    POpenSave(HWND owner);
    virtual ~POpenSave() { delete []pMultipleFiles; }

    LPCTSTR getFile() const;

    virtual bool run(infoOpenSave& info);

    virtual bool acceptItem(LPCTSTR filename) const { return true; }

  protected:
    HWND Owner;
    LPTSTR get_File();

    virtual void setPathFromHistory() {}
    virtual void savePathToHistory() {}

    virtual void setInitialFile(infoOpenSave& info);
    TCHAR filename[_MAX_PATH];

  private:
    LPTSTR pMultipleFiles;

    NO_COPY_COSTR_OPER(POpenSave)
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline
infoOpenSave::infoOpenSave(LPCTSTR* ext,  LPCTSTR filter, whichAction action,
    DWORD ix,  LPCTSTR file) :
    Ext(ext), Filter(filter), Action(action), ixFilter(ix), File(file) {}
//----------------------------------------------------------------------------
inline LPCTSTR* infoOpenSave::getExt() const
{
  return Ext;
}
//----------------------------------------------------------------------------
inline LPCTSTR infoOpenSave::getFilter() const
{
  return Filter;
}
//----------------------------------------------------------------------------
inline LPCTSTR infoOpenSave::getFile() const
{
  return File;
}
//----------------------------------------------------------------------------
inline infoOpenSave::whichAction infoOpenSave::getAction() const
{
  return Action;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline POpenSave::POpenSave(HWND owner) : Owner(owner), pMultipleFiles(0)
{
  filename[0] = 0;
}
//----------------------------------------------------------------------------
inline LPCTSTR POpenSave::getFile() const
{
  return pMultipleFiles ? pMultipleFiles : filename;
}
//----------------------------------------------------------------------------
inline LPTSTR POpenSave::get_File()
{
  return pMultipleFiles ? pMultipleFiles : filename;
}
//----------------------------------------------------------------------------
bool PChooseFolder(LPTSTR path /* in-out */, LPCTSTR title = 0, bool onlySub = false, LPCTSTR relPath = 0, HWND owner = 0);
bool PChooseFolder(HWND owner, LPTSTR path /* in-out */, LPCTSTR title = 0, bool onlySub = false, LPCTSTR relPath = 0);
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

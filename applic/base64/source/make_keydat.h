//----------- make_keydat.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef make_keydat_H_
#define make_keydat_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"
#include "pModDialog.h"
#include "p_util.h"
//----------------------------------------------------------------------------
struct infoBuff
{
  infoBuff() : dim(0), buff(0) {}
  infoBuff(uint d) : dim(d), buff(new char[d + 1]) { buff[0] = 0; }
  infoBuff(const infoBuff& other) : dim(other.dim), buff(0) { buff = new char[other.dim + 1]; CopyMemory(buff, other.buff, dim + 1); }
  ~infoBuff() { delete []buff; }

  uint dim;
  LPSTR buff;
};
//----------------------------------------------------------------------------
class make_keydat : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    make_keydat(PWin* parent, HINSTANCE hinstance = 0);
    virtual ~make_keydat();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk() {}
    void CmCancel() {}
  private:
    HFONT Font;
    void decode();
    void encode();
    void allocLoadBuff(infoBuff& ib, uint idc, bool byFile, bool need_trim);
    void saveBuff(infoBuff& ib, uint idc, bool byFile, uint len_split);
    void chooseFile(uint idc_edit);
    void enableDisable(bool dec);
};
//----------------------------------------------------------------------------
#endif

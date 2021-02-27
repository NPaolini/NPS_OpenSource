//----------- make_crypt.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef make_crypt_H_
#define make_crypt_H_
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
  infoBuff(const infoBuff& other) : dim(other.dim), buff(0) { buff = new char[other.dim]; CopyMemory(buff, other.buff, dim); }
  ~infoBuff() { delete []buff; }

  uint dim;
  LPSTR buff;
};
//----------------------------------------------------------------------------
class make_crypt : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    make_crypt(PWin* parent, HINSTANCE hinstance = 0);
    virtual ~make_crypt();

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk() {}
    void CmCancel() {}
  private:
    void common_code(infoBuff& ib);
    void decode();
    void encode();
    void allocLoadBuff(infoBuff& ib, uint idc, bool byFile);
    void saveBuff(infoBuff& ib, uint idc, bool byFile);
    void chooseFile(uint idc_edit);
    void enableDisable(bool dec);
};
//----------------------------------------------------------------------------
#endif

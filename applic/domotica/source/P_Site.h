//-------- P_Site.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_Site_H
#define P_Site_H
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PDIALOG_H_
  #include "pdialog.h"
#endif

#include "resource.h"
#include "p_vect.h"
#include "pbitmap.h"
#include "paramconn.h"
#include "connClass.h"
#include "P_BaseCommon.h"
#include "pListbox.h"
#include "progressBar.h"
#include "p_avl.h"
//----------------------------------------------------------------------------
#define MAX_NAME_EXT 3
#define MAX_NAME_DIM 8
#define MAX_FILE_DIM 5
//----------------------------------------------------------------------------
class infoFile : public genericSet
{
  public:
    char name[MAX_NAME_DIM + 1];
    char ext[MAX_NAME_EXT + 1];
    uint dim;
    infoFile() : dim(0) { ZeroMemory(name, sizeof(name)); ZeroMemory(ext, sizeof(ext)); }
    infoFile(const infoFile& other) : dim(other.dim) {
      memcpy_s(name, sizeof(name), other.name, sizeof(other.name));
      memcpy_s(ext, sizeof(ext), other.ext, sizeof(other.ext));
      }
};
//----------------------------------------------------------------------------
class fileList : public P_Avl
{
  public:
    fileList() : P_Avl(true) { }
    const infoFile* getCurrData() const;
    infoFile* getCurrData();
    bool addData(const infoFile& iF);
    bool addData(infoFile* iF);
  protected:
    virtual int Cmp(const TreeData toCheck, const TreeData current) const;
};
//----------------------------------------------------------------------------
class P_Site : public P_BaseCommon
{
  private:
    typedef P_BaseCommon baseClass;
  public:
    P_Site(PConnBase* conn, PWin* parent, uint resId = IDD_DIALOG_SITE, HINSTANCE hinstance = 0);
    virtual ~P_Site();
    virtual bool create();
private:
  PVect<PBitmap*> Bmp;
  PListBox* Lb;
protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void parseBuff(LPSTR buff);
    void parseRow(fileList& fl, LPSTR row);
    void fillLB(fileList& fl);

    int getFromLb(infoFile& iF, PListBox* lb, int sel);
    void loadCurr();
    bool retrieveFile(progressBar& bar, infoFile& iF);
    void upload();
    void download();
    void openPath();
    void openFilename();
};
//----------------------------------------------------------------------------
#endif

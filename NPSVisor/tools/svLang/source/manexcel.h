//----------- manExcel.h ----------------------------------------------
#ifndef manExcel_H_
#define manExcel_H_
//---------------------------------------------------------------------
#include "precHeader.h"
//---------------------------------------------------------------------
#include "PModDialog.h"
#include "p_txt.h"
#include "p_util.h"
#include "p_vect.h"
//---------------------------------------------------------------------
extern LPCTSTR getStringOrDef(uint code, LPCTSTR def);
//---------------------------------------------------------------------
class manExcel : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    manExcel(PWin* parent, uint idc, LPCTSTR excelFile, LPCTSTR basePath,
        const PVect<LPCTSTR>& filename, const PVect<LPTSTR>& lang);
    manExcel(PWin* parent, uint idc, LPCTSTR excelFile, LPCTSTR basePath);
    ~manExcel();
    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    void CmCancel() {}
  private:
    bool isImport;
    bool Running;
    DWORD idThread;
    HANDLE hEventClose;
    LPCTSTR ExcelFile;
    LPCTSTR basePath;
    const PVect<LPCTSTR>& Filename;
    const PVect<LPTSTR>& Lang;

    void infoFileOnJob(WORD flag, LPCTSTR file);
    friend unsigned FAR PASCAL ImpProc(void*);
    friend unsigned FAR PASCAL ExpProc(void*);

    void endWithMsg(LPCTSTR msg, bool success);


};
//---------------------------------------------------------------------
#endif

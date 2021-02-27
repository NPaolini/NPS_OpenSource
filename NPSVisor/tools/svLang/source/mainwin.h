//------------ mainWin.h -----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef mainWin_H
#define mainWin_H
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#define PAGE_EXT _T(".npt")
#define FIRST_PAGE _T("main_page") PAGE_EXT
#define STD_MSG_NAME _T("base_data") PAGE_EXT
//----------------------------------------------------------------------------
class P_ModListFiles;
class P_ModListCodes;
class P_ModEdit;
//----------------------------------------------------------------------------
struct fillInfo
{
  DWORD source;
  DWORD target;
  DWORD action;
};
//----------------------------------------------------------------------------
void checkSystem(LPTSTR path);
//----------------------------------------------------------------------------
class mainWin : public PMainWin
{
  private:
    typedef PMainWin baseClass;
  public:
    mainWin(HINSTANCE hInstance);
    ~mainWin();

    bool create();

    bool preProcessMsg(MSG& msg);
    P_ModListFiles* getListFile() { return clientFile; }
    P_ModListCodes* getListCode() { return clientCode; }
    P_ModEdit* getEditCode() { return clientEdit; }

  protected:
    LPCTSTR getClassName() const { return _T("npsvLang_Class"); };
    class pMenuBtn* menuBtn;
    class PSplitter* mainSplit;
    class PSplitWin* mainClientSplit;
    class PSplitWin* clientSplit;

    P_ModListFiles* clientFile;
    P_ModEdit* clientEdit;
    P_ModListCodes* clientCode;

    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HACCEL haccel;

    TCHAR CurrPath[_MAX_PATH];
    void setFolder();
    void save();
    void setPathAndTitle(LPCTSTR path);
    void exportExcel();
    void importExcel();
    void fill();
    void fillFieldAllFile(const fillInfo& fi);
    void checkBasePage();
};
//------------------------------------------------------------------------------
P_ModListFiles* getListFile(PWin* child);
P_ModListCodes* getListCode(PWin* child);
P_ModEdit* getEditCode(PWin* child);
//------------------------------------------------------------------------------
#endif

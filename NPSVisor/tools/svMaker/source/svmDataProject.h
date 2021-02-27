//------------------ svmDataProject.h -----------------------
//-----------------------------------------------------------
#ifndef svmDataProject_H_
#define svmDataProject_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmManZoom.h"
//-----------------------------------------------------------
#define MAX_NAME_RELPATH _MAX_PATH
#define MAX_PAGES 8
//-----------------------------------------------------------
enum pageTypeCode
{
  ptcStandard,
  ptcRecipe,
  ptcRecipeListbox,
  ptcRecipeRow,
  ptcGraphTrend,
};
//-----------------------------------------------------------
class dataProject
{
  public:
    dataProject() : currPage(0), hideTitle(false)
    {
      newPath[0] = 0;
      _tcscpy_s(systemDatPath, _T("system"));
      _tcscpy_s(systemPath, _T("system"));
      _tcscpy_s(imagePath, _T("image"));
    }

    TCHAR newPath[_MAX_PATH];
    TCHAR systemPath[MAX_NAME_RELPATH];
    TCHAR imagePath[MAX_NAME_RELPATH];
    TCHAR systemDatPath[MAX_NAME_RELPATH];

    uint currPage;
    bool hideTitle;

    svmManZoom::zoomX getCurrZoom() { return getZoom().getCurrZoom(); }
    void setCurrZoom(svmManZoom::zoomX curr) { getZoom().setCurrZoom(curr); }
    bool calcFromScreen(PRect& r) { return getZoom().calcFromScreen(r); }
    bool calcFromScreen(SIZE& s) { return getZoom().calcFromScreen(s); }
    bool calcFromScreen(POINT& s) { return getZoom().calcFromScreen(s); }

    bool calcToScreen(PRect& r) { return getZoom().calcToScreen(r); }
    bool calcToScreen(SIZE& s) { return getZoom().calcToScreen(s); }
    bool calcToScreen(POINT& s) { return getZoom().calcToScreen(s); }


    void setDirty() { InfoPage[currPage].Dirty = true; }
    void resetDirty() { InfoPage[currPage].Dirty = false; }
    bool isDirty() const { return InfoPage[currPage].Dirty; }
    bool isDirty(uint ix) const { return InfoPage[ix].Dirty; }

    DWORD getWhatPage() const { return InfoPage[currPage].whatMake; }
    void setWhatPage(DWORD val) { InfoPage[currPage].whatMake = val; }
    void setAllWhatPage(DWORD val) { for(uint i = 0; i < SIZE_A(InfoPage); ++i) InfoPage[i].whatMake = val; gWhatMake = val; }

    LPCTSTR getPageName() const { return InfoPage[currPage].pageName; }
    LPCTSTR getPageName(uint ix) const { return InfoPage[ix].pageName; }
    void setPageName(LPCTSTR pg) { _tcscpy_s(InfoPage[currPage].pageName, pg); }

    void setPageType(uint type)  { InfoPage[currPage].pageType = type; }
    uint getPageType() const { return InfoPage[currPage].pageType; }
    uint getGlobWhatPage() const { return gWhatMake; }

    LPCTSTR getSubPath() const { return InfoPage[currPage].systemPathSub; }
    void setSubPath(LPCTSTR sub)
    {
      if(sub)
        _tcscpy_s(InfoPage[currPage].systemPathSub, sub);
      else
        InfoPage[currPage].systemPathSub[0] = 0;
    }

    void reset4NewProject() {  hideTitle = false; }

    void reset4New() {
      setPageName(_T("\0"));
      setSubPath(0);
      setPageType(0);
      setWhatPage(getGlobWhatPage());
      }

  private:
    svmManZoom ManZoom;
    svmManZoom& getZoom() { return ManZoom; }
    struct infoPage {
      TCHAR pageName[_MAX_PATH];
      mutable TCHAR systemPathSub[_MAX_PATH];
      bool Dirty;
      DWORD whatMake;
      DWORD pageType;
      infoPage() : Dirty(false), whatMake(0), pageType(ptcStandard) { pageName[0] = 0; systemPathSub[0] = 0; }
      } InfoPage[MAX_PAGES];
// DWORD whatMake;
/* bit 0 -> 0 == body, 1 == header, se body
                       bit 1 -> 1 == hideHeader,
                       bit 2 -> 1 == hideFooter
                     */
  private:
    dataProject(int) {  }
    DWORD gWhatMake;
    static void init()
    {
      dataProject dummy(1);
    }
};
//-----------------------------------------------------------
dataProject& getDataProject();
//-----------------------------------------------------------
#endif

//------------------ svmDataProject.h -----------------------
//-----------------------------------------------------------
#ifndef svmDataProject_H_
#define svmDataProject_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmManZoom.h"
#include "common.h"
#include "p_util.h"
#include "resource.h"
//-----------------------------------------------------------
// nell'id c'è il nome del file delle variabili.
// Il file non può essere lo stesso del template di stampa
#define ID_FILE_DATA      995000

// nell'id c'è la lista degli id delle variabili.
#define ID_FILE_DATA_VARS 995001
// numero di id presenti (ciascuno rappresenta una serie)
#define MAX_ID_DATA_VARS 5

#define ID_FILE_PAGE_DIM 995002

#define ID_FILE_LINKED   995003


#define ID_FILE_DATA_INIT_SET_VARS 999001
/*
es.
995000,prova.ptd
995001,999001,999002,999003,999004,999005

poi dentro prova.ptd
999001,1,1,1
999002,100,120,135
999003,ecc.
riferirsi al file di specifiche stampa
*/
//-----------------------------------------------------------
#define PRN_TXT_VAR         51
#define PRN_TXT_ON_BOX_VAR  52
//-----------------------------------------------------------
// per caricamento e salvataggio su file, mantiene lo stato corrente
struct infoState
{
  uint idFont;
  int Angle;
  uint idPen;
  uint idBrush;
  COLORREF foreground;
  COLORREF background;
  uint bkMode;
  uint Align;
  infoState() : idFont(0), idPen(1), idBrush(1), foreground(cBLACK), background(cWHITE),
                bkMode(0), Align(0), Angle(0) {}

  void reset() { idFont = 0; idPen = 1; idBrush = 1; foreground = cBLACK; background = cWHITE;
                bkMode = 0; Align = 0; Angle = 0; }

  void reset4save() { idFont = -1; idPen = -1; idBrush = -1; foreground = -1; background = -1;
                bkMode = -1; Align = -1; Angle = 3601; }

  bool savePen(P_File& pf, uint id);
  bool saveBrush(P_File& pf, uint id);
  bool saveFont(P_File& pf, uint id, int angle);
  bool saveForeground(P_File& pf, COLORREF c);
  bool saveBackground(P_File& pf, COLORREF c);
  bool saveBkMode(P_File& pf, uint mode);
  bool saveAlign(P_File& pf, uint align);

  bool loadPen(LPCTSTR row);
  bool loadBrush(LPCTSTR row);
  bool loadFont(LPCTSTR row, bool linked = false);
  bool loadForeground(LPCTSTR row);
  bool loadBackground(LPCTSTR row);
  bool loadBkMode(LPCTSTR row);
  bool loadAlign(LPCTSTR row);
  };
//-----------------------------------------------------------
//-----------------------------------------------------------
struct infoProject_
{
  TCHAR currPath[_MAX_PATH];   // percorso base
  TCHAR imagePath[_MAX_PATH];  // sotto percorso relativo per immagini
  TCHAR templateName[_MAX_PATH]; // [sotto percorso] + nome file
  TCHAR varFileName[_MAX_PATH];  // [sotto percorso] + nome file
  TCHAR linkedFileName[_MAX_PATH];  // [sotto percorso] + nome file
  bool Dirty;
  svmManZoom ManZoom;
  infoState iState;
  svmManZoom& getZoom() { return ManZoom; }
  LONG StartX;
  LONG StartY;

  LONG PageWidth;
  LONG PageHeight;

  bool Initialized;

  void resetStart() { StartX = 0; StartY = 0; }
  void resetZoom() { getZoom().setCurrZoom(svmManZoom::zOne); resetStart(); }
  void clearPath()  {
    currPath[0] = 0;
    _tcscpy_s(imagePath, _T("Image"));
    templateName[0] = 0;
    varFileName[0] = 0;
    linkedFileName[0] = 0;
    }
  void clearAll()  {  clearPath(); resetZoom(); Dirty = false; }

  void fullPathImg(LPTSTR path) { _tcscpy_s(path, _MAX_PATH, currPath); appendPath(path, imagePath); }

  infoProject_() : Dirty(false), StartX(0), StartY(0), PageWidth(2100), PageHeight(-2970), Initialized(false)
    {  clearPath();  }
};
//-----------------------------------------------------------
#define MAX_PAGES 8
//-----------------------------------------------------------
#define GET__NAME(a) get##a
#define SET__NAME(a) set##a
//-----------------------------------------------------------
#define GET_SET_STR_NAME(a) \
      LPCTSTR get##a() const { return infoProject[currPage].a; } \
      LPCTSTR get##a(uint ix) const { return infoProject[ix].a; } \
      void set##a(LPCTSTR cp) { _tcscpy_s(infoProject[currPage].a, cp); }
//-----------------------------------------------------------
#define GET_SET_V_NAME(t, a) \
      t get##a() { return infoProject[currPage].a; } \
      t get##a(uint ix) { return infoProject[ix].a; } \
      void set##a(t v) { infoProject[currPage].a = v; }
//-----------------------------------------------------------
class dataProject
{
  public:
    dataProject() : currPage(0)
    {
    }
    uint currPage;

    svmManZoom::zoomX getCurrZoom() { return infoProject[currPage].getZoom().getCurrZoom(); }
    void setCurrZoom(svmManZoom::zoomX curr) { infoProject[currPage].getZoom().setCurrZoom(curr); }
    svmManZoom& getZoom() { return infoProject[currPage].getZoom(); }

    template <typename T>
    bool calcFromScreen(T& t) { return infoProject[currPage].getZoom().calcFromScreen(t); }
    template <typename T>
    bool calcFromScreenH(T& t) { return infoProject[currPage].getZoom().calcFromScreenH(t); }
    template <typename T>
    bool calcFromScreenV(T& t) { return infoProject[currPage].getZoom().calcFromScreenV(t); }

    template <typename T>
    bool calcToScreen(T& t) { return infoProject[currPage].getZoom().calcToScreen(t); }
    template <typename T>
    bool calcToScreenH(T& t) { return infoProject[currPage].getZoom().calcToScreenH(t); }
    template <typename T>
    bool calcToScreenV(T& t) { return infoProject[currPage].getZoom().calcToScreenV(t); }

    GET_SET_V_NAME(bool, Dirty)
    GET_SET_V_NAME(long, StartX)
    GET_SET_V_NAME(long, StartY)
    GET_SET_V_NAME(long, PageWidth)
    GET_SET_V_NAME(long, PageHeight)
    GET_SET_V_NAME(bool, Initialized)

    GET_SET_STR_NAME(currPath)
    GET_SET_STR_NAME(imagePath)
    GET_SET_STR_NAME(templateName)
    GET_SET_STR_NAME(varFileName)
    GET_SET_STR_NAME(linkedFileName)

    void clearAll() { infoProject[currPage].clearAll(); }
    void resetZoom() { infoProject[currPage].resetZoom(); }
    void resetStart() { infoProject[currPage].resetStart(); }

    void fullPathImg(LPTSTR path) {  infoProject[currPage].fullPathImg(path); }


    void reset4NewProject() { clearAll(); }
    infoState& getCurrState() {  return infoProject[currPage].iState; }

    void initPath();
  private:
    infoProject_ infoProject[MAX_PAGES];
};
//-----------------------------------------------------------
dataProject& getDataProject();
//-----------------------------------------------------------
class PDataProject : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    enum use { useForSave, useForOPen, useForNew };
    PDataProject(PWin* parent, use whichUse, uint id, uint* idVars = 0,  HINSTANCE hInstance = 0) :
        baseClass(parent, id, hInstance), idVars(idVars),
          whichUse(whichUse)
    {
    }
    virtual ~PDataProject() { destroy(); }

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    void enableDim();
    void enableChoose();
    void choosePath();
    void chooseFile(uint idc_edit, bool templ);
    use whichUse;
    uint* idVars;
    void setDim(LPCTSTR file);
};
//-----------------------------------------------------------
#endif

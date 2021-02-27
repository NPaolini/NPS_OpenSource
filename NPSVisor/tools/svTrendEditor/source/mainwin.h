//------------ mainWin.h -----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef mainWin_H
#define mainWin_H
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "idTextLang.h"
//----------------------------------------------------------------------------
class P_ModListFiles;
//----------------------------------------------------------------------------
struct fillInfo
{
  DWORD nCol;
  TCHAR file[_MAX_PATH];
  fillInfo() : nCol(0) { ZeroMemory(file, sizeof(file)); }
};
//----------------------------------------------------------------------------
struct pasteInfo
{
  DWORD after;
  // bit 0 -> 0 == col, 1 == row, bit 1 -> overwrite, bit 2 -> 1 == dopo, bit 3 -> 0 == primo, 1 == ultimo (se !bit2)
  DWORD type;
  DWORD fromRow; // valido solo per overwrite colonne
  pasteInfo() : after(0), type(0), fromRow(0)  {  }
};
//----------------------------------------------------------------------------
struct exportInfo
{
  // bit 0 -> 0 == tab, 1 == sql, bit 1 -> 0 == file corrente, 1 == scelta file, bit 2 -> 1 == unico file destinazione (*)
  // bit 3 -> 1 == salva selezione, valido solo per file corrente
  // (*) se unico file vanno controllati prima i file per vedere se hanno lo stesso numero di campi, eventualmente
  // proporre una finestra per eliminare quelli incompatibili, oppure raggruppare i file per numero di campi e salvare
  // in file diversi gli altri gruppi.
  DWORD type;
  PVect<LPCTSTR> fileSet; // vanno inseriti ordinati

   // contiene il numero di campi per ciascun file. In caso di file corrente, se settato bit 3, i valori, a coppie,
   // sono i range di selezione
  PVect<int> fieldNumSet;

  exportInfo() : type(0) {}
  ~exportInfo() { flushPAV(fileSet); }
  private:
    const exportInfo& operator =(const exportInfo&);
    exportInfo(const exportInfo&);
};
//----------------------------------------------------------------------------
LPCTSTR getStringOrDef(uint code, LPCTSTR def);
void setLang(HWND hwnd, uint idText);
void setLang(HWND hwnd, uint idc, uint idText);
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
    virtual bool idle(DWORD count);

  protected:
    LPCTSTR getClassName() const { return _T("svTrendEditor_Class"); };
    class pMenuBtn* menuBtn;
    class PSplitter* mainSplit;
    P_ModListFiles* clientFile;

    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    TCHAR CurrPath[_MAX_PATH];
    void setFolder();
    void save();
    void setPathAndTitle(LPCTSTR path);
    void fill();
    void checkBasePage();

    void addColumn();
    void remColumn();
    void addRow();
    void remRow();
    void menuExport();
//    void menuImport();

    void copy();
    void paste();

    void menuAdd();
    void menuRem();
};
//------------------------------------------------------------------------------
P_ModListFiles* getListFile(PWin* child);
//------------------------------------------------------------------------------
#endif

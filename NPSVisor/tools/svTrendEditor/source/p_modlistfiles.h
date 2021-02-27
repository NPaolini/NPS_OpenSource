//-------- P_ModListFiles.h -----------------------------------------
//-------------------------------------------------------------------
#ifndef P_ModListFiles_H
#define P_ModListFiles_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "fullView.h"
#include "PCustomListViewFullEdit.h"
#include "P_param_v.h"
//-------------------------------------------------------------------
struct pasteInfo;
//-------------------------------------------------------------------
typedef PVect<pvvChar> pvvv;
//-------------------------------------------------------------------
#ifdef UNICODE
  #define myCF_TEXT CF_UNICODETEXT
#else
  #define myCF_TEXT CF_TEXT
#endif
//-------------------------------------------------------------------
// file viene copiato, basePath viene mantenuto esternamente
// il salvataggio avviene effettivamente solo se il flag Dirty è attivo
// e viene attivato dalla richiesta di updateFromLV() che indica che i
// dati sono stati cambiati (se non c'è stata modifica non deve essere richiamata)
// refresh annulla tutte le eventuali modifiche e ricarica da file
class infoFile
{
  public:
    infoFile(LPCTSTR file, LPCTSTR basePath);
    ~infoFile();

    LPCTSTR getFilename() const { return Filename; }

    bool refresh();
    bool save(bool onUnicode);

    void fillLV(HWND hwLv);
    void updateFromLV(HWND hwLv);
    bool isDirty() const { return Dirty; }

  private:
    bool Dirty;
    LPCTSTR Filename;
    LPCTSTR BasePath;
    struct row
    {
      uint id;
      LPCTSTR text;
      row() : id(0), text(0) {}
    };

    PVect<row> Rows;

    void addItem(HWND hwLv, uint id, LPCTSTR text);
    void addRow(HWND hwLv, int pos, LPTSTR buff);
    void flushRow();
};
//-------------------------------------------------------------------
class P_ModListFiles : public fullView
{
  private:
    typedef fullView baseClass;
  public:
    P_ModListFiles(PWin* parent, LPCTSTR title = 0, HINSTANCE hInst = 0) :
        baseClass(parent, title, hInst), nCol(0), pF(0), szRec(0), nRec(0), Dirty(false) { }

    P_ModListFiles(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, title, hinst), nCol(0), pF(0), szRec(0), nRec(0), Dirty(false) { }

    P_ModListFiles(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, title, hinst), nCol(0), pF(0), szRec(0), nRec(0), Dirty(false) { }

    ~P_ModListFiles();

    bool create();
    bool save(LPCTSTR file);

    virtual void load(LPCTSTR file, int n_col);
    bool isDirty() const { return Dirty; }

    bool addColumn(int pos);
    bool remColumn(int pos);
    void addRow(uint type); // 0 -> posizione corrente, 1 -> prima riga, 2 -> appende
    void remRow();

    void fill(fillInfo& fi);

    int getCurrNumCol() const { return nCol - 2; }
    void setModifyDate(bool set);
    bool hasSelected();
    void copyCurrent();
    void paste(const pasteInfo& pi);
    void exportRec(const exportInfo& eI);
    void fillSelected(PVect<int>& range);

    // colnum �, zero base, a partire da quella successiva al date/time, quindi zero -> Col_1
    void getHeaderText(LPTSTR target, uint sz, uint colnum);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//    virtual void resize();
    virtual HWND getHWClient() { return *List; }

    PCustomListViewFullEdit* List;
    P_File* pF;
    int nCol;
    int szRec;
    int nRec;
    bool Dirty;

    bool copyFile();
    void modifyRec(LV_ITEM* pItem);
    void loadRec(NMLVDISPINFO *pdi);
    P_File* commonColumnRow();

    void copyRow( P_File& pf, const pvvChar& row, LPBYTE buff);
    void fillRowByClipboard(const pasteInfo& pi, const  pvvv& target);
    void copyRowCols(P_File& pf, const pvvChar& row, LPBYTE buffS, LPBYTE buffT, int offset, bool overwrite);
    void fillByClipboard(const pasteInfo& pi, const  pvvv& target);
    void formatRowClipboard(int sel, LPTSTR buff, size_t dim);

    void reloadCurr(bool forAdd);
    void reloadCurr(int n_col);
    void exportTab(const exportInfo& eI);
    void exportSql(const exportInfo& eI);

    void makeHead(LPCTSTR file, int n_col);
    friend class myPCustomListViewFullEdit;
};
//-------------------------------------------------------------------
#endif

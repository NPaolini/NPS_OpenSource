//-------- P_ModListFiles.h -----------------------------------------
//-------------------------------------------------------------------
#ifndef P_ModListFiles_H
#define P_ModListFiles_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "fullView.h"
#include "pListbox.h"
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
        baseClass(parent, title, hInst) { init(); }

    P_ModListFiles(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, title, hinst) { init(); }

    P_ModListFiles(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, title, hinst) { init(); }

      ~P_ModListFiles();
    bool create();
    bool save(bool onUnicode);

    virtual void refresh(LPCTSTR path);
    bool isDirty() const;

    void getCurrent();
    void fillListFile(PVect<LPCTSTR>& filename);
    int getCurrSel();
    void setCurrSel(int sel);
    bool setFirst();
    bool setNext();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void resize();
    virtual HWND getHWClient() { return *List; }

    PListBox* List;
    TCHAR Path[_MAX_PATH];
    PVect<infoFile*> InfoFile;
    void flush() { flushPV(InfoFile); }

    void init();
    void selChange();

    int currSel;

    bool saveStdLang(uint sel, PVect<LPTSTR>& lang, bool onUnicode);

};
//-------------------------------------------------------------------
#endif

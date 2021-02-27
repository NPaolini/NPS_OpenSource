//-------------------- pListbox.h ---------------------------
//-----------------------------------------------------------
#ifndef PLISTBOX_H_
#define PLISTBOX_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PCONTROL_H_
  #include "pControl.h"
#endif

#ifndef PRECT_H_
  #include "pRect.h"
#endif
//-----------------------------------------------------------
#define TAB _T('\t')
//-----------------------------------------------------------
// aggiunta all'altezza del font per assegnare l'altezza dell'item
#define ADD_HEIGHT 2
//-----------------------------------------------------------
// larghezza media caratteri in pixel per larghezza prefissata.
// Se la stringa è tutta in maiuscolo occorre usare la rescale() con
// un valore di ~1.2.
#define DEF_PIX_WIDTH_800x600 8.0
//-----------------------------------------------------------
#define AUTO_PIX_WIDTH  -1
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PListBox : public PControl
{
  public:
    PListBox(PWin * parent, uint id, int x, int y, int w, int h,
              int len = 255, HINSTANCE hinst = 0);

    PListBox(PWin * parent, uint id, const PRect& r,
              int len = 255, HINSTANCE hinst = 0);

    PListBox (PWin* parent, uint id, int len = 255, HINSTANCE hinst = 0);
    PListBox(HWND hwnd, HINSTANCE hInst) :  PControl(hwnd, hInst) {}

    virtual ~PListBox ();

    virtual bool create();

    // da richiamare appena dopo la costruzione dell'oggetto e prima
    // della creazione del controllo windows
    virtual void SetTabStop(int ntabs, int *tabs, int *show = 0);

    void SetColor(COLORREF text, COLORREF bkg) { C_Text = text; C_Bkg = bkg; }
    void SetColorSel(COLORREF text, COLORREF bkg) { C_TextSel = text; C_BkgSel = bkg; }

    enum tAlign { aLeft = -1, aCenter, aRight };
    // da richiamare almeno dopo la SetTabStop()
    void setAlign(int n, tAlign al);

    // da richiamare dopo che windows ha creato il controllo
    void setFont(HFONT font, bool autoDelete);
    // in caso di righe con font diversi non deve ridurre le dimensioni
    void setFontNoReduce(HFONT font, bool autoDelete);
    int getHItem() { return hItem + ADD_HEIGHT; }
    int getLenTab(int pos) { return pxTabs[pos]; }
    void rescale(int init, int end, double scale);

    void setIntegralHeight();

    void setPixelWidthChar(double  newWidth = DEF_PIX_WIDTH_800x600);
    double  getPixelWidthChar() const;
    void recalcWidth() {   if(getHandle()) SetTab(); }

    void copyColorFrom(PListBox* other);

  protected:
    virtual HPEN setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo);
    virtual void paintTxt(HDC hDC, int x, int y, LPCTSTR buff, int ix, DRAWITEMSTRUCT FAR& drawInfo);
    virtual void paintCustom(HDC hDC, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo);
    // torna il ptr alla stringa con eliminati gli spazi iniziali e finali
    virtual LPTSTR trim(LPTSTR buff);
    void setHorzExt();
  private:
    void SetTab();
    int calc_len(int len, int offs, double mult);
    LPTSTR get_next(LPTSTR str);
    int retrieveHeightFont();

  protected:
    int *pxTabs;
    int *charTabs;
    tAlign *Align;
    bool *ShowField;
    int nTabs;
    int MaxLen;
    double pixelWidthChar;

    LPTSTR Buff;
    bool first;
    COLORREF C_Text;    // colore del testo normale
    COLORREF C_TextSel;   // colore del testo selezionato
    COLORREF C_Bkg;       // colore dello sfondo normale
    COLORREF C_BkgSel;    // colore dello sfondo selezionato
    int hItem;

    virtual void DrawItem (DRAWITEMSTRUCT FAR& drawInfo);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR getClassName() const { return _T("listbox"); }
    virtual bool retrieveBuff(DRAWITEMSTRUCT FAR& drawInfo);
};
//----------------------------------------------------------------------
inline void PListBox::setPixelWidthChar(double newWidth)
{
  pixelWidthChar = newWidth;
}
//----------------------------------------------------------------------
inline double PListBox::getPixelWidthChar() const
{
  return pixelWidthChar;
}
//----------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------
#endif

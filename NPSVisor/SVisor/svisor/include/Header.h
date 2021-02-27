//--------- header.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef HEADER_H_
#define HEADER_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEBODY_H_
  #include "p_basebody.h"
#endif

#ifndef PBITMAP_H_
  #include "pbitmap.h"
#endif

#include "setPack.h"

#define REP_ALWAYS 0x7fffffff

class Header : public P_BaseBody {
  public:
    Header(PWin* parent, uint resId = IDD_HEADER, HINSTANCE hInstance = 0);
    virtual ~Header();

    virtual void refresh();
    int Height();
    int trueHeight();

    void setTitle(LPCTSTR tit);
    virtual bool create();

    virtual void remakeByLang();

    void setVisual(forceVis type) { ForceVis = type; }

    class PTextAutoPanel* getTimeBox() { return time; }

    bool isHided() const { return Hided; }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void evPaint(HDC hdc, const PRect& rect);
    class PTextFixedPanel *boxAlarm;
    // torna false se va ricaricato, true se i dati non sono più vecchi della pagina
    virtual bool getCacheFile(LPTSTR target, uint sz);
  private:
    class PTextAutoPanel *time;
    class PTextFixedPanel *title;
    class PPanel *alarm;
    long idTitle;
    int statAlarm;
    HFONT FontTitle;
    HFONT FontTime;
    gestPerif::statAlarm Which;

    void setAlarm(gestPerif::statAlarm which = gestPerif::sAlarm, unsigned repeat = REP_ALWAYS);
    void resetAlarm();
    void makeAll();

    void refreshAlarm(HDC hdc);

    DWORD ForceVis;

    friend class gestTime;
    class gestTime* gt;

    bool Hided;
    typedef P_BaseBody baseClass;
};

extern Header* allocHeader(PWin* parent);

#include "restorePack.h"

#endif


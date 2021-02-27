//-------------------- svmPropertyAlarm.h --------------------
//-----------------------------------------------------------
#ifndef svmPropertyAlarm_H_
#define svmPropertyAlarm_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
#include "svmObjAlarm.h"
#include "common_color.h"
//-----------------------------------------------------------
#define MAX_LINE_COLOR 5
//-----------------------------------------------------------
#define PA_fix_filter type1
#define PA_def_show   type2
//-----------------------------------------------------------
LPCTSTR getStrAlarm(uint ix);
//-----------------------------------------------------------
class P_SimpleColorRow
{
  public:
    enum checkFocus { LOSS = -1, NOCHANGE, GET };

    P_SimpleColorRow(PWin* parent, uint idcStatic, uint* idcEdit);
    virtual ~P_SimpleColorRow() {}

    // passando 0 come HWND si forza il settaggio con il ridisegno
    virtual checkFocus setFocus(uint* IdSB, HWND hwnd = 0);
    void setColor(COLORREF color);
    void setColorBySelf(uint* IdSB);
    COLORREF getColor();
    PColorBox* getBox() { return cbx; }
    bool hasFocus() { return Focus; }

    virtual void setAll(LPCTSTR p);
  protected:
    PEdit* rgb[3];
    PColorBox* cbx;
    bool Focus;
};
//-----------------------------------------------------------
class PropertyAlarm : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyAlarm();
    virtual ~PropertyAlarm();

    const PropertyAlarm& operator=(const PropertyAlarm& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);

    // per i bit di visualizzazione usa la classe base, questi sono per i filtri
    dataPrf DataPrf[3];

    struct lineColor {
      COLORREF fg_n;
      COLORREF bg_n;
      COLORREF fg_s;
      COLORREF bg_s;
      lineColor() : fg_n(0), bg_n(0), fg_s(0), bg_s(0) {}
      };

    lineColor lColor[MAX_LINE_COLOR];
    enum bitShow { bs_IdAlarm, bs_Stat, bs_Date, bs_Time, bs_Filter, bs_Grp, bs_GrpDescr, bs_Prph, bs_AlarmDescr, bs_Max };
    uint percWidth[bs_Max];
    uint columnPos[bs_Max];
  protected:
    virtual void clone(const Property& other);

  private:
    PropertyAlarm(const PropertyAlarm& other);
};
//-----------------------------------------------------------
inline PropertyAlarm::PropertyAlarm()
{
  PA_fix_filter = 0;
  PA_def_show = (1 << bs_Max) - 1;
  ZeroMemory(percWidth, sizeof(percWidth));
  for(uint i = 0; i < SIZE_A(columnPos); ++i)
    columnPos[i] = i + 1;
}
//-----------------------------------------------------------
#define MAX_COLOR_EDIT 4
//-----------------------------------------------------------
class svmDialogAlarm : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogAlarm(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_ALARM_PROPERTY, HINSTANCE hInst = 0);
    ~svmDialogAlarm();

    virtual bool create();
    void setCurrColor(COLORREF c);

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void fillPrf2();
    void fillPrf2Grp();
    void fillPrf2Txt();
    virtual bool useFont() const { return true; }

    int lastSel;
    P_SimpleColorRow* colorEdit[MAX_COLOR_EDIT];
    PColorBox* CurrColor;
    HBRUSH hBrInfo;
    HBRUSH evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType);
    void evHScrollBar(HWND child, int flags, int pos);
    void checkChangeEdit(uint idc);
    void selChgColor();
    uint idTimer;
    bool needRefreshLb;
    void refreshLb();
    void updateData();
    void setVirtualFocus(int set);
    bool onMyModify;
};
//-----------------------------------------------------------
#endif

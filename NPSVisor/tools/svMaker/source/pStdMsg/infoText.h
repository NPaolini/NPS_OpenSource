//--------- infoText.h ----------------------------------------------
//-------------------------------------------------------------------
#ifndef INFOTEXT_H_
#define INFOTEXT_H_
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "pedit.h"
#include "p_Vect.h"
#include "p_Txt.h"
#include "p_Util.h"
#include "language_util.h"
//-------------------------------------------------------------------
struct infoLabel
{
  int Id;
  LPCTSTR Label;
};
//-------------------------------------------------------------------
// classe di gestione per codici di std_msg in chiaro.
class infoText
{
  public:
    infoText();
    infoText(int id, LPCTSTR label, LPCTSTR text);
    infoText(const infoText& other);
    infoText(const infoLabel& info, const setOfString& set);
    ~infoText();

    const infoText& operator=(const infoText& other);

    int getId() const;
    LPCTSTR getLabel() const;
    LPCTSTR getText() const;

    void setId(int id);
    void setLabel(LPCTSTR label);
    void setText(LPCTSTR text);
  private:
    void clone(const infoText& other);
    int Id;
    LPCTSTR Label;
    LPCTSTR Text;
};
//-------------------------------------------------------------------
// se la classe alloca oggetti è meglio creare un set di puntatori anziché
// un set di oggetti. Durante gli eventuali spostamenti in memoria in PVect
// gli oggetti verrebbero riallocati mentre i puntatori sono semplicemente
// spostati.
typedef PVect<infoText*> setOfPInfoText;
//-------------------------------------------------------------------
extern void fillInfoText(setOfPInfoText& target, const setOfString& set);
//-------------------------------------------------------------------
inline
int infoText::getId() const { return Id; }
//-------------------------------------------------------------------
inline
LPCTSTR infoText::getLabel() const { return Label; }
//-------------------------------------------------------------------
inline
LPCTSTR infoText::getText() const { return Text; }
//-------------------------------------------------------------------
inline
void infoText::setId(int id) { Id = id; }
//-------------------------------------------------------------------
inline
void infoText::setLabel(LPCTSTR label) { delete []Label; Label = str_newdup(label); }
//-------------------------------------------------------------------
inline
void infoText::setText(LPCTSTR text) { delete []Text; Text = str_newdup(text); }
//-------------------------------------------------------------------
//-------------------------------------------------------------------
class PageEdit : public langEdit
{
  private:
    typedef langEdit baseClass;
  public:
    PageEdit(PWin * parent, uint resid, int textlen = 0, HINSTANCE hinst = 0) :
          baseClass(parent, resid, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//-------------------------------------------------------------------
class firstEdit : public PageEdit
{
  private:
    typedef PageEdit baseClass;
  public:
    firstEdit(PWin * parent, uint resid, int textlen = 0, HINSTANCE hinst = 0) :
          baseClass(parent, resid, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//-------------------------------------------------------------------
class lastEdit : public PageEdit
{
  private:
    typedef PageEdit baseClass;
  public:
    lastEdit(PWin * parent, uint resid, int textlen = 0, HINSTANCE hinst = 0) :
          baseClass(parent, resid, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//-------------------------------------------------------------------
class PageEditChk : public langCheckEdit
{
  private:
    typedef langCheckEdit baseClass;
  public:
    PageEditChk(PWin * parent, uint resid, uint resid2, int textlen = 0, HINSTANCE hinst = 0) :
          baseClass(parent, resid, resid2, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//-------------------------------------------------------------------
class firstEditChk : public PageEditChk
{
  private:
    typedef PageEditChk baseClass;
  public:
    firstEditChk(PWin * parent, uint resid, uint resid2, int textlen = 0, HINSTANCE hinst = 0) :
          baseClass(parent, resid, resid2, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//-------------------------------------------------------------------
class lastEditChk : public PageEditChk
{
  private:
    typedef PageEditChk baseClass;
  public:
    lastEditChk(PWin * parent, uint resid, uint resid2, int textlen = 0, HINSTANCE hinst = 0) :
          baseClass(parent, resid, resid2, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//-------------------------------------------------------------------
#endif

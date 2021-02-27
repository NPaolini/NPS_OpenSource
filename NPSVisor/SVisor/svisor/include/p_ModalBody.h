//--------P_ModalBody.h ------------------------------------------------------
#ifndef P_MODALBODY_H_
#define P_MODALBODY_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_DefBody.h"

#ifndef MAINCLIENT_H_
  #include "mainClient.h"
#endif
#include "recipe.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class manageModal
{
  public:
    manageModal(P_Body* owner = 0) : Owner(owner), exiting(false), noCaption(false), pt_offs({0, 0}) {}
    virtual ~manageModal() {}
    void setOwner(P_Body* owner) { Owner = owner; }

    virtual bool windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result);
//    virtual bool pushedBtn(int idBtn);
    virtual bool acceptChildPage(int idBtn);
    virtual void create();
    bool canRefresh() { return !exiting; }
    void setExiting() { exiting = true; }
    void adjustSize();
  protected:
    void makeRegion();
    void getSize(int& w, int& h);
    bool noCaption;
    bool exiting;
    P_Body* Owner;
    POINT pt_offs;
    virtual void addToRegion(HRGN hrgn);
    virtual void calcAllArea(PRect& area);
};
//----------------------------------------------------------------------------
class manageRecipe : public manageModal
{
  private:
    typedef manageModal baseClass;
  public:
    manageRecipe(P_Body* owner = 0) : baseClass(owner) {}
    virtual ~manageRecipe() {}
//    virtual bool pushedBtn(int idBtn);
  protected:
    virtual void addToRegion(HRGN hrgn);
    virtual void calcAllArea(PRect& area);
};
//----------------------------------------------------------------------------
class P_ModalBody : public P_DefBody
{
  public:
    P_ModalBody(PWin* parent, LPCTSTR pageDescr);
    P_ModalBody(int idParent, PWin* parent, uint resId, HINSTANCE hInstance = 0);

    virtual ~P_ModalBody();

    bool create();
    void setReady(bool first);

    P_Body* pushedBtn(int idBtn);

    virtual void refresh();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR findChildPage(int idBtn, int& idPar, int& pswLevel, bool& predefinite);
    manageModal *ManModal;
    virtual manageModal* allocManModal() { return new manageModal(this); }
    void addToRegion(HRGN hrgn);
    void calcAllArea(PRect& area);
  private:

    typedef P_DefBody baseClass;
    friend class manageModal;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class P_ModelessBody : public P_ModalBody
{
  public:
    P_ModelessBody(PWin* parent, LPCTSTR pageDescr);
    P_ModelessBody(int idParent, PWin* parent, uint resId, HINSTANCE hInstance = 0);

    virtual ~P_ModelessBody();

    void setReady(bool first);

    P_Body* pushedBtn(int idBtn);

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    void savePos(HWND hwnd);

    typedef P_ModalBody baseClass;
};
//----------------------------------------------------------------------------
class P_ModalRecipe : public TD_Recipe
{
  public:
    P_ModalRecipe(int idParent, PWin* parent, uint resId, HINSTANCE hInstance = 0);

    virtual ~P_ModalRecipe();

    bool create();
    void setReady(bool first);

    P_Body* pushedBtn(int idBtn);

    virtual void refresh();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR findChildPage(int idBtn, int& idPar, int& pswLevel, bool& predefinite);
    manageModal *ManModal;
    virtual manageModal* allocManModal() { return new manageRecipe(this); }
    void addToRegion(HRGN hrgn);
    void calcAllArea(PRect& area);
  private:

    typedef TD_Recipe baseClass;
    friend class manageRecipe;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class P_ModelessRecipe : public P_ModalRecipe
{
  public:
    P_ModelessRecipe(int idParent, PWin* parent, uint resId, HINSTANCE hInstance = 0);

    virtual ~P_ModelessRecipe();

    void setReady(bool first);

    P_Body* pushedBtn(int idBtn);

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    void savePos(HWND hwnd);

    typedef P_ModalRecipe baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif


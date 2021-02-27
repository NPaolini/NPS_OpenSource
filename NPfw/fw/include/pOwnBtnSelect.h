//-------------------- POwnBtnSelect.h ----------------------
//-----------------------------------------------------------
#ifndef POWNBTNSELECT_H_
#define POWNBTNSELECT_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pOwnBtnSwitch.h"
//-----------------------------------------------------------
/*
  Eredita da POwnBtnSwitch, ma fa parte di un gruppo di opzioni di cui solo una può
  essere attiva. Un click su un pulsante premuto non esegue alcuna azione mentre un
  click su un pulsante sollevato lo setta e resetta l'altro del gruppo che era premuto.
  Group è un contenitore per i pulsanti del gruppo. Inizialmente vuoto viene riempito
  automaticamente dagli stessi oggetti. L'oggetto, quando viene distrutto, si rimuove
  automaticamente dal contenitore. Quando viene distrutto l'ultimo oggetto viene
  distrutto anche il contenitore.
*/
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT POwnBtnSelect;
//-----------------------------------------------------------
class IMPORT_EXPORT POwnBtnSelect : public POwnBtnSwitch
{
  public:
    POwnBtnSelect(cntGroup* group, PWin * parent, uint id, int x, int y, int w, int h,
          const PVect<PBitmap*>& image, bool sizeByImage = true, LPCTSTR text = 0,
          HINSTANCE hinst = 0);
    POwnBtnSelect(cntGroup* group, PWin * parent, uint id, const PRect& r,
          const PVect<PBitmap*>& image,
          bool sizeByImage = true, LPCTSTR text = 0, HINSTANCE hinst = 0);

    POwnBtnSelect(cntGroup* group, PWin * parent, uint resid,
          const PVect<PBitmap*>& image,
          bool sizeByImage = true, HINSTANCE hinst = 0);

    ~POwnBtnSelect();
    void setState(manageBtnSwitch::state stat);
    void switchState();
    const cntGroup* getGroup() { return Group; }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    cntGroup* Group;
    virtual void unselectOther();
  private:
    typedef POwnBtnSwitch baseClass;
    void addToGroup();
    void removeFromGroup();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline
POwnBtnSelect::POwnBtnSelect(cntGroup* group, PWin * parent, uint id, int x, int y, int w, int h,
            const PVect<PBitmap*>& image, bool sizeByImage, LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, image, sizeByImage, text, hinst),
      Group(group)
{
  addToGroup();
}
//-----------------------------------------------------------
inline
POwnBtnSelect::POwnBtnSelect(cntGroup* group, PWin * parent, uint id, const PRect& r,
            const PVect<PBitmap*>& image, bool sizeByImage, LPCTSTR text, HINSTANCE hinst) :
      baseClass(parent, id, r, image, sizeByImage, text, hinst),
      Group(group)
{
  addToGroup();
}
//-----------------------------------------------------------
inline POwnBtnSelect::POwnBtnSelect(cntGroup* group, PWin * parent, uint resid,
            const PVect<PBitmap*>& image, bool sizeByImage, HINSTANCE hinst) :
        baseClass(parent, resid, image, sizeByImage, hinst),
        Group(group)
{
  addToGroup();
}
//-----------------------------------------------------------
inline POwnBtnSelect::~POwnBtnSelect()
{
  removeFromGroup();
  destroy();
}
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif

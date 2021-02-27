//-------- recipe.h ----------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef RECIPE_H_
#define RECIPE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_defbody.h"
#include "ptraspbitmap.h"
//----------------------------------------------------------------------------
#define ID_INIT_DATA_RECIPE     1001001
#define ID_END_DATA_RECIPE      (ID_INIT_DATA_RECIPE + 10)
//----------------------------------------------------------------------------
#define ID_INIT_ACT_DATA        1001101
#define ID_END_ACT_DATA         (ID_INIT_ACT_DATA + 3)
//----------------------------------------------------------------------------
#define ID_INIT_DEFAULT_VALUE   1001201
#define ID_END_DEFAULT_VALUE    (ID_INIT_DEFAULT_VALUE + 3)
//----------------------------------------------------------------------------
#define ID_DEFAULT_EDITOR 1000005
#define ID_FLAG_PLC       1000006
#define ID_OK_SEND        1000007
#define ID_MSG_1          1000008
#define ID_MSG_2          1000009
//----------------------------------------------------------------------------
#define ID_MSG_SEND_OK      1000012
#define ID_TITLE_SEND_OK    1000013
//----------------------------------------------------------------------------
#define ID_MSG_COPYMEM_OK   1000014
#define ID_TITLE_COPYMEM_OK 1000015
//----------------------------------------------------------------------------
#define ID_RECIPE_ABS_PATH  1001151
#define ID_RECIPE_SUB_PATH  1001152
//----------------------------------------------------------------------------
#define ID_RECIPE_BIT_TO_FUNCT 1001155
//----------------------------------------------------------------------------
// se non esiste viene presa l'estensione di default
// deve essere nella forma .ext
#define ID_RECIPE_EXT       1000100
//----------------------------------------------------------------------------
typedef PVect<int> pVectInt;
//-----------------------------------------------------------
void fillSet(pVectInt& v, LPCTSTR p, int offsOnVect = 0);
//----------------------------------------------------------------------------
class TD_Recipe : public P_DefBody
{
  public:
    TD_Recipe(int idPar, PWin* parent, int resId = IDD_STANDARD, HINSTANCE hinst = 0);
    virtual ~TD_Recipe();
    virtual P_Body* pushedBtn(int idBtn);
    virtual void setReady(bool first);
    virtual bool create();
    virtual void refreshBody();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void sendBitToFunct(uint id_init, DWDATA dw);
    int sentByBit;
    virtual void getFileStr(LPTSTR path);
    virtual void setDefaultValue();

    virtual bool save();
    virtual bool load();
    virtual bool erase();
    virtual bool copyMemory();

    virtual bool sendRecipe();
    virtual bool loadRecipe(LPCTSTR name);
    virtual bool getRecipeFilename(LPTSTR target);
    virtual bool saveRecipe();
    virtual bool loadActMemRecipe();

    virtual LPCTSTR getRecipeExt();
//  private:
    bool Send();

    genericPerif* getMemoryPrph();
    void openText();

    void copyFromClipboard();
    void fillRowByClipboard(LPTSTR lptstr);

    void copyToClipboard();
    int  fieldToClipboard(PVect<LPCTSTR>& set);
    virtual void prepareBitToFunct();
    bool converted;
    void convertRecipe();
  private:
    typedef P_DefBody baseClass;
    pVariable bitToFunct;

};
//----------------------------------------------------------------------------
extern void makeNewRecipeFile(LPTSTR file);
extern void makePathRecipeNew(setOfString& sStr, LPTSTR path, LPCTSTR file, bool history, LPCTSTR ext, bool addNewExt = true);
extern void makePathRecipeNew(setOfString& sStr, LPTSTR path, bool history, LPCTSTR ext, bool addNewExt = true);
//----------------------------------------------------------------------------
#endif

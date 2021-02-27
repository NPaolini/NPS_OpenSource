//--------------- svmDialogVariable.h -----------------------------
//-----------------------------------------------------------
#ifndef svmDialogVariable_H_
#define svmDialogVariable_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmDataProject.h"
#include "pcustomlistviewfulledit.h"
//-----------------------------------------------------------
struct infoVariable
{
  uint id;
  uint prph;
  uint addr;
  uint type;
  uint dec_len_offs;
  uint nBit;
  int norm_id;
  bool useId;
  infoVariable() : id(0), prph(0), addr(0), type(0), dec_len_offs(0), nBit(0), norm_id(0), useId(false) {}
  infoVariable(uint id, uint prph, uint addr, uint type, uint dec_len_offs, int norm_id, uint n_bit = 0);
};
//-----------------------------------------------------------
struct infoVariableText
{
  uint id;
  LPCTSTR text;
  infoVariableText() : id(0), text(0) {}
  ~infoVariableText() { delete []text; }
};
//-----------------------------------------------------------
class manageVariable
{
  public:
    manageVariable();
    ~manageVariable();

    PVect<infoVariable>& getVariables() { return Vars; }
    PVect<infoVariableText*>& getVariablesText() { return VarsText; }

    void reset();
    void load(LPCTSTR filename, const uint* idset);
    bool save(LPCTSTR filename, uint* idset);

  private:
    PVect<infoVariable> Vars;
    PVect<infoVariableText*> VarsText;
    infoVariableText* findText(uint id);
};
//-----------------------------------------------------------
manageVariable& getManVariables();
//-----------------------------------------------------------
class svmDialogVariable : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmDialogVariable(PWin* parent, uint id, int& selected, HINSTANCE hInst = 0) :
      baseClass(parent, id, hInst), ListVar(0), ListText(0), Selected(selected) {  }
    ~svmDialogVariable() { destroy(); }

    virtual bool create();
  protected:
    PCustomListView* ListVar;
    PCustomListViewFullEdit* ListText;
    int& Selected;
    static bool returnId;
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType) { return 0; }
    void CmOk();
    void CmCancel();
    void makeHead();
    void fillList();
    void fillListVars();

    void addRowVar(HWND hlv, const infoVariable& row, uint item = 65535);
    int addRowText(HWND hlv, const infoVariableText& row, uint item = 65535);
    void getRowVar(HWND hlv, infoVariable& row, int ix);
    void getRowText(HWND hlv, infoVariableText& row, int ix);
    bool checkNotExist(const infoVariable& row);

    void toField(const infoVariable& row);
    void fromField(infoVariable& row);

    void checkAll();

    void addField();
    void remField();
    void addText();
    void remText();
    void loadField();

    void Copy();
    void Paste();

};
//-----------------------------------------------------------
#endif

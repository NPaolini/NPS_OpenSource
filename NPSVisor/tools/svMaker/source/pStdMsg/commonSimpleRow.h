//--------- commonSimpleRow.h ---------------------------------------
#ifndef commonSimpleRow_H_
#define commonSimpleRow_H_
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "infoText.h"
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
//-------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MAX_GROUP_SR 8
//----------------------------------------------------------------------------
#define MAX_TEXT_SR (1024 * 2)
//----------------------------------------------------------------------------
#define OFFSET_DESCR_SR  10
//----------------------------------------------------------------------------
class PEditRow
{
  public:

    PEditRow(PWin* parent, uint first_id);
    virtual ~PEditRow() {}

    void getDescr(LPTSTR buff) const;
    void setDescr(LPCTSTR buff) const;
    DWORD getID();
    void setID(DWORD db);

  protected:
    PStatic* ID;
    PEdit* Descr;
    uint firstId;

};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class PEditFirstRow : public PEditRow
{
  public:

    PEditFirstRow(PWin* parent, uint first_id) : PEditRow(parent, first_id)
    {
      delete Descr;
      Descr = new firstEdit(parent, firstId + OFFSET_DESCR_SR);
    }
};
//----------------------------------------------------------------------------
class PEditLastRow : public PEditRow
{
  public:

    PEditLastRow(PWin* parent, uint first_id) : PEditRow(parent, first_id)
    {
      delete Descr;
      Descr = new lastEdit(parent, firstId + OFFSET_DESCR_SR);
    }
};
//----------------------------------------------------------------------------
struct baseRow
{
  DWORD id;
  TCHAR text[MAX_TEXT_SR];
  baseRow() : id(0) { text[0] = 0; }
};
//----------------------------------------------------------------------------
inline
void PEditRow::getDescr(LPTSTR buff) const
{
  GetWindowText(*Descr, buff, MAX_TEXT_SR - 1);
}
//----------------------------------------------------------------------------
inline
void PEditRow::setDescr(LPCTSTR buff) const
{
  SetWindowText(*Descr, buff);
}
//----------------------------------------------------------------------------
inline
DWORD PEditRow::getID()
{
  TCHAR buff[50];
  GetWindowText(*ID, buff, SIZE_A(buff));
  return _ttol(buff);
}
//----------------------------------------------------------------------------
inline
void PEditRow::setID(DWORD val)
{
  TCHAR buff[50];
  wsprintf(buff, _T("%d"), val);
  SetWindowText(*ID, buff);
}
//----------------------------------------------------------------------------
#endif
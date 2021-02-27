//-------------------- svVisualTextEdit.cpp -----------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svVisualTextEdit.h"
#include "pVarEdit.h"
//-------------------------------------
//-------------------------------------
void svVisualTextEdit::setFocus()
{
  baseClass::setFocus();
  SendMessage(*Owner, EM_SETSEL, 0, (LPARAM)-1);
}
//-------------------------------------
void svVisualTextEdit::killFocus()
{
  baseClass::killFocus();
  if(SendMessage(*Owner, EM_GETMODIFY, 0, 0)) {
    PVarEdit* ed = dynamic_cast<PVarEdit*>(Owner);
    if(ed)
      ed->getOwner()->resetTimeEdit();
    }
}
//-------------------------------------
void svVisualTextEdit::setData(LPCTSTR buff)
{
  baseClass::setData(buff);
  if(onFocus)
    SendMessage(*Owner, EM_SETSEL, 0, (LPARAM)-1);
}
//-------------------------------------
//-------------------------------------
void svVisualTextEditSep::setFocus()
{
  baseClass::setFocus();
  SendMessage(*Owner, EM_SETMODIFY, 0, 0);
  SendMessage(*Owner, EM_SETSEL, 0, (LPARAM)-1);
}
//-------------------------------------
void svVisualTextEditSep::killFocus()
{
  int modified = SendMessage(*Owner, EM_GETMODIFY, 0, 0);
  baseClass::killFocus();
  if(modified) {
    PVarEdit* ed = dynamic_cast<PVarEdit*>(Owner);
    if(ed)
      ed->getOwner()->resetTimeEdit();
    }
  else
    SendMessage(*Owner, EM_SETMODIFY, 0, 0);
}
//-------------------------------------
void svVisualTextEditSep::setData(LPCTSTR buff)
{
  baseClass::setData(buff);
  if(onFocus)
    SendMessage(*Owner, EM_SETSEL, 0, (LPARAM)-1);
}
//-------------------------------------

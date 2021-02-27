//------ headerMsg.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef HEADERMSG_H_
#define HEADERMSG_H_
//----------------------------------------------------------------------------
// messaggi riservati per il framework
#define WM_FW_FIRST (WM_APP  + 1)
#define WM_FW_LAST  (WM_APP  + 50)
//----------------------------------------------------------------------------
// da qui iniziano i messaggi disponibili per le applicazioni
#define WM_FW_FIRST_FREE  (WM_FW_LAST + 1)
//----------------------------------------------------------------------------
// messaggi del fw
#define WM_C_CHANGED_SELECTION (WM_APP + 1)
#define WM_C_DELETE_SELECTION (WM_C_CHANGED_SELECTION + 1)
//----------------------------------------------------------------------------
#endif
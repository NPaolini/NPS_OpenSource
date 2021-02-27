//-------- sv_make_dll.h ------------------------------------------------
#ifndef SV_MAKE_DLL_H_
#define SV_MAKE_DLL_H_
//------------------------------------------------------------------
#ifdef USE_SV_DLL
  #error occorre definire MAKE_SV_DLL
#endif
#ifndef MAKE_SV_DLL
  #error occorre definire MAKE_SV_DLL
#endif
//-----------------------------------------------------------
#include "precHeader.h"
//------------------------------------------------------------------
#include "sv_dll.h"
//------------------------------------------------------------------
extern HINSTANCE getHinstance();
//------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------
// routine presenti nella dll richiamate dal motore
//------------------------------------------------------------------
// Poiché viene caricata a runtime (la dll può essere omessa) le routine
// devono avere questo nome preciso.

// richiamata all'interno del refresh generale (non permette il
// rientro durante l'elaborazione, viene settato un flag di
// elaborazione in corso)

ExpImp_DLL
void npSV_Refresh();
//------------------------------------------------------------------
//------------------------------------------------------------------
// la routine disponibile per le pagine contiene l'id della pagina ed il
// nome del file di testo della pagina.
// nella dll si esegue una verifica dell'id, se è un id generico allora
// l'identificazione della pagina viene presa dal nome, altrimenti è
// l'id stesso che identifica la pagina (più eventualmente il nome).
//------------------------------------------------------------------
ExpImp_DLL
bool npSV_RefreshBody(uint idBody, LPCTSTR pageName, bool force);
// se torna false non viene eseguita la routine base.
// force indica se è necessario un refresh forzato
//------------------------------------------------------------------
//------------------------------------------------------------------
enum msg4Dll { eINIT_DLL = 1, eEND_DLL };
// richiamata all'inizio e al termine con i codici eINIT_DLL e eEND_DLL
// serve per eventuali inizializzazioni-terminazioni.

ExpImp_DLL
void npSV_System(DWORD msg);
//------------------------------------------------------------------
// puntatori a routine presenti nel sVisor e caricate a runtime
//------------------------------------------------------------------
#ifndef DEF_PTR_DEFINE
  #define DEF_PTR(a) extern npSV_##a##_ npSV_##a
#else
  #define DEF_PTR(a) npSV_##a##_ npSV_##a
#endif
//------------------------------------------------------------------
typedef LPCTSTR (*npSV_GetGlobalString_)(uint id);
DEF_PTR(GetGlobalString);
//------------------------------------------------------------------
typedef LPCTSTR (*npSV_GetLocalString_)(uint id, HINSTANCE hdll);
DEF_PTR(GetLocalString);
//------------------------------------------------------------------
typedef LPCTSTR (*npSV_GetPageString_)(uint id);
DEF_PTR(GetPageString);
//------------------------------------------------------------------
typedef SV_prfData::tResultData (*npSV_Set_)(uint idPrf, const pSV_prfData data);
DEF_PTR(Set);
//------------------------------------------------------------------
typedef SV_prfData::tResultData (*npSV_GetSet_)(uint idPrf, pSV_prfData data);
DEF_PTR(GetSet);
//------------------------------------------------------------------
typedef SV_prfData::tResultData (*npSV_Get_)(uint idPrf, pSV_prfData data);
DEF_PTR(Get);
//------------------------------------------------------------------
typedef SV_prfData::tResultData (*npSV_MultiSet_)(uint idPrf, const pSV_prfData data, uint nData);
DEF_PTR(MultiSet);
//------------------------------------------------------------------
typedef fREALDATA (*npSV_GetNormaliz_)(uint id);
DEF_PTR(GetNormaliz);
//------------------------------------------------------------------
//------------------------------------------------------------------
// routine generale per comunicare col body corrente e richiedere
// servizi generici
//------------------------------------------------------------------
// per ora l'uso principale può essere quello di creare propri dialoghi derivati
// da svModDialog per cui si necessita dell'oggetto del body corrente
// es.
// LRESULT pBody = SV_GetBodyRoutine(eSV_GET_BODY, 0, 0);
// // va fatto il cast
// P_Body* body = (P_Body*)(HANDLE_BODY)pBody;
// oppure
// PWin* body = (PWin*)(HANDLE_BODY)pBody;
//------------------------------------------------------------------
typedef LRESULT (*npSV_GetBodyRoutine_)(uint codeMsg, LPDWORD p1, LPDWORD p2);
DEF_PTR(GetBodyRoutine);
//------------------------------------------------------------------
#ifndef IDD_STANDARD
  //------------------------------------------------------------------
  // riservato, non usare
  #define IDD_EMPTY         1003
  //------------------------------------------------------------------
  // usato dai body aperti solo tramite pagina di testo
  #define IDD_STANDARD      1001
  //------------------------------------------------------------------
  // id usati dalle pagine predefinite
  #define IDD_LGRAPH2       1023
  #define IDD_RECIPE        1029
  // fino a 1039 è riservato per eventuali recipe personalizzate
  // per ora è usata solo la (IDD_RECIPE + 1) per ricette con listbox
  #define IDD_RECIPE_ROW    1040
  #define IDD_MODELESS_BODY 1041
  #define IDD_MODAL_BODY    1042
  //------------------------------------------------------------------
#endif
//------------------------------------------------------------------
#define PRF_MEMORY 1
//------------------------------------------------------------------
#define MAX_PERIFS 20
//------------------------------------------------------------------
#define MAX_ID_PERIFS (MAX_PERIFS + PRF_MEMORY)
//------------------------------------------------------------------
//------------------------------------------------------------------
#ifdef __cplusplus
 }
#endif
//------------------------------------------------------------------
#endif

//-------- config.h ----------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef CONFIG_H_
#define CONFIG_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//----------------------------------------------------------------------------
#define MAX_PERIF 8
//----------------------------------------------------------------------------
#ifndef PARAMCOM_H_
  #include "paramcom.h"
#endif

#include "setPack.h"
//----------------------------------------------------------------------------
#define SIZE_BASE ( MAX_PERIF * \
                      (_MAX_PATH * sizeof(char) + \
                        sizeof(paramCom) + \
                        sizeof(BYTE)  \
                      ) + \
                    sizeof(DWORD) \
                  )
//----------------------------------------------------------------------------
#define LIMIT_SIZE (0x1000 * sizeof(TCHAR))
//----------------------------------------------------------------------------
//#if (SIZE_BASE >= LIMIT_SIZE)
//  #define SIZE_DISP 0x200
//#else
  #define SIZE_DISP (LIMIT_SIZE - SIZE_BASE)
//#endif
//----------------------------------------------------------------------------
#define SIZE_DISP_DWORD (SIZE_DISP / (4 * sizeof(DWORD)))
//----------------------------------------------------------------------------
//#if (SIZE_DISP_DWORD < 2)
//  #define SIZE_DISP_DWORD 2
//  #define SIZE_DISP (SIZE_DISP_DWORD * 16)
//#endif
//----------------------------------------------------------------------------
#define SIZE_DISP_WORD (SIZE_DISP_DWORD * 3)
//----------------------------------------------------------------------------
// sembra ci sia un disallineamento e che vengano scritti 6 byte oltre 0x1000
// probabilmente la causa è il byte usato per i campi di bit
// per non spostare il byte (compatibilità con i dati precedenti)
// si diminuisce la disponibilità dei byte di SUB_DISP_BYTE (è di sei).
//#define SUB_DISP_BYTE 6
#define SUB_DISP_BYTE 0
//----------------------------------------------------------------------------
#define SIZE_DISP_BYTE (SIZE_DISP - SIZE_DISP_WORD * sizeof(WORD) \
                          - SIZE_DISP_DWORD * sizeof(DWORD) - SUB_DISP_BYTE)
//----------------------------------------------------------------------------
// struttura di configurazione generale
struct cfg
{
 // nomi dei programmi di gestione della comunicazione
  char pathPerif[MAX_PERIF][_MAX_PATH];

  // parametri della periferica
  paramCom parPerif[MAX_PERIF];

// nel borland, forse per ottimizzazione, vengono salvati solo due byte anziché quattro e per compatibilità ...
  WORD useMouse        :1; // attiva/disattiva la visualizzazione del mouse
  WORD shutDown        :1; // se attivo, spegne il pc all'uscita
  WORD logAllAlarm     :2; // flag per il report particolareggiato di tutti gli allarmi
                            // 0->nessuno, 1->solo allarmi, 2->tutti
  WORD numLock         :1; // per il tastierino numero

  WORD hideTaskBar     :1; // nasconde la taskBar
  WORD disableAltKey   :1; // disattiva il tasto ALT
  WORD disableCtrlKey  :1; //     "      "   "   CTRL
  WORD disableWinKey   :1; //     "      "   "   WinKey

  WORD useCardOper     :1; // attiva/disattiva l'uso della carta magnetica
  WORD disableDLL      :1; // se attivo non fa caricare le dll

  WORD disablePoweroff :1; // se attivo non esegue il poweroff, necessario per pc che non hanno lo spegnimento automatico

  WORD noAdvancedReport:1; // se attivo non visualizza controlli avanzati nella scelta del report allarmi

  WORD noShowPeriph    :1; // se attivo avvia i driver nascosti

  // byte di abilitazione della periferica
  BYTE Perif[MAX_PERIF];

    // spazio a disposizione per altre variabili dipendenti dal tipo
    // di supervisore
  DWORD dwDisp[SIZE_DISP_DWORD]; // ~1/4 dello spazio libero
  WORD wDisp[SIZE_DISP_WORD];    // ~3/8 dello spazio libero
  BYTE bDisp[SIZE_DISP_BYTE];    // il rimanente spazio libero (~3/8)
};
//----------------------------------------------------------------------------
class config
{
  public:
    config();
      // l'implementazione dipende dal supervisore
    bool setup(class P_Body *w);

    void setup(const cfg& newcfg);
    static const cfg& getAll() { return Cfg; }

    void reload();
  private:
    static cfg Cfg;
    static bool Updated;

    // l'implementazione dipende dal supervisore
    // viene chiamata dal costruttore ma non è implementata.
    // serve per inizializzare valori quando il file non esiste.
    static void Init();
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif


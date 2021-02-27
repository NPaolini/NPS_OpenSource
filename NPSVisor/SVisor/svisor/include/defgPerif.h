//--------------- defgPerif.h -----------------------------------
#ifndef DEFGPERIF_H_
#define DEFGPERIF_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef DEFIN_H_
  #include "defin.h"
#endif
//-----------------------------------------------------------------------------
// dimensione massima dati della Periferica e del file di scambio (in DATA_XXX)
// per fornire più flessibilità nella gestione dei dati
#define MAX_BDATA_PERIF (4 * 4096 * 4)
#define MAX_WORD_PERIF  (MAX_BDATA_PERIF / 2)
#define MAX_DWORD_PERIF (MAX_WORD_PERIF / 2)

// numero di dword per periferica memoria
#define SIZE_OTHER_DATA   MAX_DWORD_PERIF
//-----------------------------------------------------------------------------
// la dimensione base è in BDATA
#define MAX_DIM_PERIF (MAX_BDATA_PERIF)
//-----------------------------------------------------------------------------
// dimensione massima file per dati e comandi di comunicazione da inviare alla Perif
#define MAX_DIM_FILE (MAX_DIM_PERIF * 2)
//-----------------------------------------------------------------------------
// N.B. Tutti gli indirizzi provenienti dal svisor sono logici.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define PhtoLg(a, b)  (WDATA((a) / (sizeof(b##DATA) / sizeof(BDATA))))
#define W_PhtoLg(a)  PhtoLg(a, W)
#define DW_PhtoLg(a)  PhtoLg(a, DW)
//-----------------------------------------------------------------------------
// macro per la creazione di flag e nomi dei file in base al numero della periferica
#define FLAG_PERIF(a)         ((WORD)(0xCAFE + (a)))
#define UNFLAG_PERIF(a)         ((WORD)((a) - 0xCAFE))
//-----------------------------------------------------------------------------
#define MK_NAME_FILE_DATA(buff, id) wsprintf(buff, _T("prph_%d_data.dat"), id)
//#define MK_NAME_FILE_COMMAND(buff, id)  wsprintf(buff, _T("prph_%d_command.dat"), id)

// per il file di salvataggio della coda
#define MK_NAME_FILE_COMMAND_QUEUE(buff, prph)  wsprintf(buff, _T("prph_%d_command_queue.dat"), prph)

// nuovo nome per file di comandi, con l'aggiunta dell'id
#define MK_NAME_FILE_COMMAND(buff, prph, id)  wsprintf(buff, _T("prph_%d_command_%d.dat"), prph, id)
//-----------------------------------------------------------------------------
#define MK_NAME_CLASS(buff, id)   wsprintf(buff, _T("PRPH_%d_CommonClass"), id)
//-----------------------------------------------------------------------------
#define MK_NAME_MSG(buff, id)   wsprintf(buff, _T("WM_PRPH_%d"), id)
//-----------------------------------------------------------------------------
#endif

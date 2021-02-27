//------------- lnk_Body.h ------------------------------
#ifndef LNK_BODY_H_
#define LNK_BODY_H_

#ifndef P_DEFBODY_H_
  #include "p_defbody.h"
#endif
#include "setPack.h"

//  Ogni Body conosce solo l'Id del Body child da chiamare.
//  Viene passato anche l'Id del chiamante, che deve essere
//  memorizzato in una variabile statica (esterna alla classe
//  poiché deve sopravvivere all'oggetto).
//  L'Id del chiamante ha lo scopo di poter tornare al Body
//  precedente senza difficoltà.
//  Se l'Id del chiamante ha il valore zero, significa che non
//  si richiede una sottofinestra, ma si desidera tornare al
//  Body precedente (in genere col pulsante F1)

//  Per evitare di avere troppi Id, si usano gli stessi Id di
//  dialogo corrispondenti al Body e presenti nel file .RH

//  Per evitare troppi riferimenti incrociati, che comporterebbero
//  ricompilazioni non necessarie, ogni body definisce una funzione,
//  esterna alla classe, sul tipo della getBody() che instanzia il Body
//  richiesto. Nel file contenente il codice della
//  getBody() si aggiunge in un vettore di Id l'Id del Body e in un
//  vettore di puntatori a funzioni la funzione corrispondente

//  Tipo di funzione esportata dai vari Body.
//  parent deve essere la parent del Body chiamante, non this
typedef P_Body* fnBody(int idPar, PWin *parent);

extern P_Body *getBody(int idNewBody, PWin *parent, int idPar = 0 );

//---------------------------------------------------------------------
// nuova specifica. Si può inserire nel file di testo di descrizione
// della pagina il nuovo file di testo da richiamare se si tratta
// di una pagina generica, altrimenti l'id codificato nel programma.
// Uso nel file di testo:
//    cod,idAction,idCustom,fileTextPage
//  dove cod          => corrisponde al codice pulsante + 12
//       idAction     => codice azione
//       idCustom     => id dipendente dall'azione
//       fileTextPage => nome del file di descrizione da richiamare (*)

//  idAction:
//  1 => apertura nuova pagina, segue l'id codificato. Se pagina
//       generica l'id deve essere zero e segue il nome del file,
//       altrimenti l'id != da zero e non deve seguire nulla.

//  2 => invia tutti i campi di edit abbinati alla periferica,
//       segue l'id della periferica
//  3 => riceve tutti i campi di edit abbinati alla periferica
//       segue l'id della periferica
//  4 => esegue l'export dei dati. Segue uno se da storico, zero per dati correnti

// questa routine si trova in libreria, viene richiamata direttamente da
// P_DefBody e non occorre richiamarla
extern P_Body *getBody(PWin *parent, LPCTSTR newPageDescr, LPCTSTR currPageDescr, int newId = 0, int oldId = 0);

// routine predefinita per export dati. Deve essere implementata (anche vuota se non necessaria)
extern void exportData(PWin* parent, bool history);
#include "restorePack.h"
#endif

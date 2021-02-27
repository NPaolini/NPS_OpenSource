Specifiche routine calcolo espressioni
--------------------------------------

* operatori e relativa priorità (in ordine crescente) *

  level 0 -> operatori logici
      0_0 ->    ||               (or)
      0_1 ->    ^^               (xor)
      0_2 ->    &&               (and)
  level 1 -> operatori a bit
      1_0 ->    |                (or)
      1_1 ->    ^                (xor)
      1_2 ->    &                (and)
  level 2 ->    ==, !=           (uguale, diverso)
  level 3 ->    <, >, <=, >=     (minore, maggiore, minore o uguale, maggiore o uguale)
  level 4 ->    <<, >>           (shift a sinistra, shift a destra)

  level 5 ->    -, +             (sottrazione, addizione)
  level 6 ->    *, /, %          (molt, div, modulo)
  level 7 ->    !, ~, -, (, ), , (not logico, not a bit, inversione segno, parentesi, virgola)
  level 8 ->    digit, $, alpha  (numeri, variabili, funzioni/costanti)


* funzioni definite:
    funzioni senza argomento
    NOW     -> data-ora nel formato int64 da filetime (nanosecondi dal 1601)
    DATE    -> solo data nel formato int64 da filetime
    TIME    -> solo orario nel formato int64 da filetime
    YEAR    -> anno corrente
    MONTH   -> mese corrente
    DAY     -> giorno corrente
    HOUR    -> ora corrente
    MINUTE  -> minuti dell'ora corrente
    SEC     -> secondi

    RAND -> torna un numero casuale tra 0 e RAND_MAX (32767)


    funzioni unarie
    ABS     -> valore assoluto -> abs(expr)
    NEG     -> inverte il segno dell'espr. che segue (uguale a '-' )
    NOTL    -> not logico, risultato -> zero/uno     (uguale a '!' )
    NOTB    -> not a bit, inverte i valori dei bit   (uguale a '~' )
    SIN     -> seno
    COS     -> coseno
    EXP     -> esponenziale
    SQRT    -> radice quadrata
    LOG     -> logaritmo
    TG      -> tangente
    CTG     -> cotangente
    ASIN    -> arcoseno
    ACOS    -> arcocoseno
    ATG     -> arcotangente
    RAD     -> trasforma da gradi in radianti
    GRAD    -> trasforma da radianti in gradi

    SWAB2   -> scambia i byte due a due,         es. ABCD -> BADC
    SWAB4   -> scambia i byte quattro a quattro, es. ABCD -> DCBA
    SWAB8   -> scambia i byte otto a otto,       es. ABCDEFGH -> HGFEDCBA

    TOBCD_LE   -> trasforma il numero in BCD (LittleEndian)
    FROMBCD_LE -> trasforma il BCD in numero (LittleEndian)
    TOBCD_BE   -> trasforma il numero in BCD (BigEndian)
    FROMBCD_BE -> trasforma il BCD in numero (BigEndian)

    funzioni binarie
    POW     -> elevazione a potenza -> POW(a,b) == a elevato b
    SHL     -> shift a sinistra -> SHL(a,b) == a << b (uguale a <<)
    SHR     -> shift a destra                         (uguale a >>)
    DIFF    -> logico, torna true se i valori sono diversi (uguale a !=)

    RANDMINMAX -> torna un valore casuale compreso tra i due valori passati

    funzioni terniarie
    IF      -> il primo argomento è il test, se diverso da zero torna il
            secondo argomento, altrimenti il terzo -> IF(a,b,c)

    funzioni n_arie
    MIN     -> il minimo dei valori -> MIN(a,b,c,d,e,f,ecc)
    MAX     -> il massimo dei valori
    AVR     -> media dei valori
    SUM     -> somma dei valori
    EQU     -> uguaglianza logica, risultato -> zero/uno (simile al '==', ma per una serie di valori)

    ANDL    -> and logico  (simile al '&&', ma per una serie di valori)
    ANDB    -> and binario (simile al '&',  ma per una serie di valori)
    ORL     -> or logico   (simile al '||', ma per una serie di valori)
    ORB     -> or binario  (simile al '|',  ma per una serie di valori)
    XORL    -> xor logico  (simile al '^^', ma per una serie di valori)
    XORB    -> xor binario (simile al '^',  ma per una serie di valori)

    funzioni su blocchi di dati consecutivi, fz($var, ndata, step)
    $var deve essere una variabile (eventualmente un elemento di vettore -> $var[1])
    gli altri valori possono essere costanti o variabili
    BLKSUM  -> simile alla SUM
    BLKAVR  -> simile alla AVR
    BLKDEVSTD -> deviazione standard

    simili alle corrispondenti senza BLK_, ma invece che per i valori passati, agiscono sul blocco dati
    BLK_ORL  -> simile alla ORL
    BLK_ORB  -> simile alla ORB
    BLK_ANDL  -> simile alla ANDL
    BLK_ANDB  -> simile alla ANDB

    fz($var, ndata, step, limiteSup, limiteInf) es. target = 100, sup = 105, inf = 96
    BLK_CMK -> capacità di macchina CMK
    BLK_CPK -> capacità di processo CPK

    fz($var1, step1, $var2, step2, ndata)
    BLK_ORL_2  -> simile alla BLK_ORL ma il confronto è su due blocchi di dati
    BLK_ANDL_2 -> simile alla BLK_ANDL ma il confronto è su due blocchi di dati

    BLK_EQU_2 -> simile alla EQU ma il confronto è su due blocchi di dati
    BLK_DIFF_2 -> simile alla DIFF ma il confronto è su due blocchi di dati

* Costanti definite
    PI      -> assegna il valore pi_greco
    E       -> assegna il valore e -> exp(1)
    FSEC    -> fattore di conversione tra filetime e secondi
    FMIN    -> fattore di conversione tra filetime e minuti
    FHOUR   -> fattore di conversione tra filetime e ore
    NULL    -> usata principalmente nella funzione IF permette di non eseguire
               alcuna operazione di assegnazione.
           Negli altri casi non fa eseguire l'operazione ed il risultato
           è l'altra espressione (es. a = f(b) + NULL -> a = f(b)).

* Gestione variabili:
  La variabile deve iniziare col carattere $ ed essere seguita da un
  numero che la identifica (es. $2). Può essere seguita da una coppia
  di valori racchiusi tra parentesi quadre e questi identificano,
  rispettivamente, l'offset ed il passo (es. $2[3,2]). Questa aggiunta
  permette di avere una variabile vettore. Nel richiamare la routine di
  calcolo si può specificare un numero che identifica il ciclo corrente.
  Quindi, supponendo di richiedere la variabile 2 con ciclo 4 verrebbe
  richiesto il valore alla posizione 11 (3 + 2 * 4) del vettore.
  L'offset è zero based, quindi 11 indica il dodicesimo valore.
  Anche il ciclo è zero based;

  ##########
  Modifica variabili:
  La variabile deve iniziare col carattere $ e può essere seguita da uno o più
  dei seguenti caratteri (a-z, A-Z, 0-9, _). Le variabili "non" sono case sensitive,
  cioè -> $var_xy è uguale a $Var_xy. La lunghezza massima del nome è di 64 caratteri.
----------------------------------------------------------------------
Le funzioni e le costanti possono essere scritte indifferentemente
in maiuscolo o in minuscolo.

Per inserire valori in formato esadecimale occorre iniziare il numero
con uno zero e farlo terminare con il carattere H (es. 02a3bh).

N.B. Se si immettono solo numeri interi il risultato sarà un intero (a meno
     di casi particolari come nelle funzioni trigonometriche o nella media, o..),
     quindi la formula 5 / 2 darà come risultato 2 anche se si richiede il
     real come  valore di ritorno. Per avere il risultato real almeno uno dei
     valori deve essere un real, quindi -> 5 / 2.0 = 2.5
     (è sufficiente inserire il punto per avere un real es. 2.)
----------------------------------------------------------------------
######################################################################
----------------------------------------------------------------------
Formato file script da abbinare alla DLL per l'uso col SVisor.
----------------------------------------------------------------------
Lo script è diviso in blocchi inizianti con <blocco> e terminanti con </blocco>.
L'inizio e fine del blocco deve essere su una riga separata dal resto dei dati.
I blocchi attualmente definiti sono:

<INIT>
...
</INIT>

<VARS>
...
</VARS>

<CALC>
...
</CALC>

Nel blocco <INIT> vanno inseriti eventuali valori da impostare alla prima esecuzione
dello script. Non è possibile impostare valori di un vettore, ma solo della variabile singola.
Ogni variabile deve essere su una propria riga.
es.
<INIT>
$7 = 0.25
$18 = 1
</INIT>

Nel blocco <VARS> vanno inserite le variabili usate dallo script con le informazioni
sulla periferica, indirizzo, tipo di dato, numero di bit, offset e normalizzatore
(gli ultimi tre possono essere omessi se non necessari).
Ogni variabile deve essere su una propria riga.
es.
<VARS>
$1=1,12,4,0,0
...
</VARS>

Nel blocco <CALC> vanno inserite le espressioni di calcolo.
Ogni espressione deve essere su una propria riga.
es.

<CALC>

$1 = day
$2 = month
$3 = year
$4 = hour
$5 = minute
$6 = sec

$8 = $7 * 2
$10 = sin(rad($9))
; vettore -> offset zero, passo uno, otto elementi
$13[0, 1, 8] = $11[0, 1] + $12[0, 1]

; se $18 è diverso da zero alla variabile $14 viene assegnata la somma delle variabili
; $15 e $16, altrimenti non viene eseguita alcuna azione
$14 = if($18, $16 + $15, NULL)

</CALC>
Per commentare una riga contenente variabili occorre anteporre il simbolo ';'
----------------------------------------------------------------------
Nel programma di scrittura e test delle espressioni, nella casella per i valori di test,
è possibile inserire dati per variabili vettore separandoli con una virgola.
es.
$11 = 2,3,4,5,6,7,8
$12 = 1,1,1,1,1,1
Non è necessario inserirli tutti, dove non presenti prenderanno il valore zero.
Ad ogni esecuzione viene rielaborato lo script ed eseguito, quindi non vengono
memorizzati i valori dell'elaborazione precedente. Si possono copiare dal risultato
e incollarli nella sezione di immissione nello stesso formato (ad eccezione del vettore).

Per commentare una riga contenente variabili occorre anteporre il simbolo ';'
----------------------------------------------------------------------
##########
Aggiunte due variabili speciali (non vanno messe nella zona VARS, ma sono
gestite internamente).

La prima è $_SKIP. Il valore assegnato a questa variabile permette di effettuare
salti relativi. In pratica un valore positivo fa saltare righe (di espressioni)
successive, mentre un valore negativo fa rieseguire righe precedenti.
Dovrebbe essere usata in congiunzione con if() per salti condizionati.
Per eseguire un salto incondizionato è sufficiente assegnargli un valore
(o una variabile).
Va usata con cautela perché può generare loop infiniti o, comunque, non torna
il controllo al sVisor finché non termina la sua esecuzione.

La seconda variabile è $_ABORT. Questa, se acquisisce un valore non nullo
non prosegue nell'esecuzione delle righe restanti.

Non possono essere usate con le variabili vettore.

N.B. Prima di questa modifica il nome delle variabili era case sensitive, per
     problemi interni le variabili ora sono case insensitive.
----------------------------------------------------------------------
##########
Aggiunte etichette per salto da usare in congiunzione con $_SKIP.
L'etichetta deve essere preceduta dal carattere '#' e può essere
formata da qualsiasi carattere alfanumerico (più il carattere '_').
Es. di etichette:
#salto1
#di_nuovo
ecc.

Quindi, al posto dei valori relativi, nei salti, si può far riferimento
alle etichette.
Es.
$_skip = if($var < 10, #salto, 0)
----------------------------------------------------------------------
##########
Aggiunta la possibilità di usare variabili non abbinate ad alcuna periferica.
E' sufficiente mettere zero come periferica.
Per comodità viene accettato il formato comune alle variabili normali e cioè
prf,addr,type,ecc.
ma è possibile utilizzare una forma contratta in cui è sufficiente mettere
zero come periferica e poi zero per il tipo intero o diverso da zero per il
tipo reale.

es.
$inMemInt1=0,1,4
$inMemInt2=0,0

$inMemReal1=0,1,5
$inMemReal2=0,1

Le variabili sono valide solo durante l'esecuzione ed il loro valore viene perso
al riavvio dello script.
----------------------------------------------------------------------
##########
Aggiunta la possibilità di mettere la singola espressione su più righe.
E' sufficiente terminare le righe formanti l'espressione (meno l'ultima) con il
carattere \. Non devono esserci righe intermedie né commenti.

es.

$_skip = \
  if($test < 10, null, \
    if($test < 20, #end1, \
      if($test < 30, #end2, \
                       #end3)))

P.S. in questo esempio si nota l'utilità delle variabili in memoria interna.
     Se $test è abbinata a variabile presente in una periferica, ne viene
     richiesto per tre volte il valore al sVisor. Se viene salvata in una
     variabile interna, il suo reperimento risulta più veloce.
----------------------------------------------------------------------
##########
Aggiunta la possibilità di prelevare un dato da array usando una variabile
come indice.
Il formato è:
$value = $array[$index]

P.S. Era già possibile usare il vettore in questo modo, ma solo per indici fissi
es.
$value = $array[2]

N.B Non è possibile usare una espressione all'interno delle parentesi quadre,
    ma solo numeri o variabili
----------------------------------------------------------------------
##########
Aggiunta la possibilità di immettere un dato in un array usando una variabile
come indice.
Il formato è:
$array[$index] = $value

Naturalmente è possibile combinare le due modalità
es.
$array1[$index1] = $array2[$index2]
----------------------------------------------------------------------
##########
Aggiunta la possibilità di creare funzioni (nel nuovo blocco <FUNCT>) che possono
essere richiamate sia dalle funzioni stesse che, naturalmente, dal blocco <CALC>.
Nel blocco <FUNCT> le funzioni, i parametri passati alle funzioni e le variabili locali
alle funzioni devono iniziare col carattere '@'.
Si possono richiamare, all'interno delle funzioni, le variabili globali nel solito modo
e cioè col carattere '$' (anche come parte di array, es. $v1[4] oppure $v1[$v2]).
Le funzioni sono senza side effect, cioè non possono modificare variabili globali (alla
sinistra dell'uguale non può esserci una variabile globale, ma solo locali o parametri).
Anche le due variabili predefinite, _skip e _abort, devono iniziare col carattere '@' e
hanno lo stesso lo stesso comportamento cha hanno nel blocco <CALC>.
Le etichette di salto invece usano lo stesso carattere iniziale e cioè '#'.
Le etichette di salto sono relative alle funzioni e, quindi, è possibile utilizzare lo
stesso nome in funzioni diverse senza che interferiscano tra loro.
Le funzioni sono richiamate alle stesso modo di quelle predefinite, senza alcun carattere
particolare iniziale (possono iniziare con il range [a-z] o col carattere '_', ogni altro
carattere non è permesso.
Il numero di parametri indicati nella funzione non è obbligatorio che vengano passati
tutti al momento di richiamarla, quelli non passati varranno semplicemente zero.
Se si passano più parametri di quelli che si aspetta la funzione non viene segnalato alcun
errore, semplicemente la funzione, non conoscendoli, li ignora.
Il valore viene ritornato, dalla funzione, col classico 'return', le righe di codice
seguenti vengono ignorate. Se si arriva in fondo alla funzione senza aver incontrato alcun
return, viene restituito 'null' (che può essere restituito anche col return).
La definizione della funzione inizia col carattere '@' seguito dal nome e dalle parentesi
tonde che racchiudono i parametri, anch'essi inizianti col carattere '@'.
Il blocco del codice è racchiuso tra le parentesi graffe '{' e '}'. La prima graffa può
anche essere sulla stessa riga dell'inizio funzione (dove ci sono nome e parametri), ma
quella di chiusura deve essere su una riga a se stante, dopo l'ultima riga di codice.
Le funzioni senza parametri devono essere chiamate senza le parentesi tonde, al pari delle
funzioni predefinite, es. 'now' o 'date'.

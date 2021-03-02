Note sul contenuto delle cartelle.

NPfw:
  è la cartella che contiene la mia libreria.
  	fw:
  		è composta nella quasi totalità dalla gestione GUI per Windows
  	various:
  		contiene codice di utilità vario
  		
applic:
	sorgenti di applicazioni varie, utility
	
ntService:
	per creare applicazioni come servizio
	
mpir:
	libreria (non mia) per grandi numeri, usata per un programmino demo di RSA
	
squid_wrap:
	squid è un'applicazione proxy che, inizialmente e teoricamente, dovrebbe essere multipiattaforma.
	L'ultimo porting per windows (binario ufficiale) risale al 2010. C'è stata poi una versione successiva
	compilata in ambiente cygwin ma senza gli helper necessari (almeno per quello che serviva a me).
	Fare il porting delle ultime versioni ... un bagno di sangue, tra l'altro i sorgenti degli helper 
	per windows erano	sempre gli stessi. Visto che comunque gli helper li avevo compilati con successo,
	ho pensato che potevo usare squid su linux, ultimissima versione, senza problemi e per gli helper
	creare un ponte tra linux e windows server. Quindi un semplice programmino su linux che riceve i dati
	da squid sullo stdio, li invia tramite lan ad un servizio su windows il quale riconosce il tipo di helper
	ed avvia, se non già avviato, l'helper richiesto. Gli gira, tramite stdio, la richiesta ricevuta da linux,
	l'helper interroga windows e da il responso al servizio che lo rimanda tramite lan a linux.
	
NPSVisor:
	Supervisione per sistema di automazione industriale (e non solo).
	Il sistema è composto da un programma principale (sVisor) che comunica con driver che si connettono a periferiche.
	Il canale di comunicazione tra sVisor e driver è fatto da un file (normalmente mappato in memoria) contenente
	oltre 16.000 locazioni da 32 bit. Queste locazioni si possono leggere/scrivere tramite oggetti che sono nelle
	pagine (naturalmente anche in altri modi).
	Il sVisor ha codificato della logica di base per funzioni comuni, es. gestione ricette, trend, allarmi, ecc.
	Quando la logica di base non è più sufficiente si può intervenire tramite plugin. Ci sono plugin specifici
	per compiti particolari, altri più generici. Tra i generici ce ne sono tre che sono 'particolari'.
	Uno riguarda l'implementazione di uno pseudo linguaggio. In pratica è un risolutore di espressioni a cui è stato
	aggiunta la capacità di salti ed un IF. C'è anche un editore per poter scrivere il codice.
	L'altro permette di scrivere codice tramite linguaggi di scripting esterni, es. vbscript o jscript o python 
	(se installato activePython).
	IL terzo è un server per poter usare più sVisor sulle stesse periferiche, es. uno in ufficio, uno all'inizio
	del macchinario, un altro alla fine, ecc.
	E, tipicamente, uno è collegato fisicamente	alle periferiche e su cui risiede il plugin server, gli altri
	usano un driver particolare che si collega al server per scrivere/leggere sulle periferiche.
	
	I plugin possono essere implementati a piacimento secondo necessità, c'è un sdk	di esempio da cui prendere spunto.
	
	Certo, il progetto occorre crearlo ... c'è il gestore dei progetti 'NPSvMaker' che lo permette. Si creano
	pagine, ci si mettono oggetti, li si configura, tutto in modalità visuale, senza scrivere codice. Nei manuali,
	prossimi a venire, ci sarà una dettagliata spiegazione.
	
	Ci sono poi altri tool necessari al funzionamento del sistema.

//----------------------------
//----------------------------
function objVar(prph, addr, type, nbit, offs, norm)
{
  this.getVar = function() {  return sVisor.svScriptGet(prph, addr, type, nbit, offs, norm); }
  this.setVar = function(val) { sVisor.svScriptSet(prph, addr, type, nbit, offs, norm, val); }
}
//-------------------------------------------
function objVarAddr(prph, startAddr, type, nbit, offs, norm)
{
  this.getVar = function(addrOffs) {  return sVisor.svScriptGet(prph, startAddr + addrOffs, type, nbit, offs, norm); }
  this.setVar = function(addrOffs, val) { sVisor.svScriptSet(prph, startAddr + addrOffs, type, nbit, offs, norm, val); }
}
//-------------------------------------------
function getCurrtime()
{
  var currentTime = new Date();
  var hours = currentTime.getHours();
  var minutes = currentTime.getMinutes();
  var seconds = currentTime.getSeconds();
  if(hours < 10)
  	hours = "0" + hours;
  if(minutes < 10)
  	minutes = "0" + minutes;
  if(seconds < 10)
  	seconds = "0" + seconds;
  return "" + hours + ":" + minutes + ":" + seconds;
}
//-------------------------------------------
function StringaDataOra(quale)
{
  var ActGiorno = new objVar(1, 200, 4, 0, 0, 0);
  var ActMese = new objVar(1, 201, 4, 0, 0, 0);
  var ActAnno = new objVar(1, 202, 4, 0, 0, 0);
  var ActOra = new objVar(1, 203, 4, 0, 0, 0);
  var ActMinuti = new objVar(1, 204, 4, 0, 0, 0);
  var ActSecondi = new objVar(1, 205, 4, 0, 0, 0);

  if(quale == 1){
    var ora = ActOra.getVar() * 10000 + ActMinuti.getVar() * 100 + ActSecondi.getVar();
    if(ora < 100000)
      return  "0" + ora;
    else
      return  ora;
    }
    
  if(quale == 2)
    return ActAnno.getVar() * 10000 + ActMese.getVar() * 100 + ActGiorno.getVar();

  if(quale == 3)
    return ActGiorno.getVar() + "/" + ActMese.getVar() + "/" + ActAnno.getVar();

  if(quale == 4)
    return ActOra.getVar() + ":" + ActMinuti.getVar() + ":" + ActSecondi.getVar();
    
  ActGiorno = null;
  ActMese = null;
  ActAnno = null;
  ActOra = null;
  ActMinuti = null;
  ActSecondi = null;

}
//-------------------------------------------
function opensourceTxt(name,useDialog,percorsobase,forSave)
{
  if(useDialog) {
// var filename = sVisor.openSaveDialog(filtro, percorso_iniziale (può essere zero), forSave (0 = open, 1 = save));
// N.B. se si immette un percorso occorre anche immettere un file (o il *) completo di estensione
    var filename = sVisor.openSaveDialog("File Excel (xlsx)|*.xlsx|Tutti i File (*.*)|*.*", percorsobase, forSave);
//    var filename = sVisor.openSaveDialog("File Excel (xlsx)|*.xlsx|Tutti i File (*.*)|*.*", "\\\\192.168.0.10\\myProg\\prove\\*.xlsx", 0);
    return filename;
    }

// apre il file di testo contenente il nome del file excel da aprire
  var fso = new ActiveXObject("Scripting.FileSystemObject");
  var filename = null;
  try{
/*
  ForReading -> 1
  ForWriting -> 2
  ForAppending -> 8

  TristateUseDefault -> -2
  TristateTrue -> -1 Opens the file as Unicode.
  TristateFalse -> 0 Opens the file as ASCII.
*/
    var a = fso.OpenTextFile(name, 1, false, -1);
    filename = a.ReadLine(); // solo la prima iga
    } catch(e) {
      sVisor.alert('Error ' + e.description);
      }
  fso = null;
  return filename;
}
//-------------------------------------------
// la stringa è così formata: hh:mm:ss.cc
// uscita in formato timer
function myParseTime(val)
{
  var t = val.split(':');
  var num = parseInt(t[0]) * 60 * 60 * 100;
  num += parseInt(t[1]) * 60 * 100;
  num += parseInt(t[2]) * 100;
  t = t[2].split('.');
  num += parseInt(t[1]);
  num *= 100000;
  return num;
}
//-------------------------------------------
// la stringa è così formata: hh:mm:ss.cc
// uscita in formato hh*3600+mm*60+ss
function myParseTime2(val)
{
  var t = val.split(':');
  var num = parseInt(t[0]) * 60 * 60;
  num += parseInt(t[1]) * 60;
  num += parseInt(t[2]);
  return num;
}
//-------------------------------------------
function myUnparseTime(val)
{
	val /= 100000;
	var hour_ = parseInt((val / (60 * 60 * 100)) % 24);
	var minute_ = parseInt((val / (60 * 100)) % 60);
	var second_ = parseInt((val / 100) % 60);
	var msec_ = parseInt(val % 100);
  var result = '' + hour_ + ':' + minute_ + ':' + second_ + ',' + msec_;
	return result;
}
//-------------------------------------------
function getValue(objArr, ix, addrOffs, fix)
{
	var value = objArr[ix].getVar(addrOffs);
	return value.toFixed(fix);
}
//-------------------------------------------
function showAlert(value)
{
    sVisor.alert('value = ' + value + ', type = ' + typeof value + ', ScriptEngine = ' + ScriptEngine());
}
//-------------------------------------------
function openExcel(name, sheet, initRow, arrayToSave)
{
  var objExcel = new ActiveXObject("Excel.Application");
  var cols = new Array (1,2,4,11,18,25,32,39,46,53);
  var toCol = 10;
  var ColonnaData = cols[0];
  var value = 0;
  var valueTime = 0;
  var valueDword = 0;
  var addr = 0;
  var offsBlock = 4000;

  try{
    objExcel.WorkBooks.Open(name);
    objExcel.ActiveWorkbook.Worksheets(sheet).Activate();
    var finished = false;
    for(var row = initRow, j = 0; j < offsBlock && !finished; ++row, ++j) {
      for(var col = 0, i = 0; col < toCol; ++col, ++i) {
          value = objExcel.Cells(row, cols[col]).Value;
          // se è la prima colonna, contiene l'ora e non può essere vuota, se lo è significa che non ci sono più dati
          if(cols[col] == ColonnaData) {
            if(!value) {
              finished = true;
              break;
              }
            valueTime = myParseTime(value);
            addr = j * 2;
            arrayToSave[i].setVar(addr, valueTime);
            valueDword = myParseTime2(value);
            addr = j;
            arrayToSave[12].setVar(addr, valueDword);
            }
          else
            arrayToSave[i].setVar(j, value);
          }
         }
    } catch(e) {
      sVisor.alert('Error ' + e.description);
      }
  objExcel.Quit();
  objExcel = null;
}
//-------------------------------------------
function writeExcel(name, arrayToSave,TipoSave)
{
  var toCol = 11;

  var stringa1 = new objVar(1, 270, 14, 20, 0, 0);

  var objExcel = new ActiveXObject("Excel.Application");
  var value = 0;
  var initRow = 12;
  var offsBlock = 4000;

  objExcel.Workbooks.Add();
  objExcel.Cells(1, 1).Value = "SEVERSTALMETIZ ROPE PRODUCTION REPORT";
  objExcel.Cells(3, 1).Value = "Rope Number:";
  objExcel.Cells(3, 8).Value = getValue(arrayToSave, 13, 0, 2);
  objExcel.Cells(4, 1).Value = "Date and Time:";
  objExcel.Cells(4, 8).Value = StringaDataOra(3);
  objExcel.Cells(4, 9).Value = getCurrtime();
  //objExcel.Cells(4, 9).Value = StringaDataOra(4);
  objExcel.Cells(5, 1).Value = "Rope Lenght [mt]:";
  objExcel.Cells(5, 8).Value = getValue(arrayToSave, 13, 2, 2);
  objExcel.Cells(6, 1).Value = "Rope Diameter [mm]:";
  objExcel.Cells(6, 8).Value = getValue(arrayToSave, 13, 3, 2);
  objExcel.Cells(7, 1).Value = "Rope Specific Weight per meter [kg/dm3]:";
  objExcel.Cells(7, 8).Value = getValue(arrayToSave, 13, 4, 4);
  objExcel.Cells(8, 1).Value = "Rope Stranding Lay [mm]:";
  objExcel.Cells(8, 8).Value = getValue(arrayToSave, 13, 5, 2);
  objExcel.Cells(9, 1).Value = "Metering of Manifactured Production [Kg]:";
  objExcel.Cells(9, 8).Value = getValue(arrayToSave, 13, 6, 2);
  objExcel.Cells(10, 1).Value = "Operator:";
  objExcel.Cells(10, 8).Value = stringa1.getVar();
  objExcel.Cells(initRow, 1).Value = "Meters [mt]";
  objExcel.Cells(initRow, 2).Value = "Laufzeit [s]";
  objExcel.Cells(initRow, 3).Value = "Spule 1 [N]";
  objExcel.Cells(initRow, 4).Value = "Spule 2 [N]";
  objExcel.Cells(initRow, 5).Value = "Spule 3 [N]";
  objExcel.Cells(initRow, 6).Value = "Spule 4 [N]";
  objExcel.Cells(initRow, 7).Value = "Spule 5 [N]";
  objExcel.Cells(initRow, 8).Value = "Spule 6 [N]";
  objExcel.Cells(initRow, 9).Value = "Spule 7 [N]";
  objExcel.Cells(initRow, 10).Value = "Spule 8 [N]";
  objExcel.Cells(initRow, 11).Value = "Strand Tension [N]";

  try{
    var finished = false;
    for(var row = initRow, j = 0; j < offsBlock && !finished; ++row, ++j) {
      for(var col = 12; col > 0; --col) {
        // se è la prima colonna, contiene l'ora e non può essere vuota, se lo è significa che non ci sono più dati
        if(col == 12) {
          // uso il timer solo per controllare se la riga è vuota. E' il timer tipo 4
          value = arrayToSave[col].getVar(j);
          if(value == 0) {
            finished = true;
            break;
            }
          }
        else {
          value = getValue(arrayToSave, col, j, 2);
          if(col == 11)
            objExcel.Cells(j+initRow+1, 1).Value = value;
          else
            objExcel.Cells(j+initRow+1, col+1).Value = value;
          }
        }
      }
    } catch(e) {
      sVisor.alert('Error ' + e.description);
      }
  if(TipoSave == 1)
    objExcel.ActiveWorkbook.SaveAs(name);
  else 
    objExcel.ActiveWorkbook.SaveAs(name + "_" + StringaDataOra(2) + "_" + StringaDataOra(1) + ".xlsx");
  objExcel.Quit();
  objExcel = null;
  stringa1 = null;
}
//-------------------------------------------
function defLoadExcel()
{
  var ApriFile = new objVar(1, 252, 4, 1, 0, 0);
  var show = ApriFile.getVar();

  if(show) {
    ApriFile.setVar(0);
    ApriFile = null;
    //sVisor.alert('value = ' + show);

    var arrayObj = new Array();
    arrayObj[0] = new objVarAddr(12, 0, 10, 0, 0, 0); // per la data-ora, usa il tipo a 64 bit
    arrayObj[12] = new objVarAddr(12, 8000, 4, 0, 0, 0);
    arrayObj[1] = new objVarAddr(12, 12000, 5, 0, 0, 0);
    arrayObj[2] = new objVarAddr(13, 0, 5, 0, 0, 0);
    arrayObj[3] = new objVarAddr(13, 4000, 5, 0, 0, 0);
    arrayObj[4] = new objVarAddr(13, 8000, 5, 0, 0, 0);
    arrayObj[5] = new objVarAddr(13, 12000, 5, 0, 0, 0);
    arrayObj[6] = new objVarAddr(14, 0, 5, 0, 0, 0);
    arrayObj[7] = new objVarAddr(14, 4000, 5, 0, 0, 0);
    arrayObj[8] = new objVarAddr(14, 8000, 5, 0, 0, 0);
    arrayObj[9] = new objVarAddr(14, 12000, 5, 0, 0, 0);
    arrayObj[10] = new objVarAddr(15, 0, 5, 0, 0, 0);
    arrayObj[11] = new objVarAddr(15, 4000, 5, 0, 0, 0);
    arrayObj[13] = new objVarAddr(1, 260, 5, 0, 0, 0);

    var filename =  opensourceTxt("--",true,"C:\\Progetti Work\\102841 GCR\\102841_Sup\\doc\\*.xlsx",0);

    if(filename){
      openExcel(filename, "RawData", 2, arrayObj);
      var Eseguito = new objVar(1, 253, 4, 0, 0, 0);
      Eseguito.setVar(1);
      Eseguito = null;
      }
    }
}
//-------------------------------------------
function defWriteExcel()
{
  var ScriviFile = new objVar(1, 254, 4, 1, 0, 0);
  var show = ScriviFile.getVar();

  if(show) {
    ScriviFile.setVar(0);
    ScriviFile = null;

    var arrayObj = new Array();
    arrayObj[0] = new objVarAddr(12, 0, 10, 0, 0, 0); // per la data-ora, usa il tipo a 64 bit
    arrayObj[12] = new objVarAddr(12, 8000, 4, 0, 0, 0);
    arrayObj[1] = new objVarAddr(12, 12000, 5, 0, 0, 0);
    arrayObj[2] = new objVarAddr(13, 0, 5, 0, 0, 0);
    arrayObj[3] = new objVarAddr(13, 4000, 5, 0, 0, 0);
    arrayObj[4] = new objVarAddr(13, 8000, 5, 0, 0, 0);
    arrayObj[5] = new objVarAddr(13, 12000, 5, 0, 0, 0);
    arrayObj[6] = new objVarAddr(14, 0, 5, 0, 0, 0);
    arrayObj[7] = new objVarAddr(14, 4000, 5, 0, 0, 0);
    arrayObj[8] = new objVarAddr(14, 8000, 5, 0, 0, 0);
    arrayObj[9] = new objVarAddr(14, 12000, 5, 0, 0, 0);
    arrayObj[10] = new objVarAddr(15, 0, 5, 0, 0, 0);
    arrayObj[11] = new objVarAddr(15, 4000, 5, 0, 0, 0);
    arrayObj[13] = new objVarAddr(1, 260, 5, 0, 0, 0);

    var TipoSaveFile = new objVar(1, 256, 4, 1, 0, 0);
    var TipoSave = TipoSaveFile.getVar();
    TipoSaveFile = null;

    if(TipoSave == 1)
      var filename =  opensourceTxt("--",true,"C:\\Progetti Work\\102841 GCR\\102841_Sup\\export\\*.xlsx",1);
    else
      var filename =  "C:\\Progetti Work\\102841 GCR\\102841_Sup\\export\\export";

    if(filename){
      writeExcel(filename, arrayObj,TipoSave);
      var EseguitoScrivi = new objVar(1, 255, 4, 0, 0, 0);
      EseguitoScrivi.setVar(1);
      EseguitoScrivi = null;
      }
    }
}
//-------------------------------------------
function checkRun()
{
  var Varcheck = new objVar(1, 251, 4, 0, 0, 0);
  var value = Varcheck.getVar();
  //showAlert(value);

  if(value > 1000)
    value = 0;
  else
    ++value;
  Varcheck.setVar(value);
  Varcheck = null;
}
//-------------------------------------------
// inizializzazione variabili globali
//-------------------------------------------

//-------------------------------------------
// MAIN PROGRAM
//-------------------------------------------

checkRun();
defLoadExcel();
defWriteExcel();

// -------------------------------------------


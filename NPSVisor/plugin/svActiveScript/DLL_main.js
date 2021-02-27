//--------- esempio base di script per sVisor -------------------
//-------------------------------------------
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
function objDateVar(prph, addr, type, format)
{
  this.getVar = function() {  return sVisor.svGetDate(prph, addr, type, format); }
  this.setVar = function(val) { sVisor.svSetDate(prph, addr, type, format, val); }
}
//-------------------------------------------
function objDateVarAddr(prph, startAddr, type, format)
{
  this.getVar = function(addrOffs) {  return sVisor.svGetDate(prph, startAddr + addrOffs, type, format); }
  this.setVar = function(addrOffs, val) { sVisor.svSetDate(prph, startAddr + addrOffs, type, format, val); }
}
//-------------------------------------------
function objTimeVar(prph, addr)
{
  this.getVar = function() {  return sVisor.svGetTime(prph, addr); }
  this.setVar = function(val) { sVisor.svSetTime(prph, addr, val); }
}
//-------------------------------------------
function objTimeVarAddr(prph, startAddr)
{
  this.getVar = function(addrOffs) {  return sVisor.svGetTime(prph, startAddr + addrOffs); }
  this.setVar = function(addrOffs, val) { sVisor.svSetTime(prph, startAddr + addrOffs, val); }
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
function showAlert(value)
{
  var t = new objVar(1, 135, 4, 0, 0, 0);
  var show = t.getVar();
  if(show) {
    t.setVar(0);
    t = null;
    sVisor.alert('time = ' + getCurrtime()+ ', value = ' + value + ', type = ' + typeof value + ', ScriptEngine = ' + ScriptEngine());
    }
}
//-------------------------------------------
// inizializzazione variabili globali
//-------------------------------------------
var var1 = new objVar(1, 131, 4, 0, 0, 0);
var var2 = new objVar(1, 132, 4, 0, 0, 0);
var var3 = new objVar(1, 133, 4, 0, 0, 0);
var var4 = new objVar(1, 134, 4, 0, 0, 0);
//-------------------------------------------
//#########################################
// questo blocco viene eseguito solo all'avvio
var value = var1.getVar();
showAlert(value);

if(value > 1000)
  value = 0;
else
  ++value;
var1.setVar(value);
var1 = null;

var value2 = var2.getVar();
value2 += value;
if(value2 > 10000)
  value2 = 0;
var2.setVar(value2);

var3.setVar(value2 + value);
var4.setVar(value2 - value);

// carica la data-ora dall'indirizzo impostato nello stdMsg
var1 = new objDateVar(1, 4, 8, 1);

// data full
var2 = new objDateVar(1, 6000, 8, 1);
// solo data
var3 = new objDateVar(1, 6002, 9, 1);

value = var1.getVar();
var2.setVar(value);
var3.setVar(value);

// testo per caricarci la data così come viene ritornata
var4 = new objVar(1, 6008, 14, 24, 0, 0);
var4.setVar(value);

// carica solo l'ora dall'indirizzo impostato nello stdMsg
var1 = new objTimeVar(1, 4);
value = var1.getVar();
// solo ora
var4 = new objTimeVar(1, 6004);
var4.setVar(value);

// carica la data-ora nel formato nativo
var1 = new objVar(1, 4, 8, 0, 0, 0);
var2 = new objVar(1, 6006, 8, 0, 0, 0);
// la copia usando il formato nativo
var2.setVar(var1.getVar());

var1 = null;
var2 = null;
var3 = null;
var4 = null;
//#########################################
//-------------------------------------------
// viene usata per verificare che lo script sia in funzione
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
//------------------------------
while(sVisor.svCanContinue())
{
  checkRun();
  // tutto quello che c'è da fare
}
// -------------------------------------------

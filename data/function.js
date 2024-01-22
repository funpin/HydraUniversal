var xmlHttp2=createXmlHttpObject();
var xmlHttp=createXmlHttpObject();
function createXmlHttpObject(){
 if(window.XMLHttpRequest){
  xmlHttp=new XMLHttpRequest();
 }else{
  xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');
 }
 return xmlHttp;
}
function load(){
 if((xmlHttp.readyState==0 || xmlHttp.readyState==4) && (xmlHttp2.readyState==0 || xmlHttp2.readyState==4)){
  xmlHttp.open('PUT','/c.json',true);
  xmlHttp.send(null);
  xmlHttp.onload = function(e) {
   jsonResponse=JSON.parse(xmlHttp.responseText);
   loadBlock();
  }
  xmlHttp2.open('PUT','/config.json',true);
  xmlHttp2.send(null);
  xmlHttp2.onload = function(e) {
   jsonResponse=JSON.parse(xmlHttp2.responseText);
   loadBlock2();
  }
 }
}

function loadBlock(data2) {
 data2 = JSON.parse(xmlHttp.responseText);
 data = document.getElementsByTagName('body')[0].innerHTML;
 var new_string;
 for (var key in data2) {
  new_string = data.replace(new RegExp('{{'+key+'}}', 'g'), data2[key]);
  data = new_string;
 }
 document.getElementsByTagName('body')[0].innerHTML = new_string;
}

function loadBlock2(data222) {
 data222 = JSON.parse(xmlHttp2.responseText);
 data22 = document.getElementsByTagName('body')[0].innerHTML;
 var new_string2;
 for (var key2 in data222) {
  new_string2 = data22.replace(new RegExp('{{'+key2+'}}', 'g'), data222[key2]);
  data22 = new_string2;
 }
 document.getElementsByTagName('body')[0].innerHTML = new_string2;
}
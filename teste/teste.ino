#include <ArduinoJson.h>
#include <FS.h>
#include <StreamUtils.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClient.h>


#define APSSID "codebit"
#define APPSK  "roikROIKroik"
//#define APSSID "trojan"
//#define APPSK  "rodrigo3850"
const char *ssid = APSSID;
const char *password = APPSK;
AsyncWebServer server(80);




long uidUsuario = -1;
int tempoExpiracao = 60000; //600000
const char* filename = "/db.json";
String url;
int bloquearPonto;

bool conectarWifi();
void gerarWifi();
String loadListaUsuarios();
bool verificaToken();


const char btEditar[] PROGMEM = R"=====(
<svg version="1.1" id="Capa_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
  viewBox="0 0 399.14 399.14" style="fill:#00ff40; stroke: #000; stroke-width: 5" xml:space="preserve">
  <g>
    <circle cx="108.75" cy="72.98" r="60"/>
    <path d="M192.389,202.238c-19.946-23.984-50.006-39.258-83.639-39.258C48.689,162.98,0,211.669,0,271.73h122.897L192.389,202.238z"/>
    <path d="M300.376,136.676l38.661-38.661l60.104,60.104l-38.661,38.661L300.376,136.676z"/>
    <path d="M110.998,386.16l68.464-19.986l-48.478-48.477L110.998,386.16z"/>
    <path d="M146.381,290.67l132.782-132.782l60.104,60.104L206.485,350.774L146.381,290.67z"/>
  </g>
</svg>
)=====";

const char btExcluir[] PROGMEM = R"=====(
<svg version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
  viewBox="0 0 374.144 374.144" style="fill:#ff0000; stroke: #000; stroke-width: 5" xml:space="preserve">
  <g>
    <circle cx="108.75" cy="67.982" r="60"/>
    <path d="M274.715,167.303c-54.826,0-99.43,44.604-99.43,99.429s44.604,99.43,99.43,99.43s99.43-44.604,99.43-99.43S329.54,167.303,274.715,167.303z M336.215,281.732h-123v-30h123V281.732z"/>
    <path d="M108.75,157.982C48.689,157.982,0,206.671,0,266.732h145.285c0-32.364,11.941-61.991,31.647-84.709C158.281,166.99,134.571,157.982,108.75,157.982z"/>
  </g>
</svg>
)=====";



String processor(const String& var){
  return "rodrigooooo";
}

String processorHome(const String& var){
  if(var =="tabela-conteudo"){
    return loadListaUsuarios();
  }else{
    String ico = btExcluir;
    return "Bem vindo rodrigo ";
  } 
}

String processorAutorizacao(const String& var){
  return "Não autorizado";
}

String processor404(const String& var){
  return "Página não Encontrada";
}

String processorUsuarioSenhaincorreto(const String& var){
  return "0";
}

void onRequest(AsyncWebServerRequest *request){
  request->send(SPIFFS, "/404.html", String(), false, processor404);
}



void setup() {
    Serial.begin(9600);
    SPIFFS.begin();
    Serial.println();
    Serial.println("inicializando SPIFFS ...");
    if (!SPIFFS.begin()) {
      Serial.println("Error SPIFFS!");
      while (1);
    }
    while (!Serial) continue;

  
    

server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
   request->send(SPIFFS, "/login.html", String(), false, processor);
});

server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
   request->send(SPIFFS, "/styles.css", "text/css");
});

server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
    if(request->hasParam("email",true) && request->hasParam("senha",true)){
       String emailU = request->getParam("email",true)->value().c_str();
       String senhaU = request->getParam("senha",true)->value().c_str();

      if(emailU == "a@a" && senhaU == "123"){
        uidUsuario = millis();
        request->redirect("/home?uid=" + String(millis()));
      }else{
        request->send(SPIFFS, "/login.html", String(), false, processorUsuarioSenhaincorreto);
      }
    }else{
      request->send(SPIFFS, "/404.html", String(), false, processorAutorizacao);
    }
});

server.on("/home", HTTP_GET, [](AsyncWebServerRequest *request){
  if(request->hasParam("uid")){
    String uidU = request->getParam("uid")->value().c_str();
    if(uidU == String(uidUsuario)){
       request->send(SPIFFS, "/principal.html", String(), false, processorHome);
    }else{
       request->send(SPIFFS, "/404.html", String(), false, processorAutorizacao);
    }
  }else{
    request->send(SPIFFS, "/404.html", String(), false, processorAutorizacao);
  }
});

server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
  uidUsuario = -1;
  request->redirect("/");
});


server.onNotFound(onRequest);
server.begin();
Serial.println("HTTP server started");
   
if(!conectarWifi()){
  gerarWifi();
}else{
 
}
}

void loop() {
 if(uidUsuario > 0){
  if(!verificaToken()){
    uidUsuario = -1;
  }
 }
}


void gerarWifi(){
  Serial.print("Configurando access point...");
  WiFi.softAP("SmartPonto", "12345678");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP criado, IP address: ");
  Serial.println(myIP);
}

  
 
bool conectarWifi(){
  Serial.println(ssid);
  Serial.println(password);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  long tempo = millis()+ 30000;
  while (WiFi.status() != WL_CONNECTED && millis() <= tempo) {
    delay(500);
    Serial.print(".");
  }

  if(WiFi.status() != WL_CONNECTED){
    return false;
  }else{
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }
}







//-------------------------------------------




String loadListaUsuarios(){
  String tabela;
  StaticJsonDocument<1800> objUsuarios;

  File file = SPIFFS.open("/usuarios2.json", "r");
  if(file){
      ReadBufferingStream bufferedFile{file, 200};
      DeserializationError error = deserializeJson(objUsuarios, bufferedFile);
      if (error) {
       Serial.println("erro");
      }
  }
  file.close();
  JsonObject listaUsuarios = objUsuarios.as<JsonObject>();
  for (JsonPair kv : listaUsuarios) {
      String email = kv.key().c_str();
      JsonObject objUsuario = kv.value().as<JsonObject>();
      String nome = objUsuario["nome"];
      String senha = objUsuario["senha"];
      String uid = objUsuario["uids"];
      tabela += "<tr>";
      tabela += "<td>" + uid + "</td>";
      tabela += "<td>" + nome + "</td>";
      tabela += "<td>" + email + "</td>";
      //tabela += "<td class=\"bts-acao\">"+btAcaoEditar+" "+btAcaoExcluir+"</td>";
      tabela += "</tr>";
  }

  file = SPIFFS.open("/usuarios3.json", "r");
  if(file){
      ReadBufferingStream bufferedFile{file, 200};
      DeserializationError error = deserializeJson(objUsuarios, bufferedFile);
      if (error) {
       Serial.println("erro");
      }
  }
  file.close();
   listaUsuarios = objUsuarios.as<JsonObject>();
   
  for (JsonPair kv : listaUsuarios) {
      String email = kv.key().c_str();
      JsonObject objUsuario = kv.value().as<JsonObject>();
      String nome = objUsuario["nome"];
      String senha = objUsuario["senha"];
      String uid = objUsuario["uids"];
      tabela += "<tr>";
      tabela += "<td>" + uid + "</td>";
      tabela += "<td>" + nome + "</td>";
      tabela += "<td>" + email + "</td>";
      //tabela += "<td class=\"bts-acao\">"+btAcaoEditar+" "+btAcaoExcluir+"</td>";
      tabela += "</tr>";
  }
  
 
  return tabela;
}



bool verificaToken(){
  bool statusToken = false;
   if(uidUsuario!= -1){
    if(millis() <= uidUsuario + tempoExpiracao){
      statusToken = true;
    }
   }
 return statusToken;
}

void carregaConfiguracoes(){
  StaticJsonDocument<500> conf;
}

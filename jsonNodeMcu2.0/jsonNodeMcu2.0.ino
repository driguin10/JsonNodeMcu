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
StaticJsonDocument<500> doc;
long uidUsuario = -1;
int tempoExpiracao = 90000; //600000
String url;
int bloquearPonto;




bool conectarWifi();
void gerarWifi();
bool verificaToken();
String trataNomeArquivo(String file);
JsonObject buscarUsuarioEmail(String email);
JsonObject buscarCartao(String numeroUID);
bool adicionarCartao(String numeroUID, String json);
bool adicionarCartao(String numeroUID, String json);
String loadListaUsuarios();
bool excluirUsuario(String identificacao , bool uid);



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

String processorExcluir(const String& var){
  return "Erro ao excluir usuario";
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



 server.on("/excluir", HTTP_GET, [](AsyncWebServerRequest *request){
    if(request->hasParam("uid") && request->hasParam("email") ){
      String uidU = request->getParam("uid")->value().c_str();
      String email = request->getParam("email")->value().c_str();
      if(uidU == String(uidUsuario)){
         if(excluirUsuario(email,false)){
            request->redirect("/home?uid=" + String(uidUsuario));
          }else{
            request->send(SPIFFS, "/404.html", String(), false, processorExcluir);
          }
      }else{ 
        request->send(SPIFFS, "/404.html", String(), false, processorAutorizacao);
      }
    }else{
      request->send(SPIFFS, "/404.html", String(), false, processorAutorizacao);
    } 
  });
  

  server.onNotFound(onRequest);
  server.begin();
  Serial.println("HTTP server started");
    
    if(!conectarWifi()){
      gerarWifi();
    }else{
     
    }

   /* if(excluirUsuario("rodrigo@codebit.com",false)){
      Serial.println("excluido");
    }else{
      Serial.println("erro");
    }*/
    
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

void novoUsuario(String uid, String nome, String email, String senha){
    JsonObject user = doc["usuarios"].createNestedObject(uid);
    user["nome"] = nome;
    user["email"] = email;
    user["senha"] = senha;
}

bool loadData(){
  JsonObject listaUsuarios = doc["usuarios"].as<JsonObject>();
  if(listaUsuarios.isNull()){
    return false;
  }
   
  String urlData = doc["configuracao"]["apiUrl"];
  if(urlData==""){
    return false;
  }
  url = urlData;
  
  if(doc["configuracao"]["bloquearPonto"].isNull()){
    bloquearPonto = 0;
  }else{
    bloquearPonto = doc["configuracao"]["bloquearPonto"].as<int>();
  }
  return true;
}

String loadListaUsuarios(){
  String tabela="";
  Dir dir = SPIFFS.openDir("/cartoes/");
  while(dir.next()){
    String uid = trataNomeArquivo(dir.fileName());
    JsonObject usuario = buscarCartao(uid);
    if(usuario.size() > 0 ) {
        String nome = usuario["nome"];
        String email = usuario["email"];
        String type = usuario["type"];
        
        String btAcaoEditar = "<div class=\"bt-acao\" id=\"bt-editar\" data-id=\""+uid+"\">"+btEditar+"</div>";
        String btAcaoExcluir = "<div class=\"bt-acao\" id=\"bt-excluir\" data-id=\"uid="+ String(uidUsuario) +"&email="+email+"\">"+btExcluir+"</div>";
    
        tabela += "<tr>";
        tabela += "<td>" + nome + "</td>";
        tabela += "<td id='email'>" + email + "</td>";
        tabela += "<td>" + type + "</td>";
        tabela += "<td class=\"bts-acao\">"+btAcaoEditar+" "+btAcaoExcluir+"</td>";
        tabela += "</tr>";
    }
  }
  return tabela;
}

String trataNomeArquivo(String file){
  String nomeArquivo = file;
  nomeArquivo.replace("/cartoes/","");
  nomeArquivo.replace(".json","");
  return nomeArquivo;
}




/*
JsonObject buscarUsuarioEmail(String email){
  File root = SPIFFS.open("/cartoes/","r");
  File file = root.openNextFile();
  while(file){
    JsonObject usuario = buscarCartao(file.name());
    if(usuario.size() > 0 ) {
      if(usuario["email"] == email){
        return usuario;
      }
    } 
    file = root.openNextFile();
  }
}*/








//funções ok

bool excluirUsuario(String identificacao , bool uid){
  int qt = 0;
  if(uid){
    if(SPIFFS.remove("/cartoes/" + identificacao + ".json")){
      Serial.println(" excluido");
      return true;
    }else{
      Serial.println(" erro");
      return false;
    }
  }else{
      Dir dir = SPIFFS.openDir("/cartoes/");
      while(dir.next()){
        String uid = trataNomeArquivo(dir.fileName());
        JsonObject usuario = buscarCartao(uid);
        if(usuario.size() > 0 ) {
           if(usuario["email"] == identificacao){
              if(SPIFFS.remove("/cartoes/" + uid + ".json")){
                Serial.println("excluido");
                qt++;
              }else{
                Serial.println(" erro ao excluir");
              }
            }
        }
      }
  }

  if(qt>0){
    Serial.println("qt:");
    Serial.print(qt);
    return true;
  }else{
    return false;
     Serial.println("nenhum excluido");
  }
}

JsonObject buscarUsuarioEmail(String email){
  Dir dir = SPIFFS.openDir("/cartoes/");
  while(dir.next()){
    String uid = trataNomeArquivo(dir.fileName());
    JsonObject usuario = buscarCartao(uid);
    if(usuario.size() > 0 ) {
       if(usuario["email"] == email){
        return usuario;
      }
    }
  }
}

JsonObject buscarCartao(String numeroUID){
  JsonObject usuario;
  File file = SPIFFS.open("/cartoes/" + numeroUID + ".json", "r");
  if(file){
    ReadBufferingStream bufferedFile{file, 102};
    DeserializationError error = deserializeJson(doc, bufferedFile);
    if (!error) {
     usuario = doc.as<JsonObject>();
    }
  }
  return usuario;
}

bool adicionarCartao(String numeroUID, String json){
  bool statusFile = false;
  File file = SPIFFS.open("/cartoes/" + numeroUID + ".json", "r");
  if(!file){
     file = SPIFFS.open("/cartoes/" + numeroUID + ".json", "w");
      if(file){
        file.print(json);
        file.close();
        statusFile = true;
      }
  }
  return statusFile;
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

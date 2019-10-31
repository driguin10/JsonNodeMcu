#include <ArduinoJson.h>
#include <FS.h>
#include <StreamUtils.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClient.h>


#define APSSID "codebitt"
#define APPSK  "roikROIKroik"
const char *ssid = APSSID;
const char *password = APPSK;
AsyncWebServer server(80);

StaticJsonDocument<4000> doc;

long uidUsuario = -1;
int tempoExpiracao = 60000; //600000

char json[] = "{\"usuarios\":{\"00000001\":{\"nome\":\"rodrigo\",\"email\":\"rodrigo@codebit.com.br\",\"senha\":\"1234\"},\"00000002\":{\"nome\":\"teste\",\"email\":\"teste@codebit.com.br\",\"senha\":\"4321\"}},\"configuracao\":{\"apiUrl\":\"nerdcompannyapps.com/api/ponto/marcar-ponto\",\"bloquearPonto\":true}}";
char jsonBase[] = "{\"usuarios\":{\"00000000\":{\"nome\":\"nome teste\",\"email\":\"teste@teste.com.br\",\"senha\":\"senhateste\"}},\"configuracao\":{\"apiUrl\":\"api.com\endpoint\",\"bloquearPonto\":true}}";

const char* filename = "/db.json";
String url;
int bloquearPonto;

String processor(const String& var){
  return "rodrigooooo";
}

String processorHome(const String& var){
  Serial.println(var);
  return "Bem vindo rodrigo";
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
        Serial.print("usuario Não autorizado");
      }
    }else{
      Serial.print("Não Permitido");
    }
});

server.on("/home", HTTP_GET, [](AsyncWebServerRequest *request){
  if(request->hasParam("uid")){
    String uidU = request->getParam("uid")->value().c_str();
    if(uidU == String(uidUsuario)){
       request->send(SPIFFS, "/principal.html", String(), false, processorHome);
    }else{
       request->send(200, "text/plain", "Token negado");
    }
  }else{
    request->send(200, "text/plain", "token não existente");
  }
});

server.begin();
Serial.println("HTTP server started");
  
   if(carregarArquivo(filename)){
    if(!loadData()){
      Serial.println("Dados invalidos!!");
    }else{
      /*loadListaUsuarios();
      //findUser("00000002");
      Serial.println();
      Serial.println(obterJsonCompleto());
      removerUsuario("teste@codebit.com.br");
      Serial.println(obterJsonCompleto());
      novoUsuario("123","rod","rod@teste","222");
      Serial.println(obterJsonCompleto());
      UpdateUsuarioEmail("rod@teste","xxx","rodrigooo","rod@teste","12345");
      Serial.println();
      Serial.println(obterJsonCompleto());*/
    }
   }else{
      Serial.println("Erro db não existe!!");
   }
   /*
  Serial.print("teste: ");
  doc["usuarios"]["00000001"]["nome"] = "rods";
  
  String n = doc["usuarios"]["00000001"]["nome"];
   Serial.println(n);
  
    String output;
    serializeJson(doc, output);
    criaArquivo(output);*/
    if(!conectarWifi()){
      gerarWifi();
    }else{
     
    }
}

void loop() {
 // server.handleClient();
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


//-----------criar---------------------------

//criar novo usuario
void novoUsuario(String uid, String nome, String email, String senha){
    JsonObject user = doc["usuarios"].createNestedObject(uid);
    user["nome"] = nome;
    user["email"] = email;
    user["senha"] = senha;
}

//-----------------------------------------



//-----------Buscas--------------------------

//busca usuario por uid
JsonObject buscaUsuarioUid(String uid){
  JsonObject usuario = doc["usuarios"][uid].as<JsonObject>();
  return usuario;
}

//busca usuario por email
JsonObject buscaUsuarioEmail(String email){
  JsonObject listaUsuarios = doc["usuarios"].as<JsonObject>();
  for (JsonPair kv : listaUsuarios) {
      JsonObject usuario = kv.value().as<JsonObject>();
      if(email == usuario["email"]){
       return usuario;
      }
  }
}

//retorna todos os usuarios
JsonObject listarUsuarios(){
  JsonObject listaUsuarios = doc["usuarios"].as<JsonObject>();
  return listaUsuarios;
}

//retorna json inteiro
String obterJsonCompleto(){
  String js;
  serializeJson(doc, js);
  return js;
}

//-------------------------------------------


//-----------Update--------------------------
JsonObject UpdateUsuarioEmail(String emailBusca, String uid, String nome, String email, String senha ){
  JsonObject listaUsuarios = doc["usuarios"].as<JsonObject>();
  for (JsonPair kv : listaUsuarios) {
      JsonObject usuario = kv.value().as<JsonObject>();
      if(emailBusca == usuario["email"]){
          removerUsuario(emailBusca);
          novoUsuario(uid,nome,email,senha);
      }
  }
}

//-------------------------------------------

//remove usuario por email
bool removerUsuario(String email){
  bool removido = false;
  JsonObject listaUsuarios = doc["usuarios"].as<JsonObject>();
  for (JsonPair kv : listaUsuarios) {
      JsonObject objUsuario = kv.value().as<JsonObject>();
      if(email == objUsuario["email"]){
        listaUsuarios.remove(kv.key().c_str());
        removido = true;
      }
  }
  return removido;
}

//-------------------------------------------

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

void loadListaUsuarios(){
  JsonObject listaUsuarios = doc["usuarios"].as<JsonObject>();
  Serial.println("--------------");
  
  for (JsonPair kv : listaUsuarios) {
      String uid = kv.key().c_str();
      JsonObject objUsuario = kv.value().as<JsonObject>();
      String nome = objUsuario["nome"];
      String email = objUsuario["email"];
      String senha = objUsuario["senha"];
      
      Serial.println("uid: " + uid);
      Serial.println("nome: " + nome);
      Serial.println("enaul: " + email);
      Serial.println("senha: " + senha);
      Serial.println("--------------");
  }
  
  Serial.println("-----Configurações---------");
  String urlData = doc["configuracao"]["apiUrl"];
  url = urlData;
  bloquearPonto = doc["configuracao"]["bloquearPonto"].as<int>();
  Serial.println("api: " + url);
  Serial.print("Bloquear:");
  Serial.println(bloquearPonto);
}

bool carregarArquivo(String arquivo){
  bool statusFile = true;
  File file = SPIFFS.open(arquivo, "r");
  if(file){
      ReadBufferingStream bufferedFile{file, 102};
      DeserializationError error = deserializeJson(doc, bufferedFile);
      if (error) {
        statusFile = false;
      }
      file.close();
  }else{
    statusFile = false;
  }
  return statusFile;
}

bool criaArquivo(String arquivo,String json){
    File file = SPIFFS.open(arquivo, "w");
    if(file){
      file.print(json);
      file.close();
      return true;
    }else{
      return false;
    }
}

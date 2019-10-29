#include <ArduinoJson.h>
#include <FS.h>
#include <StreamUtils.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#define APSSID "SmartPontoDoor"
#define APPSK  "codehell666"
const char *ssid = APSSID;
const char *password = APPSK;
ESP8266WebServer server(80);


StaticJsonDocument<4000> doc;


char json[] = "{\"usuarios\":{\"00000001\":{\"nome\":\"rodrigo\",\"email\":\"rodrigo@codebit.com.br\",\"senha\":\"1234\"},\"00000002\":{\"nome\":\"teste\",\"email\":\"teste@codebit.com.br\",\"senha\":\"4321\"}},\"configuracao\":{\"apiUrl\":\"nerdcompannyapps.com/api/ponto/marcar-ponto\",\"bloquearPonto\":true}}";
char jsonBase[] = "{\"usuarios\":{\"00000000\":{\"nome\":\"nome teste\",\"email\":\"teste@teste.com.br\",\"senha\":\"senhateste\"}},\"configuracao\":{\"apiUrl\":\"api.com\endpoint\",\"bloquearPonto\":true}}";

const char* filename = "/db.json";
String url;
int bloquearPonto;


void handleRoot() {                          // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", "<form action=\"/login\" method=\"POST\"><input type=\"text\" name=\"username\" placeholder=\"Username\"></br><input type=\"password\" name=\"password\" placeholder=\"Password\"></br><input type=\"submit\" value=\"Login\"></form><p>Try 'John Doe' and 'password123' ...</p>");
}

void handleLogin() {                         // If a POST request is made to URI /login
  if( ! server.hasArg("username") || ! server.hasArg("password") 
      || server.arg("username") == NULL || server.arg("password") == NULL) { // If the POST request doesn't have username and password data
    server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
    return;
  }
  if(server.arg("username") == "rodrigo" && server.arg("password") == "123") { // If both the username and the password are correct


JsonObject listaUsuarios = doc["usuarios"].as<JsonObject>();

  String x = server.arg("username") +"<br> <ul>";
  
  for (JsonPair kv : listaUsuarios) {
      String uid = kv.key().c_str();
      JsonObject objUsuario = kv.value().as<JsonObject>();
      String nome = objUsuario["nome"];
      String email = objUsuario["email"];
      String senha = objUsuario["senha"];

      x+="<li> uid: " + uid + " - nome: " + nome + " - email: " + email + " - senha: " + senha+"</li>";
      
  }
  x+="</ul>";
    
    server.send(200, "text/html", x);
  
  
  
  
  
  
  
  
  
  } else {                                                                              // Username and password don't match
    server.send(401, "text/plain", "401: Unauthorized");
  }
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
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

  //gerarWifi();
  conectarWifi();
    
  server.on("/", HTTP_GET, handleRoot);        // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/login", HTTP_POST, handleLogin); // Call the 'handleLogin' function when a POST request is made to URI "/login"
  server.onNotFound(handleNotFound);    
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
}

void loop() {
  server.handleClient();
}

void gerarWifi(){
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}

void conectarWifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin("codebit", "roikROIKroik");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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

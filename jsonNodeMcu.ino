#include <ArduinoJson.h>
#include <FS.h>
#include <StreamUtils.h>
StaticJsonDocument<4000> doc;


char json[] = "{\"usuarios\":{\"00000001\":{\"nome\":\"rodrigo\",\"email\":\"rodrigo@codebit.com.br\",\"senha\":\"1234\"},\"00000002\":{\"nome\":\"teste\",\"email\":\"teste@codebit.com.br\",\"senha\":\"4321\"}},\"configuracao\":{\"apiUrl\":\"nerdcompannyapps.com/api/ponto/marcar-ponto\",\"bloquearPonto\":true}}";
char jsonBase[] = "{\"usuarios\":{\"00000000\":{\"nome\":\"nome teste\",\"email\":\"teste@teste.com.br\",\"senha\":\"senhateste\"}},\"configuracao\":{\"apiUrl\":\"api.com\endpoint\",\"bloquearPonto\":true}}";

const char* filename = "/db.json";
String url;
int bloquearPonto;




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

   if(carregarArquivo(filename)){
    if(!loadData()){
      Serial.println("Dados invalidos!!");
    }else{
      loadListaUsuarios();
      
      //findUser("00000002");

     

      
      Serial.println();
     
      Serial.println(obterJsonCompleto());

      
      removeUsuario("teste@codebit.com.br");

       Serial.println(obterJsonCompleto());
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
}



String obterJsonCompleto(){
  String js;
  serializeJson(doc, js);
  return js;
}


bool removeUsuario(String email){
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

JsonObject findUserUid(String UID){
  JsonObject Usuario = doc["usuarios"][UID].as<JsonObject>();
  return Usuario;
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

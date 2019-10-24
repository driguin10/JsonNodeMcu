#include <ArduinoJson.h>

void setup() {
  Serial.begin(9600);
  while (!Serial) continue;

  StaticJsonDocument<4000> doc;


  char json[] = "{\"usuarios\":{\"00000001\":{\"nome\":\"rodrigo\",\"email\":\"rodrigo@codebit.com.br\",\"senha\":\"1234\"},\"00000002\":{\"nome\":\"teste\",\"email\":\"teste@codebit.com.br\",\"senha\":\"4321\"}},\"configuracao\":{\"apiUrl\":\"nerdcompannyapps.com/api/ponto/marcar-ponto\",\"bloquearPonto\":true}}";


  DeserializationError error = deserializeJson(doc, json);
  
 

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

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


 String url = doc["configuracao"]["apiUrl"];
 int bloquearPonto = doc["configuracao"]["bloquearPonto"].as<int>();
 
 Serial.println("api: " + url);
 Serial.print("Bloquear:");
 Serial.println(bloquearPonto);

 
 
JsonObject Usuario = listaUsuarios["00000001"].as<JsonObject>();

if(!Usuario.isNull()){
  String nome = Usuario["nome"];
  Serial.println(nome);
}else{
  Serial.println("não localizado");
}

  
  
}

void loop() {
  
}

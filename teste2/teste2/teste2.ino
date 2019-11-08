#include <ArduinoJson.h>
#include <FS.h>
#include <StreamUtils.h>


StaticJsonDocument<5454> cartoes;




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

   if(carregarArquivo()){
      Serial.println("Ok");
      String usuario = cartoes["abcde180"].as<String>();
      Serial.println("usuario:"+usuario);
      
   }else{
    Serial.println("Error arquivo");
   }
}

void loop() {
 
}




bool carregarArquivo(){
  bool statusFile = true;
  File file = SPIFFS.open("/cartoes.json", "r");
  if(file){
      ReadBufferingStream bufferedFile{file, 102};
      DeserializationError error = deserializeJson(cartoes, bufferedFile);
      if (error) {
        statusFile = false;
      }
      file.close();
  }else{
    statusFile = false;
  }
  return statusFile;
}

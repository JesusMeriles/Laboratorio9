#include <WiFi.h>  
#include "config.h"
#include <Arduino_JSON.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
/*
 * Sensor de Luz
 */
 #define ADC_VREF_mV    3300.0 // 3.3v en millivoltios
#define ADC_RESOLUTION 4096.0
#define LIGHT_SENSOR_PIN       36 
int datoADC;
float porcentaje=0.0;
float factor=100.0/ADC_RESOLUTION;

boolean var1;
/*
 * Sensor de presencia
 */
 const int PIN_TO_SENSOR = 21; // GIOP19 pin conectado al sensor
int pinStateCurrent   = LOW;  // estado actual
int pinStatePrevious  = LOW;  // estado previo

 #define COUNT_LOW 1638
 #define COUNT_HIGH 7864
 #define TIMER_WIDTH 16
 int i;

 /*
  * PWMVALUE
  */
#define PWM1_Ch    0    //canales
#define PWM1_Res   8    //resolucion    
#define PWM1_Freq  1000  //frecuencia
#define PWM2_Ch    1
#define PWM2_Res   8
#define PWM2_Freq  1000
#define PWM3_Ch    2
#define PWM3_Res   8
#define PWM3_Freq  1000
int pwm1,pwm2,pwm3;
int PWM1_DutyCycle = 0;


 int ahora1 = 1638;

int ahora2 =1638;
String pwmValue;
 

int rele = 18;
int ledRojo = 19;
int ledVerde = 22;
int ledAzul = 21;
 

AsyncWebServer server(80);


                 //Definimos el puerto de comunicaciones
// Variable to store the HTTP request
String header;
 

JSONVar info;
JSONVar sensorP;
JSONVar sensorL;





void initWiFi() {
// conectamos al Wi-Fi
  WiFi.begin(ssid, password);
  // Mientras no se conecte, mantenemos un bucle con reintentos sucesivos
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      // Esperamos un segundo
      Serial.println("Conectando a la red WiFi..");
    }
  Serial.println();
  Serial.println(WiFi.SSID());
  Serial.print("Direccion IP:\t");
  // Imprimimos la ip que le ha dado nuestro router
  Serial.println(WiFi.localIP());
}


 String getinfo(){

  info["dip"] = String(WiFi.localIP());
  info["status"] =  String(WiFi.status());
  info["ssid"] =  String(WiFi.SSID());
  info["rsid"] =  WiFi.RSSI();
  String jsonString = JSON.stringify(info);
  return jsonString;
}
String getlight(){

  info["dip"] = String(WiFi.localIP());
  info["status"] =  String(WiFi.status());
  info["ssid"] =  String(WiFi.SSID());
  info["rsid"] =  WiFi.RSSI();
  String jsonString = JSON.stringify(info);
  return jsonString;
}
String getpres(){

  sensorP["dip"] = String(WiFi.localIP());
 sensorP["status"] =  String(WiFi.status());
  sensorP["ssid"] =  String(WiFi.SSID());
  sensorP["rsid"] =  WiFi.RSSI();
  String jsonString = JSON.stringify(info);
  return jsonString;
}


void setup() {


  Serial.begin(115200); // inicializando el pouerto serial

  ledcAttachPin(rele, PWM1_Ch);
  ledcSetup(PWM1_Ch, PWM1_Freq, PWM1_Res);


  pinMode(PIN_TO_SENSOR, INPUT); // Configurando el pin como entrada//sensor de mov
  
  pinMode(rele,OUTPUT);
  pinMode(ledRojo,OUTPUT);
pinMode(ledVerde,OUTPUT);
pinMode(ledAzul,OUTPUT);

//  DS18B20.begin();      // initializando el sensor DS18B20
  initWiFi();

 // digitalWrite(PinLedR, LOW); digitalWrite(PinLedG, LOW); digitalWrite(PinLedB, LOW);  
  if(!SPIFFS.begin())
     { Serial.println("ha ocurrido un error al montar SPIFFS");
       return; }
  
  //Serial.println(WiFi.localIP());
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html",String(), false);
  });    
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/style.css", "text/css");
            });      
            /*  
server.on("/ADC", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });
  */

   
  
  
   server.on("/INFO", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getinfo();
    request->send(200, "application/json", json);
    json = String();
  });

server.on("/ON", HTTP_GET, [](AsyncWebServerRequest *request){
            digitalWrite(rele,HIGH);

             //String json = getserv();
             Serial.print("Encendido");
           request->send(0);
   // json = String();
            });
server.on("/OFF", HTTP_GET, [](AsyncWebServerRequest *request){
            digitalWrite(rele,LOW);
            
             //String json = getserv();
            Serial.print("Apagado");
           request->send(0);
   // json = String();
            });
server.on("/SLIDER", HTTP_POST, [](AsyncWebServerRequest *request){
            pwmValue = request->arg("bomb");
            Serial.print("PWM:\t");
            Serial.println(pwmValue);
            ledcWrite(PWM1_Ch, pwmValue.toInt()); 
            request->redirect("/");
                  
            });  

server.on("/LUZ", HTTP_GET, [](AsyncWebServerRequest *request){
            digitalWrite(rele,HIGH);

            datoADC = analogRead(LIGHT_SENSOR_PIN);
              //porcentaje=factor*datoADC;
              Serial.print("Valor Analogico = ");
              Serial.print(datoADC);   
              Serial.print("  Porcentaje = ");
              Serial.print(porcentaje);  
              if (datoADC < 40) {
                Serial.println("%  => Oscuro");
              } else if (datoADC < 800) {
                Serial.println("% => Tenue");
              } else if (datoADC < 2000) {
                Serial.println("% => Claro");
              } else if (datoADC < 3200) {
                Serial.println("% => Luminoso");
              } else {
                Serial.println("% => Muy Luminoso");
              }
    
            Serial.print("Apagado");
           request->send(0);
   // json = String();
            });

server.on("/MOVON", HTTP_GET, [](AsyncWebServerRequest *request){
  var1=true;
  while (var1=true){
 pinStatePrevious = pinStateCurrent;             // salvando el estado anteorior
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);   // leyendo el estado nuevo

  if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {   // Cambio de estado del pin: LOW -> HIGH
    Serial.println("Movimiento detectado!");
    digitalWrite(rele,LOW);
    // Todo lo que se quiera hacer en este estado
  }
  else
  if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {   // Cambio de estadi del pin: HIGH -> LOW
    digitalWrite(rele,HIGH);
    Serial.println("Sin movimiento!");
    // Todo lo que se quiera hacer en este estado
  }
  }
  request->send(0);
   // json = String();
            });
server.on("/MOVOFF", HTTP_GET, [](AsyncWebServerRequest *request){
  var1=false;
  while (var1=true){
 pinStatePrevious = pinStateCurrent;             // salvando el estado anteorior
  pinStateCurrent = digitalRead(PIN_TO_SENSOR);   // leyendo el estado nuevo

  if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {   // Cambio de estado del pin: LOW -> HIGH
    Serial.println("Movimiento detectado!");
    digitalWrite(rele,LOW);
    // Todo lo que se quiera hacer en este estado
  }
  else
  if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {   // Cambio de estadi del pin: HIGH -> LOW
    digitalWrite(rele,HIGH);
    Serial.println("Sin movimiento!");
    // Todo lo que se quiera hacer en este estado
  }
  }
  request->send(0);
   // json = String();
            });
           
  
  server.begin();

  
  
}  

void loop(){
 
}

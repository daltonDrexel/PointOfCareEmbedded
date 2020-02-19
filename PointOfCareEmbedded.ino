#include <math.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


const int heatSensorPin = A0;  // Analog input pin that senses Vout
const int heaterControlPin = 16;  
const int ledOnePin = 17;  
const int ledTwoPin = 18;  
int sensorValue = 0;       // sensorPin default value
float Vin = 3.3;           // Input voltage
float Vout = 0;            // Vout default value
float Rref = 9900;         // Reference resistor's value in ohms (you can give this value in kiloohms or megaohms - the resistance of the tested resistor will be given in the same units)
float R = 0;               // Tested resistors default value
float Ro = 11080;          
float To = 294.15;
float B = 3950;
float T = 0;




IPAddress ip(69, 69, 69, 69); 
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0); 

bool heatOn = false;
bool informedPhone = false;

const char* ssid = "NodeET";
const char* password = "123456789";

ESP8266WebServer server(80);

void exciteLEDBois(){
  digitalWrite(ledOnePin, HIGH);
  digitalWrite(ledTwoPin, HIGH);
}

void turnOffLEDBois(){
  digitalWrite(ledOnePin, LOW);
  digitalWrite(ledTwoPin, LOW);
}

void informPhone(){
  server.send(200, "text/plain", "Chip up to temperature");
}

void startNDV(){
  Serial.println("inndvsecond");
  while(true){
  //T = senseT();
  heatOn = T <=65;
  if(heatOn){
  analogWrite(16,1023);  
  }
  else{
  analogWrite(16,0);  
    if(!informedPhone){
    informPhone();
    informedPhone = true;
    Serial.println("Sent 200");
    }
  }
  Serial.print("T: ");                         
  Serial.println(T);
  delay(250);
  }
}

float senseT(){
  sensorValue = analogRead(heatSensorPin);  // Read Vout on analog input pin A0 (Arduino can sense from 0-1023, 1023 is 5V)
  Vout = (Vin * sensorValue) / 1023;    // Convert Vout to volts
  R = Vout*Rref/(Vin-Vout);
  float temp = (log(R/Ro)/B);
  float tempp = temp + (1/To);
  float ret = (1/tempp) - 273.15;

  //Added code to account for DeltaT present in thermistor
  ret = ret+3;
  return ret;
  }
 
void handleRoot() {
  Serial.println("inrootfunc");
}
void handleNDV(){
  Serial.println("inndvroot");
  startNDV();
}

void setup ()
{
  Serial.begin(9600);      // Initialize serial communications at 9600 bps

  //D0 Heater Control Pin
  pinMode(heaterControlPin, OUTPUT);
  //D4 LED Pin
  pinMode(ledOnePin, OUTPUT);
  //D3 LED Pin
  pinMode(ledTwoPin, OUTPUT);

  //Wifi Soft AP Setup will start the server and respond to GET requests, must be disconnected from data on phone end. 
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  //SoftAP server routes
  server.on ( "/", handleRoot );
  server.on ( "/ndv", handleNDV );
  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
    } );
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop ()
{
  server.handleClient(); //Handling of incoming requests
}

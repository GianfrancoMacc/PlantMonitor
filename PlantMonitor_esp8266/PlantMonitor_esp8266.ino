/*********
 * Plant monitor in a esp8266
 * Read values from  a moisture sensor, DHT11 and display them in a web server with additional neoPixel strip form visual 
 * 
 * Code by:
 * Gianfranco Maccagnan 
 * Project team:
 * Gianfranco Maccagnan, Jack Hadlow and Fernando Fernandez
 * 
 * DHT code based by: Rui Santos at https://randomnerdtutorials.com  
 * ESP8266 Wifi based on the Wifi Client example
*********/
#include <Adafruit_NeoPixel.h> //for the neoPixel controls
#include <ESP8266WiFi.h>//wifi connection for web server
#include "DHT.h"        //DHT11 thermometer and moisture

#define DHTTYPE DHT11   // DHT 11
#define PIN 14 //neopixel pin

//starting neoPixel strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, PIN, NEO_GRB + NEO_KHZ800);

//moisture sensor variables
int sensorPin = A0;
int sensorValue = 0;
int percent = 0;

//Use your network details
const char* ssid = "---------------";
const char* password = "------------";

// Web Server on port 80
WiFiServer server(80);

// DHT Sensor
const int DHTPin = 4;
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Temporary variables for DHT sensor
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];


//Strings for the client print in the web server
String font = "https://fonts.googleapis.com/css?family=Nunito:400,800";
String tempId = "tempId";
String temp= "temp";
String humId = "humId";
String hum=  "hum";
String moistId = "moistId";
String moist=  "moist";




// only runs once on boot
void setup() {
  
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);

  //Initialize NeoPixel
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  //Initialize dht
  dht.begin();
  
  // Connecting to WiFi network
  Serial.println();

  //set neoPixel to purple when connecting
  strip.setPixelColor(0, strip.Color(150,0,150));
  strip.setPixelColor(1, strip.Color(150,0,150));
  strip.setPixelColor(2, strip.Color(150,0,150));
  strip.show();

  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Starting the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(10000);
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());

  
}

// runs over and over again
void loop() {
  
  // Looking for new clients
  WiFiClient client = server.available();

  
  
  
  if (client) {
    Serial.println("New client");
    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();


        if (c == '\n' && blank_line) {
          
            // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
            float h = dht.readHumidity();
            // Read temperature as Celsius (the default)
            float t = dht.readTemperature();
            // Read temperature as Fahrenheit (isFahrenheit = true)
            float f = dht.readTemperature(true);

            sensorValue = analogRead(sensorPin);
            percent = map(sensorValue, 1023, 465, 0, 100);

            
            // Check if any reads failed and exit early (to try again).
            if (isnan(h) || isnan(t) || isnan(f)) {
              Serial.println("Failed to read from DHT sensor!");
              strcpy(celsiusTemp,"Failed");
              strcpy(fahrenheitTemp, "Failed");
              strcpy(humidityTemp, "Failed");         
            }

           
            else{
              // Computes temperature values in Celsius and Humidity
              float hic = dht.computeHeatIndex(t, h, false);       
              dtostrf(hic, 6, 2, celsiusTemp);             
              float hif = dht.computeHeatIndex(f, h);
              dtostrf(hif, 6, 2, fahrenheitTemp);         
              dtostrf(h, 6, 2, humidityTemp);
              // You can delete the following Serial.print's, it's just for debugging purposes
              Serial.print("Humidity: ");
              Serial.print(h);
              Serial.print(" %\t Temperature: ");
              Serial.print(t);
              Serial.print(" *C ");
              Serial.print("Moisture: ");
              Serial.print(percent);
              Serial.print("% ");


              if(percent < 20){
//              //neoPixel low water colour
              strip.setPixelColor(0, strip.Color(200,0,0));
              strip.setPixelColor(1, strip.Color(200,0,0));
              strip.setPixelColor(2, strip.Color(200,0,0));
              strip.show();
            }
            if(percent < 50 && percent > 20){
//              //neoPixel medium water colour
              strip.setPixelColor(0, strip.Color(200,140,0));
              strip.setPixelColor(1, strip.Color(200,140,0));
              strip.setPixelColor(2, strip.Color(200,140,0));
              strip.show();
            }
            if(percent < 80 && percent >50){
//              //neoPixel moderate water colour
              strip.setPixelColor(0, strip.Color(0,0,150));
              strip.setPixelColor(1, strip.Color(0,0,150));
              strip.setPixelColor(2, strip.Color(0,0,150));
              strip.show();
            }
            if(percent > 80){
//              //neoPixel high water colour
              strip.setPixelColor(0, strip.Color(0,150,0));
              strip.setPixelColor(1, strip.Color(0,150,0));
              strip.setPixelColor(2, strip.Color(0,150,0));
              strip.show();
            }

        }
            // Web server print-out
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            
            //Print webpage displaying Temperature, humidity and moisture
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head><link href="+(font)+" rel="+"stylesheet"+"></head>");
                    
            client.println("<body><h1>Bulb</h1><h3 id="+tempId+">Temperature: </h3>");
            client.println("<h2 id="+temp+">"+celsiusTemp+"</h2>");

            client.println("<h3 id="+humId+">Humidity: </h3>");
            client.println("<h2 id="+hum+">"+fahrenheitTemp+"</h2>");

            client.println("<h3 id="+moistId+">Moisture: </h3>");
            client.println("<h2 id="+moist+">"+percent+"%</h2>");

            client.println("</body></html>");
            //extreamly long line of css just for show
            client.println("<style>body{ margin: 0; background-color: #AFEDAE; } h1{ text-align: center; font-family: 'Nunito', sans-serif; background-color: #9593C2; color: #172736; margin: 0; padding: 20px 0 20px 0; font-weight: 800; font-size: 2.5em; } p{ text-align: center; font-family: 'Nunito', sans-serif; font-size: 1.5em; color: #172736; margin: 0; padding: 20px 0 20px 0; background-color: #D9D1A2; } #temp{ padding: 0 0 40px 0; background-color: #8EDD97; font-weight: 800; } #tempId{ background-color: #8EDD97; padding: 20px 0 0 0; font-weight: 400; } #hum{ padding: 0 0 40px 0; background-color: #74BA72; font-weight: 800; } #humId{ background-color: #74BA72; padding: 20px 0 0 0; font-weight: 400; } #moist{ padding: 0 0 40px 0; background-color: #6F966A; font-weight: 800; } #moistId{ background-color: #6F966A; padding: 20px 0 0 0; font-weight: 400; } h2{ text-align: center; font-family: 'Nunito', sans-serif; font-size: 3em; color: #172736; margin: 0; } h3{ font-family: 'Nunito', sans-serif; text-align: center; font-size: 2em; color: #172736; margin: 0; }</style>");     
            break;
        }
        if (c == '\n') {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // when finds a character on the current line
          blank_line = false;
        }
      }
      
    }  
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
  
}   
//converting moisture values into percentages
int convertToPercent(int value)
{
  int percentValue = 0;
  percentValue = map(value, 1023, 465, 0, 100);
  return percentValue;
}


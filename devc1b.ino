#include <WiFi.h>
#include <DHT.h>

const char* ssid = "MTD01-AP";
const char* pass = "username";

WiFiServer server(80);
String httpRequestHeader;

String output18State = "off";  // off = high
const int relebomba = 18;

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN,DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.print("Setting Bomba pin...");
  pinMode(relebomba, OUTPUT);
  digitalWrite(relebomba, HIGH);
  Serial.print("Setting DHT...");
  dht.begin();
  Serial.println("Setting AP (Access Point)");
  WiFi.softAP(ssid,pass);
  IPAddress IP = WiFi.softAPIP();
  Serial.println("AP IP address: ");
  Serial.print(IP);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if(isnan(h) || isnan(t)){
    Serial.println(F("Failed to read from dht sensor"));
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%  Temperature: ");
  Serial.print(t);
  
  if(client){                             
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        httpRequestHeader += c;
        if(c == '\n'){
          if(currentLine.length() == 0){
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Switch WaterPump State
	          if(httpRequestHeader.indexOf("GET /18/on") >= 0){
              Serial.println("GPIO 18 on");
              output18State = "on";
              digitalWrite(relebomba, LOW);
            }else if(httpRequestHeader.indexOf("GET /18/off") >= 0){
              Serial.println("GPIO 18 off");
              output18State = "off";
              digitalWrite(relebomba, HIGH);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            // <head>
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">"); 
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            // <body>
            client.println("<body><h1>MTD-01</h1>");
            client.println("<hr></hr>");   
            client.println("<h4>Water Pump</h4>");      
            //client.println("<p>GPIO 18 - State " + output18State + "</p>");      
            if (output18State=="off") {
              client.println("<p><a href=\"/18/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/18/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            client.println("<hr></hr>");   
            client.println("<h4>DHT</h4>");      
            client.println("<p>Temperature --> " + String(t) + " C</p>");     
            client.println("<p>Humidity ----->" + String(h) + " %</p>");
            // </body>
            client.println("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    httpRequestHeader = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

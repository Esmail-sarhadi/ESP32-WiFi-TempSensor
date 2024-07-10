#include <WiFi.h>
#include <WiFiUdp.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "HUAWEI";
const char* password = "sam6612599";

int numNode = 2;
float numPeriod = 3000;
int numCounter = 0;
long previousMillis = 0;
float numTemperature;
float numHumidity;
String postData;
int WiFi_RSSI;

WiFiUDP Udp;
IPAddress localDevIP(192, 168, 8, 111);
IPAddress remoteDevIP(192, 168, 8, 102);
IPAddress gateway(192, 168, 8, 1);
IPAddress subnet(255, 255, 255, 0);
unsigned int localUdpPort = 61500;                                        //Local port to listen on.
unsigned int remoteUdpPort = 61501;
char incomingPacket[255];                                                 //Buffer for incoming packets.
char replyPacket[50];                                                     //A reply string to send back.
//****************************************************************************************************
void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_OFF);                                                  //Prevents reconnection issue.
  delay(1000);
  WiFi.mode(WIFI_STA);                                                  //This line hides the viewing of ESP as wifi hotspot.

  delay(500);
  WiFi.config(localDevIP, gateway, subnet);
  delay(500);

  Serial.printf(">> Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(">> Connected.");
  WiFi_RSSI = WiFi.RSSI();
  Serial.print(">> RSSI:");
  Serial.println(WiFi_RSSI);

  Udp.begin(localUdpPort);
  Serial.printf(">> Now sending data from IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  dht.begin();
}
//****************************************************************************************************
void loop()
{
  unsigned long currentMillis = millis();
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    //Receive incoming UDP packets.
    Serial.printf(">> Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf(">> UDP packet contents: %s\n", incomingPacket);
    numPeriod = atof (incomingPacket);
    Serial.print(">> Period number is: ");
    Serial.println(numPeriod, 6);
  }

  if (currentMillis - previousMillis > numPeriod)
  {
    previousMillis = currentMillis;                                       

    numTemperature = dht.readTemperature();
    numHumidity = dht.readHumidity();

    Serial.print(">> Temperature: ");
    Serial.println(numTemperature, 2);
    Serial.print(">> Humidity: ");
    Serial.println(numHumidity, 2);

    WiFi_RSSI = WiFi.RSSI();

    numCounter += 1;
    postData = String("Node=") + numNode + " & Counter=" + numCounter + " & Temperature=" + String(numTemperature, 2) + " & Humidity=" + String(numHumidity, 2) + " & RSSI=" + String(WiFi_RSSI, DEC);           //POST Data
    postData.toCharArray(replyPacket, 70);

    //Send back a reply, to the IP address and port we got the packet from.
    Udp.beginPacket(remoteDevIP , remoteUdpPort);
    Serial.print(">> IP: ");
    Serial.print(remoteDevIP);
    Serial.print(" & Port: ");
    Serial.println(remoteUdpPort);
    Udp.print(replyPacket);
    Udp.endPacket();
    Serial.println(">> UDP process is done.");

    delay(100);
    Serial.println("********************************************************************");
  }
}
//****************************************************************************************************

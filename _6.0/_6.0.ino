#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <AltSoftSerial.h>


// gps
AltSoftSerial myGPS; // RX (GPS TP, DP 8 def), TX (GPS RX, DP 9 def)
TinyGPS gps; // create gps object

long lat, lon; // create variable for latitude and longitude object
unsigned long DelayTime = 30000, ReadTime = 7000;
String Arsp, Grsp;
bool encoded;


//sim

//SIM800 TX is connected to Arduino D2
#define SIM800_TX_PIN 2

//SIM800 RX is connected to Arduino D3
#define SIM800_RX_PIN 3

//Create software serial object to communicate with SIM800
SoftwareSerial SIM(SIM800_TX_PIN, SIM800_RX_PIN);



void SendLoc(bool &encoded);
void ReadMode();

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(57600);

  //Being serial communication with Arduino and SIM800
  SIM.begin(4800);
  delay(500);
  SIM.println("AT\n"); //checking
  delay(1000);
  //Set SMS format to ASCII
  SIM.write("AT+CMGF=1\r\n");
  delay(1000);
  // gps
  myGPS.begin(9600);
  //Serial.println("Goodnight moon!");
  Serial.println("Setup Complete!");
  // set the data rate for the SoftwareSerial port

}

void loop() { // run over and over
  encoded = false;
  //myGPS.listen();
  delay(1000);
  if(millis()%DelayTime < 3000){
    while (!encoded) {
      SendLoc(encoded);

    }
    delay(1000);
  } 
  
  
  if(millis()% ReadTime < 3000){
    ReadMode();
    
    delay(100);
  }
}



void SendLoc(bool &encoded) {
  if (myGPS.available()) {

    char c = myGPS.read();
    if (gps.encode( c )) { // encode gps data
      encoded = true;
      gps.get_position(&lat, &lon); // get latitude and longitude
      //display position
      Serial.print("Position: ");
      Serial.print("lat: "); Serial.print(lat); Serial.print(" "); // print latitude
      Serial.print("lon: "); Serial.println(lon); // print longitude
      String Message = "c " + String(lat, DEC) + " " + String(lon, DEC);
      //Send new SMS command and message number
      SIM.write("AT+CMGS=\"+447936663084\"\r\n");
      delay(1000);
      SIM.write(Message.c_str());
      delay(1000);

      //Send Ctrl+Z / ESC to denote SMS message is complete
      SIM.write((char)26);
      Serial.println("SMS Sent!");
      delay(200);
      SIM.println();
      Serial.println();
    }
  }
}

void ReadMode() {
  SIM.listen();

  Serial.println("reading...");
  Serial.println(SIM.available());
  SIM.write("AT+CNMI=2, 2, 0, 0, 0\n"); //Set to notification for new message, New message indication
  if (SIM.available()) {
    //Check if GSM Send any data
    delay(1000);
    Grsp = SIM.readString(); //Read data received from SIM800L GSM Module
    delay(200);
    Serial.println(Grsp);
    if(!strstr(Grsp.c_str(), "c ")){
      SIM.write("AT+CMGDA=\"DEL ALL\"\n");
      delay(500);
      Serial.println( "All Messages Deleted" );
    }
    if (strstr(Grsp.c_str(), "normal")) {
      DelayTime = 120000;
      ReadTime = 7000;
      Serial.println(DelayTime);
    }
    else if (strstr(Grsp.c_str(), "panic")) {
      DelayTime = 10000;
      ReadTime = 7000;
      Serial.println(DelayTime);
    }
     else if (strstr(Grsp.c_str(), "secure")) {
      DelayTime = 30000;
      ReadTime = 7000;
      Serial.println(DelayTime);
    }
    delay(200);
    SIM.println();
    Serial.println();
  }
}


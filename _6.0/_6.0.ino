#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <AltSoftSerial.h>
#include <LowPower.h>
#include <SPI.h>
#include <ADXL362.h>

// below is for IMU
ADXL362 xl;
int16_t interruptPin = 2;          //Setup ADXL362 interrupt output to Interrupt 0 (digital pin 2)
int16_t interruptStatus = 0;
int16_t XValue, YValue, ZValue, Temperature;


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
      String Message = String(lat, DEC) + " " + String(lon, DEC);
      //Send new SMS command and message number
      SIM.write("AT+CMGS=\"+447541241808\"\r\n");
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
  SIM.write("AT+CNMI=1, 2, 0, 0, 0\n"); //Set to notification for new message, New message indication
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


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(57600);

  //setup IMU thresholds
  xl.begin();
  xl.setupDCActivityInterrupt(300, 10);   // 300 code activity threshold.  With default ODR = 100Hz, time threshold of 10 results in 0.1 second time threshold
  xl.setupDCInactivityInterrupt(80, 200);   // 80 code inactivity threshold.  With default ODR = 100Hz, time threshold of 30 results in 2 second time threshold
  xl.beginMeasure();                      // DO LAST! enable measurement mode
  xl.checkAllControlRegs();
  delay(100);

  //Begin serial communication with Arduino and SIM800
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
  interruptStatus = digitalRead(interruptPin);
  if((interruptStatus != 0) || (millis()%DelayTime < 3000)) {
    while (!encoded) {
      SendLoc(encoded);
    }
  }

  
  if(millis()% ReadTime < 3000){
    ReadMode();
    delay(100);
  }
}

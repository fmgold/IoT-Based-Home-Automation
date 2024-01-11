#include <Servo.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

Servo myservo;

int pos ;

#include <Firebase_ESP_Client.h>
//-----------------------------------------------------------------------
//Provide the token generation process info.
#include <addons/TokenHelper.h>

//#include <Servo.h>
//Servo myservo;

//Provide the RTDB payload printing
//info and other helper functions.
#include <addons/RTDBHelper.h>
//------------------------------------
//-----------------------------------------------------------------------
/* 1. Define the WiFi credentials */
//#define WIFI_SSID "fmgold"
//#define WIFI_PASSWORD "12345678000"

#define WIFI_SSID "user"
#define WIFI_PASSWORD "**********"
//-----------------------------------------------------------------------
/* 2. Define the API Key */
#define API_KEY ""
//-----------------------------------------------------------------------
/* 3. Define the RTDB URL */
#define DATABASE_URL "iot-home-appliances-default-rtdb.firebaseio.com"
//-----------------------------------------------------------------------
String room_no = "room1";
//-----------------------------------------------------------------------
// define the GPIO connected with Relays and switches
#define Relay1  15
#define Relay2   4
#define Relay3   5
#define Relay4   19
#define Relay5  18
#define Relay6   21
#define Relay7   23
#define pump   22
//#define servo 13

#define Switch1 12
#define Switch2 14
#define Switch3 27
#define Switch4 26
#define Switch5 25
#define Switch6 33
#define Switch7 32

int LDR = 34;

#define WIFI_LED 2
//int angle[] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90};

int stateRelay1 = 0, stateRelay2 = 0, stateRelay3 = 0, stateRelay4 = 0, stateRelay5 = 0, stateRelay6 = 0, stateRelay7 = 0, statepump = 0, stateServo =  0 ;
//-----------------------------------------------------------------------

String stream_path = "";
String event_path = "";
String stream_data = "";

FirebaseJson jsonData;

volatile bool dataChanged = false;

bool signupOK = false;

bool resetPressed = true;

bool uploadBucket = false;

String bucketData = "", bucketPath = "";

FirebaseData stream;
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

void streamCallback(FirebaseStream data)
{
  stream_path = data.streamPath().c_str();
  event_path = data.dataPath().c_str();

  if (String(data.dataType().c_str()) == "json") {
    jsonData = data.to<FirebaseJson>();
  } else {
    //intData(), floatData()
    stream_data = data.stringData();
  }
  Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                stream_path,
                event_path,
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); //see addons/RTDBHelper.h
  Serial.println();

  Serial.printf("Received stream payload size: %d (Max. %d)\n\n",
                data.payloadLength(), data.maxPayloadLength());

  dataChanged = true;
}

/******************************************************************************************
   void streamTimeoutCallback(bool timeout)
 ******************************************************************************************/
void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(),
                  stream.errorReason().c_str());
}


/******************************************************************************************
   void setup()
 ******************************************************************************************/
int angle[] = {0, 10, 20, 30, 40, 50, 60};

void setup()
{
  Serial.begin(115200);
  //-----------------------------------------------------------------------
  pinMode(Relay1, OUTPUT); digitalWrite(Relay1, LOW);
  pinMode(Relay2, OUTPUT); digitalWrite(Relay2, LOW);
  pinMode(Relay3, OUTPUT); digitalWrite(Relay3, LOW);
  pinMode(Relay4, OUTPUT); digitalWrite(Relay4, LOW);
  pinMode(Relay5, OUTPUT); digitalWrite(Relay5, LOW);
  pinMode(Relay6, OUTPUT); digitalWrite(Relay6, LOW);
  pinMode(Relay7, OUTPUT); digitalWrite(Relay7, LOW);
  pinMode(pump, OUTPUT); digitalWrite(pump, LOW);
  myservo.attach(13);

  pinMode(WIFI_LED, OUTPUT);

  pinMode(Switch1, INPUT_PULLUP);
  pinMode(Switch2, INPUT_PULLUP);
  pinMode(Switch3, INPUT_PULLUP);
  pinMode(Switch4, INPUT_PULLUP);
  pinMode(Switch5, INPUT_PULLUP);
  pinMode(Switch6, INPUT_PULLUP);
  pinMode(Switch7, INPUT_PULLUP);


  //-----------------------------------------------------------------------
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //-----------------------------------------------------------------------
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  //-----------------------------------------------------------------------
  /*Or Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signUp ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  //-----------------------------------------------------------------------


  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  //-----------------------------------------------------------------------
  //Recommend for ESP8266 stream, adjust the buffer size to match your stream data size
#if defined(ESP8266)
  stream.setBSSLBufferSize(2048 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
#endif
  //-----------------------------------------------------------------------
  if (!Firebase.RTDB.beginStream(&stream, room_no))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());
  //-----------------------------------------------------------------------
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
  //-----------------------------------------------------------------------

  myservo.write(0);
}


/******************************************************************************************
   void loop()
 ******************************************************************************************/
void loop()
{
  int lightIntensity = analogRead(LDR);
  int lightInt = map(lightIntensity, 0, 4100, 0, 99);
  //--------------------------------------------------------------------------
  if (WiFi.status() != WL_CONNECTED) {
    //Serial.println("WiFi Not Connected");
    digitalWrite(WIFI_LED, HIGH); //Turn off WiFi LED
  }
  else {
    //Serial.println("WiFi Connected");
    digitalWrite(WIFI_LED, LOW); //Turn on WiFi LED
  }
  //--------------------------------------------------------------------------
  if (lightInt < 30) {
    digitalWrite(Relay7, HIGH);
  }
  if (lightInt >= 31) {
    digitalWrite(Relay7, LOW);
  }
  listenSwitches();
  if (Firebase.ready() && signupOK) {
    if (uploadBucket == true) {
      uploadBucket = false;
      String URL = room_no + "/" + bucketPath;
      Serial.println(URL);
      Serial.printf("Set String... %s\n\n", Firebase.RTDB.setString(&fbdo, URL, bucketData) ? "ok" : fbdo.errorReason().c_str());
    }
    if (resetPressed == true) {
      resetPressed = false;
      Serial.printf("Set Test String... %s\n\n", Firebase.RTDB.setString(&fbdo, "/test", "0") ? "ok" : fbdo.errorReason().c_str());
    }
    bucketPath = "";
    bucketData = "";

  }

  if (digitalRead(Switch1) == LOW) {
    stateRelay1 = 0;
    stateRelay2 = 0;
    stateRelay3 = 0;
    stateRelay4 = 0;
    stateRelay5 = 0;
    statepump = 0;
    stateServo = 0;
    digitalWrite(Relay1, LOW);
    digitalWrite(Relay2, LOW);
    digitalWrite(Relay3, LOW);
    digitalWrite(Relay4, LOW);
    digitalWrite(Relay5, LOW);
    digitalWrite(Relay6, LOW);
    digitalWrite(Relay7, LOW);
    //    uploadBucket = true;
    //    bucketData = String(stateRelay1);
    //    bucketData = String(stateRelay2);
    //    bucketData = String(stateRelay3);
    //    bucketData = String(stateRelay4);
    //    bucketData = String(stateRelay5);
    //    bucketData = String(stateRelay6);
    //    bucketData = String(stateRelay7);
    //    bucketData = String(statePump);
    //    bucketPath = "L1";
    //    bucketPath = "L1";
    //    bucketPath = "L1";
    //    bucketPath = "L1";
    //    bucketPath = "L1";
    //    bucketPath = "L1";

  }

  //--------------------------------------------------------------------------
  if (dataChanged)
  {
    dataChanged = false;
    Serial.println("dataChanged");
    stream_data.replace("\\\"", "");
    //____________________________________________________________________
    if (event_path == "/BD22") {
      Serial.println("relay1:" + stream_data);
      stateRelay1 = stream_data.toInt();
      digitalWrite(Relay1, stateRelay1);
    }
    //____________________________________________________________________
    else if (event_path == "/L1") {
      Serial.println("relay2:" + stream_data);
      stateRelay2 = stream_data.toInt();
      digitalWrite(Relay2, stateRelay2);
    }
    //____________________________________________________________________
    else if (event_path == "/L3") {
      Serial.println("relay3:" + stream_data);
      stateRelay3 = stream_data.toInt();
      digitalWrite(Relay3, stateRelay3);
    }
    //____________________________________________________________________
    else if (event_path == "/BD11") {
      Serial.println("relay4:" + stream_data);
      stateRelay4 = stream_data.toInt();
      digitalWrite(Relay4, stateRelay4);
    }
    //____________________________________________________________________
    else if (event_path == "/BD12") {
      Serial.println("relay1:" + stream_data);
      stateRelay5 = stream_data.toInt();
      digitalWrite(Relay5, stateRelay5);
    }
    //____________________________________________________________________
    else if (event_path == "/BD21") {
      Serial.println("relay6:" + stream_data);
      stateRelay6 = stream_data.toInt();
      digitalWrite(Relay6, stateRelay6);
    }
    //____________________________________________________________________
    //    else if (event_path == "/OT1") {
    //      Serial.println("relay7:" + stream_data);
    //      stateRelay7 = stream_data.toInt();
    //      digitalWrite(Relay7, stateRelay7);
    //    }
    //____________________________________________________________________
    else if (event_path == "/OT2") {
      Serial.println("SERVO:" + stream_data);
      statepump = stream_data.toInt();
      Serial.println(statepump);
      digitalWrite(pump, statepump);
    }

    //____________________________________________________________________
    else if (event_path == "/OT2") {
      Serial.println("SERVO:" + stream_data);
      statepump = stream_data.toInt();
      Serial.println(statepump);
      digitalWrite(pump, statepump);
    }

    else if (event_path == "/OT3") {
      Serial.println("servo:" + stream_data);
      stateServo = stream_data.toInt();
      //digitalWrite(Relay7, stateRelay7);
      if (stateServo ==  1) {
        Serial.println("servo on");
        //
        for (pos = 80; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
          myservo.write(pos);              // tell servo to go to position in variable 'pos'
          delay(15);                       // waits 15ms for the servo to reach the position
        }


      }
      if (stateServo ==  0) {
        Serial.println("servo off");
        for (pos = 0; pos <= 80; pos += 1) { // goes from 0 degrees to 180 degrees
          // in steps of 1 degree
          myservo.write(pos);              // tell servo to go to position in variable 'pos'
          delay(15);                       // waits 15ms for the servo to reach the position
        }

      }

    }
    //____________________________________________________________________
    else if (event_path == "/") {
      reloadRelayStates();
    }
    //____________________________________________________________________
    stream_data = "";
  }
  //--------------------------------------------------------------------------
}


void reloadRelayStates() {
  //jsonData.toString(Serial, true);
  String strKey, strVal;
  //_________________________________________________________
  size_t count = jsonData.iteratorBegin();
  for (size_t i = 0; i < count; i++) {
    FirebaseJson::IteratorValue value = jsonData.valueAt(i);
    //-----------------------------------
    strVal = value.value.c_str();
    strVal.replace("\\", "");
    strVal.replace("\"", "");
    strKey = value.key.c_str();
    Serial.println("strKey:" + strKey);
    Serial.println("strVal:" + strVal);
    //-----------------------------------
    if (strKey == "L1") {
      stateRelay1 = strVal.toInt();
      digitalWrite(Relay1, stateRelay1);
      //Serial.print("relay1:");Serial.println(stateRelay1);
    }
    else if (strKey == "L2") {
      stateRelay2 = strVal.toInt();
      digitalWrite(Relay2, stateRelay2);
      //Serial.print("relay2:");Serial.println(stateRelay2);
    }
    else if (strKey == "L3") {
      stateRelay3 = strVal.toInt();
      digitalWrite(Relay3, stateRelay3);
      //Serial.print("relay3:");Serial.println(stateRelay3);
    }
    else if (strKey == "BD11") {
      stateRelay4 = strVal.toInt();
      digitalWrite(Relay4, stateRelay4);
      //Serial.print("relay4:");Serial.println(stateRelay4);
    }
    else if (strKey == "BD12") {
      stateRelay5 = strVal.toInt();
      digitalWrite(Relay5, stateRelay5);
      //Serial.print("relay2:");Serial.println(stateRelay2);
    }
    else if (strKey == "BD21") {
      stateRelay6 = strVal.toInt();
      digitalWrite(Relay6, stateRelay6);
      //Serial.print("relay3:");Serial.println(stateRelay3);
    }
    else if (strKey == "OT1") {
      stateRelay6 = strVal.toInt();
      digitalWrite(Relay7, stateRelay7);
      //Serial.print("relay4:");Serial.println(stateRelay4);
    }
    else if (strKey == "OT2") {
      statepump = strVal.toInt();
      digitalWrite(pump, statepump);
      //Serial.print("relay4:");Serial.println(stateRelay4);
    }
  }
  //required for free the used memory in iteration (node data collection)
  jsonData.iteratorEnd();
  jsonData.clear();
  //_________________________________________________________
}



void listenSwitches()
{
  //String URL = room_no;
  if (digitalRead(Switch1) == LOW) {
    stateRelay1 = 0;
    stateRelay2 = 0;
    stateRelay3 = 0;
    stateRelay4 = 0;
    stateRelay5 = 0;
    statepump = 0;
    stateServo = 0;
    digitalWrite(Relay1, LOW);
    digitalWrite(Relay2, LOW);
    digitalWrite(Relay3, LOW);
    digitalWrite(Relay4, LOW);
    digitalWrite(Relay5, LOW);
    digitalWrite(Relay6, LOW);
    digitalWrite(Relay7, LOW);
    uploadBucket = true;
    bucketData = String(stateRelay1);
    bucketPath = "L1";

  }
  else if (digitalRead(Switch2) == LOW) {
    stateRelay2 = !stateRelay2;
    digitalWrite(Relay2, stateRelay2);
    uploadBucket = true;
    bucketData = String(stateRelay2);
    bucketPath = "L2";
  }
  else if (digitalRead(Switch3) == LOW) {
    stateRelay3 = !stateRelay3;
    digitalWrite(Relay3, stateRelay3);
    uploadBucket = true;
    bucketData = String(stateRelay3);
    bucketPath = "L3";
  }
  else if (digitalRead(Switch4) == LOW) {
    stateRelay4 = !stateRelay4;
    digitalWrite(Relay4, stateRelay4);
    uploadBucket = true;
    bucketData = String(stateRelay4);
    bucketPath = "L4";
  }
  else if (digitalRead(Switch5) == LOW) {
    stateRelay5 = !stateRelay5;
    digitalWrite(Relay5, stateRelay5);
    uploadBucket = true;
    bucketData = String(stateRelay5);
    bucketPath = "L2";
  }
  else if (digitalRead(Switch6) == LOW) {
    stateRelay6 = !stateRelay6;
    digitalWrite(Relay6, stateRelay6);
    uploadBucket = true;
    bucketData = String(stateRelay6);
    bucketPath = "L3";
  }
  else if (digitalRead(Switch7) == LOW) {
    stateRelay4 = !stateRelay4;
    digitalWrite(Relay7, stateRelay7);
    uploadBucket = true;
    bucketData = String(stateRelay7);
    bucketPath = "L4";
  }
}



void FirebaseWrite(String URL, int data)
{

  if (!Firebase.ready() && !signupOK)
  {
    Serial.println("Write Failed: Firebase not ready OR signup not OK");
    return;
  }

  URL = room_no + "/" + URL;
  Serial.println(URL);
  Serial.println(String(data));
  // Write an Int number on the database path (URL) room1/L1, room1/L2 etc.
  if (Firebase.RTDB.setString(&fbdo, URL, String(data))) {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

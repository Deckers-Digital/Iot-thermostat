//-----------------------------------------------------------------------
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <analogWrite.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>

#define BLYNK_PRINT Serial
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

const int rs = 22, en = 23, d4 = 25, d5 = 26, d6 = 27, d7 = 14;
LiquidCrystal Lcd(rs, en, d4, d5, d6, d7);

//-----------------------------------------------------------------------

int MainSwitch = 1;

int ThermometerButtonPin = 12;
int RoomPin = 21;

int lamp = 18;
int ventLamp = 13;
int sliderData;

const int potPin = 34;
const int HeaterUsagePin = 35;
const int VentUsagePin = 32;

int RoomPinState;

int tempRead;
int potRead;

int HeaterUsage;
int VentUsage;

float MinVolt = 0;
float MaxVolt = 3.3;

float Intensity = 250;
float DisplayIntensity;

int ButtonActive;

bool RoomClick = false;

int pinDataMain;
int pinDataKitchen;
int pinDataBedroom;
int pinDataGarage;

int ButtonStateMain1 = 0;
int ButtonStateMain2;

int ButtonStateKitchen1 = 0;
int ButtonStateKitchen2;

int ButtonStateBedroom1 = 0;
int ButtonStateBedroom2;

int ButtonStateGarage1 = 0;
int ButtonStateGarage2;

int DoubleClick1 = 0;
int DoubleClick2;
int Doubleclicked = 0;

int LongHold1 = 0;
int LongHold2;
int Status = 0;

int PressTime;
int PressTime2;

int Room = 1;

int WantedTempMain = 25;
int WantedTempKitchen = 25;
int WantedTempBedroom = 25;
int WantedTempGarage = 25;

float TempMain = 24;
float TempKitchen = 25;
float TempBedroom = 27;
float TempGarage = 17;

WidgetLED MainLedHeater(V20);
WidgetLED MainLedVent(V21);
WidgetLED KitchenLedHeater(V22);
WidgetLED KitchenLedVent(V23);
WidgetLED BedroomLedHeater(V24);
WidgetLED BedroomLedVent(V25);
WidgetLED GarageLedHeater(V26);
WidgetLED GarageLedVent(V27);


//-----------------------------------------------------------------------

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "t7coMXZgfhEsukQP5jvA3rbSno7PW6DU";

// Your WiFi credentials.
// Set password to "" for open networks.
const char* ssid = "LAPTOP-Vince";
const char* pass = "2#n0193F";

const char* temperature_topic = "home/livingroom/temperature";
const char* humidity_topic = "home/livingroom/humidity";

const char* mqttServer = "192.168.137.105";
const int mqttPort = 1883;
const char* mqttUser = "vincent";
const char* mqttPassword = "raspberry";
const char* clientID = "client_livingroom"; // MQTT client ID

//-----------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  Lcd.begin(16, 2);

  MainSwitch = 1;

  pinMode(ThermometerButtonPin, INPUT_PULLUP);
  pinMode(RoomPin, INPUT_PULLUP);

  pinMode(lamp, OUTPUT);
  pinMode(ventLamp, OUTPUT);

  Status = 1;

  dht.begin();

  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //set up wifi connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);

  Serial.println("WiFi connected");

  Blynk.begin(auth, ssid, pass, "server.wyns.it", 8081);

  //set a callback function for when data is received from broker
  client.setCallback(callback);

  // connect to broker
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword ))
    {
      Serial.println("connected");
    } else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }


  // subscribe to topic
  client.subscribe("home/kitchen/temp");


}

//-----------------------------------------------------------------------

BLYNK_CONNECTED() {
  Blynk.syncAll();
}

BLYNK_WRITE(V0) {
  MainSwitch = param.asInt();
}

BLYNK_WRITE(V1) {

  pinDataMain = param.asInt();
}

BLYNK_WRITE(V2) {

  pinDataKitchen = param.asInt();
}

BLYNK_WRITE(V3) {

  pinDataBedroom = param.asInt();
}

BLYNK_WRITE(V4) {

  pinDataGarage = param.asInt();
}

BLYNK_WRITE(V10) {

  WantedTempMain = param.asInt();

}

BLYNK_WRITE(V11) {

  WantedTempKitchen = param.asInt();

}

BLYNK_WRITE(V12) {

  WantedTempBedroom = param.asInt();

}

BLYNK_WRITE(V13) {

  WantedTempGarage = param.asInt();

}

BLYNK_WRITE(V5) {

  Intensity = param.asInt();

}

//-----------------------------------------------------------------------


void loop() {

  Blynk.run();
  client.loop();

  if (MainSwitch == 1) {

    BlynkWrites();
    SerialPrints();

    float TempTemp = dht.readTemperature();
    potRead = analogRead(potPin);

    HeaterUsage = map(analogRead(HeaterUsagePin), 0, 4095, MinVolt, MaxVolt);

    VentUsage = map(analogRead(VentUsagePin), 0, 4095, MinVolt, MaxVolt);

    if (isnan(TempTemp)) {

    } else {
      TempMain = TempTemp;
    }

    if (Room == 1) {

      TempRoom1();
      DisplayRoom1();

    } else if (Room == 2) {

      TempRoom2();
      DisplayRoom2();

    } else if (Room == 3) {

      TempRoom3();
      DisplayRoom3();

    } else if (Room == 4) {

      TempRoom4();
      DisplayRoom4();

    }

    int b = checkButton();
    if (b == 1) clickEvent();
    //if (b == 2) doubleClickEvent();
    if (b == 3) HoldEvent();

    int b2 = checkButton2();
    if (b2 == 1) RoomclickEvent();
    else RoomClick = false;
    //if (b2 == 2) doubleClickEvent();
    if (b2 == 3) HoldEvent2();
  } else {
    Lcd.setCursor(0, 0);
    Lcd.print("Thermostat          ");
    Lcd.setCursor(0, 1);
    Lcd.print("Offline             ");
  }
}

//-----------------------------------------------------------------------

void SerialPrints() {

  //Serial.println(analogRead(34));
  //Serial.println(HeaterUsage);
  //Serial.println(VentUsage);
  //Serial.println(pinData);
  //Serial.println(temperature);
  //Serial.println(WantedTemp);

}

//-----------------------------------------------------------------------


void clickEvent() {

  if (Room == 1) {

    if (ButtonStateMain1 == 0) {

      if (ButtonStateMain2 == 1)
      {
        Serial.println("Button Pressed 2");
        ButtonStateMain2 = 0;
        analogWrite(lamp, 0);
        analogWrite(ventLamp, 0);
        pinDataMain = 0;

      } else {
        Serial.println("Button Pressed");
        pinDataMain = 1;
        ButtonStateMain2 = 1;
      }
    }
  }

  else if (Room == 2) {

    if (ButtonStateKitchen1 == 0) {

      if (ButtonStateKitchen2 == 1)
      {
        Serial.println("Button Pressed 2");
        ButtonStateKitchen2 = 0;
        analogWrite(lamp, 0);
        analogWrite(ventLamp, 0);
        pinDataKitchen = 0;

      } else {
        Serial.println("Button Pressed");
        pinDataKitchen = 1;
        ButtonStateKitchen2 = 1;
      }
    }
  }

  else if (Room == 3) {

    if (ButtonStateBedroom1 == 0) {

      if (ButtonStateBedroom2 == 1)
      {
        Serial.println("Button Pressed 2");
        ButtonStateBedroom2 = 0;
        analogWrite(lamp, 0);
        analogWrite(ventLamp, 0);
        pinDataBedroom = 0;

      } else {
        Serial.println("Button Pressed");
        pinDataBedroom = 1;
        ButtonStateBedroom2 = 1;
      }
    }
  }
  else if (Room == 4) {

    if (ButtonStateGarage1 == 0) {

      if (ButtonStateGarage2 == 1)
      {
        Serial.println("Button Pressed 2");
        ButtonStateGarage2 = 0;
        analogWrite(lamp, 0);
        analogWrite(ventLamp, 0);
        pinDataGarage = 0;

      } else {
        Serial.println("Button Pressed");
        pinDataGarage = 1;
        ButtonStateGarage2 = 1;
      }

    }
  }
}

/*void doubleClickEvent() {

   if (DoubleClick1 == 0) {

      if (DoubleClick2 == 1)
      {
        Serial.println("Double click 2");
        DoubleClick2 = 0;
        Doubleclicked = 0;
        //delay(120);
      } else {
        Serial.println("Double clicked");
        Doubleclicked = 1;
        DoubleClick2 = 1;
        //delay(120);
      }
    }
  }*/

void HoldEvent () {

  int timer = millis();
  int verschil = 0;

  if (Room == 1 ) {
    while (digitalRead(ThermometerButtonPin) == LOW) {
      verschil = millis() - timer;

      if ( verschil > PressTime)
      {
        Serial.println("In hold");
        WantedTempMain = map(analogRead(34), 0, 4095, 0, 30);
        Serial.print("Ingestelde temperatuur: ");
        Serial.println(WantedTempMain);
        Lcd.setCursor(0, 1);
        Lcd.print("Inst. Temp.: ");
        Lcd.print(WantedTempMain);
      }
    }

  } else if (Room == 2) {
    while (digitalRead(ThermometerButtonPin) == LOW) {
      verschil = millis() - timer;

      if ( verschil > PressTime)
      {
        Serial.println("In hold");
        WantedTempKitchen = map(analogRead(34), 0, 4095, 0, 30);
        Serial.print("Ingestelde temperatuur: ");
        Serial.println(WantedTempKitchen);
        Lcd.setCursor(0, 1);
        Lcd.print("Inst. Temp.: ");
        Lcd.print(WantedTempKitchen);
      }
    }

  } else if (Room == 3) {
    while (digitalRead(ThermometerButtonPin) == LOW) {
      verschil = millis() - timer;

      if ( verschil > PressTime)
      {
        Serial.println("In hold");
        WantedTempBedroom = map(analogRead(34), 0, 4095, 0, 30);
        Serial.print("Ingestelde temperatuur: ");
        Serial.println(WantedTempBedroom);
        Lcd.setCursor(0, 1);
        Lcd.print("Inst. Temp.: ");
        Lcd.print(WantedTempBedroom);
      }
    }

  } else if (Room == 4) {
    while (digitalRead(ThermometerButtonPin) == LOW) {
      verschil = millis() - timer;

      if ( verschil > PressTime)
      {
        Serial.println("In hold");
        WantedTempGarage = map(analogRead(34), 0, 4095, 0, 30);
        Serial.print("Ingestelde temperatuur: ");
        Serial.println(WantedTempGarage);
        Lcd.setCursor(0, 1);
        Lcd.print("Inst. Temp.: ");
        Lcd.print(WantedTempGarage);
      }
    }
  }
}

void RoomclickEvent() {

  Serial.println("Room switch");
  if (Room <= 3) {
    Room++;
    ButtonActive = 0;
    Serial.print("Room: ");
    Serial.println(Room);
  } else {
    Room = 1;
  }

}

void HoldEvent2() {

  int timer2 = millis();
  int verschil2 = 0;

  while (digitalRead(RoomPin) == LOW) {
    verschil2 = millis() - timer2;

    if ( verschil2 > PressTime2)
    {
      Intensity = map(analogRead(34), 0, 4095, 0, 250);
      DisplayIntensity = map(analogRead(34), 0, 4095, 0, 100);

      Lcd.setCursor(0, 1);
      Lcd.print("Intensety: ");
      Lcd.print(DisplayIntensity);
    }
  }
}



//-----------------------------------------------------------------------

void DisplayRoom1 () {
  /*Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");*/
  Lcd.setCursor(0, 0);
  Lcd.print("-Main- ");
  Lcd.setCursor(0, 1);
  Lcd.print("Temp.: ");
  Lcd.print(TempMain);
  Lcd.println(" *C ");
}

void TempRoom1() {

  if (pinDataMain == 1 && Doubleclicked == 0) {
    if (WantedTempMain == 0) {

    } else if (WantedTempMain + 1 > TempMain) {
      Serial.println("Heater on   ");
      MainLedHeater.on();
      MainLedVent.off();
      Blynk.virtualWrite(V1, 1);
      Lcd.setCursor(7, 0);
      Lcd.print("Heat: On    ");
      analogWrite(lamp, Intensity);
      analogWrite(ventLamp, 0);
    } else if (WantedTempMain < TempMain) {
      MainLedVent.on();
      MainLedHeater.off();
      Blynk.virtualWrite(V1, 1);
      Serial.println("Vent on");
      Lcd.setCursor(7, 0);
      analogWrite(ventLamp, Intensity);
      analogWrite(lamp, 0);
      Lcd.print("Vent: On     ");
    }
  } else {
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V1, 0);
    MainLedHeater.off();
    MainLedVent.off();
    analogWrite(lamp, 0);
    analogWrite(ventLamp, 0);
    Lcd.setCursor(7, 0);
    Lcd.print("Natural     ");
  }
}

void DisplayRoom2 () {

  Lcd.setCursor(0, 0);
  Lcd.print("-Kit.-");
  Lcd.setCursor(0, 1);
  Lcd.print("Temp.: ");
  Lcd.print(TempKitchen);
  Lcd.println(" *C ");
}

void TempRoom2 () {

  if (pinDataKitchen == 1 && Doubleclicked == 0) {
    if (WantedTempKitchen == 0) {

    } else if (WantedTempKitchen + 1 > TempKitchen) {
      KitchenLedHeater.on();
      KitchenLedVent.off();
      Blynk.virtualWrite(V2, 1);
      Serial.println("Heater on");
      Lcd.setCursor(7, 0);
      Lcd.print("Heat: On    ");
      analogWrite(lamp, Intensity);
      analogWrite(ventLamp, 0);
    } else if (WantedTempKitchen < TempKitchen) {
      KitchenLedVent.on();
      KitchenLedHeater.off();
      Blynk.virtualWrite(V2, 1);
      Serial.println("Vent on");
      Lcd.setCursor(7, 0);
      analogWrite(ventLamp, Intensity);
      analogWrite(lamp, 0);
      Lcd.print("Vent: On    ");
    }
  } else {
    Blynk.virtualWrite(V2, 0);
    KitchenLedHeater.off();
    KitchenLedVent.off();
    analogWrite(lamp, 0);
    analogWrite(ventLamp, 0);
    Lcd.setCursor(7, 0);
    Lcd.print("Natural     ");
  }
}

void DisplayRoom3 () {

  Lcd.setCursor(0, 0);
  Lcd.print("-Bed.-");
  Lcd.setCursor(0, 1);
  Lcd.print("Temp.: ");
  Lcd.print(TempBedroom);
  Lcd.println(" *C ");
}

void TempRoom3 () {

  if (pinDataBedroom == 1 && Doubleclicked == 0) {
    if (WantedTempBedroom == 0) {

    } else if (WantedTempBedroom + 1 > TempBedroom) {
      BedroomLedHeater.on();
      BedroomLedVent.off();
      Blynk.virtualWrite(V3, 1);
      Serial.println("Heater on");
      Lcd.setCursor(7, 0);
      Lcd.print("Heat: On   ");
      analogWrite(lamp, Intensity);
      analogWrite(ventLamp, 0);
    } else if (WantedTempBedroom < TempBedroom) {
      BedroomLedVent.on();
      BedroomLedHeater.off();
      Blynk.virtualWrite(V3, 1);
      Serial.println("Vent on");
      Lcd.setCursor(7, 0);
      analogWrite(ventLamp, Intensity);
      analogWrite(lamp, 0);
      Lcd.print("Vent: On   ");
    }
  } else {
    BedroomLedHeater.off();
    BedroomLedVent.off();
    Blynk.virtualWrite(V3, 0);
    analogWrite(lamp, 0);
    analogWrite(ventLamp, 0);
    Lcd.setCursor(7, 0);
    Lcd.print("Natural   ");
  }
}


void DisplayRoom4 () {

  Lcd.setCursor(0, 0);
  Lcd.print("-Gar.-");
  Lcd.setCursor(0, 1);
  Lcd.print("Temp.: ");
  Lcd.print(TempGarage);
  Lcd.println(" *C ");
}

void TempRoom4() {
  if (pinDataGarage == 1 && Doubleclicked == 0) {
    if (WantedTempBedroom == 0) {

    } else if (WantedTempGarage + 1 > TempGarage) {
      GarageLedHeater.on();
      GarageLedVent.off();
      Blynk.virtualWrite(V4, 1);
      Serial.println("Heater on");
      Lcd.setCursor(7, 0);
      Lcd.print("Heat: On   ");
      analogWrite(lamp, Intensity);
      analogWrite(ventLamp, 0);
    } else if (WantedTempGarage < TempGarage) {
      GarageLedVent.on();
      GarageLedHeater.off();
      Blynk.virtualWrite(V4, 1);
      Serial.println("Vent on");
      Lcd.setCursor(7, 0);
      analogWrite(ventLamp, Intensity);
      analogWrite(lamp, 0);
      Lcd.print("Vent: On   ");

    } else {
      GarageLedHeater.off();
      GarageLedVent.off();
      Blynk.virtualWrite(V4, 0);
      analogWrite(lamp, 0);
      analogWrite(ventLamp, 0);
      Lcd.setCursor(7, 0);
      Lcd.print("Natural   ");
    }
  }
}

//---------------------------------------------------------------------------------------------------------------

void BlynkWrites () {

  Blynk.virtualWrite(V15, TempMain);
  Blynk.virtualWrite(V16, TempKitchen);
  Blynk.virtualWrite(V17, TempBedroom);
  Blynk.virtualWrite(V18, TempGarage);

  Blynk.virtualWrite(V20, HeaterUsage);
  Blynk.virtualWrite(V21, VentUsage);

  Blynk.virtualWrite(V30, WantedTempMain);
  Blynk.virtualWrite(V31, WantedTempKitchen);
  Blynk.virtualWrite(V32, WantedTempBedroom);
  Blynk.virtualWrite(V33, WantedTempGarage);
  //Blynk.virtualWrite(V0, pinData);

  Blynk.virtualWrite(V6, Intensity);
}

//----------------------------------------------------------------------------------------------------------------

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println("-----------------------");

}

//================================================================================================================
//  MULTI-CLICK:  One Button, Multiple Events

// Button timing variables
int debounce = 20;          // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap = 250;            // max ms between clicks for a double click event
int holdTime = 1000;        // ms hold period: how long to wait for press+hold event
int longHoldTime = 3000;    // ms long hold period: how long to wait for press+hold event

// Button variables
boolean buttonVal = LOW;   // value read from button
boolean buttonLast = HIGH;  // buffered value of the button's previous state
boolean DCwaiting = false;  // whether we're waiting for a double click (down)
boolean DConUp = false;     // whether to register a double click on next release, or whether to wait and click
boolean singleOK = true;    // whether it's OK to do a single click
long downTime = -1;         // time the button was pressed down
long upTime = -1;           // time the button was released
boolean ignoreUp = false;   // whether to ignore the button release because the click+hold was triggered
boolean waitForUp = false;        // when held, whether to wait for the up event
boolean holdEventPast = false;    // whether or not the hold event happened already
boolean longHoldEventPast = false;// whether or not the long hold event happened already

int checkButton() {
  int event = 0;
  buttonVal = digitalRead(ThermometerButtonPin);
  // Button pressed down
  if (buttonVal == LOW && buttonLast == HIGH && (millis() - upTime) > debounce)
  {
    downTime = millis();
    ignoreUp = false;
    waitForUp = false;
    singleOK = true;
    holdEventPast = false;
    longHoldEventPast = false;
    if ((millis() - upTime) < DCgap && DConUp == false && DCwaiting == true)  DConUp = true;
    else  DConUp = false;
    DCwaiting = false;
  }
  // Button released
  else if (buttonVal == HIGH && buttonLast == LOW && (millis() - downTime) > debounce)
  {
    if (not ignoreUp)
    {
      upTime = millis();
      if (DConUp == false) DCwaiting = true;
      else
      {
        event = 2;
        DConUp = false;
        DCwaiting = false;
        singleOK = false;
      }
    }
  }
  // Test for normal click event: DCgap expired
  if ( buttonVal == HIGH && (millis() - upTime) >= DCgap && DCwaiting == true && DConUp == false && singleOK == true && event != 2)
  {
    event = 1;
    Serial.println("Button pressed");

    ButtonActive = 1;

    DCwaiting = false;
  }
  // Test for hold
  if (buttonVal == LOW && (millis() - downTime) >= holdTime) {
    // Trigger "normal" hold
    if (not holdEventPast)
    {
      event = 3;
      waitForUp = true;
      ignoreUp = true;
      DConUp = false;
      DCwaiting = false;
      //downTime = millis();
      holdEventPast = true;
    }
    // Trigger "long" hold
    if ((millis() - downTime) >= longHoldTime)
    {
      if (not longHoldEventPast)
      {
        event = 4;
        longHoldEventPast = true;
      }
    }
  }
  buttonLast = buttonVal;
  return event;
}

//================================================================================================================
//  MULTI-CLICK:  One Button, Multiple Events

// Button timing variables
int debounce2 = 20;          // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap2 = 250;            // max ms between clicks for a double click event
int holdTime2 = 1000;        // ms hold period: how long to wait for press+hold event
int longHoldTime2 = 3000;    // ms long hold period: how long to wait for press+hold event

// Button variables
boolean buttonVal2 = LOW;   // value read from button
boolean buttonLast2 = HIGH;  // buffered value of the button's previous state
boolean DCwaiting2 = false;  // whether we're waiting for a double click (down)
boolean DConUp2 = false;     // whether to register a double click on next release, or whether to wait and click
boolean singleOK2 = true;    // whether it's OK to do a single click
long downTime2 = -1;         // time the button was pressed down
long upTime2 = -1;           // time the button was released
boolean ignoreUp2 = false;   // whether to ignore the button release because the click+hold was triggered
boolean waitForUp2 = false;        // when held, whether to wait for the up event
boolean holdEventPast2 = false;    // whether or not the hold event happened already
boolean longHoldEventPast2 = false;// whether or not the long hold event happened already

int checkButton2() {
  int event2 = 0;
  buttonVal2 = digitalRead(RoomPin);
  // Button pressed down
  if (buttonVal2 == LOW && buttonLast2 == HIGH && (millis() - upTime2) > debounce2)
  {
    downTime2 = millis();
    ignoreUp2 = false;
    waitForUp2 = false;
    singleOK2 = true;
    holdEventPast2 = false;
    longHoldEventPast2 = false;
    if ((millis() - upTime2) < DCgap2 && DConUp2 == false && DCwaiting2 == true)  DConUp2 = true;
    else  DConUp2 = false;
    DCwaiting2 = false;
  }
  // Button released
  else if (buttonVal2 == HIGH && buttonLast2 == LOW && (millis() - downTime2) > debounce2)
  {
    if (not ignoreUp2)
    {
      upTime2 = millis();
      if (DConUp2 == false) DCwaiting2 = true;
      else
      {
        event2 = 2;
        DConUp2 = false;
        DCwaiting2 = false;
        singleOK2 = false;
      }
    }
  }
  // Test for normal click event: DCgap expired
  if ( buttonVal2 == HIGH && (millis() - upTime2) >= DCgap2 && DCwaiting2 == true && DConUp2 == false && singleOK2 == true && event2 != 2)
  {
    event2 = 1;

    DCwaiting2 = false;
  }
  // Test for hold
  if (buttonVal2 == LOW && (millis() - downTime2) >= holdTime2) {
    // Trigger "normal" hold
    if (not holdEventPast2)
    {
      event2 = 3;
      waitForUp2 = true;
      ignoreUp2 = true;
      DConUp2 = false;
      DCwaiting2 = false;
      //downTime = millis();
      holdEventPast2 = true;
    }
    // Trigger "long" hold
    if ((millis() - downTime2) >= longHoldTime2)
    {
      if (not longHoldEventPast2)
      {
        event2 = 4;
        longHoldEventPast2 = true;
      }
    }
  }
  buttonLast2 = buttonVal2;
  return event2;
}

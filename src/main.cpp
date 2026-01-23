// This example uses an ESP32 Development Board
// to connect to shiftr.io.
//
// You can check on your device after a successful
// connection here: https://www.shiftr.io/try.
//
// by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt

#include <WiFi.h>
#include <MQTT.h>

// #include <MQTTClient.h>

const char ssid[] = "OnePlus 10 Pro 5G-76e8";
const char pass[] = "g3se674x";

// Что то еще, проверка
// Что то еще, проверка 2
// еще что то 3


//const char MQTT_BROKER_ADRRESS[] = "91.149.232.230";  // CHANGE TO MQTT BROKER'S ADDRESS
const char MQTT_BROKER_ADRRESS[] = "77.51.217.41"; // CHANGE TO MQTT BROKER'S ADDRESS
const int MQTT_PORT = 1883;
//const int MQTT_PORT = 8883;
const char MQTT_CLIENT_ID[] = "YOUR-NAME-esp32-001"; // CHANGE IT AS YOU DESIRE
const char MQTT_USERNAME[] = "userMosquitoPSP"; // CHANGE IT IF REQUIRED, empty if not required
const char MQTT_PASSWORD[] = "9546595465Psp!"; // CHANGE IT IF REQUIRED, empty if not required

//Внешний датчик Холла
const int hallPin = 25;     // Пин, к которому подключен DO датчика
const int ledPin = 2;       // Встроенный светодиод (или внешний)
int hallState = 0;


// Пины
//const int hallPin = 18; // Пин, к которому подключен выход датчика
volatile unsigned int pulseCount = 0;
unsigned long lastMillis_rpm = 0;
float rpm = 0;


WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;
unsigned long lastMillis_wifi = 0;
unsigned long count_fps = 0;
//unsigned long time = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("./.");
    delay(2000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.print(".>.");
    delay(3000);
  }

  Serial.println("\nconnected!");

  client.subscribe("/hello");
  client.subscribe("/times");
  client.subscribe("/fps");
  client.subscribe("/hall");
  client.subscribe("/rpm");
  client.subscribe("/pulseCount");
  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}


// Прерывание: срабатывает при появлении магнита
void IRAM_ATTR handleInterrupt() {
  pulseCount++;
}



void setup() {
  //Serial.begin(115200);
  Serial.begin(9600);
  WiFi.begin(ssid, pass);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  
  //client.begin("public.cloud.shiftr.io", net);

  client.begin(MQTT_BROKER_ADRRESS, net);  
  client.onMessage(messageReceived);
  connect();

  pinMode(ledPin, OUTPUT);
  pinMode(hallPin, INPUT); // Датчик A3144 выдает логический сигнал

  //pinMode(hallPin, INPUT_PULLUP); // Используем встроенную подтяжку, если модуль без нее
  attachInterrupt(digitalPinToInterrupt(hallPin), handleInterrupt, FALLING); // FALLING - переход с HIGH на LOW



}



void loop() {

  hallState = digitalRead(hallPin);

  client.loop();
  //delay(500);  // <- fixes some issues with WiFi stability
count_fps=count_fps+1; // счетчик итераций
lastMillis_wifi = millis();

  // publish a message roughly every second.
  if (millis() - lastMillis_wifi > 1000) {
    //lastMillis = millis();
    if (!client.connected()) {
      connect();
    }

   }



  // if (!client.connected()) {
  //   connect();
  // }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/hello", "world");


    //time = millis();
    char buffer[12]; // Буфер достаточного размера
    sprintf(buffer, "%lu", lastMillis); // %lu для unsigned long
    // Теперь buffer содержит строку, например, "12345"
    client.publish("/times", buffer);


    //char buffer[12]; // Буфер достаточного размера
    sprintf(buffer, "%lu", count_fps); // %lu для unsigned long
    // Теперь buffer содержит строку, например, "12345"
    client.publish("/fps", buffer);


    count_fps=0;

  int hallValue = hallRead(); // Чтение значения датчика Холла
  Serial.print("Значение: ");
  Serial.println(hallValue);

    //char buffer[12]; // Буфер достаточного размера
    sprintf(buffer, "%lu", hallValue); // %lu для unsigned long
    // Теперь buffer содержит строку, например, "12345"
    client.publish("/hall", buffer);




      // Если магнитное поле обнаружено (выход LOW)
      if (hallState == LOW) {
        digitalWrite(ledPin, HIGH); // Включить светодиод
        Serial.println("Магнит обнаружен!");
      } else {
        digitalWrite(ledPin, LOW);  // Выключить светодиод
        Serial.println("Магнита нет");
      }



  // Расчет RPM каждую секунду
  if (millis() - lastMillis_rpm >= 1000) {
    detachInterrupt(digitalPinToInterrupt(hallPin)); // Отключаем прерывания на время расчета

    // RPM = (импульсы за сек) * 60
    rpm = (pulseCount * 60.0); 

    Serial.print("RPM: ");
    Serial.println(rpm);

   //char buffer[12]; // Буфер достаточного размера
    sprintf(buffer, "%lu", rpm); // %lu для unsigned long
    // Теперь buffer содержит строку, например, "12345"
    client.publish("/rpm", buffer);

    //char buffer[12]; // Буфер достаточного размера
    sprintf(buffer, "%lu", pulseCount); // %lu для unsigned long
    // Теперь buffer содержит строку, например, "12345"
    client.publish("/pulseCount", buffer);


    pulseCount = 0; // Сбрасываем счетчик
    lastMillis_rpm = millis(); // Обновляем время
    attachInterrupt(digitalPinToInterrupt(hallPin), handleInterrupt, FALLING); // Включаем прерывания

 


  }

   }






}
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <time.h>

#define uS_TO_S_FACTOR 1000000  // Biến chuyển từ micro giây sang giây
#define TIME_TO_SLEEP 600        // Thời gian thức dậy (giây)

//---- WiFi settings
const char* wifi_ssids[] = { "Khoa Khoa", "Galaxy A224103" };                    // SSID
const char* wifi_passwords[] = { "12345678", "khoakhoa" };                      // Mật khẩu tương ứng
const int wifi_count = sizeof(wifi_ssids) / sizeof(wifi_ssids[0]);  // Số mạng WiFi

//---- MQTT Broker settings
const char* mqtt_server = "49b6dcd6236247be8bcfe1416017e3b6.s1.eu.hivemq.cloud";
const char* mqtt_username = "group15_iot";
const char* mqtt_password = "Group15@iot";
const int mqtt_port = 8883;

WiFiClientSecure espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

const char* sensor1_topic = "sensor1";
static const char* root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

// Cấu hình gp2y10
int measurePin = 34;
int ledPower = 14;

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;  // Thông số bụi mịn (dvi: ug/m^3)

// Cấu hình mq7
#define coefficient_A 19.709
#define coefficient_B 0.652
#define v_in 3.3
#define coPin 32

float coDensity = 0;  // Thông số khí CO (dvi: ug/m^3)

void setup() {
  Serial.begin(115200);
  pinMode(ledPower, OUTPUT);
  pinMode(measurePin, INPUT);

  // Connect WiFi
  connectWiFi();

  // Thiết lập đồng bộ thời gian qua NTP
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");  // Múi giờ GMT+7 (Hanoi)
  while (!time(nullptr)) {
    Serial.print("Waiting for NTP sync...");
    delay(1000);
  }

  // Connect MQTT
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  if (!client.connected()) reconnect();
  client.loop();

  // Lấy dữ liệu từ cảm biến
  delay(1000);
  getPM25();
  getCO();

  publishMessage(sensor1_topic, "Khoa's home", true);  // Gửi dữ liệu lên Broker MQTT
  delay(1000);

  // Ngắt kết nối MQTT và WiFi
  client.disconnect();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // Thiết lập chế độ deep sleep
  deepSleep();
}

void loop() {
  // don't use loop()
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);

  for (int i = 0; i < wifi_count; i++) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(wifi_ssids[i]);

    WiFi.begin(wifi_ssids[i], wifi_passwords[i]);

    // Chờ kết nối trong 10 giây
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(1000);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("\nConnected to ");
      Serial.println(wifi_ssids[i]);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      return;  // Thoát khỏi vòng lặp nếu kết nối thành công
    } else {
      Serial.println("\nFailed to connect to " + String(wifi_ssids[i]));
    }
  }
  deepSleep();  // Ngủ nếu ko kết nối được WiFi
}

void reconnect() {
  int attempts = 0;  // Số lần thử kết nối MQTT
  while (!client.connected() || attempts < 10) {
    Serial.print("Attempting MQTT connection…");
    String clientId = "ESP32Client-";  // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect MQTT
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      return;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 seconds");  // Wait 1 seconds before retrying
      delay(1000);
    }
    attempts++;
  }
  deepSleep();  // Ngủ nếu ko kết nối được MQTT
}

void publishMessage(const char* topic, String location, boolean retained) {
  // Lấy thời gian hiện tại
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  // Chuyển thời gian thành chuỗi ISO 8601 (YYYY-MM-DDTHH:MM:SS)
  char timestamp[30];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", timeinfo);

  // Tạo JSON payload
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["deviceId"] = "device001";
  jsonDoc["co"] = coDensity;
  jsonDoc["pm25"] = dustDensity;
  jsonDoc["timestamp"] = timestamp;
  jsonDoc["location"] = location;

  // Chuyển JSON thành chuỗi
  char jsonBuffer[200];
  serializeJson(jsonDoc, jsonBuffer);

  // Gửi payload qua MQTT
  int attempts = 0;  // Số lần thử publish message
  while (attempts < 10) {
    if (client.publish(topic, jsonBuffer, retained)) {
      Serial.println("Message published [" + String(topic) + "]: " + String(jsonBuffer));
      return;
    } else {
      attempts++;
      delay(1000);
    }
  }
  Serial.println("Failed to publish message");
}

void getPM25() {
  digitalWrite(ledPower, LOW);      // Bật IR LED
  delayMicroseconds(samplingTime);  // Delay 0.28ms

  while (voMeasured == 0) {
    voMeasured = analogRead(measurePin);  // Đọc giá trị ADC
    delay(10);
  }

  delayMicroseconds(deltaTime);  // Delay 0.04ms
  digitalWrite(ledPower, HIGH);  // Tắt LED
  delayMicroseconds(sleepTime);  // Delay 9.68ms

  // Tính điện áp từ giá trị ADC
  calcVoltage = voMeasured * (3.3 / 4095.0);  // Chuyển đổi giá trị ADC sang điện áp
  calcVoltage += 0.65;                        // Bù điện áp

  // Tính mật độ bụi
  dustDensity = (0.172 * calcVoltage - 0.1) * 1000;
  if (dustDensity < 0) dustDensity = 0;  // Loại bỏ giá trị âm

  Serial.print("Voltage: ");
  Serial.print(calcVoltage);  // In điện áp đã tính toán
  Serial.print(" V, Dust Density: ");
  Serial.print(dustDensity);  // In mật độ bụi
  Serial.println(" µg/m³");
}

void getCO() {
  float value = analogRead(coPin);
  value -= 200;
  float v_out = value * (v_in / 4095.0);
  float ratio = (v_in - v_out) / v_out;

  coDensity = (float)pow((ratio / coefficient_A), (-1 / coefficient_B));  // ppm
  // concentration (mg/m3) = 0.0409 x concentration (ppm) x molecular weight
  // molecular weight = 28.01 g/mol
  coDensity = (0.0409 * 28.01 * coDensity) * 1000;  // ug/m^3
  if (isnan(coDensity) || isinf(coDensity)) coDensity = 0;

  Serial.print("Voltage: ");
  Serial.print(v_out);
  Serial.print(" V, MQ-7 CO: ");
  Serial.print(coDensity);
  Serial.println(" µg/m³");
}

void deepSleep() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush();
  // Băt đầu vào chế độ ngủ
  esp_deep_sleep_start();
}

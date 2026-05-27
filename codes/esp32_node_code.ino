// ============================================================
//  TEKNOFEST 2026 - NEFES PROJESİ
//  ESP32 Düğüm (Node) Kodu
//  Sensör verilerini toplar, ESP-NOW ile Gateway'e gönderir,
//  enerji tasarrufu için Deep Sleep moduna geçer.
// ============================================================

#include <esp_now.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include <ArduinoJson.h>

// ─── Yapılandırma ────────────────────────────────────────────
#define DEVICE_ID        "NODE_01"   // Her node için benzersiz değiştir
#define SLEEP_DURATION_S 30          // Uyku süresi (saniye)
#define SEND_TIMEOUT_MS  3000        // Gönderim zaman aşımı (ms)

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ─── Veri Yapısı ─────────────────────────────────────────────
typedef struct __attribute__((packed)) struct_message {
  char    id[32];
  float   latitude;
  float   longitude;
  int     heartRate;
  int     spo2;
  int     battery;
  int64_t timestamp;
} struct_message;

struct_message myData;
volatile bool sendComplete = false;
volatile esp_now_send_status_t sendStatus;

// ─── Callback: Gönderim sonucu ───────────────────────────────
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  sendStatus   = status;
  sendComplete = true;
  if (status == ESP_NOW_SEND_SUCCESS)
    Serial.println("[ESP-NOW] Gönderim başarılı ✓");
  else
    Serial.println("[ESP-NOW] HATA: Gönderim başarısız!");
}

// ─── Sensör okuma fonksiyonları ──────────────────────────────
// Gerçek sensör entegrasyonunda bu fonksiyonları güncelleyin
int readHeartRate() { return random(60, 105); }
int readSpO2()      { return random(94, 100); }
int readBattery()   { return random(35, 100); }

// ─── Setup ───────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[" + String(DEVICE_ID) + "] setup() başladı");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("[HATA] ESP-NOW başlatılamadı! Yeniden başlatılıyor...");
    delay(1000);
    ESP.restart();
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[HATA] Peer eklenemedi!");
  }

  // Veri paketini hazırla
  memset(&myData, 0, sizeof(myData));
  strncpy(myData.id, DEVICE_ID, sizeof(myData.id) - 1);
  myData.latitude  = 39.9334;   // Gerçek GPS verisiyle değiştir
  myData.longitude = 32.8597;
  myData.heartRate = readHeartRate();
  myData.spo2      = readSpO2();
  myData.battery   = readBattery();
  myData.timestamp = esp_timer_get_time() / 1000;

  Serial.printf("[%s] Sensör okundu → HR:%dbpm SpO₂:%d%% Bat:%d%%\n",
    DEVICE_ID, myData.heartRate, myData.spo2, myData.battery);

  // Gönder
  Serial.printf("[%s] esp_now_send() → broadcast gönderiliyor...\n", DEVICE_ID);
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  if (result != ESP_OK)
    Serial.printf("[HATA] esp_now_send(): %s\n", esp_err_to_name(result));

  // Callback bekle
  unsigned long t0 = millis();
  while (!sendComplete && (millis() - t0) < SEND_TIMEOUT_MS) delay(10);
  if (!sendComplete) Serial.println("[UYARI] Gönderim zaman aşımına uğradı.");

  // Deep Sleep
  Serial.printf("[%s] esp_sleep_enable_timer_wakeup(%ds)\n", DEVICE_ID, SLEEP_DURATION_S);
  Serial.printf("[%s] esp_deep_sleep_start() → zzz\n", DEVICE_ID);
  Serial.flush();
  esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_DURATION_S * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {
  // Deep Sleep kullanıldığında burası çalışmaz.
  // Tüm mantık setup() içinde yer alır.
}

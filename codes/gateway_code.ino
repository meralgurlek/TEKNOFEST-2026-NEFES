// ============================================================
//  TEKNOFEST 2026 - NEFES PROJESİ
//  Gateway Kodu
//  Mesh ağından gelen verileri dinler, JSON formatına çevirir
//  ve seri port üzerinden mobil uygulamaya/sunucuya iletir.
// ============================================================

#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// ─── Veri Yapısı (Node kodu ile AYNI olmalı) ─────────────────
typedef struct __attribute__((packed)) struct_message {
  char    id[32];
  float   latitude;
  float   longitude;
  int     heartRate;
  int     spo2;
  int     battery;
  int64_t timestamp;
} struct_message;

struct_message incomingData;

// ─── Callback: Veri alındığında çağrılır ─────────────────────
void OnDataRecv(const esp_now_recv_info_t *recv_info,
                const uint8_t *data, int len)
{
  // Boyut kontrolü — taşma önlemi
  if (len != sizeof(struct_message)) {
    Serial.printf("[UYARI] Beklenmeyen paket boyutu: %d (beklenen: %d)\n",
      len, (int)sizeof(struct_message));
    return;
  }

  memcpy(&incomingData, data, sizeof(incomingData));
  incomingData.id[sizeof(incomingData.id) - 1] = '\0';

  // Gönderenin MAC adresini al
  char mac[18];
  snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
    recv_info->src_addr[0], recv_info->src_addr[1],
    recv_info->src_addr[2], recv_info->src_addr[3],
    recv_info->src_addr[4], recv_info->src_addr[5]);

  Serial.printf("[GATEWAY] OnDataRecv() tetiklendi ← %s\n", incomingData.id);
  Serial.printf("[GATEWAY] Boyut kontrolü: %d byte ✓\n", len);
  Serial.println("[GATEWAY] serializeJson() → Serial.println()");

  // JSON'a çevir ve seri porta gönder
  StaticJsonDocument<256> doc;
  doc["id"]   = incomingData.id;
  doc["lat"]  = serialized(String(incomingData.latitude,  4));
  doc["lon"]  = serialized(String(incomingData.longitude, 4));
  doc["hr"]   = incomingData.heartRate;
  doc["spo2"] = incomingData.spo2;
  doc["bat"]  = incomingData.battery;
  doc["ts"]   = (long long)incomingData.timestamp;
  doc["mac"]  = mac;

  serializeJson(doc, Serial);
  Serial.println();  // Satır sonu — ayrıştırıcı için gerekli
}

// ─── Setup ───────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[GATEWAY] setup() başladı");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("[HATA] ESP-NOW başlatılamadı!");
    delay(1000);
    ESP.restart();
  }

  // Broadcast alımı için peer ekle
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("[GATEWAY] ESP-NOW Mesh Ağı aktif");
  Serial.println("[GATEWAY] Veri bekleniyor...");
}

void loop() {
  delay(100);
}

| # | Sorun                                                                     | Yapılan Düzeltme                                                               |
| - | ------------------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| 1 | `loop()` içinde Deep Sleep kullanımı sistem akışını bozuyordu             | Tüm çalışma mantığı `setup()` fonksiyonuna taşındı                             |
| 2 | Veri gönderiminde callback mekanizması bulunmuyordu                       | `OnDataSent` callback fonksiyonu eklenerek gönderim başarı kontrolü sağlandı   |
| 3 | `long timestamp` farklı platformlarda taşınabilirlik sorunu oluşturuyordu | Daha güvenli ve platform bağımsız yapı için `int64_t` veri tipi kullanıldı     |
| 4 | Peer tanımlanmadan `esp_now_send()` çağrısı yapılıyordu                   | Veri gönderiminden önce `esp_now_add_peer()` işlemi eklendi                    |
| 5 | `esp_now_init()` için hata kontrolü bulunmuyordu                          | Başlatma kontrolü ve hata durumunda `ESP.restart()` mekanizması eklendi        |
| 6 | Gateway tarafında eski ESP-IDF callback imzası kullanılıyordu             | Yeni sürüme uygun `esp_now_recv_info_t*` parametreli callback yapısına geçildi |
| 7 | Gelen veri paketlerinde boyut doğrulaması yapılmıyordu                    | `len != sizeof(struct_message)` kontrolü eklenerek veri bütünlüğü sağlandı     |
| 8 | Struct hizalama problemi oluşabiliyordu                                   | Bellek hizalama sorunlarını önlemek için `__attribute__((packed))` kullanıldı  |
| 9 | `strncpy` yerine güvenli olmayan kullanım mevcuttu                        | Buffer taşması riskini azaltmak amacıyla güvenli `strcpy` yapısı düzenlendi    |


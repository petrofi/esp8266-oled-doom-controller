# Sorun Giderme

## `ESP8266WiFi.h: No such file or directory`

Yanlış kart seçilmiştir veya ESP8266 kart paketi kurulu değildir.

Seç:

```text
Araçlar > Kart > esp8266 > NodeMCU 1.0 (ESP-12E Module)
```

## `WebSocketsServer.h: No such file or directory`

Arduino IDE Kütüphane Yöneticisi'nden:

```text
WebSockets
```

kütüphanesini kur.

Yazar genellikle Markus Sattler / Links2004 olarak görünür.

## `Adafruit_SH110X.h: No such file or directory`

Kütüphane Yöneticisi:

```text
Adafruit SH110X
```

kur.

Ayrıca:

```text
Adafruit GFX Library
```

kurulu olmalı.

## OLED tamamen siyah

Kontrol et:

```text
VCC -> 3.3V
GND -> GND
SDA -> D2
SCL -> D1
```

`config.h`:

```cpp
#define OLED_ADDR 0x3C
```

Bazı ekranlarda adres `0x3D` olabilir.

## OLED SSD1306 ise

Bu proje SH1106 için hazırlanmıştır.

1.3" OLED modüllerin büyük bölümü SH1106 olsa da kart üzerindeki sürücüyü doğrula.

## Telefon Wi-Fi ağına bağlanıyor fakat sayfa açılmıyor

Tarayıcıya tam olarak:

```text
http://192.168.4.1
```

yaz.

`https://` kullanma.

Mobil veri bazı telefonlarda otomatik devreye girebilir. Gerekirse geçici olarak mobil veriyi kapat.

## Telefon "internet yok" diyor

Normal.

ESP8266 kendi yerel Wi-Fi ağını oluşturur ve internete bağlı değildir.

"Bu ağa bağlı kal" seçeneğini seç.

## Telefon kontrolü gecikiyor

Bu sürüm WebSocket kullanır. Yine de:

- ESP8266 ile telefon arasını yakın tut.
- CPU frekansını 160 MHz seç.
- Telefonun DOOM-ESP8266 ağından ayrılmadığını kontrol et.
- Mobil veriyi geçici olarak kapatmayı dene.
- OLED'i 400 kHz I²C'de kullan.
- Çok eski telefon tarayıcılarında Chrome/Safari güncelle.

## Karakter kendi kendine yürümeye devam ediyor

Kod 350 ms WebSocket zaman aşımı kullanır.

Bağlantı kaybolursa telefon girdileri otomatik sıfırlanır.

Eğer fiziksel buton bağlıysa yanlış bağlantıyı kontrol et.

## Kart açılmıyor

GERİ butonu `D3 / GPIO0` pinindedir.

Kart açılırken GERİ butonu basılıysa ESP8266 programlama modunda kalabilir.

Butonu bırak ve resetle.

## Kod yüklenmiyor

Şunları kontrol et:

- Doğru COM portu
- NodeMCU 1.0 seçili
- USB kablosu veri destekliyor
- GERİ butonu basılı değil
- Gerekirse fiziksel butonları geçici olarak sök

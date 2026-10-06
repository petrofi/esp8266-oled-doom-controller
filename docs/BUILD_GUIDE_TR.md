# Baştan Sona Yapım Rehberi

## Aşama 1 — Donanımı hazırla

Gerekli ana parçalar:

- NodeMCU ESP8266
- SH1106 128×64 OLED
- 5 buton
- Breadboard
- Jumper kablolar
- USB veri kablosu

Önce yalnızca OLED'i bağlaman tavsiye edilir. Kod çalıştıktan sonra fiziksel butonları tek tek ekleyebilirsin.

## Aşama 2 — Arduino IDE

Arduino IDE 2.x kullanabilirsin.

ESP8266 kart paketini kur.

Kart:

```text
NodeMCU 1.0 (ESP-12E Module)
```

olmalı.

ESP32-S3, Arduino Uno veya başka kart seçiliyse `ESP8266WiFi.h` bulunamaz.

## Aşama 3 — Kütüphaneler

Kur:

- Adafruit GFX Library
- Adafruit SH110X
- WebSockets

## Aşama 4 — Kodu aç

Arduino IDE'de:

```text
ESP8266_OLED_DOOM/ESP8266_OLED_DOOM.ino
```

dosyasını aç.

Arduino IDE aynı klasördeki `config.h` ve `web_ui.h` dosyalarını otomatik olarak kullanır.

## Aşama 5 — Yükleme

ESP8266'yı USB ile bağla.

Doğru COM portunu seç.

Kod yüklenirken D3 / GPIO0 üzerindeki GERİ butonuna basılı tutma.

## Aşama 6 — İlk açılış

OLED'de şunlar görünür:

```text
ESP8266 OLED DOOM
WiFi: DOOM-ESP8266
Pass: doom1234
Open: 192.168.4.1
```

## Aşama 7 — Telefon

Telefonun Wi-Fi listesinden:

```text
DOOM-ESP8266
```

ağını seç.

Şifre:

```text
doom1234
```

Tarayıcı:

```text
http://192.168.4.1
```

## Aşama 8 — Oyun

Kontroller:

- Sol: sola dön
- Sağ: sağa dön
- İleri: yürü
- Geri: geri yürü
- FIRE: ateş

Ölürsen veya bütün düşmanları öldürürsen FIRE ile yeniden başlatabilirsin.

## Performans

Önerilen:

```text
CPU Frequency: 160 MHz
```

OLED I²C hızı kodda 400 kHz olarak ayarlanmıştır.

WebSocket kontrolü oyun render döngüsünden bağımsız şekilde mümkün olduğunca sık işlenir.

# Fiziksel Bağlantı Rehberi

Bu sayfa NodeMCU ESP8266, SH1106 OLED ve 5 fiziksel butonun bağlantısını adım adım anlatır.

## 1. OLED

OLED'i breadboard'a yerleştir.

| OLED | NodeMCU |
|---|---|
| GND | GND |
| VCC | 3.3V |
| SCL | D1 |
| SDA | D2 |

OLED'i **5V yerine 3.3V** ile beslemek en güvenli seçimdir.

## 2. SOL butonu

Bir bacağını `D5 / GPIO14` pinine bağla.

Diğer bacağını `GND` pinine bağla.

Kod `INPUT_PULLUP` kullandığı için ekstra direnç gerekmez.

## 3. SAĞ butonu

Bir bacağını `D6 / GPIO12` pinine bağla.

Diğer bacağını `GND` pinine bağla.

## 4. İLERİ butonu

Bir bacağını `D7 / GPIO13` pinine bağla.

Diğer bacağını `GND` pinine bağla.

## 5. GERİ butonu

Bir bacağını `D3 / GPIO0` pinine bağla.

Diğer bacağını `GND` pinine bağla.

### D3 uyarısı

D3, ESP8266'nın boot pinlerinden biridir.

Kartı açarken, resetlerken veya kod yüklemeye başlarken GERİ butonuna basılı tutma.

Normal çalışma başladıktan sonra butonu kullanabilirsin.

## 6. ATEŞ butonu

Bu buton diğerlerinden farklıdır.

Bir bacağını `D0 / GPIO16` pinine bağla.

Diğer bacağını `3.3V` pinine bağla.

Kod tarafında:

```cpp
pinMode(PIN_FIRE, INPUT_PULLDOWN_16);
```

kullanılır.

Bu nedenle normal durumda pin LOW, butona basınca HIGH olur.

## Breadboard mantığı

```text
OLED:
3.3V  ---------------- VCC
GND   ---------------- GND
D1    ---------------- SCL
D2    ---------------- SDA

BUTONLAR:
D5 ----[ SOL ]-------- GND
D6 ----[ SAG ]-------- GND
D7 ----[ ILERI ]------ GND
D3 ----[ GERI ]------- GND

D0 ----[ ATES ]------- 3.3V
```

## Telefon kontrolü kullanacaksan

Fiziksel butonları hiç bağlamadan da proje çalışır.

Yalnızca:

- ESP8266
- OLED
- USB güç

yeterlidir.

Telefon ve fiziksel butonlar bağlandıysa iki kontrol yöntemi aynı anda çalışır.

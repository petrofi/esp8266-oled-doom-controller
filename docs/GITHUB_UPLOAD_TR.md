# GitHub'a Yükleme

## Önerilen repo adı

```text
esp8266-oled-doom-controller
```

## Önerilen açıklama

```text
Doom-inspired 128x64 raycasting FPS for ESP8266 + SH1106 OLED with 5 physical buttons and low-latency phone controls over WebSocket.
```

## GitHub web arayüzü ile

1. GitHub'da `New repository` seç.
2. Repository name:
   `esp8266-oled-doom-controller`
3. Description alanına yukarıdaki açıklamayı yapıştır.
4. Public seçebilirsin.
5. README, .gitignore veya lisansı GitHub'dan ayrıca oluşturma; ZIP içinde zaten var.
6. `Create repository`.
7. ZIP'i bilgisayarında çıkar.
8. Çıkardığın klasörün içindeki tüm dosyaları GitHub repo sayfasına sürükleyip bırak.
9. Commit mesajı:
   `Initial release: ESP8266 OLED DOOM controller`
10. `Commit changes`.

## Git komutları ile

ZIP'i çıkardıktan sonra klasör içinde terminal aç:

```bash
git init
git add .
git commit -m "Initial release: ESP8266 OLED DOOM controller"
git branch -M main
git remote add origin https://github.com/KULLANICI_ADIN/esp8266-oled-doom-controller.git
git push -u origin main
```

`KULLANICI_ADIN` kısmını kendi GitHub kullanıcı adınla değiştir.

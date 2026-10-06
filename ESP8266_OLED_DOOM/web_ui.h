#pragma once
#include <Arduino.h>

const char WEB_PAGE[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="tr">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<meta name="theme-color" content="#050505">
<title>ESP8266 DOOM Controller</title>
<style>
  *{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
  html,body{margin:0;width:100%;height:100%;background:#050505;color:#fff;font-family:Arial,sans-serif;overflow:hidden;touch-action:none}
  body{display:flex;align-items:center;justify-content:center}
  .wrap{width:min(100vw,620px);height:100vh;display:flex;flex-direction:column;justify-content:space-between;padding:18px 18px 24px}
  .top{text-align:center}
  h1{font-size:20px;margin:0 0 6px;letter-spacing:1px}
  #status{font-size:13px;opacity:.8}
  .pad{display:grid;grid-template-columns:1fr 1fr 1fr;grid-template-rows:92px 92px 92px;gap:10px;align-items:stretch}
  button{border:2px solid #555;border-radius:18px;background:#151515;color:white;font-size:34px;font-weight:bold;user-select:none;touch-action:none}
  button.on,button:active{background:#eee;color:#111;transform:scale(.97)}
  #fwd{grid-column:2;grid-row:1}
  #left{grid-column:1;grid-row:2}
  #right{grid-column:3;grid-row:2}
  #back{grid-column:2;grid-row:3}
  #fire{grid-column:3;grid-row:3;font-size:18px;border-color:#888}
  .hint{text-align:center;font-size:12px;opacity:.65;line-height:1.4}
</style>
</head>
<body>
<div class="wrap">
  <div class="top">
    <h1>ESP8266 OLED DOOM</h1>
    <div id="status">WebSocket bağlanıyor…</div>
  </div>

  <div class="pad">
    <button id="fwd"  class="key" data-bit="4">▲</button>
    <button id="left" class="key" data-bit="1">◀</button>
    <button id="right"class="key" data-bit="2">▶</button>
    <button id="back" class="key" data-bit="8">▼</button>
    <button id="fire" class="key" data-bit="16">FIRE</button>
  </div>

  <div class="hint">
    Aynı anda birden fazla tuşa basabilirsin.<br>
    Telefon “internet yok” derse DOOM-ESP8266 ağına bağlı kal.
  </div>
</div>

<script>
let ws=null;
let mask=0;
const statusEl=document.getElementById('status');

function sendState(){
  if(ws && ws.readyState===WebSocket.OPEN){
    ws.send(String(mask));
  }
}

function connectWS(){
  const host=location.hostname || "192.168.4.1";
  ws=new WebSocket(`ws://${host}:81/`);
  ws.onopen=()=>{
    statusEl.textContent="Bağlı • düşük gecikmeli kontrol aktif";
    sendState();
  };
  ws.onclose=()=>{
    statusEl.textContent="Bağlantı koptu • yeniden bağlanıyor…";
    mask=0;
    clearVisuals();
    setTimeout(connectWS,400);
  };
  ws.onerror=()=>{};
}

function clearVisuals(){
  document.querySelectorAll('.key').forEach(b=>b.classList.remove('on'));
}

document.querySelectorAll('.key').forEach(btn=>{
  const bit=Number(btn.dataset.bit);

  const down=(e)=>{
    e.preventDefault();
    try{ btn.setPointerCapture(e.pointerId); }catch(_){}
    mask |= bit;
    btn.classList.add('on');
    if(bit===16 && navigator.vibrate) navigator.vibrate(12);
    sendState();
  };

  const up=(e)=>{
    e.preventDefault();
    mask &= ~bit;
    btn.classList.remove('on');
    sendState();
  };

  btn.addEventListener('pointerdown',down);
  btn.addEventListener('pointerup',up);
  btn.addEventListener('pointercancel',up);
  btn.addEventListener('lostpointercapture',up);
});

window.addEventListener('blur',()=>{
  mask=0;
  clearVisuals();
  sendState();
});

document.addEventListener('visibilitychange',()=>{
  if(document.hidden){
    mask=0;
    clearVisuals();
    sendState();
  }
});

// Lightweight heartbeat. It refreshes the latest state without HTTP polling.
setInterval(sendState,100);

connectWS();
</script>
</body>
</html>
)rawliteral";

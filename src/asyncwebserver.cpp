#include "asyncwebserver.h"
#include "messages.h"
#include "webhandler.h"
#include <ArduinoJson.h>
#ifdef ENABLE_STORAGE
#include <Preferences.h>
#endif

#ifdef ENABLE_SERVER

AsyncWebServer server(80);

void initWebServer()
{
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers",
                                       "Accept, Content-Type, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.on("/", HTTP_GET, startGui);
  server.onNotFound(
      [](AsyncWebServerRequest *request) { request->send(404, "text/plain", "Page not found!"); });

  // Route to handle
  // http://your-server/message?text=Hello&repeat=3&id=42&delay=30&graph=1,2,3,4&miny=0&maxy=15
  server.on("/api/message", HTTP_GET, handleMessage);
  server.on("/api/removemessage", HTTP_GET, handleMessageRemove);

  server.on("/api/info", HTTP_GET, handleGetInfo);

  // Handle API request to set an active plugin by ID
  server.on("/api/plugin", HTTP_PATCH, handleSetPlugin);

  // Handle API request to set the brightness (0..255);
  server.on("/api/brightness", HTTP_PATCH, handleSetBrightness);
  server.on("/api/data", HTTP_GET, handleGetData);

  // Scheduler
  server.on("/api/schedule", HTTP_POST, handleSetSchedule);
  server.on("/api/schedule/clear", HTTP_GET, handleClearSchedule);
  server.on("/api/schedule/stop", HTTP_GET, handleStopSchedule);
  server.on("/api/schedule/start", HTTP_GET, handleStartSchedule);

  server.on("/api/storage/clear", HTTP_GET, handleClearStorage);

  // Configuration endpoints
  server.on("/api/config", HTTP_GET, handleGetConfig);
  server.on(
      "/api/config",
      HTTP_POST,
      [](AsyncWebServerRequest *request) {
        // Response is sent after full body is received.
      },
      nullptr,
      handleSetConfigBody);
  server.on("/api/config/reset", HTTP_POST, handleResetConfig);

  // City Clock config API
  server.on("/api/cityclock", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef ENABLE_STORAGE
    Preferences prefs;
    prefs.begin("cityclock", true);
    JsonDocument doc;
    doc["cityIndex"] = prefs.getInt("cityIdx", 0);
    doc["nightStart"] = prefs.getInt("nightStart", 20);
    doc["nightEnd"] = prefs.getInt("nightEnd", 23);
    prefs.end();
    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
#else
    request->send(200, "application/json", "{\"cityIndex\":0,\"nightStart\":20,\"nightEnd\":23}");
#endif
  });

  // POST /api/cityclock — save city index to NVS (no plugin switch)
  server.on(
      "/api/cityclock", HTTP_POST,
      [](AsyncWebServerRequest *request) {},
      NULL,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t) {
#ifdef ENABLE_STORAGE
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data, len);
        if (err)
        {
          request->send(400, "application/json", "{\"error\":\"JSON 格式錯誤\"}");
          return;
        }
        if (!doc["cityIndex"].is<int>())
        {
          request->send(400, "application/json", "{\"error\":\"缺少 cityIndex\"}");
          return;
        }
        int idx = doc["cityIndex"].as<int>();
        if (idx != 0)
        {
          request->send(400, "application/json", "{\"error\":\"目前僅支援 cityIndex=0（台北）\"}");
          return;
        }
        int nightStart = 20;
        int nightEnd = 23;
        if (doc["nightStart"].is<int>()) nightStart = doc["nightStart"].as<int>();
        if (doc["nightEnd"].is<int>()) nightEnd = doc["nightEnd"].as<int>();
        if (nightStart < 0 || nightStart > 23 || nightEnd < 0 || nightEnd > 23)
        {
          request->send(400, "application/json", "{\"error\":\"nightStart/nightEnd must be 0-23\"}");
          return;
        }
        Preferences prefs;
        prefs.begin("cityclock", false);
        prefs.putInt("cityIdx", idx);
        prefs.putInt("nightStart", nightStart);
        prefs.putInt("nightEnd", nightEnd);
        prefs.end();
        Serial.printf("[API] Saved cityclock cityIdx=%d night=%d~%d\n", idx, nightStart, nightEnd);
        request->send(200, "application/json", "{\"ok\":true}");
#else
        request->send(501, "application/json", "{\"error\":\"裝置未啟用儲存功能\"}");
#endif
      });

  server.on("/marquee", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", R"(
<!DOCTYPE html>
<html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1.0"><title>Marquee</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;display:flex;justify-content:center;align-items:center;padding:20px}.container{background:#fff;border-radius:20px;box-shadow:0 20px 60px rgba(0,0,0,.3);max-width:600px;width:100%;padding:40px}h1{color:#333;margin-bottom:10px}label{display:block;margin-top:15px;font-weight:600}input[type=text]{width:100%;padding:10px;margin-top:5px;border:2px solid #e0e0e0;border-radius:8px;font-size:16px}input[type=text]:focus{outline:0;border-color:#667eea}input[type=range]{width:100%;margin-top:5px}button{margin-top:20px;padding:12px 30px;border:none;border-radius:8px;font-weight:600;cursor:pointer;background:#667eea;color:#fff;font-size:16px}button:hover{background:#5a6fd6}#status{margin-top:15px;padding:10px;border-radius:5px;display:none;font-size:13px}#status.ok{background:#d4edda;color:#155724;display:block}#status.err{background:#f8d7da;color:#721c24;display:block}.speed-val{font-size:13px;color:#999;margin-top:3px}</style></head><body><div class="container"><h1>Marquee</h1><p style="color:#666;margin-bottom:10px">Scrolling text on LED matrix (Latin + Cyrillic)</p><div id="status"></div><label>Text<input type="text" id="txt" placeholder="Hello! Привет!" maxlength="500"></label><label>Speed<input type="range" id="spd" min="10" max="200" value="50"><span class="speed-val" id="spdVal">50ms</span></label><button id="send">Send</button></div><script>
const spdEl=document.getElementById('spd'),spdVal=document.getElementById('spdVal'),statusEl=document.getElementById('status');
spdEl.oninput=()=>{spdVal.textContent=spdEl.value+'ms'};
let ws,marqueeId=null;
function show(t,ok){statusEl.textContent=t;statusEl.className=ok?'ok':'err';setTimeout(()=>{statusEl.style.display='none'},4000)}
function connect(){
  const h=location.host||'localhost';
  ws=new WebSocket('ws://'+h+'/ws');
  ws.onopen=()=>{show('Connected',1)};
  ws.onmessage=(e)=>{
    try{const d=JSON.parse(e.data);
      if(d.plugins){for(const p of d.plugins){if(p.name==='Marquee'){marqueeId=p.id;break}}}
    }catch(ex){}
  };
  ws.onerror=()=>{show('WebSocket error',0)};
  ws.onclose=()=>{setTimeout(connect,3000)};
}
connect();
document.getElementById('send').onclick=()=>{
  const txt=document.getElementById('txt').value;
  if(!txt){show('Enter text first',0);return}
  if(!ws||ws.readyState!==1){show('Not connected',0);return}
  if(marqueeId===null){show('Plugin not found',0);return}
  const msg={event:'marquee',text:txt,speed:parseInt(spdEl.value),plugin:marqueeId};
  ws.send(JSON.stringify(msg));
  show('Sent!',1);
};
</script></body></html>
    )");
  });

  server.on("/cityclock", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", R"(
<!DOCTYPE html>
<html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1.0"><title>城市時鐘設定</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;display:flex;justify-content:center;align-items:center;padding:20px}.container{background:#fff;border-radius:20px;box-shadow:0 20px 60px rgba(0,0,0,.3);max-width:600px;width:100%;padding:40px}h1{color:#333;margin-bottom:10px}p{color:#666;margin-bottom:10px}label{display:block;margin-top:15px;font-weight:600}select,input{width:100%;padding:12px;margin-top:5px;border:2px solid #e0e0e0;border-radius:8px;font-size:16px;background:#fff;cursor:pointer}button{margin-top:20px;padding:12px 30px;border:none;border-radius:8px;font-weight:600;cursor:pointer;background:#667eea;color:#fff;font-size:16px;width:100%;transition:background .2s}button:hover{background:#5a6fd6}.secondary{background:#4b5563}.secondary:hover{background:#374151}#status{margin-top:15px;padding:10px;border-radius:5px;display:none;font-size:13px}#status.ok{background:#d4edda;color:#155724;display:block}#status.err{background:#f8d7da;color:#721c24;display:block}.info{background:#e7f3ff;border-left:4px solid #2196F3;padding:10px;margin-top:20px;border-radius:4px;font-size:13px;color:#1565c0;line-height:1.6}</style></head><body><div class="container"><h1>城市時鐘設定</h1><p>設定夜間時段切換明日趨勢顯示</p><div id="status"></div><label>城市<select id="citySelect"><option value="0">台北 (UTC+8)</option></select></label><label>夜間開始小時 (0-23)<input type="number" id="nightStart" min="0" max="23" value="20"></label><label>夜間結束小時 (0-23)<input type="number" id="nightEnd" min="0" max="23" value="23"></label><button id="save">儲存設定</button><button id="applyNow" class="secondary">立即套用</button><div class="info">目前僅支援台北。夜間時段會控制 Forecast 何時顯示明日趨勢（支援跨日，例如 19~6）。</div></div><script>
const cityEl=document.getElementById('citySelect'),nightStartEl=document.getElementById('nightStart'),nightEndEl=document.getElementById('nightEnd'),statusEl=document.getElementById('status');
function show(t,ok){statusEl.textContent=t;statusEl.className=ok?'ok':'err';statusEl.style.display='block';setTimeout(()=>{statusEl.style.display='none'},4000)}
async function load(){
  try{
    const r=await fetch('/api/cityclock');
    if(!r.ok)return;
    const d=await r.json();
    if(d.cityIndex!==undefined)cityEl.value='0';
    if(d.nightStart!==undefined)nightStartEl.value=String(d.nightStart);
    if(d.nightEnd!==undefined)nightEndEl.value=String(d.nightEnd);
  }catch(e){show('讀取失敗',0)}
}
load();
async function saveCityClock(isApplyNow){
  const idx=parseInt(cityEl.value);
  const nightStart=parseInt(nightStartEl.value);
  const nightEnd=parseInt(nightEndEl.value);
  if(Number.isNaN(nightStart)||Number.isNaN(nightEnd)||nightStart<0||nightStart>23||nightEnd<0||nightEnd>23){show('夜間時段必須是 0-23',0);return}
  try{
    const r=await fetch('/api/cityclock',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({cityIndex:idx,nightStart,nightEnd})});
    if(!r.ok){show('儲存失敗',0);return}
    show(isApplyNow?'已儲存，將在幾秒內套用':'已儲存',1);
  }catch(e){show('錯誤：'+e.message,0)}
}
document.getElementById('save').onclick=()=>saveCityClock(false);
document.getElementById('applyNow').onclick=()=>saveCityClock(true);
</script></body></html>
    )");
  });

  // Forecast config API
  server.on("/api/forecast", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef ENABLE_STORAGE
    Preferences prefs;
    prefs.begin("forecast", true);
    JsonDocument doc;
    doc["cityIndex"] = prefs.getInt("cityIdx", 0);
    prefs.end();
    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
#else
    request->send(200, "application/json", "{\"cityIndex\":1}");
#endif
  });

  server.on(
      "/api/forecast", HTTP_POST,
      [](AsyncWebServerRequest *request) {},
      NULL,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t) {
#ifdef ENABLE_STORAGE
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, data, len);
        if (err)
        {
          request->send(400, "application/json", "{\"error\":\"JSON 格式錯誤\"}");
          return;
        }
        if (!doc["cityIndex"].is<int>())
        {
          request->send(400, "application/json", "{\"error\":\"缺少 cityIndex\"}");
          return;
        }
        int idx = doc["cityIndex"].as<int>();
        if (idx != 1)
        {
          request->send(400, "application/json", "{\"error\":\"目前僅支援 cityIndex=1（台北）\"}");
          return;
        }
        Preferences prefs;
        prefs.begin("forecast", false);
        prefs.putInt("cityIdx", idx);
        prefs.end();
        Serial.printf("[API] Saved forecast cityIdx=%d\n", idx);
        request->send(200, "application/json", "{\"ok\":true}");
#else
        request->send(501, "application/json", "{\"error\":\"裝置未啟用儲存功能\"}");
#endif
      });

  server.on("/forecast", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", R"(
<!DOCTYPE html>
<html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1.0"><title>Forecast 設定</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;display:flex;justify-content:center;align-items:center;padding:20px}.container{background:#fff;border-radius:20px;box-shadow:0 20px 60px rgba(0,0,0,.3);max-width:600px;width:100%;padding:40px}h1{color:#333;margin-bottom:10px}p{color:#666;margin-bottom:10px}label{display:block;margin-top:15px;font-weight:600}select{width:100%;padding:12px;margin-top:5px;border:2px solid #e0e0e0;border-radius:8px;font-size:16px;background:#fff;cursor:pointer}button{margin-top:20px;padding:12px 30px;border:none;border-radius:8px;font-weight:600;cursor:pointer;background:#667eea;color:#fff;font-size:16px;width:100%;transition:background .2s}button:hover{background:#5a6fd6}#status{margin-top:15px;padding:10px;border-radius:5px;display:none;font-size:13px}#status.ok{background:#d4edda;color:#155724;display:block}#status.err{background:#f8d7da;color:#721c24;display:block}.info{background:#e7f3ff;border-left:4px solid #2196F3;padding:10px;margin-top:20px;border-radius:4px;font-size:13px;color:#1565c0;line-height:1.6}</style></head><body><div class="container"><h1>天氣預報設定</h1><p>設定 Forecast 插件的顯示城市</p><div id="status"></div><label>城市<select id="citySelect"><option value="1">台北</option></select></label><button id="save">儲存設定</button><div class="info">目前僅支援台北。資料來源為 Open-Meteo，約每 30 分鐘更新。</div></div><script>
const cityEl=document.getElementById('citySelect'),statusEl=document.getElementById('status');
function show(t,ok){statusEl.textContent=t;statusEl.className=ok?'ok':'err';statusEl.style.display='block';setTimeout(()=>{statusEl.style.display='none'},4000)}
async function load(){
  try{
    const r=await fetch('/api/forecast');
    if(!r.ok)return;
    const d=await r.json();
    if(d.cityIndex!==undefined)cityEl.value=String(d.cityIndex);
  }catch(e){show('讀取失敗',0)}
}
load();
document.getElementById('save').onclick=async()=>{
  const idx=parseInt(cityEl.value);
  try{
    const r=await fetch('/api/forecast',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({cityIndex:idx})});
    if(!r.ok){show('儲存失敗',0);return}
    show('已儲存',1);
  }catch(e){show('錯誤：'+e.message,0)}
};
</script></body></html>
    )");
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", R"(
<!DOCTYPE html>
<html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1.0"><title>LED Config</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;display:flex;justify-content:center;align-items:center;padding:20px}.container{background:#fff;border-radius:20px;box-shadow:0 20px 60px rgba(0,0,0,.3);max-width:600px;width:100%;padding:40px}h1{color:#333;margin-bottom:10px}#msg{padding:12px;margin:15px 0;border-radius:5px;display:none;word-wrap:break-word;font-size:13px;max-height:100px;overflow-y:auto}#msg.ok{background:#d4edda;color:#155724;border:1px solid #c3e6cb}#msg.err{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb}label{display:block;margin-top:15px;font-weight:600}input{width:100%;padding:10px;margin-top:5px;border:2px solid #e0e0e0;border-radius:8px}input:focus{outline:0;border-color:#667eea}button{margin-top:20px;padding:12px 20px;border:none;border-radius:8px;font-weight:600;cursor:pointer;margin-right:10px}#save{background:#667eea;color:#fff}#reset{background:#ff4757;color:#fff}.hint{font-size:11px;color:#999;margin-top:3px}</style></head><body><div class="container"><h1>⚙️ LED Config</h1><div id="msg"></div><form id="form"><label>Weather Location<input id="wl" placeholder="Hamburg"><span class="hint">City name (e.g., Turin, Milan, Hamburg)</span></label><label>NTP Server<input id="ntp" placeholder="de.pool.ntp.org"><span class="hint">Time sync server</span></label><label>Timezone<input id="tz" placeholder="CET-1CEST,M3.5.0,M10.5.0/3"><span class="hint">POSIX timezone (CET-1CEST for Italy/Germany)</span></label><button type="submit" id="save">💾 Save</button><button type="button" id="reset">⚠️ Reset</button></form></div><script>const msg=document.getElementById('msg');function show(t,ok){msg.textContent=t;msg.className=ok?'ok':'err';msg.style.display='block';console.log((ok?'✅':'❌')+' '+t);setTimeout(()=>{msg.style.display='none'},6e3)}async function load(){console.log('[Config] Loading...');try{const r=await fetch('/api/config');console.log('[Config] Response:',r.status);if(!r.ok){show('Load failed ('+r.status+')',0);return}const d=await r.json();console.log('[Config] Data:',d);document.getElementById('wl').value=d.weatherLocation||'';document.getElementById('ntp').value=d.ntpServer||'';document.getElementById('tz').value=d.tzInfo||'';show('✅ Loaded',1)}catch(e){console.error('[Config] Load error:',e);show('Load error: '+e.message,0)}}document.getElementById('form').onsubmit=async e=>{e.preventDefault();const data={weatherLocation:document.getElementById('wl').value,ntpServer:document.getElementById('ntp').value,tzInfo:document.getElementById('tz').value,autoStartSchedule:false};console.log('[Config] Saving:',data);try{const r=await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(data)});console.log('[Config] Response:',r.status);if(!r.ok){const err=await r.text();console.error('[Config] Error:',err);show('Save failed: '+err,0);return}show('✅ Saved!',1);setTimeout(load,500)}catch(e){console.error('[Config] Exception:',e);show('Save error: '+e.message,0)}};document.getElementById('reset').onclick=async()=>{if(!confirm('Reset all?'))return;console.log('[Config] Resetting');try{const r=await fetch('/api/config/reset',{method:'POST'});if(!r.ok){show('Reset failed',0);return}show('✅ Reset!',1);setTimeout(load,500)}catch(e){show('Reset error: '+e.message,0)}};load()</script></body></html>
    )");
  });

  server.begin();
}

#endif

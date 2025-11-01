#include <WiFi.h>
#include <WebServer.h>
#include <libstorage.h>
#include <libprovision.h>

static WebServer server(80);
static bool s_isProvisioning = false;

static const char FORM_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Configuración Wi-Fi</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }
    
    .container {
      background: white;
      border-radius: 16px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
      padding: 40px;
      width: 100%;
      max-width: 420px;
    }
    
    h3 {
      color: #333;
      margin-bottom: 24px;
      font-size: 24px;
      text-align: center;
    }
    
    .form-group {
      margin-bottom: 20px;
    }
    
    label {
      display: block;
      color: #555;
      font-weight: 600;
      margin-bottom: 8px;
      font-size: 14px;
    }
    
    input {
      width: 100%;
      padding: 12px 16px;
      font-size: 16px;
      border: 2px solid #e0e0e0;
      border-radius: 8px;
      transition: all 0.3s ease;
      outline: none;
    }
    
    input:focus {
      border-color: #667eea;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
    }
    
    button {
      width: 100%;
      padding: 14px;
      font-size: 16px;
      font-weight: 600;
      color: white;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      border: none;
      border-radius: 8px;
      cursor: pointer;
      transition: transform 0.2s ease, box-shadow 0.2s ease;
      margin-top: 8px;
    }
    
    button:hover {
      transform: translateY(-2px);
      box-shadow: 0 8px 20px rgba(102, 126, 234, 0.4);
    }
    
    button:active {
      transform: translateY(0);
    }
    
    .icon {
      text-align: center;
      font-size: 48px;
      margin-bottom: 16px;
    }
    
    @media (max-width: 480px) {
      .container {
        padding: 30px 24px;
      }
      
      h3 {
        font-size: 20px;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <h3>Configurar Wi-Fi</h3>
    <form method='POST' action='/save'>
      <div class="form-group">
        <label for="ssid">Nombre de red (SSID)</label>
        <input type="text" id="ssid" name='ssid' placeholder="Mi-Red-WiFi" required autocomplete="off">
      </div>
      <div class="form-group">
        <label for="password">Contraseña</label>
        <input type="password" id="password" name='password' placeholder="••••••••" autocomplete="new-password">
      </div>
      <button type='submit'>Conectar</button>
    </form>
  </div>
</body>
</html>
)HTML";

static void handleRoot() { server.send(200, "text/html", FORM_HTML); }

static void handleSave() {
  if (!server.hasArg("ssid")) { server.send(400, "text/plain", "ssid requerido"); return; }
  String ssid = server.arg("ssid");
  String pwd  = server.arg("password");
  if (!saveWiFiCredentials(ssid, pwd)) { server.send(500, "text/plain", "no se pudo guardar"); return; }
  server.send(200, "text/plain", "Guardado. Reiniciando...");
  delay(500);
  ESP.restart();
}

void startProvisioningAP() {
  WiFi.mode(WIFI_AP);
  String apName = String("ESP32-Setup-") + String((uint32_t)ESP.getEfuseMac(), HEX);
  WiFi.softAP(apName.c_str());
  IPAddress ip = WiFi.softAPIP();
  Serial.print("Provisioning AP "); Serial.print(apName); Serial.print(" IP "); Serial.println(ip);
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();
  s_isProvisioning = true;
}

void provisioningLoop() {
  if (!s_isProvisioning) return;
  server.handleClient();
}

bool isProvisioning() { return s_isProvisioning; }

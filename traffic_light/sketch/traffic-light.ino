#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

#define LED_RED      D7
#define LED_YELLOW   D6
#define LED_GREEN    D5

#define LED_ON HIGH
#define LED_OFF LOW

#define INPUT_SERVICE    D1
#define INPUT_AUTOMATIC  D2

#define SECONDS(ms) ((ms) * 1000) 

#define RED_GREEN_DT SECONDS(10)
#define YELLOW_DT SECONDS(1)

typedef enum {
  STATE_RED = 0,
  STATE_REDYELLOW,
  STATE_GREEN,
  STATE_YELLOW,
  STATE_SIZE
} light_state;

light_state cur_state = STATE_RED;
int last_tick = 0;
int dt = SECONDS(10);

int cur_mode = INPUT_AUTOMATIC;

String wifi_ssid = "";
String wifi_pwd = "";

ESP8266WebServer server(80);

void update_light() {
  if (millis() >= last_tick + dt) {
    last_tick = millis();
    if (cur_mode == INPUT_AUTOMATIC) {
      if (cur_state == STATE_RED || cur_state == STATE_GREEN) {
        dt = YELLOW_DT;
      } else {
        dt = RED_GREEN_DT;
      }
      cur_state = (light_state)((cur_state + 1) % STATE_SIZE);
    } else {
      dt = YELLOW_DT;
      cur_state = (light_state)(cur_state == STATE_YELLOW ? -1 : STATE_YELLOW);
    }
  }
}

void check_mode() {
  if (digitalRead(INPUT_AUTOMATIC) == HIGH && cur_mode != INPUT_AUTOMATIC) {
    cur_mode = INPUT_AUTOMATIC;
    dt = RED_GREEN_DT;
    cur_state = STATE_RED;
  } else if (digitalRead(INPUT_SERVICE) == HIGH && cur_mode != INPUT_SERVICE) {
    cur_mode = INPUT_SERVICE;
    dt = YELLOW_DT;
    cur_state = STATE_YELLOW;
  }
}

void display_light() {
  if (cur_mode == INPUT_AUTOMATIC) {
    switch (cur_state) {
      case STATE_RED:
      digitalWrite(LED_RED, LED_ON);
      digitalWrite(LED_YELLOW, LED_OFF);
      digitalWrite(LED_GREEN, LED_OFF);
      break;
      case STATE_YELLOW:
      digitalWrite(LED_RED, LED_OFF);
      digitalWrite(LED_YELLOW, LED_ON);
      digitalWrite(LED_GREEN, LED_OFF);
      break;
      case STATE_GREEN:
      digitalWrite(LED_RED, LED_OFF);
      digitalWrite(LED_YELLOW, LED_OFF);
      digitalWrite(LED_GREEN, LED_ON);
      break;
      case STATE_REDYELLOW:
      digitalWrite(LED_RED, LED_ON);
      digitalWrite(LED_YELLOW, LED_ON);
      digitalWrite(LED_GREEN, LED_OFF);
      break;
    }
  } else {
    if (cur_state == STATE_YELLOW) {
      digitalWrite(LED_RED, LED_OFF);
      digitalWrite(LED_YELLOW, LED_ON);
      digitalWrite(LED_GREEN, LED_OFF);
    } else {
      digitalWrite(LED_RED, LED_OFF);
      digitalWrite(LED_YELLOW, LED_OFF);
      digitalWrite(LED_GREEN, LED_OFF);
    }
  }
}

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: 'Segoe UI', sans-serif; background: #121212; color: white; text-align: center; display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh; margin: 0; }
        .card { background: #1e1e1e; padding: 2rem; border-radius: 20px; box-shadow: 0 15px 35px rgba(0,0,0,0.7); width: 320px; border: 1px solid #333; }
        h1 { font-weight: 300; margin: 0 0 20px 0; color: #00d4ff; letter-spacing: 1px; }
        .light-indicator { width: 60px; height: 60px; border-radius: 50%; margin: 0 auto 15px auto; background: %COLOR_HEX%; box-shadow: 0 0 20px %COLOR_HEX%; transition: 0.5s; }
        .status-box { background: rgba(255,255,255,0.05); padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .label { font-size: 0.8rem; color: #888; text-transform: uppercase; margin-bottom: 5px; }
        .value { font-size: 1.2rem; font-weight: bold; display: block; }
        .btn-group { display: flex; flex-direction: column; gap: 10px; }
        .btn { display: block; padding: 12px; border: none; border-radius: 8px; font-size: 0.9rem; cursor: pointer; transition: 0.2s; font-weight: bold; }
        .btn-mode { background: #333; color: #ddd; border: 1px solid #444; }
        .btn-mode.active { background: #00d4ff; color: #121212; border-color: #00d4ff; }
        .override-row { display: flex; gap: 10px; margin-top: 10px; border-top: 1px solid #333; padding-top: 20px; }
        .btn-red { background: #661a1a; color: #ff9999; flex: 1; }
        .btn-green { background: #1a6634; color: #99ffbb; flex: 1; }
    </style>
</head>
<body>
    <div class="card">
        <h1>Traffic Control</h1>
        <div class="light-indicator"></div>
        <div class="status-box">
            <span class="label">Current Status</span>
            <span class="value" id="state-text" style="color: %COLOR_HEX%">%LIGHT_STATE%</span>
            <span class="label" style="margin-top:10px">System Mode</span>
            <span id="mode-text" class="value">%MODE%</span>
        </div>
        <div class="btn-group">
            <button id="btn-auto" onclick="cmd('auto')" class="btn btn-mode %AUTO_ACTIVE%">AUTOMATIC</button>
            <button id="btn-maint" onclick="cmd('maintenance')" class="btn btn-mode %MAINT_ACTIVE%">MAINTENANCE</button>
            <div class="override-row">
                <button onclick="cmd('set_red')" class="btn btn-red">FORCE RED</button>
                <button onclick="cmd('set_green')" class="btn btn-green">FORCE GREEN</button>
            </div>
        </div>
    </div>
    <script>
        function cmd(path) {
            fetch('/' + path).then(() => {
                // Small delay to let the D1 Mini process the state change before we ask for status
                setTimeout(updateStatus, 50); 
            });
        }
        function updateStatus() {
            fetch('/status').then(r => r.json()).then(data => {
                document.querySelector('.light-indicator').style.background = data.color;
                document.querySelector('.light-indicator').style.boxShadow = '0 0 20px ' + data.color;
                document.getElementById('state-text').innerText = data.state;
                document.getElementById('state-text').style.color = data.color;
                document.getElementById('mode-text').innerText = data.mode;
                
                // Update active button highlights
                document.getElementById('btn-auto').classList.toggle('active', data.mode === 'AUTOMATIC');
                document.getElementById('btn-maint').classList.toggle('active', data.mode === 'MAINTENANCE');
            });
        }
        setInterval(updateStatus, 1000);
    </script>
</body>
</html>
)=====";

void handle_server_root() {
  String s = INDEX_HTML;
  String color = "#ffcc00";
  String state = "YELLOW";

  if (cur_mode == INPUT_SERVICE) {
    state = "MAINTENANCE";
  } else {
    if (cur_state == STATE_RED) { 
      color = "#ff4d4d"; 
      state = "RED"; 
    }
    else if (cur_state == STATE_GREEN) { 
      color = "#4dff88"; 
      state = "GREEN"; 
    }
  }

  s.replace("%COLOR_HEX%", color);
  s.replace("%LIGHT_STATE%", state);
  s.replace("%MODE%", (cur_mode == INPUT_SERVICE ? "MAINTENANCE" : "AUTOMATIC"));
  s.replace("%AUTO_ACTIVE%", (cur_mode == INPUT_AUTOMATIC ? "active" : ""));
  s.replace("%MAINT_ACTIVE%", (cur_mode == INPUT_SERVICE ? "active" : ""));

  server.send(200, "text/html", s);
}

void setup_server() {
  server.on("/", handle_server_root);

  server.on("/status", []() {
    String color = "#ffcc00"; 
    String state = "YELLOW";
    if (cur_mode == INPUT_SERVICE) {
        state = "MAINTENANCE";
    } else {
        if (cur_state == STATE_RED) { 
          color = "#ff4d4d"; 
          state = "RED"; 
        }
        else if (cur_state == STATE_GREEN) { 
          color = "#4dff88"; 
          state = "GREEN"; 
        }
    }
    
    String mode_str = (cur_mode == INPUT_SERVICE ? "MAINTENANCE" : "AUTOMATIC");
    String json = "{\"color\":\"" + color + "\", \"state\":\"" + state + "\", \"mode\":\"" + mode_str + "\"}";
    server.send(200, "application/json", json);
  });

  server.on("/auto", []() { 
    if (cur_mode != INPUT_AUTOMATIC) {
      cur_mode = INPUT_AUTOMATIC; 
      cur_state = STATE_RED;
      last_tick = millis(); 
      dt = RED_GREEN_DT;
      server.send(200); 
    }
  });
  server.on("/maintenance", []() { 
    if (cur_mode != INPUT_SERVICE) {
      cur_mode = INPUT_SERVICE; 
      cur_state = STATE_YELLOW;
      last_tick = millis(); 
      dt = YELLOW_DT;
      server.send(200); 
    }
  });
  server.on("/set_red", []() { 
    if (cur_mode == INPUT_AUTOMATIC) {
      cur_state = STATE_RED; 
      last_tick = millis(); 
      dt = RED_GREEN_DT;
      server.send(200); 
    }
  });
  server.on("/set_green", []() { 
    if (cur_mode == INPUT_AUTOMATIC) {
      cur_state = STATE_GREEN; 
      last_tick = millis(); 
      dt = RED_GREEN_DT;
      server.send(200); 
    }
  });

  server.begin();
}

void read_wifi() {
  if (!LittleFS.begin()) {
    Serial.println("[ERROR] LittleFS mount failed");
    return;
  }
  File file = LittleFS.open("/wifi.txt", "r");
  if (!file) {
    Serial.println("[ERROR] Failed to open wifi.txt");
  }

  wifi_ssid = file.readStringUntil('\n');
  wifi_pwd = file.readStringUntil('\n');

  file.close();
}

void setup() {
  Serial.begin(74880);
  Serial.println();

  last_tick = millis();

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);

  read_wifi();

  WiFi.begin(wifi_ssid.c_str(), wifi_pwd.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println();
  Serial.print("Connected to ");
  Serial.print(wifi_ssid);
  Serial.println("!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  setup_server();
}

void loop() {
  server.handleClient();
  check_mode();
  update_light();
  display_light();
}
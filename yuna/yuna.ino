// ============================================================================
// YUNA - ESP32 Quadruped Robot Controller
// Developed & Customized by SELADEVS (https://seladevs.my)
// ============================================================================
// Control Methods:
//   1. WiFi Web Controller (AP or STA mode)
//   2. PS5 DualSense Controller via Bluetooth (BluePad32)
//
// Dependencies (Arduino Library Manager):
//   1. ESP32Servo
//   2. ArduinoJson
//   3. Bluepad32 (for PS5 controller support)
// 
// 舵机与 esp32 引脚接线图, 数据口方向为后 (tail)
// 
//     前 (head)
//         -----               -----
//         |  2  |             |  3  |
//         |pin25|             |Pin18|
//         ----- -----   ----- -----
//             |  0  | |  1  |
//             |Pin12| |Pin16|
//              -----   -----
//             |  4  | |  5  |
//             |Pin13| |Pin17|
//         ----- -----   ----- -----
//         |  6  |             |  7  |
//         |Pin26|             |Pin19|
//         -----               -----
//     后 (tail)
//
// PS5 Controller Button Map:
//   Face Buttons:  △=Hello, ○=Dance, ✕=Home, □=Sit
//   D-Pad:         ▲=Forward, ▼=Backward, ◄=TurnL, ►=TurnR
//   Shoulders:     L1=MoonwalkL, R1=MoonwalkR
//   L1+Combos:     △=Cheer, ○=Boxing, ✕=Scared, □=ShakeHand,
//                  ▲=Jump, ▼=Hide, ◄=Twist, ►=Stretch
//   R1+Combos:     △=WaveHand, ○=FrontBack, ✕=PushUp, □=UpDown,
//                  ▲=TipToe, ▼=Pee, ◄=OmniWalk, ►=Run
//   Analog Sticks: Left stick for proportional movement
// ============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include "minikame.h"
#include "WiFiSetup.h"
#include "Config.h"

// PS5 Controller (conditionally included)
#if ENABLE_PS5_CONTROLLER
  #include "PS5Controller.h"
#endif

const int LED = 2;

WebServer server(80);

MiniKame robot;
bool running = 0;
int input = 0;

DynamicJsonDocument jsonDoc(1024);

String indexHtml = "<!DOCTYPE html><html><head>    <meta charset='utf-8'>    <meta name='viewport' content='width=device-width, initial-scale=1.0'>    <title>SELADEVS | Quad Robot Controller</title>    <style>        :root {            --bg-base: #0c0507;            --bg-card: rgba(26, 12, 15, 0.85);            --bg-btn: #250f14;            --bg-btn-hover: #3b141c;            --red-primary: #ff2a2a;            --red-glow: rgba(255, 42, 42, 0.45);            --red-subtle: rgba(255, 42, 42, 0.15);            --border: #4d1820;            --text-main: #f5f5f7;            --text-muted: #b8868c;        }        * {            box-sizing: border-box;            user-select: none;            -webkit-user-select: none;            margin: 0;            padding: 0;        }        body {            min-height: 100vh;            background: radial-gradient(circle at 50% 10%, #29080e 0%, #0d0406 65%, #050203 100%);            color: var(--text-main);            font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto, \"Helvetica Neue\", Arial, sans-serif;            display: flex;            flex-direction: column;            align-items: center;            justify-content: center;            padding: 20px 15px;        }        .header {            display: flex;            align-items: center;            gap: 16px;            margin-bottom: 24px;            text-align: left;        }        .logo-wrapper {            position: relative;            width: 58px;            height: 58px;            border-radius: 14px;            background: linear-gradient(135deg, rgba(255, 42, 42, 0.2), rgba(0,0,0,0.6));            border: 1px solid rgba(255, 42, 42, 0.4);            display: flex;            align-items: center;            justify-content: center;            box-shadow: 0 0 20px var(--red-glow);            overflow: hidden;            flex-shrink: 0;        }        .brand-logo {            width: 48px;            height: 48px;            object-fit: contain;            filter: drop-shadow(0 0 6px rgba(255, 42, 42, 0.6));        }        .header-info h1 {            font-size: 24px;            font-weight: 800;            letter-spacing: 2px;            text-transform: uppercase;            color: #ffffff;            line-height: 1.1;        }        .header-info h1 span {            color: var(--red-primary);            text-shadow: 0 0 12px var(--red-glow);        }        .tagline {            display: flex;            align-items: center;            gap: 8px;            font-size: 11px;            letter-spacing: 1.5px;            color: var(--text-muted);            font-weight: 600;            margin-top: 4px;            text-transform: uppercase;        }        .status-dot {            width: 7px;            height: 7px;            border-radius: 50%;            background: var(--red-primary);            box-shadow: 0 0 8px var(--red-primary);            animation: pulse 2s infinite;        }        @keyframes pulse {            0%, 100% { opacity: 1; transform: scale(1); }            50% { opacity: 0.4; transform: scale(0.85); }        }        .main-container {            display: flex;            flex-wrap: wrap;            justify-content: center;            align-items: stretch;            gap: 20px;            width: 100%;            max-width: 1100px;        }        .panel {            background: var(--bg-card);            border: 1px solid var(--border);            border-radius: 20px;            padding: 24px;            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.6), inset 0 1px 0 rgba(255, 255, 255, 0.05);            backdrop-filter: blur(10px);            -webkit-backdrop-filter: blur(10px);            display: flex;            flex-direction: column;        }        .panel-nav {            align-items: center;            justify-content: center;            min-width: 280px;        }        .panel-skills {            flex: 1;            min-width: 320px;        }        .panel-title {            font-size: 12px;            font-weight: 700;            text-transform: uppercase;            letter-spacing: 2px;            color: var(--text-muted);            margin-bottom: 20px;            display: flex;            align-items: center;            justify-content: space-between;            width: 100%;            border-bottom: 1px solid rgba(255, 42, 42, 0.12);            padding-bottom: 8px;        }        .panel-title span {            color: var(--red-primary);            font-weight: 800;        }        .dpad {            display: flex;            flex-direction: column;            align-items: center;            justify-content: center;            margin: auto;        }        .dpad-row {            display: flex;            align-items: center;            gap: 12px;            margin: 12px 0;        }        .button {            border: 1px solid var(--border);            background: var(--bg-btn);            color: #fff;            border-radius: 14px;            display: flex;            justify-content: center;            align-items: center;            cursor: pointer;            transition: all 0.18s cubic-bezier(0.4, 0, 0.2, 1);            position: relative;            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.35);        }        .button:hover {            background: var(--bg-btn-hover);            border-color: var(--red-primary);            box-shadow: 0 6px 16px var(--red-glow);            transform: translateY(-2px);        }        .button:active {            background: var(--red-primary);            color: #ffffff;            border-color: #ff5252;            box-shadow: 0 0 24px var(--red-primary);            transform: translateY(1px);        }        .dpad .button {            width: 82px;            height: 82px;            font-size: 32px;            color: var(--red-primary);        }        .dpad .button:active {            color: #fff;        }        .dpad .button#home {            background: linear-gradient(135deg, #3d1217, #22080c);            border-color: var(--red-primary);            font-size: 22px;            border-radius: 50%;        }        .skills-grid {            display: grid;            grid-template-columns: repeat(4, 1fr);            gap: 12px;            flex: 1;        }        .skills-grid .button {            min-height: 54px;            font-size: 14px;            font-weight: 600;            letter-spacing: 0.5px;            padding: 10px 8px;            text-align: center;            border-radius: 12px;            background: linear-gradient(180deg, #240d12 0%, #1a080c 100%);        }        .skills-grid .button:hover {            color: #ffffff;            border-color: var(--red-primary);        }        .terminal {            margin-top: 20px;            width: 100%;            max-width: 1100px;            background: #120507;            border: 1px solid var(--border);            border-radius: 12px;            padding: 10px 18px;            display: flex;            align-items: center;            justify-content: space-between;            font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;            font-size: 13px;            color: var(--text-muted);            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.4);        }        .terminal-cmd {            color: var(--red-primary);            font-weight: 700;        }        .terminal-credits {            font-size: 11px;            color: #7a4a50;        }        @media (max-width: 800px) {            .skills-grid {                grid-template-columns: repeat(3, 1fr);            }        }        @media (max-width: 580px) {            body {                padding: 12px 8px;            }            .header-info h1 {                font-size: 20px;            }            .dpad .button {                width: 72px;                height: 72px;                font-size: 26px;            }            .skills-grid {                grid-template-columns: repeat(2, 1fr);                gap: 8px;            }            .skills-grid .button {                min-height: 48px;                font-size: 13px;            }            .terminal {                flex-direction: column;                gap: 6px;                text-align: center;            }        }    </style></head><body>    <header class=\"header\">        <div class=\"logo-wrapper\">            <img src=\"https://www.seladevs.my/branding/seladevs-logo.png\"                  alt=\"SELADEVS\"                  class=\"brand-logo\"                  onerror=\"this.onerror=null; this.src='https://seladevs.my/branding/seladevs-logo.png';\" />        </div>        <div class=\"header-info\">            <h1>SELADEVS <span>QUAD BOT</span></h1>            <div class=\"tagline\">                <span class=\"status-dot\"></span>                <span>COLLECTIVE_09 · ESP32 READY</span>            </div>        </div>    </header>    <div class=\"main-container\">        <div class=\"panel panel-nav\">            <div class=\"panel-title\">                <span>NAVIGATION</span>                <span>DPAD</span>            </div>            <div class=\"dpad\">                <div class=\"button\" id=\"forward\">&#9650;</div>                <div class=\"dpad-row\">                    <div class=\"button direction\" id=\"turn_L\">&#9664;</div>                    <div class=\"button direction\" id=\"home\">&#9679;</div>                    <div class=\"button direction\" id=\"turn_R\">&#9654;</div>                </div>                <div class=\"button\" id=\"backward\">&#9660;</div>            </div>        </div>        <div class=\"panel panel-skills\">            <div class=\"panel-title\">                <span>SKILLS & TRICKS</span>                <span>20 ACTIONS</span>            </div>            <div class=\"skills-grid\">                <div class=\"button\" id=\"hello\">Hello</div>                <div class=\"button\" id=\"shake_hand\">ShakeHand</div>                <div class=\"button\" id=\"cheer\">Cheer</div>                <div class=\"button\" id=\"boxing\">Boxing</div>                <div class=\"button\" id=\"dance\">Dance</div>                <div class=\"button\" id=\"twist\">Twist</div>                <div class=\"button\" id=\"wave_hand\">WaveHand</div>                <div class=\"button\" id=\"omni_walk\">OmniWalk</div>                <div class=\"button\" id=\"moonwalk_L\">Moonwalk L</div>                <div class=\"button\" id=\"moonwalk_R\">Moonwalk R</div>                <div class=\"button\" id=\"up_down\">UpDown</div>                <div class=\"button\" id=\"front_back\">FrontBack</div>                <div class=\"button\" id=\"push_up\">PushUp</div>                <div class=\"button\" id=\"stretch\">Stretch</div>                <div class=\"button\" id=\"tiptoe\">TipToe</div>                <div class=\"button\" id=\"jump\">Jump</div>                <div class=\"button\" id=\"sit\">Sit</div>                <div class=\"button\" id=\"hide\">Hide</div>                <div class=\"button\" id=\"scared\">Scared</div>                <div class=\"button\" id=\"pee\">Pee</div>            </div>        </div>    </div>    <div class=\"terminal\">        <div id=\"status-line\">SYSTEM STATUS: <span class=\"terminal-cmd\">READY</span></div>        <div class=\"terminal-credits\">SELADEVS.MY · QUADRUPED ROBOT</div>    </div>    <script>        const statusEl = document.getElementById('status-line');        function buttonClick(c) {            statusEl.innerHTML = 'EXECUTING &gt; <span class=\"terminal-cmd\">' + c.toUpperCase() + '</span>...';            fetch('/control', {                method: 'POST',                headers: {                    'Content-Type': 'application/json'                },                body: JSON.stringify({ command: c })            })            .then(response => response.json())            .then(data => {                statusEl.innerHTML = 'SUCCESS &gt; <span class=\"terminal-cmd\">' + c.toUpperCase() + '</span> EXECUTED';            })            .catch(error => {                statusEl.innerHTML = 'ERROR &gt; <span class=\"terminal-cmd\">' + c.toUpperCase() + '</span> FAILED';            });        }        document.querySelectorAll('.button').forEach(button => {            button.addEventListener('click', () => buttonClick(button.id));        });    </script></body></html>";


void setup(void) {

  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);

  Serial.begin(115200);  // Increased baud rate for better debug output
  Serial.println();
  Serial.println("============================================");
  Serial.println("  YUNA Quadruped Robot - SELADEVS");
  Serial.println("  WiFi + PS5 DualSense Controller");
  Serial.println("============================================");

  // Initialize the robot
  robot.init();
  robot.home();
  Serial.println("Robot initialized and in home position.");

  // Initialize WiFi
  bool wifiConnected = initWiFi();

  // 根据WiFi连接状态设置LED
  digitalWrite(LED, wifiConnected ? 1 : 0);

  if (wifiConnected) {
    // 启动Web服务器
    server.begin();
    server.on("/", HTTP_GET, handleRoot);
    server.on("/control", HTTP_POST, handlePost);
    // PS5 controller status endpoint
    server.on("/ps5status", HTTP_GET, handlePS5Status);
    server.onNotFound(handleNotFound);

    Serial.println("HTTP server started: ready for connections");
  } else {
    Serial.println("WiFi configuration failed, please check Config.h");
  }

  // Initialize PS5 Controller (works independently of WiFi)
  #if ENABLE_PS5_CONTROLLER
    Serial.println();
    Serial.println("Initializing PS5 DualSense controller...");
    initPS5Controller(&robot);
    Serial.println("PS5 controller ready for pairing!");
    Serial.println("  -> Hold CREATE + PS button on DualSense");
  #else
    Serial.println("PS5 controller support is DISABLED in Config.h");
  #endif

  Serial.println();
  Serial.println("============================================");
  Serial.println("  System ready! Waiting for input...");
  Serial.println("============================================");
}

void loop() {
  // Handle WiFi web server requests
  server.handleClient();

  // Handle PS5 controller input
  #if ENABLE_PS5_CONTROLLER
    updatePS5Controller();
  #endif
}


void handleRoot() {
  server.send(200, "text/html", indexHtml);
}

void handlePost() {
  String postBody = server.arg("plain");
  Serial.print("WiFi Command: ");
  Serial.println(postBody);
  jsonDoc.clear();

  DeserializationError error = deserializeJson(jsonDoc, postBody);

  if (error) {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    return;
  }

  const char *command = jsonDoc["command"];

  // 处理命令
  if (command) {

    bool commandProcessed = processCommand(command);

    if (commandProcessed) {
      server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Command executed\"}");
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Unknown command\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No command specified\"}");
  }
}

// PS5 controller status endpoint for web UI
void handlePS5Status() {
  String json = "{\"ps5_connected\":";
  #if ENABLE_PS5_CONTROLLER
    json += isPS5Connected() ? "true" : "false";
  #else
    json += "false";
  #endif
  json += "}";
  server.send(200, "application/json", json);
}

bool processCommand(const char* command) {
  String cmd = String(command);
  
  if (cmd == "forward") {
    robot.walk(2, 250);
    return true;
  }
  else if (cmd == "walk") {
    robot.walk(2, 250);
    return true;
  }
  else if (cmd == "trot") {
    robot.forward(2, 400);
    return true;
  } 
  else if (cmd == "turn_L") {
    robot.turnL(2, 550);
    return true;
  }
  else if (cmd == "home") {
    robot.home();
    return true;
  }
  else if (cmd == "turn_R") {
    robot.turnR(2, 550);
    return true;
  }
  else if (cmd == "backward") {
    robot.backward(2, 550);
    return true;
  }
  else if (cmd == "hello") {
    robot.hello();
    return true;
  }
  else if (cmd == "omni_walk") {
    robot.omniWalk(2, 500, true, 2);
    return true;
  }
  else if (cmd == "moonwalk_L") {
    robot.moonwalkL(4, 1000);
    return true;
  }
  else if (cmd == "moonwalk_R") {
    robot.moonwalkR(4, 1000);
    return true;
  }
  else if (cmd == "dance") {
    robot.dance(3, 1000);
    return true;
  }
  else if (cmd == "up_down") {
    robot.upDown(2, 1000);
    return true;
  }
  else if (cmd == "push_up") {
    robot.pushUp(2, 1000);
    return true;
  }
  else if (cmd == "front_back") {
    robot.frontBack(2, 500);
    return true;
  }
  else if (cmd == "wave_hand") {
    robot.waveHand(3, 1000);
    return true;
  }
  else if (cmd == "hide") {
    robot.hide(1, 1000);
    return true;
  }
  else if (cmd == "scared") {
    robot.scared();
    return true;
  }
  else if (cmd == "sit") {
    robot.sit();
    return true;
  }
  else if (cmd == "stretch") {
    robot.stretch();
    return true;
  }
  else if (cmd == "shake_hand") {
    robot.shakeHand();
    return true;
  }
  else if (cmd == "cheer") {
    robot.cheer();
    return true;
  }
  else if (cmd == "boxing") {
    robot.boxing();
    return true;
  }
  else if (cmd == "twist") {
    robot.twist(4, 500);
    return true;
  }
  else if (cmd == "tiptoe") {
    robot.tiptoe();
    return true;
  }
  else if (cmd == "jump") {
    robot.jump();
    return true;
  }
  else if (cmd == "pee") {
    robot.pee();
    return true;
  }
  else {
    // 未知命令
    return false;
  }
}

// 设置处理404情况的函数'handleNotFound'
void handleNotFound() {
  server.send(404, "text / plain", "404: Not found");  // 发送 HTTP 状态 404 (未找到页面) 并向浏览器发送文字 "404: Not found"
}

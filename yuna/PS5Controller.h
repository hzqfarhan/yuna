#ifndef PS5_CONTROLLER_H
#define PS5_CONTROLLER_H

// ============================================================================
// PS5 DualSense Controller Handler for YUNA Quadruped Robot
// Uses BluePad32 library for ESP32 Bluetooth gamepad support
// Developed by SELADEVS (https://seladevs.my)
// ============================================================================

#include <Bluepad32.h>
#include "minikame.h"
#include "Config.h"

// Controller state
static ControllerPtr myController = nullptr;
static MiniKame* _robotRef = nullptr;
static bool _controllerConnected = false;
static unsigned long _lastActionTime = 0;

// Prevent action spam — minimum interval between actions (ms)
static const unsigned long ACTION_COOLDOWN = 300;

// Forward declarations
void onConnectedController(ControllerPtr ctl);
void onDisconnectedController(ControllerPtr ctl);
void processControllerInput(ControllerPtr ctl);

// ============================================================================
// Initialization
// ============================================================================
void initPS5Controller(MiniKame* robot) {
  _robotRef = robot;

  Serial.println("PS5 Controller: Initializing BluePad32...");
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // Forget previously paired controllers (optional — helps with fresh pairing)
  // BP32.forgetBluetoothKeys();

  Serial.println("PS5 Controller: Ready for pairing!");
  Serial.println("  -> Hold CREATE + PS button on DualSense to pair");
}

// ============================================================================
// Connection Callbacks
// ============================================================================
void onConnectedController(ControllerPtr ctl) {
  if (myController == nullptr) {
    Serial.println("PS5 Controller: CONNECTED!");
    Serial.printf("  Model: %s\n", ctl->getModelName().c_str());
    myController = ctl;
    _controllerConnected = true;

    // Flash LED to indicate connection
    digitalWrite(2, HIGH);
  } else {
    Serial.println("PS5 Controller: Another controller tried to connect, rejecting.");
    ctl->disconnect();
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  if (myController == ctl) {
    Serial.println("PS5 Controller: DISCONNECTED!");
    myController = nullptr;
    _controllerConnected = false;
  }
}

// ============================================================================
// Check if controller is connected
// ============================================================================
bool isPS5Connected() {
  return _controllerConnected && myController != nullptr && myController->isConnected();
}

// ============================================================================
// Cooldown helper — prevents rapid-fire action spam
// ============================================================================
bool canExecuteAction() {
  unsigned long now = millis();
  if (now - _lastActionTime >= ACTION_COOLDOWN) {
    _lastActionTime = now;
    return true;
  }
  return false;
}

// ============================================================================
// ============================================================================
// Update loop — call this in loop()
// ============================================================================
void updatePS5Controller() {
  // Must call BP32.update() to process Bluetooth events
  BP32.update();

  if (!isPS5Connected() || _robotRef == nullptr || !myController->hasData()) {
    return;
  }

  processControllerInput(myController);
}

// D-Pad constants fallback if not defined
#ifndef DPAD_UP
  #define DPAD_UP 0x01
#endif
#ifndef DPAD_DOWN
  #define DPAD_DOWN 0x02
#endif
#ifndef DPAD_RIGHT
  #define DPAD_RIGHT 0x04
#endif
#ifndef DPAD_LEFT
  #define DPAD_LEFT 0x08
#endif

// ============================================================================
// Main input processing
// ============================================================================
void processControllerInput(ControllerPtr ctl) {
  // Read all inputs
  int16_t axisLX = ctl->axisX();      // Left stick X: -511 to 512
  int16_t axisLY = ctl->axisY();      // Left stick Y: -511 to 512
  int16_t axisRX = ctl->axisRX();     // Right stick X
  int16_t axisRY = ctl->axisRY();     // Right stick Y
  int16_t brake  = ctl->brake();      // L2 trigger: 0 to 1023
  int16_t throttle = ctl->throttle(); // R2 trigger: 0 to 1023

  // Individual button checks using Controller methods
  bool btnTriangle = ctl->y();       // △ Triangle
  bool btnCircle   = ctl->b();       // ○ Circle
  bool btnCross    = ctl->a();       // ✕ Cross
  bool btnSquare   = ctl->x();       // □ Square
  bool btnL1       = ctl->l1();      // L1
  bool btnR1       = ctl->r1();      // R1

  // D-Pad
  uint8_t dpad = ctl->dpad();
  bool dpadUp    = (dpad & DPAD_UP);
  bool dpadDown  = (dpad & DPAD_DOWN);
  bool dpadLeft  = (dpad & DPAD_LEFT);
  bool dpadRight = (dpad & DPAD_RIGHT);

  // Miscellaneous buttons
  bool btnL3 = ctl->thumbL();   // Left stick click
  bool btnR3 = ctl->thumbR();   // Right stick click

  // ================================================================
  // PRIORITY 1: L1 + Button Combos (Skills Set 1)
  // ================================================================
  if (btnL1) {
    if (!canExecuteAction()) return;

    if (btnTriangle) {
      Serial.println("PS5: L1+△ → Cheer");
      _robotRef->cheer();
    }
    else if (btnCircle) {
      Serial.println("PS5: L1+○ → Boxing");
      _robotRef->boxing();
    }
    else if (btnCross) {
      Serial.println("PS5: L1+✕ → Scared");
      _robotRef->scared();
    }
    else if (btnSquare) {
      Serial.println("PS5: L1+□ → ShakeHand");
      _robotRef->shakeHand();
    }
    else if (dpadUp) {
      Serial.println("PS5: L1+▲ → Jump");
      _robotRef->jump();
    }
    else if (dpadDown) {
      Serial.println("PS5: L1+▼ → Hide");
      _robotRef->hide(1, 2000);
    }
    else if (dpadLeft) {
      Serial.println("PS5: L1+◄ → Twist");
      _robotRef->twist(4, 1000);
    }
    else if (dpadRight) {
      Serial.println("PS5: L1+► → Stretch");
      _robotRef->stretch();
    }
    // L1 alone = MoonwalkL
    else {
      Serial.println("PS5: L1 → MoonwalkL");
      _robotRef->moonwalkL(4, 2000);
    }
    return;
  }

  // ================================================================
  // PRIORITY 2: R1 + Button Combos (Skills Set 2)
  // ================================================================
  if (btnR1) {
    if (!canExecuteAction()) return;

    if (btnTriangle) {
      Serial.println("PS5: R1+△ → WaveHand");
      _robotRef->waveHand(3, 2000);
    }
    else if (btnCircle) {
      Serial.println("PS5: R1+○ → FrontBack");
      _robotRef->frontBack(2, 1000);
    }
    else if (btnCross) {
      Serial.println("PS5: R1+✕ → PushUp");
      _robotRef->pushUp(2, 2000);
    }
    else if (btnSquare) {
      Serial.println("PS5: R1+□ → UpDown");
      _robotRef->upDown(2, 2000);
    }
    else if (dpadUp) {
      Serial.println("PS5: R1+▲ → TipToe");
      _robotRef->tiptoe();
    }
    else if (dpadDown) {
      Serial.println("PS5: R1+▼ → Pee");
      _robotRef->pee();
    }
    else if (dpadLeft) {
      Serial.println("PS5: R1+◄ → OmniWalk");
      _robotRef->omniWalk(2, 1000, true, 2);
    }
    else if (dpadRight) {
      Serial.println("PS5: R1+► → Run");
      _robotRef->run(3, 5000);
    }
    // R1 alone = MoonwalkR
    else {
      Serial.println("PS5: R1 → MoonwalkR");
      _robotRef->moonwalkR(4, 2000);
    }
    return;
  }

  // ================================================================
  // PRIORITY 3: Face Buttons (no modifier held)
  // ================================================================
  if (btnTriangle && canExecuteAction()) {
    Serial.println("PS5: △ → Hello");
    _robotRef->hello();
    return;
  }
  if (btnCircle && canExecuteAction()) {
    Serial.println("PS5: ○ → Dance");
    _robotRef->dance(3, 2000);
    return;
  }
  if (btnCross && canExecuteAction()) {
    Serial.println("PS5: ✕ → Home");
    _robotRef->home();
    return;
  }
  if (btnSquare && canExecuteAction()) {
    Serial.println("PS5: □ → Sit");
    _robotRef->sit();
    return;
  }

  // L3 click = Home position
  if (btnL3 && canExecuteAction()) {
    Serial.println("PS5: L3 → Home");
    _robotRef->home();
    return;
  }

  // ================================================================
  // PRIORITY 4: R2 Trigger → Run (fast forward)
  // ================================================================
  if (throttle > 100 && canExecuteAction()) {
    Serial.println("PS5: R2 → Run");
    _robotRef->run(2, 1500);
    return;
  }

  // ================================================================
  // PRIORITY 5: D-Pad Movement (digital, fixed steps)
  // ================================================================
  if (dpadUp && canExecuteAction()) {
    Serial.println("PS5: D-Pad ▲ → Walk");
    _robotRef->walk(2, 250);
    return;
  }
  if (dpadDown && canExecuteAction()) {
    Serial.println("PS5: D-Pad ▼ → Backward");
    _robotRef->backward(2, 550);
    return;
  }
  if (dpadLeft && canExecuteAction()) {
    Serial.println("PS5: D-Pad ◄ → TurnL");
    _robotRef->turnL(2, 550);
    return;
  }
  if (dpadRight && canExecuteAction()) {
    Serial.println("PS5: D-Pad ► → TurnR");
    _robotRef->turnR(2, 550);
    return;
  }

  // ================================================================
  // PRIORITY 5: Analog Stick Movement (proportional control)
  // ================================================================
  // Left stick for forward/backward/turning
  // Apply deadzone
  if (abs(axisLY) > PS5_STICK_DEADZONE || abs(axisLX) > PS5_STICK_DEADZONE) {
    if (!canExecuteAction()) return;

    // Calculate speed from stick magnitude (1 = slow, 2 = medium steps)
    float magnitude = sqrt(axisLX * axisLX + axisLY * axisLY) / 512.0;
    int steps = (magnitude > 0.7) ? 3 : 1;
    int speed = (int)(700 - (magnitude * 350)); // Faster when pushed further
    if (speed < 300) speed = 300;
    if (speed > 700) speed = 700;

    // Determine primary direction
    if (abs(axisLY) > abs(axisLX)) {
      // Forward or backward
      if (axisLY < -PS5_STICK_DEADZONE) {
        Serial.printf("PS5: Stick → Walk (steps=%d)\n", steps);
        _robotRef->walk(steps, 250);
      } else {
        Serial.printf("PS5: Stick → Backward (speed=%d, steps=%d)\n", speed, steps);
        _robotRef->backward(steps, speed);
      }
    } else {
      // Turn left or right
      if (axisLX < -PS5_STICK_DEADZONE) {
        Serial.printf("PS5: Stick → TurnL (speed=%d)\n", speed);
        _robotRef->turnL(steps, speed);
      } else {
        Serial.printf("PS5: Stick → TurnR (speed=%d)\n", speed);
        _robotRef->turnR(steps, speed);
      }
    }
  }
}

#endif // PS5_CONTROLLER_H

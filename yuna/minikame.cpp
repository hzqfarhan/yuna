#include "minikame.h"


void MiniKame::init() {
  // Map between servos and board pins
  board_pins[0] = 12; // Servo S0
  board_pins[1] = 16; // Servo S1
  board_pins[2] = 25; // Servo S2
  board_pins[3] = 18; // Servo S3
  board_pins[4] = 13; // Servo S4
  board_pins[5] = 17; // Servo S5
  board_pins[6] = 26; // Servo S6
  board_pins[7] = 19; // Servo S7

  // Trim values for zero position calibration.

    trim[0] = 0; // 前左大腿
    trim[1] = 0; // 前右大腿
    trim[2] = 0; // 前左小腿
    trim[3] = 0; // 前右小腿
    trim[4] = 0; // 后左大腿
    trim[5] = 0; // 后右大腿
    trim[6] = 0; // 后左小腿
    trim[7] = 0; // 后右小腿
    
  // Set reverse movement
  for (int i = 0; i < 8; i++) reverse[i] = false;

  // Initialize rest state and speed factor
  _isResting = false;
  _speedFactor = 1.0;

  // Init an oscillator for each servo
  for (int i = 0; i < 8; i++) {
    oscillator[i].start();
    servo[i].attach(board_pins[i]);
  }
  home();
}

// -- Servo power management (adopted from quad-mpy) --
void MiniKame::attachServos() {
  for (int i = 0; i < 8; i++) {
    if (!servo[i].attached()) {
      servo[i].attach(board_pins[i]);
    }
  }
  if (_isResting) {
    _isResting = false;
  }
}

void MiniKame::detachServos() {
  for (int i = 0; i < 8; i++) {
    if (servo[i].attached()) {
      servo[i].detach();
    }
  }
  _isResting = true;
}

bool MiniKame::isResting() {
  return _isResting;
}

// -- Speed factor for PS5 proportional control --
void MiniKame::setSpeedFactor(float factor) {
  if (factor < 0.0) factor = 0.0;
  if (factor > 1.0) factor = 1.0;
  _speedFactor = factor;
}

float MiniKame::getSpeedFactor() {
  return _speedFactor;
}

void MiniKame::reverseServo(int id) {
  if (reverse[id])
    reverse[id] = 0;
  else
    reverse[id] = 1;
}


void MiniKame::setServo(int id, float target) {
  if (!reverse[id])
    servo[id].writeMicroseconds(angToUsec(target + trim[id]));
  else
    servo[id].writeMicroseconds(angToUsec(180 - (target + trim[id])));
  _servo_position[id] = target;
}

float MiniKame::getServo(int id) {
  return _servo_position[id];
}


void MiniKame::moveServos(int time, float target[8]) {
  attachServos();  // Ensure servos are attached (quad-mpy pattern)
  if (time > 10) {
    for (int i = 0; i < 8; i++)	_increment[i] = (target[i] - _servo_position[i]) / (time / 10.0);
    _final_time =  millis() + time;

    while (millis() < _final_time) {
      _partial_time = millis() + 10;
      for (int i = 0; i < 8; i++) setServo(i, _servo_position[i] + _increment[i]);
      while (millis() < _partial_time); //pause
    }
  }
  else {
    for (int i = 0; i < 8; i++) setServo(i, target[i]);
  }
  for (int i = 0; i < 8; i++) _servo_position[i] = target[i];
}

void MiniKame::execute(float steps, int period[8], int amplitude[8], int offset[8], int phase[8]) {
  attachServos();  // Ensure servos are attached (quad-mpy pattern)

  for (int i = 0; i < 8; i++) {
    oscillator[i].setPeriod(period[i]);
    oscillator[i].setAmplitude(amplitude[i]);
    oscillator[i].setPhase(phase[i]);
    oscillator[i].setOffset(offset[i]);
  }

  unsigned long global_time = millis();

  for (int i = 0; i < 8; i++) oscillator[i].setTime(global_time);

  _final_time = millis() + period[0] * steps;
  while (millis() < _final_time) {
    for (int i = 0; i < 8; i++) {
      setServo(i, oscillator[i].refresh());
    }
    yield();
  }
}

int MiniKame::angToUsec(float value) {
  return value / 180 * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) + MIN_PULSE_WIDTH;
}

// ============================================================================
// 定义动作 (Action Definitions)
// ============================================================================

// -- Home position (adopted quad-mpy rest-state pattern) --
void MiniKame::home() {
  int ap = 2;
  int hi = 2;
  int position[] = {90 + ap, 90 - ap, 90 - hi, 90 + hi, 90 - ap, 90 + ap, 90 + hi, 90 - hi};
  for (int i = 0; i < 8; i++) setServo(i, position[i]);
}

// -- Forward gait (trot gait with correct outward X-stance) --
void MiniKame::forward(float steps, int t) {
  int x_amp = 30;
  int z_amp = 25;
  int ap = 20;
  int hi = 20;
  int period[8];
  for (int i = 0; i < 8; i++) {
    period[i] = t;
  }
  int amplitude[8] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[8] = {
    90 - ap,
    90 + ap,
    90 - hi,
    90 + hi,
    90 + ap,
    90 - ap,
    90 + hi,
    90 - hi
  };
  int phase[8] = {0, 0, 90, 90, 180, 180, 90, 90};
  execute(steps, period, amplitude, offset, phase);
}

// -- Backward gait --
void MiniKame::backward(float steps, int t) {
  int x_amp = 30;
  int z_amp = 25;
  int ap = 20;
  int hi = 20;
  int period[8] = {t, t, t, t, t, t, t, t};
  int amplitude[8] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[8] = {
    90 - ap,
    90 + ap,
    90 - hi,
    90 + hi,
    90 + ap,
    90 - ap,
    90 + hi,
    90 - hi
  };
  int phase[8] = {180, 180, 90, 90, 0, 0, 90, 90};
  execute(steps, period, amplitude, offset, phase);
}

// -- Turn Left --
void MiniKame::turnL(float steps, int t) {
  int x_amp = 25;
  int z_amp = 20;
  int ap = 15;
  int hi = 25;
  int period[] = {t, t, t, t, t, t, t, t};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {90 - ap, 90 + ap, 90 - hi, 90 + hi, 90 + ap, 90 - ap, 90 + hi, 90 - hi };
  int phase[] = {180, 0, 90, 90, 0, 180, 90, 90};

  execute(steps, period, amplitude, offset, phase);
}

// -- Turn Right --
void MiniKame::turnR(float steps, int t) {
  int x_amp = 25;
  int z_amp = 20;
  int ap = 15;
  int hi = 25;
  int period[] = {t, t, t, t, t, t, t, t};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {90 - ap, 90 + ap, 90 - hi, 90 + hi, 90 + ap, 90 - ap, 90 + hi, 90 - hi };
  int phase[] = {0, 180, 90, 90, 180, 0, 90, 90};

  execute(steps, period, amplitude, offset, phase);
}


void MiniKame::hello() {

  int a = 50;
  int b = 30;
  int c = 20;
  int d = 70;
  float state1[8] = {90 - a, 90, 90 + c, 90 - c,
                     90 + c, 90 - c, 90 - d, 90 + d};

  float state2[8] = {90 - a, 90 + b, 90 + c, 90 + d,
                     90 + c, 90 - c, 90 - d, 90 + d};

  float state3[8] = {90 - a, 90 - b, 90 + c, 90 + d,
                     90 + c, 90 - c, 90 - d, 90 + d};

  float state4[8] = {90, 90, 90, 90, 90, 90, 90, 90};

  moveServos(300, state1);

  for (int i = 0; i < 3; i++) {
    moveServos(200, state2);
    moveServos(200, state3);
  }

  delay(300);
  moveServos(200, state4);
}


// -- OmniWalk (updated from quad-mpy: ap=0 for cleaner omni-walk) --
void MiniKame::omniWalk(float steps=2, int t=1000, bool side=true, float turn_factor=2) {
  int x_amp = 20;
  int z_amp = 15;
  int ap = 0;       // quad-mpy: was 15, now 0 for cleaner omni-walk
  int hi = 23;
  int front_x = 6 * (1 - pow(turn_factor, 2));
  int period[] = {t, t, t, t, t, t, t, t};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {    90 + ap - front_x,
                      90 - ap + front_x,
                      90 - hi,
                      90 + hi,
                      90 - ap - front_x,
                      90 + ap + front_x,
                      90 + hi,
                      90 - hi
                 };

  int phase[8];
  if (side) {
    int phase1[] =  {0,   0,   90,  90,  180, 180, 90,  90};
    int phase2R[] = {0,   180, 90,  90,  180, 0,   90,  90};
    for (int i = 0; i < 8; i++)
      phase[i] = phase1[i] * (1 - turn_factor) + phase2R[i] * turn_factor;
  }
  else {
    int phase1[] =  {0,   0,   90,  90,  180, 180, 90,  90};
    int phase2L[] = {180, 0,   90,  90,  0,   180, 90,  90};
    for (int i = 0; i < 8; i++)
      phase[i] = phase1[i] * (1 - turn_factor) + phase2L[i] * turn_factor + oscillator[i].getPhaseProgress();
  }

  execute(steps, period, amplitude, offset, phase);
}

 
// -- MoonwalkL (updated from quad-mpy: uses z_amp+offset formula) --
void MiniKame::moonwalkL(float steps=4, int t=2000) {
  int z_amp = 25;   // quad-mpy: was 30, now 25
  int o = 5;        // quad-mpy: offset adjustment
  int period[] = {t, t, t, t, t, t, t, t};
  int amplitude[] = {0, 0, z_amp, z_amp, 0, 0, z_amp, z_amp};
  int offset[] = {90, 90, 90 - z_amp - o, 90 + z_amp + o, 90, 90, 90 + z_amp + o, 90 - z_amp - o};
  int phase[] = {0, 0, 0, 80, 0, 0, 160, 290};

  execute(steps, period, amplitude, offset, phase);
}


void MiniKame::dance(float steps=3, int t=2000) {
  int x_amp = 0;
  int z_amp = 30;
  int ap = 0;
  int hi = 20;
  int period[] = {t, t, t, t, t, t, t, t};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {90 + ap, 90 - ap, 90 - hi, 90 + hi, 90 - ap, 90 + ap, 90 + hi, 90 - hi};
  int phase[] = {0, 0, 0, 270, 0, 0, 90, 180};

  execute(steps, period, amplitude, offset, phase);
}


void MiniKame::upDown(float steps=2, int t=2000) {
  int x_amp = 0;
  int z_amp = 35;
  int ap = 10;
  int hi = 15;
  int front_x = 0;
  int period[8] = {t, t, t, t, t, t, t, t};
  int amplitude[8] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[8] = {
    90 + ap - front_x,
    90 - ap + front_x,
    90 - hi,
    90 + hi,
    90 - ap - front_x,
    90 + ap + front_x,
    90 + hi,
    90 - hi
  };
  int phase[8] = {0, 0, 90, 270, 180, 180, 270, 90};
  execute(steps, period, amplitude, offset, phase);
}


void MiniKame::pushUp(float steps=2, int t=2000) {
  int z_amp = 40;
  int x_amp = 45;
  int hi = 0;
  int b = 35;
  int period[8];
  for (int i = 0; i < 8; i++) {
    period[i] = t;
  }
  int amplitude[8] = {0, 0, z_amp, z_amp, 0, 0, 0, 0};
  int offset[8] = {
    90,
    90,
    90 - hi,
    90 + hi,
    90 + x_amp,
    90 - x_amp,
    90 + b,
    90 - b
  };
  int phase[8] = {0, 0, 90, -90, 0, 0, 0, 0};
  execute(steps, period, amplitude, offset, phase);
}


// -- FrontBack (updated from quad-mpy: z_amp=20, ap=15 for smoother motion) --
void MiniKame::frontBack(float steps=2, int t=1000) {
  int x_amp = 30;
  int z_amp = 20;   // quad-mpy: was 25, now 20
  int ap = 15;      // quad-mpy: was 20, now 15
  int hi = 30;
  int period[] = {t, t, t, t, t, t, t, t};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {90 + ap, 90 - ap, 90 - hi, 90 + hi, 90 - ap, 90 + ap, 90 + hi, 90 - hi};
  int phase[] = {0, 180, 270, 90, 0, 180, 90, 270};

  execute(steps, period, amplitude, offset, phase);
}


void MiniKame::waveHand(float steps=3, int t=2000) {
  int period[8];
  for (int i = 0; i < 8; i++) {
    period[i] = t;
  }
  int amplitude[8] = {20, 0, 0, 30, 0, 0, 0, 0};
  int offset[8] = {
    90 - 50,
    90,
    90 + 20,
    90 + 60,
    90,
    90,
    90,
    90
  };
  int phase[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  execute(steps, period, amplitude, offset, phase);
}


void MiniKame::hide(float steps=1.0, int t=2000) {
  int a = 60;
  int b = 70;
  int period[8] = {t, t, t, t, t, t, t, t};
  int amplitude[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  int offset[8] = {
    90 - a,
    90 + a,
    90 + b,
    90 - b,
    90 + a,
    90 - a,
    90 - b,
    90 + b
  };
  int phase[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  execute(steps, period, amplitude, offset, phase);
}


void MiniKame::scared() {
  int ap = 10;
  int hi = 40;

  float sentado[8] = {90 - 15, 90 + 15, 90 - hi, 90 + hi, 90 - 20, 90 + 20, 90 + hi, 90 - hi};
  float salto[8] = {90 - ap, 90 + ap, 160, 20, 90 + ap * 3, 90 - ap * 3, 20, 160};

  moveServos(600, sentado);
  moveServos(1000, salto);
  delay(1000);
}

void MiniKame::run(float steps, int t) {
  int x_amp = 35;
  int z_amp = 25;
  int ap = 20;
  int hi = 20;
  int front_x = 8;
  int period[] = {t, t, t, t, t, t, t, t};
  int amplitude[] = {x_amp, x_amp, z_amp, z_amp, x_amp, x_amp, z_amp, z_amp};
  int offset[] = {    90 + ap - front_x,
                      90 - ap + front_x,
                      90 - hi,
                      90 + hi,
                      90 - ap - front_x,
                      90 + ap + front_x,
                      90 + hi,
                      90 - hi
                 };
  int phase[] = {0, 0, 90, 90, 180, 180, 90, 90};

  execute(steps, period, amplitude, offset, phase);
}

// -- 8-Step Kinematic Creep Gait (adopted from quad-mpy) --
void MiniKame::walk(float steps, int t) {
  float a = 24.0f;
  float ao = 50.0f;
  float b = 8.0f;
  float c = -30.0f;
  float co = 10.0f;

  float step1[8] = {
    90.0f + 2.0f * a - ao, 90.0f - 4.0f * a + ao,
    90.0f + c + 5.0f * b,  90.0f - c - 4.0f * b,
    90.0f + 3.0f * a - co, 90.0f - 1.0f * a + co,
    90.0f - c - 4.0f * b - 10.0f, 90.0f + c + 6.0f * b
  };
  float step2[8] = {
    90.0f + 2.3f * a - ao, 90.0f - 2.0f * a + ao,
    90.0f + c + 5.0f * b,  90.0f - c - 0.0f * b,
    90.0f + 3.3f * a - co, 90.0f - 1.3f * a + co,
    90.0f - c - 4.0f * b - 10.0f, 90.0f + c + 6.0f * b
  };
  float step3[8] = {
    90.0f + 3.0f * a - ao, 90.0f - 1.0f * a + ao,
    90.0f + c + 4.0f * b,  90.0f - c - 6.0f * b,
    90.0f + 4.0f * a - co, 90.0f - 2.0f * a + co,
    90.0f - c - 4.0f * b - 10.0f, 90.0f + c + 5.0f * b
  };
  float step4[8] = {
    90.0f + 3.3f * a - ao, 90.0f - 1.3f * a + ao,
    90.0f + c + 4.0f * b,  90.0f - c - 6.0f * b,
    90.0f + 2.0f * a - co, 90.0f - 2.3f * a + co,
    90.0f - c - 0.0f * b - 10.0f, 90.0f + c + 5.0f * b
  };
  float step5[8] = {
    90.0f + 4.0f * a - ao, 90.0f - 2.0f * a + ao,
    90.0f + c + 4.0f * b,  90.0f - c - 5.0f * b,
    90.0f + 1.0f * a - co, 90.0f - 3.0f * a + co,
    90.0f - c - 6.0f * b - 10.0f, 90.0f + c + 4.0f * b
  };
  float step6[8] = {
    90.0f + 2.0f * a - ao, 90.0f - 2.3f * a + ao,
    90.0f + c + 0.0f * b,  90.0f - c - 5.0f * b,
    90.0f + 1.3f * a - co, 90.0f - 3.3f * a + co,
    90.0f - c - 6.0f * b - 10.0f, 90.0f + c + 4.0f * b
  };
  float step7[8] = {
    90.0f + 1.0f * a - ao, 90.0f - 3.0f * a + ao,
    90.0f + c + 6.0f * b,  90.0f - c - 4.0f * b,
    90.0f + 2.0f * a - co, 90.0f - 4.0f * a + co,
    90.0f - c - 5.0f * b - 10.0f, 90.0f + c + 4.0f * b
  };
  float step8[8] = {
    90.0f + 1.3f * a - ao, 90.0f - 3.3f * a + ao,
    90.0f + c + 6.0f * b,  90.0f - c - 4.0f * b,
    90.0f + 2.3f * a - co, 90.0f - 2.0f * a + co,
    90.0f - c - 5.0f * b - 10.0f, 90.0f + c + 0.0f * b
  };

  int cycles = (int)steps;
  if (cycles < 1) cycles = 1;

  for (int s = 0; s < cycles; s++) {
    moveServos(t, step1);
    moveServos(t / 3, step2);
    moveServos(t, step3);
    moveServos(t / 3, step4);
    moveServos(t, step5);
    moveServos(t / 3, step6);
    moveServos(t, step7);
    moveServos(t / 3, step8);
  }
}

void MiniKame::moonwalkR(float steps, int t) {
  int z_amp = 25;   // quad-mpy: was 30, now 25 to match moonwalkL
  int o = 5;        // quad-mpy: offset adjustment
  int period[] = {t, t, t, t, t, t, t, t};
  int amplitude[] = {0, 0, z_amp, z_amp, 0, 0, z_amp, z_amp};
  int offset[] = {90, 90, 90 - z_amp - o, 90 + z_amp + o, 90, 90, 90 + z_amp + o, 90 - z_amp - o};
  int phase[] = {0, 0, 80, 0, 0, 0, 290, 160};

  execute(steps, period, amplitude, offset, phase);
}

void MiniKame::sit() {
  float sit_pose[8] = {
    90 - 15, 90 + 15, 50, 130,   // 前腿挺直站立
    90 - 20, 90 + 20, 140, 40    // 后腿趴平坐下
  };
  moveServos(500, sit_pose);
}

void MiniKame::stretch() {
  float bow_pose[8] = {
    90 - 25, 90 + 25, 140, 40,   // 前腿压低胸部贴地
    90 + 15, 90 - 15, 45, 135    // 后腿撑高翘起
  };
  moveServos(600, bow_pose);
  delay(1200);
  home();
}

void MiniKame::shakeHand() {
  float sit_pose[8] = {
    90 - 15, 90 + 15, 50, 130,
    90 - 20, 90 + 20, 140, 40
  };
  moveServos(400, sit_pose);
  delay(200);

  float paw_up[8] = {
    90 - 15, 90 + 35, 50, 45,
    90 - 20, 90 + 20, 140, 40
  };
  float paw_shake[8] = {
    90 - 15, 90 + 35, 50, 80,
    90 - 20, 90 + 20, 140, 40
  };

  for (int i = 0; i < 3; i++) {
    moveServos(200, paw_up);
    moveServos(200, paw_shake);
  }
  delay(300);
  home();
}

void MiniKame::cheer() {
  float sit_pose[8] = {
    90 - 15, 90 + 15, 50, 130,
    90 - 20, 90 + 20, 140, 40
  };
  moveServos(400, sit_pose);
  delay(200);

  float cheer1[8] = {
    90 - 45, 90 + 45, 135, 45,
    90 - 20, 90 + 20, 140, 40
  };
  float cheer2[8] = {
    90 - 15, 90 + 15, 135, 45,
    90 - 20, 90 + 20, 140, 40
  };

  for (int i = 0; i < 3; i++) {
    moveServos(250, cheer1);
    moveServos(250, cheer2);
  }
  delay(300);
  home();
}

void MiniKame::boxing() {
  float sit_pose[8] = {
    90 - 15, 90 + 15, 50, 130,
    90 - 20, 90 + 20, 140, 40
  };
  moveServos(400, sit_pose);
  delay(200);

  float punch_left[8] = {
    90 - 50, 90 + 10, 50, 50,
    90 - 20, 90 + 20, 140, 40
  };
  float punch_right[8] = {
    90 - 10, 90 + 50, 130, 130,
    90 - 20, 90 + 20, 140, 40
  };

  for (int i = 0; i < 3; i++) {
    moveServos(200, punch_left);
    moveServos(200, punch_right);
  }
  delay(300);
  home();
}

void MiniKame::twist(float steps, int t) {
  int amp = 25;
  int period[] = {t, t, t, t, t, t, t, t};
  int amplitude[] = {amp, amp, 0, 0, amp, amp, 0, 0};
  int offset[] = {90, 90, 88, 92, 90, 90, 92, 88};
  int phase[] = {0, 0, 0, 0, 180, 180, 0, 0};
  execute(steps, period, amplitude, offset, phase);
  home();
}

void MiniKame::tiptoe() {
  float tall_pose[8] = {
    90, 90, 40, 140,
    90, 90, 40, 140
  };
  moveServos(400, tall_pose);
  delay(1200);
  home();
}

void MiniKame::jump() {
  float squat[8] = {
    90, 90, 120, 60,
    90, 90, 60, 120
  };
  float pop[8] = {
    90, 90, 45, 135,
    90, 90, 135, 45
  };
  for (int i = 0; i < 2; i++) {
    moveServos(250, squat);
    moveServos(120, pop);
    delay(100);
  }
  home();
}

void MiniKame::pee() {
  float lean[8] = {
    90 - 15, 90 - 10, 75, 105,
    90 - 15, 90 - 10, 105, 75
  };
  moveServos(300, lean);

  float lift1[8] = {
    90 - 15, 90 - 10, 75, 105,
    90 - 15, 45,      105, 30
  };
  float lift2[8] = {
    90 - 15, 90 - 10, 75, 105,
    90 - 15, 45,      105, 60
  };
  moveServos(300, lift1);
  for (int i = 0; i < 2; i++) {
    moveServos(200, lift2);
    moveServos(200, lift1);
  }
  delay(400);
  home();
}

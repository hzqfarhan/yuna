#ifndef CONFIG_H
#define CONFIG_H

#include <WiFi.h>  // 需要包含以使用IPAddress类型

// ============================================================================
// WiFi Configuration
// ============================================================================

// 模式选择，true 为 AP 模式，false 为 STA 模式(默认)
const bool USE_AP_MODE = true;

// STA 模式(默认), 需要连路由器, 请使用 2.4G 频段
const char *ssid = "小亦站";
const char *password = "88889999";
IPAddress local_ip(192, 168, 2, 192);  // 设置固定 IP 地址
IPAddress gateway(192, 168, 2, 1);     // 路由器网关地址
IPAddress subnet(255, 255, 255, 0);    // 子网掩码

// AP 模式配置
const char *ap_ssid = "yuna";
const char *ap_password = "88889999";
IPAddress ap_local_ip(192, 168, 4, 1);
IPAddress ap_gateway(192, 168, 4, 1);
IPAddress ap_subnet(255, 255, 255, 0);

// ============================================================================
// PS5 DualSense Controller Configuration
// ============================================================================

// Enable/disable PS5 controller support
#define ENABLE_PS5_CONTROLLER 1

// Analog stick deadzone (0-511)
// Higher = less sensitive, prevents drift
// Recommended: 50-100
const int PS5_STICK_DEADZONE = 80;

// Action cooldown in milliseconds
// Prevents the same action from being triggered too rapidly
// Lower = more responsive, Higher = prevents accidental double-tap
const unsigned long PS5_ACTION_COOLDOWN_MS = 300;

// Controller LED color when connected (R, G, B: 0-255)
// Sets the DualSense light bar color
const uint8_t PS5_LED_R = 255;
const uint8_t PS5_LED_G = 0;
const uint8_t PS5_LED_B = 0;

#endif

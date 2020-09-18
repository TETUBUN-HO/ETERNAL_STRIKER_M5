//
//ETERNAL STRIKER Rr M5
//T.KATAKOTO (TETUBUN-HO) TEAM Redherring(STUDIO Sequence)
//18/10/28 Add STICK> 18/11/4 ADD I2C STICK 19/3/25
//19/6/23 NEO PIXEL library change
//
//ETERNAL_STRIKER_M5 LICENSE:
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvStartvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//BSD 4-Clause License
//
//Copyright (c) 2020, TETUBUN-HO TEAM Redherring/STUDIO Sequence
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//1. Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//2. Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//3. All advertising materials mentioning features or use of this software
//   must display the following acknowledgement:
//   This product includes software developed by the <organization>.
//4. Neither the name of the <organization> nor the
//   names of its contributors may be used to endorse or promote products
//   derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ''AS IS'' AND ANY
//EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^End^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//
#define SDC_SYSTEM_ON
#define NEOPIXEL_ON
//
#define C16(_rr,_gg,_bb) ((ushort)(((_rr & 0xF8) << 8) | ((_gg & 0xFC) << 3) | ((_bb & 0xF8) >> 3)))
#define C8(_rr,_gg,_bb) ((ushort)(((_rr & 0xE0)) | ((_gg & 0xE0) >>3) | (_bb >> 6)))
#define PI 3.141592653589793
//
#include <M5Stack.h>
#ifdef SDC_SYSTEML_ON
#include "M5StackUpdater.h"
#endif
#include "Sprite_t_k_cst.h"
//
#include <Wire.h>
#include "SPIFFS.h"
//
#ifdef NEOPIXEL_ON
#include "NeoPixelBrightnessBus.h"
#define M5STACK_FIRE_NEO_NUM_LEDS  10
#define M5STACK_FIRE_NEO_DATA_PIN  15
NeoPixelBrightnessBus<NeoRgbFeature, Neo800KbpsMethod> strip(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN);
#endif
//
int sys_ver = 155;
byte NEO_P_MODE = 0;
//
//Stick Read SYSTEM
//
short MIN_XA, MIN_YA, MAX_XA, MAX_YA;
short BASE_XA, BASE_YA, BASE_VEC;
short READ_PIN_X = 35, READ_PIN_Y = 36; //ANALOG PIN
short DUAL_BUTTON_R_PIN = 26; //ANALOG PIN
short DUAL_BUTTON_B_PIN = 36; //ANALOG PIN
short SB_FLAG, SB_FLAG_R, SB_FLAG_B; //BUTTON State
short DUAL_BUTTON_FLAG;
int SX_AXIS, SY_AXIS; //-64<>64 Value
int AXIS_STUB = 8;
#define JOY_ADDR 0x52
//
#include "Resource.h"
//
TFT_eSprite_CST img = TFT_eSprite_CST(&M5.Lcd);
//
bool started = false, gameover = false;
unsigned long FrameTime = 0;
unsigned long systimer = 0;
byte Sound_mode = 0;
boolean but_A = false, but_B = false, but_TOP = false, but_DOWN = false, but_LEFT = false, but_RIGHT = false; //2018/11/4 STICK Add
boolean S_but_A = false, S_but_B = false, S_but_C = false;
byte CTL_Mode = 0;
int game_speed = 45;// 45msec 22fps
//Decision counter for long press. *game_speed = real time
byte LONG_P_CT = 2; //100ms
byte Color_mode = 0; //0:ORIGINAL 1:Monotone(unused) 2:liquid crystal
//
#define SPF_NOR (0<<1) //8bitSPRITE BITMAP
#define SPF_MSX (1<<1) //1bitSPRITE
#define SPF_LINE (2<<1)
#define SPF_FILL (3<<1)
#define SPF_CHAR (4<<1)
#define SPF_MOD (5<<1)//Scaling and rotation
#define SPF_TRAN (6<<1)//Fake_Translucent
#define SPF_F_TRA (1<<4)
#define SPR_MAX 96
#define SPR_LAYER_MAX 4
#define SPR_SET_MAX 32
//
uint8_t  *_sprite_img8;
//
byte spr_f[SPR_MAX], spr_n[SPR_MAX]; //f bit0:run_flag bit1-3:type(0:NORMAL(BITMAP) 1:MSX_SPRITE 2:LINE) bit4:Transparency
short spr_x[SPR_MAX], spr_y[SPR_MAX], spr_p0[SPR_MAX], spr_p1[SPR_MAX], spr_p2[SPR_MAX], spr_p3[SPR_MAX], spr_x2[SPR_MAX], spr_y2[SPR_MAX];
byte spr_nm;
byte spr_lnm[SPR_LAYER_MAX];
byte spr_layer[SPR_LAYER_MAX][SPR_MAX];
//SPRITE
short put_sprite(byte f, byte n, short x, short y, short z = 0, short p0 = 0, short p1 = 0, short x2 = 0, short y2 = 0);
//SPRITE
void get_sprite(short nm, const uint8_t* adr, short w, short h, short w2 = 1, short h2 = 1);
//
const uint8_t* spp_adr[SPR_SET_MAX];
short spp_w[SPR_SET_MAX][2], spp_h[SPR_SET_MAX][2];
//
short obj[32][5];
//
int scene, GS_systimer, GS_mode, stage;
//
void setup() {
  // put your setup code here, to run once:
  randomSeed(analogRead(0));
  M5.begin();
  //
  //SD_MENU_KICK
  #ifdef SDC_SYSTEML_ON
  if (digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
  #endif
  //M5.Lcd.setRotation(2);
  Wire.begin(21, 22, 400000);
  SPIFFS.begin();
  //
  Stick_Init();
  //
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  img.setColorDepth(8);
  _sprite_img8 = (uint8_t*)img.createSprite(240, 240);
  //
  #ifdef NEOPIXEL_ON
  strip.Begin();
  strip.Show();
  strip.SetBrightness(64);
  #endif
  //
  init();
  //
  M5.Lcd.fillScreen(TFT_BLACK);
  //2018/10/27 T.K Change
  snd_init();
  //
  delay(100);
}
void init() {
  //SET SPRITE IMAGE SET
  get_sprite(0, FILE_0, (short)FILE_0_W, (short)FILE_0_H); //MY_SHIP
  get_sprite(1, FILE_1, (short)FILE_1_W, (short)FILE_1_H);
  get_sprite(2, FILE_2, (short)FILE_2_W, (short)FILE_2_H);
  get_sprite(3, FILE_3, (short)FILE_3_W, (short)FILE_3_H);
  get_sprite(4, FILE_4, (short)FILE_4_W, (short)FILE_4_H); //ENEMY_BLT
  get_sprite(5, FILE_5, (short)FILE_5_W, (short)FILE_5_H);
  get_sprite(6, FILE_6, (short)FILE_6_W, (short)FILE_6_H);
  get_sprite(7, FILE_7, (short)FILE_7_W, (short)FILE_7_H); //MYSHIP FLASH
  get_sprite(8, FILE_8, (short)FILE_8_W, (short)FILE_8_H);
  get_sprite(9, FILE_9, (short)FILE_9_W, (short)FILE_9_H);
  get_sprite(10, FILE_10, (short)FILE_10_W, (short)FILE_10_H);
  get_sprite(11, FILE_11, (short)FILE_11_W, (short)FILE_11_H);
  get_sprite(12, FILE_12, (short)FILE_12_W, (short)FILE_12_H);
  get_sprite(13, FILE_13, (short)FILE_13_W, (short)FILE_13_H);
  get_sprite(14, FILE_14, (short)FILE_14_W, (short)FILE_14_H);
  get_sprite(15, FILE_15, (short)FILE_15_W, (short)FILE_15_H);
  get_sprite(16, FILE_16, (short)FILE_16_W, (short)FILE_16_H);
  get_sprite(17, FILE_17, (short)FILE_17_W, (short)FILE_17_H);
  get_sprite(18, FILE_18, (short)FILE_18_W, (short)FILE_18_H);
  get_sprite(19, FILE_19, (short)FILE_19_W, (short)FILE_19_H);
  get_sprite(20, FILE_20, (short)FILE_20_W, (short)FILE_20_H);
  get_sprite(21, FILE_21, (short)FILE_21_W, (short)FILE_21_H);
  get_sprite(22, FILE_22, (short)FILE_22_W, (short)FILE_22_H); //ITEM
  get_sprite(23, FILE_23, (short)FILE_23_W, (short)FILE_23_H); //MYSHIP FLASH2
  get_sprite(24, FILE_24, (short)FILE_24_W, (short)FILE_24_H); //STAGE2 BACK
  get_sprite(25, FILE_25, (short)FILE_25_W, (short)FILE_25_H); //STAGE1 BACK
  get_sprite(31, FILE_FONT, (short)FILE_FONT_W, (short)FILE_FONT_H); //SPRITE FONT
  //GAME SYSTEM INITIALIZE
  gameinit();
}
uint32_t spr_wdat[16];
int led_pat[] = {0, 1, 3, 5, 8, 12, 16, 14, 9, 3};
int led_ct = 0, led_r = 255, led_g = 0, led_b = 0;
void loop() {
  // put your main code here, to run repeatedly:
  FrameTime = millis();
  KeyPadLoop();
  //
  gameloop();
  paint();
  //M5.Lcd.drawFloat(1000.0f/(millis() -FrameTime), 3, 0, 180, 4);
  //
  M5.update();
  //
  Loop_T_ADJ(0);
  if (NEO_P_MODE == 0 && led_ct % 2 == 0) {
    #ifdef NEOPIXEL_ON
    for (uint8_t n = 0; n < M5STACK_FIRE_NEO_NUM_LEDS; n++) {
      int s = (int)led_pat[(n + led_ct / 2) % 10];
      RgbColor color = RgbColor(led_r * s / 16, led_g * s / 16, led_b * s / 16);
      strip.SetPixelColor(n, color);
    }
    strip.Show();
    #endif
  }
  led_ct++;
  //
  systimer++;
}
//========================================================================
//SPRITE WRAPER
//========================================================================
byte spr_scr_t[] = {1, 1, 2, 1, 1, 1, 2, 1};
byte spr_scr_stx[] = {9, 8, 6, 5, 4, 3, 1, 0};
byte spr_scr_sty[] = {0, 1, 2, 4, 5, 6, 7, 9};
//SPRITE Draw reservation
short put_sprite(byte f, byte n, short x, short y, short z, short p0, short p1, short x2, short y2) {
  if (spr_nm >= SPR_MAX || spr_lnm[z] >= SPR_MAX || z >= SPR_LAYER_MAX)return -1;
  //Set Parameter to Overall storage
  f |= 1;
  spr_f[spr_nm] = f; spr_n[spr_nm] = n;
  spr_x[spr_nm] = x; spr_y[spr_nm] = y;
  spr_p0[spr_nm] = p0; spr_p1[spr_nm] = p1;
  spr_x2[spr_nm] = x2; spr_y2[spr_nm] = y2;
  //Set Parameter to layer
  spr_layer[z][spr_lnm[z]++] = spr_nm;
  //
  spr_nm++;
  //
}
//SPRITE STRING OUT
short put_sprite_str(char * str1, short x, short y, short cl) {
  int l = 0;
  while (l < 256) {
    if (str1[l] == 0)break;
    put_sprite(SPF_CHAR, 5, x + 5 + l * 10, y + 8, SPR_LAYER_MAX - 1, cl, str1[l++]);
  }
}
//SPRITE STRING OUT CENTER
short put_sprite_str_center(char * str1, short x, short y, short cl) {
  int l = 0, l2 = 0;
  while (l2 < 256) {
    if (str1[l2] == 0)break;
    l2++;
  }; l2--;
  while (l < 256) {
    if (str1[l] == 0)break;
    put_sprite(SPF_CHAR, 5, x + l * 10 - l2 * 5, y + 8, SPR_LAYER_MAX - 1, cl, str1[l++]);
  }
}
//SPRITE Image set settings
void get_sprite(short nm, const uint8_t* adr, short w, short h, short w2, short h2) {
  spp_adr[nm] = adr; //const
  spp_w[nm][0] = w; spp_h[nm][0] = h;
  spp_w[nm][1] = w2; spp_h[nm][1] = h2;
}
//SPRITE drawing
void paint_sprite() {
  byte bt_d[4];
  //SPRITE SYSTEM
  int s_base = 0, nm, sx = 0, sy = 0, c = 0xffffff, n, k, l, crn;
  for (int l = 0; l < SPR_LAYER_MAX; l++) {
    // M5.Lcd.drawFloat(spr_lnm[l], 3, 0, 160+l*20, 4);
    for (int m = 0; m < spr_lnm[l]; m++) {
      int i = spr_layer[l][m];
      if (spr_f[i] & 1) {
        nm = spr_n[i];
        sx = spr_x[i]; sy = spr_y[i];
        switch ((spr_f[i] & 14)) {
          case SPF_NOR:
          default:
            sx = sx - (spp_w[nm][0] >> 1); sy = sy - (spp_h[nm][0] >> 1);
            if ((spr_f[i]&SPF_F_TRA) != 0)
              img.pushImage_D8AD(sx, sy, spp_w[nm][0], spp_h[nm][0], spp_adr[nm], SPBB_NOR | SPR_CLIP | SPR_ZERO_T);
            else
              img.pushImage_D8AD(sx, sy, spp_w[nm][0], spp_h[nm][0], spp_adr[nm], SPBB_NOR | SPR_CLIP);
            break;
          case SPF_TRAN:
            sx = sx - (spp_w[nm][0] >> 1); sy = sy - (spp_h[nm][0] >> 1);
            if ((spr_f[i]&SPF_F_TRA) != 0)
              img.pushImage_D8AD(sx, sy, spp_w[nm][0], spp_h[nm][0], spp_adr[nm], SPBB_TRA | SPR_CLIP | SPR_ZERO_T);
            else
              img.pushImage_D8AD(sx, sy, spp_w[nm][0], spp_h[nm][0], spp_adr[nm], SPBB_TRA | SPR_CLIP);
            break;
          case SPF_MOD://Scaling and rotation
            img.pushImage_D8AD(sx, sy, spp_w[nm][0], spp_h[nm][0], spp_adr[nm], SPBB_MOD, 0xffff, spr_p0[i], spr_p1[i]);
            break;
          case SPF_FILL:
            sx = sx - (spr_x2[i] >> 1); sy = sy - (spr_y2[i] >> 1);
            img.fillRect(sx, sy, spr_x2[i], spr_y2[i], spr_p0[i]);
            break;
          case SPF_CHAR:
            sx = sx - 5; sy = sy - 8;
            if (spr_p1[i] < 0x20 || spr_p1[i] >= 0x80)break;
            c = spr_p1[i] - 0x20;
            if (spr_p0[i] == 0)
              img.pushPartImage_D8(sx, sy, (c & 31) * 10, (c >> 5) * 16, 10, 16, spp_w[31][0], spp_adr[31], SPBB_NOR | SPR_ZERO_T);
            else
              img.pushPartImage_D8(sx, sy, (c & 31) * 10, (c >> 5) * 16, 10, 16, spp_w[31][0], spp_adr[31], SPBB_NOR | SPR_COLOR | SPR_ZERO_T, spr_p0[i]);
            break;
        }
      };
      spr_f[i] &= (~1);
    }
    spr_lnm[l] = 0;
  }
  spr_nm = 0;
}
//
//SOUND関係ダミー
//
void snd_init() {
  if (Sound_mode == 2)return;
}
//
void snd_play(int nm, const char *filename, float v) {
  if (Sound_mode == 2 || (Sound_mode == 1 && nm != 0))return;
}
//
void snd_stop(int nm) {
  if (Sound_mode == 2)return;
}
//*****************************
//      Stick Read SYSTEM
//*****************************
// Stick_Initialize
void Stick_Init() {
  //DUAL BUTTON CHEAK
  pinMode(DUAL_BUTTON_R_PIN, INPUT);
  pinMode(DUAL_BUTTON_B_PIN, INPUT);
  if (digitalRead(DUAL_BUTTON_R_PIN) && digitalRead(DUAL_BUTTON_R_PIN))DUAL_BUTTON_FLAG = -1;
}
//
//Stick Read
//RETURN > int SX_AXIS,SY_AXIS;//-64<>64 Value
//If the pin base voltage is below a certain level, disconnect the system.
//
void Stick_Read() {
  int x, y;
  //I2C JOYPAD DATA READ 2018/11/14 Add
  Wire.requestFrom(JOY_ADDR, 3);
  if (Wire.available()) {
    //BOOSTER 0-255 > -128~127 > -96~95 > HI_CUT > -64~63
    x = Wire.read(); y = Wire.read(); SB_FLAG = Wire.read();
    if (x + y + SB_FLAG != 0) {
      SX_AXIS = (x - 128) * 2 / 3;
      if (SX_AXIS < -64)SX_AXIS = -64;
      else if (SX_AXIS > 64)SX_AXIS = 64;
      SY_AXIS = (127 - y) * 2 / 3;
      if (SY_AXIS < -64)SY_AXIS = -64;
      else if (SY_AXIS > 64)SY_AXIS = 64;
    }
    else
    {
      SX_AXIS = 0;
      SY_AXIS = 0;
      SB_FLAG = 0;
    }
  }
  //DUAL BUTTON READ
  if (DUAL_BUTTON_FLAG != 0) {
    pinMode(DUAL_BUTTON_R_PIN, INPUT);
    pinMode(DUAL_BUTTON_B_PIN, INPUT);
    if (digitalRead(DUAL_BUTTON_R_PIN))
      SB_FLAG_R = 0;
    else
      SB_FLAG_R = -1;
    if (digitalRead(DUAL_BUTTON_B_PIN))
      SB_FLAG_B = 0;
    else
      SB_FLAG_B = -1;
  }
}
//=======================================================================
//INPUT SYSTEM 2018/10/9 T.K Change
//=======================================================================
//2018/10/9 T.K Change
void ClearKeys() {
  but_A = false;
  but_B = false;
  but_LEFT = false;
  but_RIGHT = false;
  but_TOP = false;
  but_DOWN = false;
  S_but_A = false;
  S_but_B = false;
  S_but_C = false;
}
//========================================================================
//KEY INFO (A=bit3,C=bit2,B=bit4)
//2018/10/9 T.K Replacement
short Key = 0, R_Key = 0, Btn_A_ct = 0, Btn_B_ct = 0, Btn_C_ct = 0;
bool KeyPadLoop() {
  Stick_Read();
  ClearKeys();
  if (M5.BtnA.read()) {
    Key |= 4;  //Acquire long press.
    Btn_A_ct++;
    if (Btn_A_ct > LONG_P_CT)Key |= 64;
  }
  else
  { if ((Key & 4) && !(Key & 64))R_Key |= 4;
    Key &= (~4); Key &= (~64); Btn_A_ct = 0;
  }
  //
  if (M5.BtnC.read()) {
    Key |= 8;  //Acquire long press.
    Btn_C_ct++;
    if (Btn_C_ct > LONG_P_CT)Key |= 128;
  }
  else
  { if ((Key & 8) && !(Key & 128))R_Key |= 8;
    Key &= (~8); Key &= (~128); Btn_C_ct = 0;
  }
  //
  if (M5.BtnB.read()) {
    Key |= 16;  //Acquire long press.
    Btn_B_ct++;
  }
  else
  { if (Key & 16) {
      if (Btn_B_ct < LONG_P_CT)R_Key |= 16;
      else
        R_Key |= 32;
    }
    Key &= (~16); Btn_B_ct = 0;
  }
  //
  if ((R_Key & 4) || Key & 64) {
    R_Key &= (~4);
    but_LEFT = true;
  }
  if ((R_Key & 8) || Key & 128) {
    R_Key &= (~8);
    but_RIGHT = true;
  }
  if ((R_Key & 16)) {
    R_Key &= (~16);
    but_A = true;
  }
  if ((R_Key & 32)) {
    R_Key &= (~32);
    but_B = true;
  }
  //
  if (SX_AXIS < -1 * AXIS_STUB) {
    but_LEFT = true;
  }
  if (SX_AXIS > AXIS_STUB) {
    but_RIGHT = true;
  }
  if (SY_AXIS < -1 * AXIS_STUB) {
    but_TOP = true;
  }
  if (SY_AXIS > AXIS_STUB) {
    but_DOWN = true;
  }
  if (SB_FLAG != 0) {
    S_but_A = true;
  }
  if (SB_FLAG_R != 0) {
    S_but_B = true;
  }
  if (SB_FLAG_B != 0) {
    S_but_C = true;
  }
  //
  if (but_LEFT || but_RIGHT || but_TOP || but_DOWN || but_A)return true;
  return false;
}
//========================================================================
//NEO PIXEL USE ON/OFF
//========================================================================
void ClearNPX() {
  #ifdef NEOPIXEL_ON
  if (NEO_P_MODE == 0) {
    pinMode(M5STACK_FIRE_NEO_DATA_PIN, OUTPUT);
    digitalWrite(M5STACK_FIRE_NEO_DATA_PIN, LOW);
    strip.SetBrightness(64);
    //  strip.clear();
  }
  RgbColor color = RgbColor(0, 0, 0xff);
  for (int i = 0; i < M5STACK_FIRE_NEO_NUM_LEDS; i++)strip.SetPixelColor(i, color);
  strip.Show();//Blue light emission to clear
  delay(200);
  color = RgbColor(0, 0, 0);
  for (int i = 0; i < M5STACK_FIRE_NEO_NUM_LEDS; i++)strip.SetPixelColor(i, color);
  strip.Show();//Clear
  if (NEO_P_MODE == 1) {
    strip.SetBrightness(0);
    //  strip.clear();
    digitalWrite(M5STACK_FIRE_NEO_DATA_PIN, LOW);
    pinMode(M5STACK_FIRE_NEO_DATA_PIN, INPUT);
  }
  #endif
}
//========================================================================
//GAME_SYSTEM -MIDP BASE-
//From here, the content body.
//========================================================================
#define DISPLAY_WIDTH2 0
#define DISPLAY_HEIGHT2 0
#define DISPLAY_WIDTH3 120
#define DISPLAY_HEIGHT3 120
int GS_key = 0;
int ecount, ect2, emode, stoper;
int r_timer, score, over_f, item, work0, lap;
int o_nm, my_nm, ept, dif, nw_nm , bomb_nm, bomb_ct;
int hi_sc[3];
//
int s_fl, s_f2, se_e0;
//
#define MAX_PAT 24
//
#define MAX_OBJ 64
//
int pt_x[MAX_PAT];
int pt_y[MAX_PAT];
//
byte o_f[MAX_OBJ];
byte o_a[MAX_OBJ];
int o_h[MAX_OBJ];
int o_x[MAX_OBJ];
int o_y[MAX_OBJ];
int o_pt[MAX_OBJ];
int o_ptf[MAX_OBJ];
byte o_hn[MAX_OBJ];
int o_p0[MAX_OBJ];
int o_p1[MAX_OBJ];
int o_p2[MAX_OBJ];
int o_p3[MAX_OBJ];
int o_p4[MAX_OBJ];
int o_p5[MAX_OBJ];
int o_p6[MAX_OBJ];
int o_p7[MAX_OBJ];
//
char str_out[256];
//
uint8_t SaveData[16];
//
int image_sz[] = { 30, 30, 21, 21, 21, 21, 21, 21 , 9, 9, 0, 0, 120, 60, 30, 30,
                   24, 24 , 21, 21 , 21, 21 , 69, 45 , 60, 30, 60, 30, 10, 18,
                   18, 20, 18, 18 , 36, 36, 33, 33 , 53, 41, 53, 41 , 120, 120
                 };
//
int GS_sin[] = {0, 19, 38, 55, 70, 83, 92, 98 , 99, 98, 92, 83, 70, 55, 38, 19, 0, -19, -38, -55, -70, -83, -92, -98, -99, -98, -92, -83, -70, -55, -38, -19, 0, 19, 38, 55, 70, 83, 92, 98, 99, 0};
byte GS_atan[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  8, 4, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0,
  8, 6, 4, 2, 2, 2, 1, 1, 1, 1, 1, 1,
  8, 7, 5, 4, 3, 3, 2, 2, 2, 2, 1, 1,
  8, 7, 6, 5, 4, 3, 3, 3, 2, 2, 2, 2,
  8, 7, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2,
  8, 7, 7, 6, 5, 5, 4, 4, 3, 3, 3, 2,
  8, 7, 7, 6, 5, 5, 4, 4, 4, 3, 3, 3,
  8, 7, 7, 6, 6, 5, 5, 4, 4, 4, 3, 0,
  8, 7, 7, 6, 6, 5, 5, 5, 4, 4, 4, 3,
  8, 8, 7, 7, 6, 6, 5, 5, 5, 4, 4, 4,
  8, 8, 7, 7, 6, 6, 6, 5, 5, 4, 4, 4
};
//ステージデータ
byte stage_pt[] = {
  1, 0, 1, 0 , 1, 0, 1, 1 , 1, 1, 64, 0, 0, 0, //0
  2, 0, 3, 0, 4, 0, 5, 64, 0, 0, 0, 96, 0, 0,
  2, 0, 0, 3, 0, 0, 2, 0, 0, 3, 64, 0, 0, 0,
  8, 8, 8, 8, 8, 64, 0, 0, 0, 0, 0, 0, 0, 0,
  8, 8, 8, 8, 0, 0, 8, 8, 8, 64, 0, 0, 0, 0, //4
  16, 16, 16, 16, 16, 16, 16, 16, 16, 64, 0, 0, 0, 0,
  2, 0, 3, 0, 6, 0, 7, 64, 0, 0, 0, 96, 0, 0,
  6, 0, 7, 0, 4, 0, 5, 0, 2, 0, 3, 0, 0, 0,
  5, 0, 4, 0, 0, 7, 0, 6, 64, 0, 0, 0, 96, 0, //8
  1, 1, 1, 1, 1, 1, 64, 0, 0, 0, 0, 96, 0, 0,
  1, 2, 1, 3, 1, 0, 1, 0, 1, 4, 1, 5, 64, 0,
  0, 13, 11, 0, 13, 0, 64, 0, 96, 0, 0, 0, 0, 0,
  0, 10, 0, 11, 0, 12, 0, 64, 0, 0, 96, 0, 0, 0, //12
  0, 12, 0, 11, 0, 10, 0, 64, 0, 0, 96, 0, 0, 0,
  0, 13, 0, 64, 0, 0, 96, 0, 0, 0, 0, 0, 0, 0,
  14, 14, 14, 0, 14, 14, 14, 64, 0, 0, 0, 0, 0, 0,
  14, 14, 14, 14, 14, 64, 0, 0, 0, 0, 0, 0, 0, 0, //16
  16, 16, 1, 16, 16, 1, 16, 16, 1, 64, 0, 0, 0, 96,
  7, 0, 6, 0, 5, 0, 4, 0, 3, 0, 2, 0, 0, 0,
  11, 0, 13, 0, 11, 0, 12, 0, 10, 0, 11, 0, 0, 0,
  0, 0, 9, 0, 0, 0, 0, 96, 0, 0, 0, 0, 0, 0, //20
  15, 0, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  17, 0, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  18, 0, 96, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
byte stage_map[] = {
  0, 6, 3, 9, 1, 4, 2, 0,//ST1
  3, 6, 9, 8, 4, 0, 7, 2,
  9, 3, 1, 4, 10, 3, 8, 20,
  10, 6, 7, 0, 9, 2, 4, 3,
  3, 9, 1, 4, 0, 0, 0, 0,

  15, 0, 3, 16, 14, 4, 5, 12, //ST2
  16, 4, 0, 13, 3, 15, 14, 21,
  16, 3, 5, 3, 11, 17, 5, 14,
  15, 0, 22, 5, 3, 16, 14, 5,
  15, 16, 17, 4, 0, 0, 0, 0,

  0, 1, 2, 3, 4, 5, 6, 7,//ST3
  8, 9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 0, 20, 23, 21,

};
//
//GAME SYSTEM INITIALIZE
//
void gameinit()
{
  for (int i = 0; i < MAX_OBJ; i++) o_f[o_nm] = 0;
  Data_Load();
}
//
//GAME SYSTEM MAIN LOOP
//
byte kct = 0;
void gameloop() {
  //SCENE PROCESSING
  switch (scene)
  {
    case 0://TITLE
    default:
      //INITIALIZE
      if (GS_systimer == 0) {
        Data_Save();
        LONG_P_CT = 4; //
        led_r = 0; led_g = 0; led_b = 0;
      }
      //MENU PROCESSING
      //NEO_PIXEL ON/OFF
      if (but_LEFT && but_RIGHT && kct == 0) {
        NEO_P_MODE++; NEO_P_MODE %= 2;
        ClearNPX();
        kct = 8;
      }
      else
      {
        if (but_LEFT && kct == 0) {
          stage--; if (stage < 0)stage = 2;
          kct = 8;
        }
        if (but_RIGHT && kct == 0) {
          stage++; if (stage > 2)stage = 0;
          kct = 8;
        }
      }
      if (kct > 0)kct--;
      if (but_A || but_B || S_but_A) {
        CTL_Mode = 0;
        if (S_but_A)CTL_Mode = 1;
        R_Key &= (~48);
        snd_play(1, "/enter.mp3", 0.2f);
        GS_mode = 0;
        work0 = 0;
        GS_systimer = -1;
        scene = 1;
        work0 = 0;
        r_timer = 3000;
        item = 0;
        score = 0;
        ecount = 0;
        lap = 1;
        emode = 0;
        stoper = 0;
        bomb_nm = 0; bomb_ct = 0;
        //DLOG=0;
        int n;
        for (n = 0; n < MAX_OBJ; n++) {
          o_f[n] = 0;
        }
        over_f = 0;
      }
      //DISPLAY OUT
      put_sprite(SPF_FILL, 0, DISPLAY_WIDTH3, DISPLAY_HEIGHT3, 0, 0xffff, 0, 240, 240);
      put_sprite(SPF_NOR, 5, DISPLAY_WIDTH3, DISPLAY_HEIGHT2 + 84, 1);
      //
      put_sprite_str_center("SELECT STAGE", DISPLAY_WIDTH3, DISPLAY_HEIGHT2 + 168, 1);
      if (stage == 2)
        put_sprite_str_center("ENDLESS STAGE", DISPLAY_WIDTH3, DISPLAY_HEIGHT2 + 188, 2);
      else
      {
        sprintf(str_out, "STAGE %1d", (stage + 1));
        put_sprite_str_center(str_out, DISPLAY_WIDTH3, DISPLAY_HEIGHT2 + 188, 2);
      }
      sprintf(str_out, "HI-SCORE:%5d", hi_sc[stage]);
      put_sprite_str(str_out, DISPLAY_WIDTH2 + 3, DISPLAY_HEIGHT2, 1);
      //M5S Ver SP_STATUS
      if (NEO_P_MODE == 0)sprintf(str_out, "NPX:ON");
      else
        sprintf(str_out, "NPX:OFF");
      put_sprite_str(str_out, DISPLAY_WIDTH2 + 3 - 12, DISPLAY_HEIGHT2 + 222, C8(0, 24, 240));
      sprintf(str_out, "Version:%3dNS", sys_ver);
      put_sprite_str(str_out, DISPLAY_WIDTH3 + 5, DISPLAY_HEIGHT2 + 222, C8(0, 24, 240));
      break;
    case 1://GAME MAIN
      if (GS_systimer == 0) {
        LONG_P_CT = 2; //
        if (Get_OBJ((byte)0, (byte)0) != 65535) {
          o_pt[o_nm] = 0;
          o_x[o_nm] = 0;
          o_y[o_nm] = 0;
          o_p3[o_nm] = 0;
          if (stage == 0)o_p4[o_nm] = 0;
          if (stage == 1)o_p4[o_nm] = 40;
          if (stage == 2)o_p4[o_nm] = 80;
          o_p5[o_nm] = 0;
          o_p6[o_nm] = 0;
          o_p7[o_nm] = 0;
        }
#ifdef FIRE_MODE
        if (stage == 2)snd_play(0, "/bgm002.mp3", 0.3f);
        else if (stage == 1)snd_play(0, "/bgm003.mp3", 0.3f);
        else
          snd_play(0, "/bgm001.mp3", 0.3f);
#else
        if (stage == 2)snd_play(0, "/bgm002.mp3", 0.3f);
        else if (stage == 1)snd_play(0, "/bgm001.mp3", 0.3f);
        else
          snd_play(0, "/bgm001.mp3", 0.3f);
#endif
      };
      //DISPLAY OUT
      if (stage != 2)sprintf(str_out, "R_T:%4d", r_timer);
      else
        sprintf(str_out, "  NAP:%2d", ept);
      put_sprite_str(str_out, DISPLAY_WIDTH3 + 35, DISPLAY_HEIGHT2, 0);
      sprintf(str_out, "LIFE:");
      for (int i = 0; i < o_h[my_nm]; i++)strcat(str_out, "@");
      //sprintf(str_out,"LIFE:%2d",o_h[my_nm]);
      put_sprite_str(str_out, DISPLAY_WIDTH2 + 3, DISPLAY_HEIGHT2, 0);
      sprintf(str_out, "ENE:%3d%%", bomb_nm * 100 / 180);
      short p = C8(255, 0, 0);
      if (bomb_nm >= 180)p = C8(0, 255, 255);
      else if (bomb_nm >= 120)p = C8(0, 255, 0);
      else if (bomb_nm >= 60)p = C8(255, 255, 0);
      put_sprite_str(str_out, DISPLAY_WIDTH2 + 3, DISPLAY_HEIGHT2 + 16, p);
      if (item % 3 == 1)sprintf(str_out, "POW:%1d >", o_p1[my_nm]);
      else if (item % 3 == 2)sprintf(str_out, "POW:%1d >>", o_p1[my_nm]);
      else
        sprintf(str_out, "POW:%1d >>>", o_p1[my_nm]);
      put_sprite_str(str_out, DISPLAY_WIDTH2 + 3, DISPLAY_HEIGHT2 + 222, 0);
      sprintf(str_out, "SCORE:%5d", score);
      put_sprite_str(str_out, DISPLAY_WIDTH3 + 5, DISPLAY_HEIGHT2 + 222, 0);
      //
      if (over_f == 1) {
        sprintf(str_out, "GAME OVER");
        put_sprite_str_center(str_out, DISPLAY_WIDTH3, DISPLAY_HEIGHT2 + 92, 0xe0);
      }
      else if (over_f == 2) {
        sprintf(str_out, "TIME OVER");
        put_sprite_str_center(str_out, DISPLAY_WIDTH3, DISPLAY_HEIGHT2 + 92, 0xfc);
      }
      //
      if (DUAL_BUTTON_FLAG != 0 && S_but_C);
      else
        Run_OBJ();
      break;
  }
  GS_systimer++;
}
//========================================================================
//DATA SAVE/LOAD >SPIFFS
//========================================================================
void Data_Save() {
  //encode
  for (int i = 0; i < 3; i++) {
    SaveData[3 + (i << 2)] = (hi_sc[i] >> 24) & 0xff;
    SaveData[2 + (i << 2)] = (hi_sc[i] >> 16) & 0xff;
    SaveData[1 + (i << 2)] = (hi_sc[i] >> 8) & 0xff;
    SaveData[0 + (i << 2)] = (hi_sc[i] & 0xff);
  }
  //
  File f = SPIFFS.open("/SS_ESRr_M5.data", "w");
  if (f) {
    f.write(SaveData, sizeof(SaveData));
    f.close();
  }
  else
    f.close();
}
void Data_Load() {
  //
  File f = SPIFFS.open("/SS_ESRr_M5.data", "r");
  if (f && !f.isDirectory()) {
    f.read(SaveData, sizeof(SaveData));
    f.close();
    //decode
    for (int i = 0; i < 3; i++)
      hi_sc[i] = SaveData[0 + (i << 2)] | (SaveData[1 + (i << 2)] << 8) | (SaveData[2 + (i << 2)] << 16) | (SaveData[3 + (i << 2)] << 24);
    //
  }
  else
    f.close();
  //
}
//========================================================================
//SYSTEM OTHERS
//========================================================================
void Loop_T_ADJ(byte m) { //Speed adjustment
  while (-1) {
    if (FrameTime + game_speed < millis())break;
    delay(1);
  }
}
//get next INT_Random
int Next_I_Random() {
  return (random(0xffff) - 0x8000);
}
//
// Draw GAME Graphics
//
void paint() {
  int i;
  switch (scene)
  {
    case 1:
      for (i = MAX_OBJ - 1; i >= 0; i--) {
        if (o_f[i] == 0 || o_pt[i] == 0)continue;
        if (o_ptf[i] == 0) {
          put_sprite(SPF_NOR | SPF_F_TRA, o_pt[i] - 1, DISPLAY_WIDTH3 + o_x[i] / 50, DISPLAY_HEIGHT3 + o_y[i] / 50, 1);
          continue;
        }
        else if (o_ptf[i] == 1) {
          put_sprite(SPF_NOR, o_pt[i] - 1, DISPLAY_WIDTH3 + o_x[i] / 50, DISPLAY_HEIGHT3 + o_y[i] / 50, 0);
          continue;
        }
        else if (o_ptf[i] == 2) {
          put_sprite(SPF_MOD, o_pt[i] - 1, DISPLAY_WIDTH3 + o_x[i] / 50, DISPLAY_HEIGHT3 + o_y[i] / 50, 1, ((o_p2[i] + 16) & 31) * 36000 / 32, 10000);
          continue;
        }
        else if (o_ptf[i] == 3) {
          put_sprite(SPF_TRAN, o_pt[i] - 1, DISPLAY_WIDTH3 + o_x[i] / 50, DISPLAY_HEIGHT3 + o_y[i] / 50, 1);
          continue;
        }
      }
      break;
  }
  //Sprite Draw
  paint_sprite();
  // //STAB Clear
  // img.fillRect(0,0,40,240,0);img.fillRect(280,0,40,240,0);
  //FLASH
  img.pushSprite(40, 0);
}
int Get_OBJ(byte f, byte a) {
  int i;
  f++;
  for (i = 0; i < MAX_OBJ; i++) {
    if (o_f[i] == 0) {
      o_f[i] = f;
      o_a[i] = a;
      o_hn[i] = 0;
      o_p0[i] = 0;
      o_ptf[i] = 0;
      break;
    }
  }
  if (i == MAX_OBJ)i = 65535;
  else
    o_nm = i;
  return i;
}
int Get_OBJ2(byte f, byte a) {
  int i;
  f++;
  for (i = MAX_OBJ - 1; i >= 0; i--) {
    if (o_f[i] == 0) {
      o_f[i] = f;
      o_a[i] = a;
      o_hn[i] = 0;
      o_p0[i] = 0;
      o_ptf[i] = 0;
      break;
    }
  }
  if (i < 0)i = 65535;
  else
    o_nm = i;
  return i;
}
int Get_OBJ3(byte f, byte a) {
  int i;
  f++;
  for (i = nw_nm; i < MAX_OBJ; i++) {
    if (o_f[i] == 0) {
      o_f[i] = f;
      o_a[i] = a;
      o_hn[i] = 0;
      o_p0[i] = 0;
      o_ptf[i] = 0;
      break;
    }
  }
  if (i == MAX_OBJ) {
    f--;
    i = Get_OBJ(f, a);
  }
  else
    o_nm = i;
  return i;
}
void Hchk_OBJ(int x, int y, int k) {
  int j, n;
  for (j = 0; j < MAX_OBJ; j++) {
    if ((o_f[j] & 2) != 2)continue;
    if (o_x[j] < (o_x[nw_nm] + x) && o_x[j] >= (o_x[nw_nm] - x) && o_y[j] < (o_y[nw_nm] + y) && o_y[j] >= (o_y[nw_nm] - y)) {
      o_hn[j] = (byte)(1 + nw_nm);
      o_h[nw_nm]--;
      if (o_h[nw_nm] == 0)Ene_cr(k);
      else if (o_a[j] == 2 && o_p0[j] != 0) {
        o_h[nw_nm]--;
        if (o_h[nw_nm] == 0)Ene_cr(k);
      }
      break;
    }
  }
}
//
void Ene_c2(int j, int x, int y) {
  int k;
  if (!(o_p7[my_nm] == 0 && o_x[my_nm] < (o_x[nw_nm] + x) && o_x[my_nm] >= (o_x[nw_nm] - x) && o_y[my_nm] < (o_y[nw_nm] + y) && o_y[my_nm] >= (o_y[nw_nm] - y)) || o_h[nw_nm] == 0)return;
  o_hn[my_nm] = (byte)(1 + nw_nm);

  for (k = 0; k < 5; k++) {
    o_h[nw_nm]--;
    if (o_h[nw_nm] == 0) {
      Ene_cr(j);
      break;
    }
  }
}
//
void Ene_cr(int j) {
  int n;
  score = score + j;
  if ((o_f[nw_nm] & 4) == 0 && o_p3[nw_nm] == ect2)ecount--;
  snd0();
  //
  o_f[nw_nm] = 0;
  o_pt[nw_nm] = 0;
  for (n = 0; n < 3; n++) {
    if (Get_OBJ((byte)0, (byte)4) != 65535) {
      o_x[o_nm] = o_x[nw_nm] + (Next_I_Random() % 500);
      o_y[o_nm] = o_y[nw_nm] + (Next_I_Random() % 500);
      o_p0[o_nm] = n & 1;
      o_ptf[o_nm] = 0;
      o_pt[o_nm] = 4;
    };
  };
  if (Next_I_Random() % 8 == 0)
    if (Get_OBJ((byte)0, (byte)3) != 65535) {
      o_x[o_nm] = o_x[nw_nm];
      o_y[o_nm] = o_y[nw_nm];
      o_p0[o_nm] = -700;
      o_ptf[o_nm] = 0;
      o_pt[o_nm] = 23;
      if (0 < o_x[o_nm])
        o_p1[o_nm] = -140;
      else
        o_p1[o_nm] = 140;
    };
}
void End_scene() {
  if (score > hi_sc[stage]) {
    hi_sc[stage] = score;
    //SaveScore();
  }
  scene = 0;
  GS_systimer = -1;
}
void snd0() {
  snd_play(1 + (GS_systimer & 1), "/exp2.mp3", 0.1f);
  //if (MA_F < 2) {
  //  theAP2.play();
  //}
}
//dummy
void startTimer(short i) {
}
//
int Set_Eblt() {
  if (Get_OBJ3((byte)0, (byte)5) != 65535) {
    o_p4[o_nm] = 4;
    o_x[o_nm] = o_x[nw_nm];
    o_y[o_nm] = o_y[nw_nm];
    if (o_x[nw_nm] > o_x[my_nm] && o_y[nw_nm] > o_y[my_nm])o_p0[o_nm] = 24 + GS_atan[(o_y[nw_nm] - o_y[my_nm]) / 1200 * 12 + (o_x[nw_nm] - o_x[my_nm]) / 1200];
    if (o_x[nw_nm] > o_x[my_nm] && o_y[nw_nm] <= o_y[my_nm])o_p0[o_nm] = 24 - GS_atan[(o_y[my_nm] - o_y[nw_nm]) / 1200 * 12 + (o_x[nw_nm] - o_x[my_nm]) / 1200];
    if (o_x[nw_nm] <= o_x[my_nm] && o_y[nw_nm] > o_y[my_nm])o_p0[o_nm] = 8 - GS_atan[(o_y[nw_nm] - o_y[my_nm]) / 1200 * 12 + (o_x[my_nm] - o_x[nw_nm]) / 1200];
    if (o_x[nw_nm] <= o_x[my_nm] && o_y[nw_nm] <= o_y[my_nm])o_p0[o_nm] = 8 + GS_atan[(o_y[my_nm] - o_y[nw_nm]) / 1200 * 12 + (o_x[my_nm] - o_x[nw_nm]) / 1200];
    o_ptf[o_nm] = 0;
    o_pt[o_nm] = 5;
    return o_p0[o_nm];
  };
  return 16;
}
//
void Get_enemy(int i) {
  int j;
  j = stage_pt[stage_map[o_p4[i]] * 14 + o_p5[i]];
  if (j == 96) {
    o_p5[i] = 0;
    if (stage == 2)
    {
      o_p4[i] = 90 + Next_I_Random() % 11;
      if (GS_systimer >= lap * 1200) {
        if (dif < 0)dif++;
        if (dif < 10)dif++;
        lap++;
        if (lap % 4 == 1)
          o_p4[i] = 101;
        else
          o_p4[i] = 102 + (Next_I_Random() & 1);
      }
      ept = stage_map[o_p4[i]];
    }
    else
      o_p4[i]++;
    emode = 0;
    ecount = 0;
    ect2++;
    j = stage_pt[stage_map[o_p4[i]] * 14 + o_p5[i]];
  }
  if (j == 64) {
    emode = 1;
    j = 0;
  }
  if (emode == 1 && ecount == 0) {
    o_p5[i] = 0;
    if (stage == 2)
    {
      o_p4[i] = 90 + Next_I_Random() % 11;
      if (GS_systimer >= lap * 1200) {
        if (dif < 0)dif++;
        if (dif < 10)dif++;
        lap++;
        if (lap % 4 == 1)
          o_p4[i] = 101;
        else
          o_p4[i] = 102 + (Next_I_Random() & 1);
      }
      ept = stage_map[o_p4[i]];
    }
    else
      o_p4[i]++;
    emode = 0;
    score = score + 10;
    j = stage_pt[stage_map[o_p4[i]] * 14 + o_p5[i]];
  }
  switch (j) {
    case 0:
      break;
    case 1: //垂直落下雑魚０
      if (Get_OBJ((byte)0, (byte)8) != 65535) {
        o_x[o_nm] = (Next_I_Random() % 50) * 100;
        o_y[o_nm] = -7000;
        o_h[o_nm] = 3;
        o_p1[o_nm] = 0;
        o_p3[o_nm] = ect2;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 11;
        ecount++;
      };
      break;
    case 2: //クモタイプ右
    case 3: //クモタイプ左
    case 4: //クモタイプ右
    case 5: //クモタイプ左
    case 6: //クモタイプ右
    case 7: //クモタイプ左
      if (Get_OBJ((byte)0, (byte)9) != 65535) {
        if (j == 2)o_x[o_nm] = 4000;
        if (j == 3)o_x[o_nm] = -4000;
        if (j == 4)o_x[o_nm] = 2600;
        if (j == 5)o_x[o_nm] = -2600;
        if (j == 6)o_x[o_nm] = 1200;
        if (j == 7)o_x[o_nm] = -1200;
        o_y[o_nm] = -7000;
        o_h[o_nm] = 7;
        o_p1[o_nm] = 13;
        o_p3[o_nm] = ect2;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 10;
        ecount++;
      };
      break;
    case 8: //くるくる０
      if (Get_OBJ((byte)0, (byte)10) != 65535) {
        o_x[o_nm] = -7000;
        o_y[o_nm] = 500;
        o_h[o_nm] = 4;
        o_p1[o_nm] = 0;
        o_p2[o_nm] = 10;
        o_p3[o_nm] = ect2;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 9;
        ecount++;
      };
      if (Get_OBJ((byte)0, (byte)11) != 65535) {
        o_x[o_nm] = 7000;
        o_y[o_nm] = 500;
        o_h[o_nm] = 4;
        o_p1[o_nm] = 0;
        o_p2[o_nm] = 22;
        o_p3[o_nm] = ect2;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 9;
        ecount++;
      };
      break;
    case 9: //ボス
      if (Get_OBJ((byte)4, (byte)6) != 65535) {
        o_x[o_nm] = 0;
        o_y[o_nm] = -7000;
        o_h[o_nm] = 180;
        o_p1[o_nm] = 12;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 12;
        stoper = 1;
      };
      break;
    case 10: //炸裂弾
    case 11: //炸裂弾
    case 12: //炸裂弾
      if (Get_OBJ((byte)0, (byte)14) != 65535) {
        if (j == 10)o_x[o_nm] = 3600;
        if (j == 11)o_x[o_nm] = 0;
        if (j == 12)o_x[o_nm] = -3600;
        o_y[o_nm] = -7000;
        o_h[o_nm] = 12;
        o_p1[o_nm] = 16;
        o_p3[o_nm] = ect2;
        o_p2[o_nm] = Next_I_Random() & 3;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 17;
        ecount++;
      };
      break;
    case 13: //炸裂弾*2
      if (Get_OBJ((byte)0, (byte)14) != 65535) {
        o_x[o_nm] = 2800;
        o_y[o_nm] = -7000;
        o_h[o_nm] = 12;
        o_p1[o_nm] = 16;
        o_p2[o_nm] = Next_I_Random() & 3;
        o_p3[o_nm] = ect2;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 17;
        ecount++;
      };
      if (Get_OBJ((byte)0, (byte)14) != 65535) {
        o_x[o_nm] = -2800;
        o_y[o_nm] = -7000;
        o_h[o_nm] = 12;
        o_p1[o_nm] = 16;
        o_p2[o_nm] = Next_I_Random() & 3;
        o_p3[o_nm] = ect2;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 17;
        ecount++;
      };
      break;
    case 14: //くるくる１
      if (Get_OBJ((byte)0, (byte)15) != 65535) {
        o_x[o_nm] = -6000;
        o_y[o_nm] = 7000;
        o_h[o_nm] = 5;
        o_p1[o_nm] = 0;
        o_p2[o_nm] = 1;
        o_p3[o_nm] = ect2;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 16;
        ecount++;
      };
      if (Get_OBJ((byte)0, (byte)16) != 65535) {
        o_x[o_nm] = 6000;
        o_y[o_nm] = 7000;
        o_h[o_nm] = 5;
        o_p1[o_nm] = 0;
        o_p2[o_nm] = 31;
        o_p3[o_nm] = ect2;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 16;
        ecount++;
      };
      break;
    case 15: //大雑魚
      if (Get_OBJ((byte)4, (byte)17) != 65535) {
        o_x[o_nm] = 0;
        o_y[o_nm] = -7000;
        o_h[o_nm] = 120;
        o_p1[o_nm] = 12;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 18;
        stoper = 1;
      };
      break;
    case 16: //クモ２
      if (Get_OBJ((byte)0, (byte)18) != 65535) {
        o_x[o_nm] = (Next_I_Random() % 50) * 100;
        o_y[o_nm] = -7000;
        o_h[o_nm] = 5;
        o_p1[o_nm] = 0;
        o_p3[o_nm] = ect2;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 10;
        ecount++;
      };
      break;
    case 17: //ボス
      if (Get_OBJ((byte)4, (byte)19) != 65535) {
        o_x[o_nm] = 0;
        o_y[o_nm] = -7000;
        o_h[o_nm] = 260;
        o_p1[o_nm] = 0;
        o_p2[o_nm] = 10;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 20;
        stoper = 1;
      };
      break;
    case 18: //大雑魚2
      if (Get_OBJ((byte)4, (byte)20) != 65535) {
        o_x[o_nm] = 0;
        o_y[o_nm] = -7000;
        o_h[o_nm] = 100;
        o_p1[o_nm] = 12;
        o_ptf[o_nm] = 0;
        o_pt[o_nm] = 19;
        stoper = 1;
      };
      break;
  }
  o_p5[i]++;
  if (o_p5[i] > 13) {
    o_p5[i] = 0;
    if (stage == 2)
    {
      o_p4[i] = 90 + Next_I_Random() % 11;
      if (GS_systimer >= lap * 1200) {
        if (dif < 0)dif++;
        if (dif < 10)dif++;
        lap++;
        if (lap % 4 == 1)
          o_p4[i] = 101;
        else
          o_p4[i] = 102 + (Next_I_Random() & 1);
      }
      ept = stage_map[o_p4[i]];
    }
    else
      o_p4[i]++;
    emode = 0;
    ecount = 0;
    ect2++;
  }
}
//
//オブジェクトドライブ
//
void Run_OBJ() {
  int i, j, n, k;
  for (i = 0; i < MAX_OBJ; i++) {
    if (o_f[i] == 0)continue;
    nw_nm = i;
    switch (o_a[i]) {
      //main task
      case 0:
        if (o_p7[i] == 0) {
          dif = 0;
          //自機作成
          if (Get_OBJ((byte)0, (byte)1) != 65535) {
            o_x[o_nm] = 0;
            if (CTL_Mode == 0)o_y[o_nm] = 3000 + 1450; //3BUTTON Mode
            else
              o_y[o_nm] = 3000;//PAD Mode
            o_h[o_nm] = 5;
            o_p1[o_nm] = 1; //武装
            o_p2[o_nm] = 0;
            o_p3[o_nm] = 0;
            o_p4[o_nm] = 0; //ライフアップ係数
            o_p5[o_nm] = 0; //パワーアップ係数
            o_p6[o_nm] = 0; //フルパワー時カウンタ
            o_p7[o_nm] = 0;
            o_ptf[o_nm] = 0;
            o_pt[o_nm] = 1;
            my_nm = o_nm;
          };
          switch (stage) {
            case 0:
              //空
              for (j = 0; j < 3; j++) {
                if (Get_OBJ2((byte)0, (byte)13) != 65535) {
                  o_x[o_nm] = 0;
                  o_y[o_nm] = j * 6000 - 9000;
                  o_p0[o_nm] = 400;
                  o_p1[o_nm] = 9000;
                  o_ptf[o_nm] = 1;
                  o_pt[o_nm] = 26;
                }
              }
              break;
            case 1:
              dif = 2;
              //月
              if (Get_OBJ2((byte)0, (byte)13) != 65535) {
                o_x[o_nm] = 2000;
                o_y[o_nm] = -15000;
                o_p0[o_nm] = 6;
                o_p1[o_nm] = 9000;
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 22;
              }
              break;
            case 2:
              dif = -10;
              //森
              for (j = 0; j < 5; j++) {
                if (Get_OBJ2((byte)0, (byte)13) != 65535) {
                  o_x[o_nm] = -3000;
                  o_y[o_nm] = j * 3000 - 7500;
                  o_p1[o_nm] = 7500;
                  o_p0[o_nm] = 200;
                  o_ptf[o_nm] = 1;
                  o_pt[o_nm] = 7;
                }
                if (Get_OBJ2((byte)0, (byte)13) != 65535) {
                  o_x[o_nm] = 3000;
                  o_y[o_nm] = j * 3000 - 7500;
                  o_p1[o_nm] = 7500;
                  o_p0[o_nm] = 200;
                  o_ptf[o_nm] = 1;
                  o_pt[o_nm] = 7;
                }
              }
              break;
          }
          o_p7[i] = 1;
        }
        if (stoper == 0) {
          o_p6[i]++;
          if ((o_p6[i] % 7) == 0) {
            Get_enemy(i);
          }
        }
        if (stage == 1) put_sprite(SPF_NOR, 24, DISPLAY_WIDTH3, DISPLAY_HEIGHT3, 0); //BACKGROUND ST2
        else if (stage == 0 && Next_I_Random() % 22 == 0) //BACKGROUND OBJ ST1
          if (Get_OBJ((byte)0, (byte)7) != 65535) {
            o_x[o_nm] = (Next_I_Random() % 70) * 100;
            o_y[o_nm] = -7000;
            o_h[o_nm] = 3;
            o_p0[o_nm] = (Next_I_Random() % 2 + 3) * 100;
            o_p1[o_nm] = 0;
            o_ptf[o_nm] = 3;
            o_pt[o_nm] = 13 + (Next_I_Random() & 1);
          };
        if (bomb_nm <= 20 * 9)bomb_nm++;
        if (bomb_ct > 0)bomb_ct--;
        if (o_h[my_nm] == 0 && o_p3[i] == 0) {
          o_p3[i] = 60;
          over_f = 1;
        }
        if (o_p3[i] != 0) {
          o_p3[i]--;
          if (o_p3[i] == 0)End_scene();
        }
        if (stage == 2)break;
        r_timer--;
        if (r_timer == 60)over_f = 2;
        if (r_timer == 0) {
          score = score + o_h[my_nm] * 10;
          End_scene();
          //if (s_fl == 1) {
          // s_fl = 0;
          snd_stop(0);//player.stop();
          // }
          //s_f2 = 0;
        }
        break;
      //
      //my_ship
      //
      case 1:
        if (o_h[i] == 0)
        {
          if (o_p3[i] == 0) {
            // if (s_fl == 1) {
            //  s_fl = 0;
            snd_stop(0);//player.stop();
            // }
            //s_f2 = 0;
          }
          if ((o_p3[i] % 3) == 0)startTimer(300);
          if ((o_p3[i] % 4) == 0)snd0();
          o_pt[i] = 1;
          for (n = 0; n < 3; n++) {
            if (Get_OBJ((byte)0, (byte)4) != 65535) {
              o_x[o_nm] = o_x[i] + (Next_I_Random() % 500);
              o_y[o_nm] = o_y[i] + (Next_I_Random() % 500);
              o_p0[o_nm] = n & 1;
              o_pt[o_nm] = 4;
              o_ptf[o_nm] = 0;
            };
          };
          o_p3[i]++;
          break;
        }

        if (item > (o_p4[i] + 1) * 6) {
          o_p4[i]++;
          o_h[i]++;
          if (dif < 0)dif++;
          startTimer(300);
        }
        if (item > (o_p5[i] + 1) * 3) {
          o_p5[i]++;
          if (o_p1[i] < 3) {
            o_p1[i]++;
            if (o_p1[i] == 3)o_p6[i] = 140;
          }
          else
          {
            score = score + 10;
            o_p6[i] = 140;
            if (dif < 10)dif++;
          }
          startTimer(200);
        }
        o_pt[i] = 1;
        if (o_hn[i] != 0) {
          if (dif > -10)dif--;
          if (bomb_nm >= 20 * 6) {
            bomb_nm -= 20 * 6; bomb_ct = o_p7[i] = 10;
            snd_play(2, "/change.mp3", 0.1f);
          }
          else
          {
            o_h[i]--;
            snd_play(1, "/shot3.mp3", 0.1f);
          }
          o_hn[i] = 0;
          o_pt[i] = 4;
          o_p7[i] = 10;
          if (o_p1[i] != 0)o_p1[i]--;
          startTimer(500);
        }
        k = 250;
        if (bomb_nm >= 20 * 3)
          if ((Key & 16) != 0 || S_but_A || ((Key & 4) != 0 && (Key & 8) != 0)) {
            bomb_nm -= 20 * 3;
            bomb_ct = o_p7[i] = 15;
            Key &= (~(4 | 8));
            Key = GS_key;
            snd_play(1, "/change.mp3", 0.1f);
          }
        GS_key = Key;
        if ((GS_key & (64 | 128)) != 0) k = 500;
        if (CTL_Mode == 1) {
          if (SX_AXIS < -1 * AXIS_STUB || SX_AXIS > AXIS_STUB)o_x[i] += SX_AXIS * 560 / 64;
          if (SY_AXIS < -1 * AXIS_STUB || SY_AXIS > AXIS_STUB)o_y[i] += SY_AXIS * 560 / 64;
        }
        else
        {
          if ((GS_key & 4) != 0)o_x[i] = o_x[i] - k;
          else if ((GS_key & 8) != 0)o_x[i] = o_x[i] + k;
          if ((GS_key & 1) != 0)o_y[i] = o_y[i] - k;
          else if ((GS_key & 2) != 0)o_y[i] = o_y[i] + k;
        }
        if (o_x[i] < -5000)o_x[i] = -5000;
        if (o_x[i] > 5000)o_x[i] = 5000;
        if (o_y[i] < -4000)o_y[i] = -4000;
        if (o_y[i] > 5000)o_y[i] = 5000;
        switch (o_p1[i]) {
          case 0:
            if (GS_systimer % 2 == 0)break;
            if (Get_OBJ((byte)2, (byte)2) != 65535) {
              o_pt[o_nm] = 2;
              o_ptf[o_nm] = 0;
              o_x[o_nm] = o_x[i];
              o_y[o_nm] = o_y[i];
              o_p0[o_nm] = 0;
            };
            break;
          case 1:
            if (GS_systimer % 2 == 0)break;
            if (Get_OBJ((byte)2, (byte)2) != 65535) {
              o_pt[o_nm] = 3;
              o_ptf[o_nm] = 0;
              o_x[o_nm] = o_x[i];
              o_y[o_nm] = o_y[i];
              o_p0[o_nm] = 1;
            };
            break;
          case 2:
            if (GS_systimer % 2 == 0)break;
            if (Get_OBJ((byte)2, (byte)2) != 65535) {
              if (GS_systimer % 8 == 1 || GS_systimer % 8 == 5)o_p1[o_nm] = 0;
              else if (GS_systimer % 8 == 3)o_p1[o_nm] = -150;
              else
                o_p1[o_nm] = 150;
              o_pt[o_nm] = 3;
              o_ptf[o_nm] = 0;
              o_x[o_nm] = o_x[i];
              o_y[o_nm] = o_y[i];
              o_p0[o_nm] = 2;
            };
            break;
          case 3:
            if (o_p6[i] > 0) {
              o_p6[i]--;
              if (GS_systimer % 2 == 0)
                if (Get_OBJ((byte)2, (byte)2) != 65535) {
                  o_p1[o_nm] = 190 - (GS_systimer % 4) * 70;
                  o_pt[o_nm] = 3;
                  o_ptf[o_nm] = 0;
                  o_x[o_nm] = o_x[i];
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = 2;
                };
              if (GS_systimer % 2 == 1)
                if (Get_OBJ((byte)2, (byte)2) != 65535) {
                  o_p1[o_nm] = -190 + ((GS_systimer + 1) % 4) * 70;
                  o_pt[o_nm] = 3;
                  o_ptf[o_nm] = 0;
                  o_x[o_nm] = o_x[i];
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = 2;
                };
            }
            else
            {
              if (GS_systimer % 2 != 0)
                if (Get_OBJ((byte)2, (byte)2) != 65535) {
                  if (GS_systimer % 8 == 1 || GS_systimer % 8 == 5)o_p1[o_nm] = 0;
                  else if (GS_systimer % 8 == 3)o_p1[o_nm] = -170;
                  else
                    o_p1[o_nm] = 170;
                  o_pt[o_nm] = 3;
                  o_ptf[o_nm] = 0;
                  o_x[o_nm] = o_x[i];
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = 2;
                };
            }
            break;
        }
        if (o_h[i] < 2) {
          led_r = 255;
          led_g = 0;
          led_b = 0;
        }
        else if (o_h[i] < 3) {
          led_r = 240;
          led_g = 32;
          led_b = 0;
        }
        else if (o_h[i] < 4) {
          led_r = 192;
          led_g = 192;
          led_b = 0;
        }
        else
        {
          led_r = 0;
          led_g = 240;
          led_b = 0;
        }
        if (o_p7[i] > 0) {
          o_p7[i]--;
          if ((o_p7[i] & 1))
          {
            led_r = 255;
            led_g = 255;
            led_b = 255;
          }
          else
          {
            led_r = 0;
            led_g = 0;
            led_b = 0;
          }
        }
        if (bomb_ct != 0)
        {
          o_pt[i] = 1 + 23 * (bomb_ct & 1);
          if ((o_p7[i] & 1))
          {
            led_r = 255;
            led_g = 255;
            led_b = 255;
          }
          else
          {
            led_r = 255;
            led_g = 0;
            led_b = 255;
          }
        }
        else
          o_pt[i] = 1 + 7 * (o_p7[i] & 1);
        break;
      //my_blt
      case 2:
        switch (o_p0[i]) {
          case 0:
          case 1:
            if (o_hn[i] != 0)o_f[i] = 0;
            o_y[i] = o_y[i] - 1200;
            if (o_y[i] < -7000)o_f[i] = 0;
            break;
          case 2:
            if (o_hn[i] != 0)o_f[i] = 0;
            o_x[i] = o_x[i] - o_p1[i];
            o_y[i] = o_y[i] - 1200;
            if (o_y[i] < -7000)o_f[i] = 0;
            break;
        }
        break;
      //item
      case 3:
        o_y[i] = o_y[i] + o_p0[i];
        o_x[i] = o_x[i] + o_p1[i];
        if (o_p0[i] < 900)o_p0[i] = o_p0[i] + 75;
        if (o_y[i] < -7000)o_pt[i] = 0;
        else
          o_pt[i] = 23;
        if (o_x[i] < -7000 || o_x[i] > 7000 || o_y[i] > 7000)o_f[i] = 0;
        if (o_x[my_nm] < (o_x[i] + 1000) && o_x[my_nm] >= (o_x[i] - 1000) && o_y[my_nm] < (o_y[i] + 1200) && o_y[my_nm] >= (o_y[i] - 1200)) {
          o_f[i] = 0;
          o_p7[my_nm] = 10;
          //
          if (o_p1[my_nm] == 0) {
            o_p1[my_nm]++;
          }
          item++;
          snd_play(2, "/enter.mp3", 0.1f);
          score = score + 5;
        }
        break;
      //exp
      case 4:
        o_y[i] = o_y[i] + (o_p0[i] - 2) * 80;
        if (o_p0[i] > 4) {
          if (o_p0[i] % 2 == 0)o_pt[i] = 0;
          else
            o_pt[i] = 4;
        }
        if (o_p0[i] > 10) {
          o_f[i] = 0;
          o_pt[i] = 0;
        }
        o_p0[i]++;
        break;
      //enemy-blt
      case 5:
        o_x[i] = o_x[i] + GS_sin[o_p0[i]] * o_p4[i];
        o_y[i] = o_y[i] - GS_sin[o_p0[i] + 8] * o_p4[i];
        if (o_p7[my_nm] == 0 && o_x[my_nm] < (o_x[i] + 300) && o_x[my_nm] >= (o_x[i] - 300) && o_y[my_nm] < (o_y[i] + 400) && o_y[my_nm] >= (o_y[i] - 400)) {
          o_f[i] = 0;
          o_pt[i] = 0;
          o_hn[my_nm] = (byte)(1 + i);
        }
        if (o_x[i] < -7000 || o_x[i] > 7000 || o_y[i] < -7000 || o_y[i] > 7000)o_f[i] = 0;
        break;
      //enemy　4　ボス
      case 6:
        switch (o_p0[i]) {
          case 0:
            o_y[i] = o_y[i] + o_p1[i] * 60;
            o_p1[i]--;
            if (o_p1[i] == 0) {
              o_p0[i]++;
              o_p2[i] = Next_I_Random() & 3;
            }
            break;
          case 1:
            if (o_p2[i] < 2 && o_p1[i] > 2) {
              Set_Eblt();
              if ((o_p1[i] & 1) == 1)snd_play(1 + ((o_p1[i] >> 1) & 1), "/shot0.mp3", 0.1f);
            }
            if (o_p2[i] == 2 && o_p1[i] == 4) {
              snd_play(1, "/shot1.mp3", 0.1f);
              n = Set_Eblt();
              if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                o_p4[o_nm] = 4;
                o_x[o_nm] = o_x[i];
                o_y[o_nm] = o_y[i];
                o_p0[o_nm] = (n + 1) & 31;
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 5;
              };
              if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                o_p4[o_nm] = 4;
                o_x[o_nm] = o_x[i];
                o_y[o_nm] = o_y[i];
                o_p0[o_nm] = (n - 1) & 31;
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 5;
              };
            }
            if (o_p2[i] == 3 && o_p1[i] == 9) {
              snd_play(1, "/shot2.mp3", 0.1f);
              if (Get_OBJ3((byte)4, (byte)12) != 65535) {
                o_h[o_nm] = 3;
                o_p0[o_nm] = 180;
                o_p1[o_nm] = 0;
                o_x[o_nm] = o_x[i] + 1400;
                o_y[o_nm] = o_y[i] + 300;
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 15;
              };
              if (Get_OBJ3((byte)4, (byte)12) != 65535) {
                o_h[o_nm] = 3;
                o_p0[o_nm] = 100;
                o_p1[o_nm] = 200;
                o_x[o_nm] = o_x[i] + 1400;
                o_y[o_nm] = o_y[i] + 300;
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 15;
              };
              if (Get_OBJ3((byte)4, (byte)12) != 65535) {
                o_h[o_nm] = 3;
                o_p0[o_nm] = 240;
                o_p1[o_nm] = -200;
                o_x[o_nm] = o_x[i] + 1400;
                o_y[o_nm] = o_y[i] + 300;
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 15;
              };
              if (Get_OBJ3((byte)4, (byte)12) != 65535) {
                o_h[o_nm] = 3;
                o_p0[o_nm] = 180;
                o_p1[o_nm] = 0;
                o_x[o_nm] = o_x[i] - 1400;
                o_y[o_nm] = o_y[i] + 300;
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 15;
              };
              if (Get_OBJ3((byte)4, (byte)12) != 65535) {
                o_h[o_nm] = 3;
                o_p0[o_nm] = 240;
                o_p1[o_nm] = 200;
                o_x[o_nm] = o_x[i] - 1400;
                o_y[o_nm] = o_y[i] + 300;
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 15;
              };
              if (Get_OBJ3((byte)4, (byte)12) != 65535) {
                o_h[o_nm] = 3;
                o_p0[o_nm] = 100;
                o_p1[o_nm] = -200;
                o_x[o_nm] = o_x[i] - 1400;
                o_y[o_nm] = o_y[i] + 300;
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 15;
              };
            }
            o_p1[i]++;
            if (o_p1[i] == 12) {
              o_p1[i] = 0;
              o_p2[i] = 4 + (Next_I_Random() & 15);
              o_p5[i] = Next_I_Random() & 63;
              if (o_y[i] > -2300)o_p5[i] = o_p5[i] * -1;
              if ((Next_I_Random() & 1) == 0)o_p0[i] = 2;
              else
                o_p0[i] = 3;
            }
            break;
          case 2:
            o_x[i] = o_x[i] - o_p1[i] * 50;
            o_y[i] = o_y[i] + o_p5[i];
            if (o_p1[i] < 12)o_p1[i]++;
            o_p2[i]--;
            if (o_p2[i] == 0 || o_x[i] < -4000) {
              o_p1[i] = 0;
              o_p0[i] = 1;
              o_p2[i] = Next_I_Random() & 3;
            }
            break;
          case 3:
            o_x[i] = o_x[i] + o_p1[i] * 50;
            o_y[i] = o_y[i] + o_p5[i];
            if (o_p1[i] < 12)o_p1[i]++;
            o_p2[i]--;
            if (o_p2[i] == 0 || o_x[i] > 4000) {
              o_p1[i] = 0;
              o_p0[i] = 1;
              o_p2[i] = Next_I_Random() & 3;
            }
            break;

        }
        Hchk_OBJ(1200, 1200, 50);
        Ene_c2(50, 900, 900);
        if (o_h[i] == 0)stoper = 0;
        break;
      //back_obj
      case 7:
        o_y[i] = o_y[i] + o_p0[i];
        if (o_y[i] > 7000)o_f[i] = 0;
        break;
      //enemy　0 落下
      case 8:
        o_y[i] = o_y[i] + 250;
        if (o_y[i] > 7000)o_f[i] = 0;
        if (Next_I_Random() % (20 - dif) == 0 && o_y[i] < 3000) {
          Set_Eblt();
        }
        Hchk_OBJ(1100, 1100, 1);
        Ene_c2(1, 700, 800);
        break;
      //enemy　1　クモ０
      case 9:
        switch (o_p0[i]) {
          case 0:
            o_y[i] = o_y[i] + o_p1[i] * 60;
            o_p1[i]--;
            if (o_p1[i] == 0) {
              o_p0[i]++;
            }
            break;
          case 1:
            if (o_p1[i] > 2) {
              Set_Eblt();
              if ((o_p1[i] & 1) == 1)snd_play(1 + ((o_p1[i] >> 1) & 1), "/shot0.mp3", 0.1f);
            }
            o_p1[i]++;
            if (o_p1[i] == 10) {
              o_p1[i] = 0;
              if (o_x[i] > 0)o_p0[i] = 2;
              else
                o_p0[i] = 3;
            }
            break;
          case 2:
            o_x[i] = o_x[i] - 80;
            o_y[i] = o_y[i] - o_p1[i] * 50;
            if (o_p1[i] < 12)o_p1[i]++;
            break;
          case 3:
            o_x[i] = o_x[i] + 80;
            o_y[i] = o_y[i] - o_p1[i] * 60;
            if (o_p1[i] < 12)o_p1[i]++;
            break;
        }
        if (o_y[i] < -7000)o_f[i] = 0;
        Hchk_OBJ(1100, 1100, 3);
        Ene_c2(3, 700, 800);
        break;
      //enemy　2くるくる
      case 10:
        switch (o_p0[i]) {
          case 0:
            o_x[i] = o_x[i] + (GS_sin[o_p2[i]] * 4) + 50;
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4;
            if ((o_p1[i] % 2) == 0)o_p2[i] = (o_p2[i] - 1) & 31;
            o_p1[i]++;
            if (o_p1[i] == 24) {
              o_p0[i]++;
              o_p1[i] = 0;
            }
            break;
          case 1:
            o_x[i] = o_x[i] + GS_sin[o_p2[i]] * 4;
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4;
            o_p2[i] = (o_p2[i] - 1) & 31;
            o_p1[i]++;
            if (o_p1[i] == 20) {
              o_p0[i]++;
              o_p1[i] = 0;
            }
            if (Next_I_Random() % (24 - dif) == 0 && o_y[i] < 4000) {
              Set_Eblt();
            }
            break;
          case 2:
            o_x[i] = o_x[i] + GS_sin[o_p2[i]] * 4 - 20;
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4;
            if ((o_p1[i] % 4) == 0)o_p2[i] = (o_p2[i] + 1) & 31;
            o_p1[i]++;
            break;
        }
        if (o_y[i] > 7000)o_f[i] = 0;
        Hchk_OBJ(1100, 1100, 2);
        Ene_c2(2, 700, 800);
        o_ptf[i] = 2;
        break;
      //enemy　2くるくる
      case 11:
        switch (o_p0[i]) {
          case 0:
            o_x[i] = o_x[i] + (GS_sin[o_p2[i]] * 4) - 50;
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4;
            if ((o_p1[i] % 2) == 0)o_p2[i] = (o_p2[i] + 1) & 31;
            o_p1[i]++;
            if (o_p1[i] == 24) {
              o_p0[i]++;
              o_p1[i] = 0;
            }
            break;
          case 1:
            o_x[i] = o_x[i] + GS_sin[o_p2[i]] * 4;
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4;
            o_p2[i] = (o_p2[i] + 1) & 31;
            o_p1[i]++;
            if (o_p1[i] == 20) {
              o_p0[i]++;
              o_p1[i] = 0;
            }
            if (Next_I_Random() % (24 - dif) == 0 && o_y[i] < 4000) {
              Set_Eblt();
            }
            break;
          case 2:
            o_x[i] = o_x[i] + GS_sin[o_p2[i]] * 4 + 20;
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4;
            if ((o_p1[i] % 4) == 0)o_p2[i] = (o_p2[i] - 1) & 31;
            o_p1[i]++;
            break;
        }
        if (o_y[i] > 7000)o_f[i] = 0;
        Hchk_OBJ(1100, 1100, 2);
        Ene_c2(2, 700, 800);
        o_ptf[i] = 2;
        break;
      //blt　0 落下弾
      case 12:
        o_x[i] = o_x[i] + o_p1[i];
        o_y[i] = o_y[i] + o_p0[i];
        o_p0[i] = o_p0[i] + 30;
        if (o_y[i] > 7000)o_f[i] = 0;
        Hchk_OBJ(1200, 1200, 1);
        Ene_c2(1, 800, 900);
        break;
      //back_obj1
      case 13:
        o_y[i] = o_y[i] + o_p0[i];
        if (o_y[i] >= o_p1[i])o_y[i] = o_y[i] - o_p1[i] * 2;
        break;
      //enemy　5  BOMB
      case 14:
        switch (o_p0[i]) {
          case 0:
            o_y[i] = o_y[i] + o_p1[i] * 40;
            o_p1[i]--;
            if (o_p1[i] == 0) {
              o_p0[i]++;
            }
            break;
          case 1:
            if (o_p1[i] == 3) {
              snd_play(1, "/exp0.mp3", 0.2f);
              for (n = 0; n < 8; n++) {
                if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                  o_p4[o_nm] = 4;
                  o_x[o_nm] = o_x[i];
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = (o_p2[i] + n * 4) & 31;
                  o_ptf[o_nm] = 0;
                  o_pt[o_nm] = 5;
                }
              }
            }
            o_p1[i]++;
            if (o_p1[i] == 6) {
              o_f[i] = 0;
              o_pt[i] = 0;
              for (n = 0; n < 3; n++) {
                if (Get_OBJ((byte)0, (byte)4) != 65535) {
                  o_x[o_nm] = o_x[i] + (Next_I_Random() % 500);
                  o_y[o_nm] = o_y[i] + (Next_I_Random() % 500);
                  o_p0[o_nm] = n & 1;
                  o_ptf[o_nm] = 0;
                  o_pt[o_nm] = 4;
                };
              };
            }
            break;
        }
        Hchk_OBJ(1200, 1200, 5);
        Ene_c2(5, 800, 900);
        o_ptf[i] = 2;
        break;

      //enemy　6くるくる1
      case 15:
        switch (o_p0[i]) {
          case 0:
            o_x[i] = o_x[i] + (GS_sin[o_p2[i]] * 4 - 20);
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4;
            o_p1[i]++;
            if (o_p1[i] == 24) {
              o_p0[i]++;
              o_p1[i] = 0;
            }
            break;
          case 1:
            o_x[i] = o_x[i] + (GS_sin[o_p2[i]] * 4);
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4 - 30;
            o_p2[i] = (o_p2[i] + 1) & 31;
            o_p1[i]++;
            if (o_p1[i] == 51) {
              o_p0[i]++;
              o_p1[i] = 0;
            }
            if (Next_I_Random() % (29 - dif) == 0)Set_Eblt();
            break;
          case 2:
            o_x[i] = o_x[i] + (GS_sin[o_p2[i]] * 4);
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4;
            if ((o_p1[i] % 2) == 0)o_p2[i] = (o_p2[i] - 1) & 31;
            o_p1[i]++;
            if (o_p1[i] == 11) {
              o_p0[i]++;
              o_p1[i] = 0;
            }
            break;
          case 3:
            o_x[i] = o_x[i] + (GS_sin[o_p2[i]] * (4 + o_p1[i] / 8));
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * (4 + o_p1[i] / 8);
            o_p1[i]++;
            break;
        }
        if (o_y[i] > 7000)o_f[i] = 0;
        Hchk_OBJ(1100, 1100, 2);
        Ene_c2(2, 700, 800);
        o_ptf[i] = 2;
        break;
      //enemy　6くるくる1
      case 16:
        switch (o_p0[i]) {
          case 0:
            o_x[i] = o_x[i] + (GS_sin[o_p2[i]] * 4 + 20);
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4;
            o_p1[i]++;
            if (o_p1[i] == 24) {
              o_p0[i]++;
              o_p1[i] = 0;
            }
            break;
          case 1:
            o_x[i] = o_x[i] + (GS_sin[o_p2[i]] * 4);
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4 - 30;
            o_p2[i] = (o_p2[i] - 1) & 31;
            o_p1[i]++;
            if (o_p1[i] == 51) {
              o_p0[i]++;
              o_p1[i] = 0;
            }
            if (Next_I_Random() % (29 - dif) == 0)Set_Eblt();
            break;
          case 2:
            o_x[i] = o_x[i] + (GS_sin[o_p2[i]] * 4);
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * 4;
            if ((o_p1[i] % 2) == 0)o_p2[i] = (o_p2[i] + 1) & 31;
            o_p1[i]++;
            if (o_p1[i] == 11) {
              o_p0[i]++;
              o_p1[i] = 0;
            }
            break;
          case 3:
            o_x[i] = o_x[i] + (GS_sin[o_p2[i]] * (4 + o_p1[i] / 8));
            o_y[i] = o_y[i] - GS_sin[o_p2[i] + 8] * (4 + o_p1[i] / 8);
            o_p1[i]++;
            break;
        }
        if (o_y[i] > 7000)o_f[i] = 0;
        Hchk_OBJ(1100, 1100, 2);
        Ene_c2(2, 700, 800);
        o_ptf[i] = 2;
        break;
      //enemy　7　大雑魚
      case 17:
        switch (o_p0[i]) {
          case 0:
            o_y[i] = o_y[i] + o_p1[i] * 60;
            o_p1[i]--;
            if (o_p1[i] == 0) {
              o_p0[i]++;
              o_p2[i] = 0;
            }
            break;
          case 1:
            if (o_x[i] < o_x[my_nm] - 150)o_x[i] = o_x[i] + 150;
            else if (o_x[i] > o_x[my_nm] + 150)o_x[i] = o_x[i] - 150;
            if (o_p1[i] == 9) {
              snd_play(1, "/shot2.mp3", 0.1f);
              if (Get_OBJ3((byte)4, (byte)12) != 65535) {
                o_h[o_nm] = 5;
                o_p0[o_nm] = 180;
                o_p1[o_nm] = 0;
                o_x[o_nm] = o_x[i];
                o_y[o_nm] = o_y[i];
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 15;
              };
              if (Get_OBJ3((byte)4, (byte)12) != 65535) {
                o_h[o_nm] = 5;
                o_p0[o_nm] = 160;
                o_p1[o_nm] = 80;
                o_x[o_nm] = o_x[i];
                o_y[o_nm] = o_y[i];
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 15;
              };
              if (Get_OBJ3((byte)4, (byte)12) != 65535) {
                o_h[o_nm] = 5;
                o_p0[o_nm] = 160;
                o_p1[o_nm] = -80;
                o_x[o_nm] = o_x[i];
                o_y[o_nm] = o_y[i];
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 15;
              };
            }
            o_p1[i]++;
            if (o_p1[i] == 16) {
              o_p1[i] = 0;
              o_p0[i] = 1 + random(3);//(abs(Next_I_Random()) % 3);
            }
            break;
          case 2:
            if (o_p1[i] == 0) {
              if (o_x[i] > o_x[my_nm] && o_y[i] > o_y[my_nm])o_p3[i] = 24 + GS_atan[(o_y[i] - o_y[my_nm]) / 1200 * 12 + (o_x[i] - o_x[my_nm]) / 1200];
              if (o_x[i] > o_x[my_nm] && o_y[i] <= o_y[my_nm])o_p3[i] = 24 - GS_atan[(o_y[my_nm] - o_y[i]) / 1200 * 12 + (o_x[i] - o_x[my_nm]) / 1200];
              if (o_x[i] <= o_x[my_nm] && o_y[i] > o_y[my_nm])o_p3[i] = 8 - GS_atan[(o_y[i] - o_y[my_nm]) / 1200 * 12 + (o_x[my_nm] - o_x[i]) / 1200];
              if (o_x[i] <= o_x[my_nm] && o_y[i] <= o_y[my_nm])o_p3[i] = 8 + GS_atan[(o_y[my_nm] - o_y[i]) / 1200 * 12 + (o_x[my_nm] - o_x[i]) / 1200];
            }

            if (o_p1[i] == 4) {
              snd_play(1, "/shot1.mp3", 0.1f);
              for (k = 0; k < 3; k++) {
                if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                  o_p4[o_nm] = 4;
                  o_x[o_nm] = o_x[i];
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = (o_p3[i] - 1 + k) & 31;
                  o_ptf[o_nm] = 0;
                  o_pt[o_nm] = 5;
                };
              }
            }
            if ((o_p1[i] == 8 || o_p1[i] == 12)) {
              snd_play(1, "/shot1.mp3", 0.1f);
              for (k = 0; k < 5; k++) {
                if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                  o_p4[o_nm] = 4;
                  o_x[o_nm] = o_x[i];
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = (o_p3[i] - 2 + k) & 31;
                  o_ptf[o_nm] = 0;
                  o_pt[o_nm] = 5;
                };
              }

            }

            o_p1[i]++;
            if (o_p1[i] == 16) {
              o_p1[i] = 0;
              o_p0[i] = 1 + random(3);
            }
            break;
          case 3:
            if (o_x[i] < o_x[my_nm] - 110)o_x[i] = o_x[i] + 200;
            else if (o_x[i] > o_x[my_nm] + 110)o_x[i] = o_x[i] - 200;
            o_p1[i]++;
            if (o_p1[i] == 14 || Next_I_Random() % 4 == 0) {
              o_p1[i] = 0;
              o_p0[i] = 1 + (Next_I_Random() & 1);
            }
            break;
        }
        Hchk_OBJ(1200, 1200, 25);
        Ene_c2(25, 900, 1000);
        if (o_h[i] == 0)stoper = 0;
        break;
      //enemy　8 クモ２
      case 18:
        switch (o_p0[i]) {
          case 0:
            o_y[i] = o_y[i] + 280;
            if (o_y[i] > 7000)o_f[i] = 0;
            if (o_y[i] > o_y[my_nm] - 2000) {
              if (o_x[i] < o_x[my_nm])o_p0[i] = 1;
              else
                o_p0[i] = 2;
              o_p1[i] = 0;
            }
            break;
          case 1:
            o_y[i] = o_y[i] + 140;
            o_x[i] = o_x[i] + o_p1[i];
            o_p1[i] = o_p1[i] + 60;
            if (o_x[i] > 7000)o_f[i] = 0;
            break;
          case 2:
            o_y[i] = o_y[i] + 140;
            o_x[i] = o_x[i] + o_p1[i];
            o_p1[i] = o_p1[i] - 60;
            if (o_x[i] < -7000)o_f[i] = 0;
            break;
        }
        Hchk_OBJ(1100, 1100, 1);
        Ene_c2(1, 700, 800);
        break;
      //enemy　9　ボス２
      case 19:
        switch (o_p0[i]) {
          case 0:
            o_x[i] = o_x[i] + o_p1[i] * 20;
            o_y[i] = o_y[i] + o_p2[i] * 20;
            if (o_x[i] < o_x[my_nm] && o_p1[i] < 16)o_p1[i]++;
            if (o_x[i] > o_x[my_nm] && o_p1[i] > -16)o_p1[i]--;
            n = o_y[my_nm] - 7500;
            if (n < -4500)n = -4500;
            if (o_y[i] < n && o_p2[i] < 16)o_p2[i]++;
            if (o_y[i] > n && o_p2[i] > -16)o_p2[i]--;
            if (o_p4[i] < 34) {
              if ((o_p4[i] & 1) == 1)
                if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                  o_p4[o_nm] = 5;
                  o_x[o_nm] = o_x[i] - 600;
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = 16;
                  o_ptf[o_nm] = 0;
                  o_pt[o_nm] = 5;
                };
              if ((o_p4[i] & 1) == 0)
                if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                  o_p4[o_nm] = 5;
                  o_x[o_nm] = o_x[i] + 600;
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = 16;
                  o_ptf[o_nm] = 0;
                  o_pt[o_nm] = 5;
                };
              o_pt[i] = 20 + (o_p4[i] & 1);
            }
            o_p4[i]++;
            if (o_p4[i] > 60)o_p4[i] = 0;
            if (o_p4[i] > 45 && Next_I_Random() % 28 == 0) {
              o_p0[i]++;
              o_p1[i] = 0;
              o_p2[i] = 0;
            }
            break;
          case 1:
            if (o_p1[i] == 7) {
              snd_play(1, "/shot2.mp3", 0.1f);
              if (Get_OBJ3((byte)4, (byte)14) != 65535) {
                o_x[o_nm] = o_x[i];
                o_y[o_nm] = o_y[i];
                o_h[o_nm] = 12;
                o_p0[o_nm] = 0;
                o_p1[o_nm] = 16;
                o_p2[o_nm] = Next_I_Random() & 3;
                o_ptf[o_nm] = 0;
                o_pt[o_nm] = 17;
              };
            }
            o_p1[i]++;
            if (o_p1[i] == 14) {
              o_p0[i] = 0;
              o_p1[i] = 0;
            }
            break;
        }
        Hchk_OBJ(1300, 1200, 75);
        Ene_c2(75, 1000, 900);
        if (o_h[i] == 0)stoper = 0;
        break;
      //enemy　10　大雑魚２
      case 20:
        switch (o_p0[i]) {
          case 0:
            o_y[i] = o_y[i] + o_p1[i] * 60;
            o_p1[i]--;
            if (o_p1[i] == 0) {
              o_p0[i]++;
              o_p2[i] = 0;
            }
            break;
          case 1:
            if (o_p1[i] < 3) {
              if (o_x[i] < o_x[my_nm] - 150)o_x[i] = o_x[i] + 150;
              else if (o_x[i] > o_x[my_nm] + 150)o_x[i] = o_x[i] - 150;
            }
            if (o_p1[i] == 9) {
              snd_play(1, "/shot1.mp3", 0.1f);
              for (k = 0; k < 4; k++) {
                if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                  o_p4[o_nm] = 6 - k;
                  o_x[o_nm] = o_x[i] + 800;
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = 15;
                  o_ptf[o_nm] = 0;
                  o_pt[o_nm] = 5;
                };
                if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                  o_p4[o_nm] = 6 - k;
                  o_x[o_nm] = o_x[i] - 800;
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = 17;
                  o_ptf[o_nm] = 0;
                  o_pt[o_nm] = 5;
                };
                if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                  o_p4[o_nm] = 6 - k;
                  o_x[o_nm] = o_x[i] + 400;
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = 16;
                  o_ptf[o_nm] = 0;
                  o_pt[o_nm] = 5;
                };
                if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                  o_p4[o_nm] = 6 - k;
                  o_x[o_nm] = o_x[i] - 400;
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = 16;
                  o_ptf[o_nm] = 0;
                  o_pt[o_nm] = 5;
                };
              }

            }
            o_p1[i]++;
            if (o_p1[i] == 16) {
              o_p1[i] = 0;
              o_p0[i] = 2 + (Next_I_Random() & 1);
              if (Next_I_Random() % (17 - dif) == 0)o_p0[i] = 1;
            }
            break;
          case 2:
            if (o_p1[i] < 4) {
              if (o_x[i] < o_x[my_nm] - 110)o_x[i] = o_x[i] + 200;
              else if (o_x[i] > o_x[my_nm] + 110)o_x[i] = o_x[i] - 200;
            }
            if (o_p1[i] == 8) {
              if (o_x[i] > o_x[my_nm] && o_y[i] > o_y[my_nm])o_p3[i] = 24 + GS_atan[(o_y[i] - o_y[my_nm]) / 1200 * 12 + (o_x[i] - o_x[my_nm]) / 1200];
              if (o_x[i] > o_x[my_nm] && o_y[i] <= o_y[my_nm])o_p3[i] = 24 - GS_atan[(o_y[my_nm] - o_y[i]) / 1200 * 12 + (o_x[i] - o_x[my_nm]) / 1200];
              if (o_x[i] <= o_x[my_nm] && o_y[i] > o_y[my_nm])o_p3[i] = 8 - GS_atan[(o_y[i] - o_y[my_nm]) / 1200 * 12 + (o_x[my_nm] - o_x[i]) / 1200];
              if (o_x[i] <= o_x[my_nm] && o_y[i] <= o_y[my_nm])o_p3[i] = 8 + GS_atan[(o_y[my_nm] - o_y[i]) / 1200 * 12 + (o_x[my_nm] - o_x[i]) / 1200];
              snd_play(1, "/shot1.mp3", 0.1f);
              for (k = 0; k < 5; k++) {
                if (Get_OBJ3((byte)0, (byte)5) != 65535) {
                  o_p4[o_nm] = 6 - k;
                  o_x[o_nm] = o_x[i];
                  o_y[o_nm] = o_y[i];
                  o_p0[o_nm] = o_p3[i];
                  o_ptf[o_nm] = 0;
                  o_pt[o_nm] = 5;
                };
              }
            }
            o_p1[i]++;
            if (o_p1[i] == 16) {
              o_p1[i] = 0;
              o_p0[i] = 2 + (Next_I_Random() & 1);
              if (Next_I_Random() % (13 - dif) == 0)o_p0[i] = 1;
            }
            break;
          case 3:
            if (o_x[i] < o_x[my_nm] - 110)o_x[i] = o_x[i] + 200;
            else if (o_x[i] > o_x[my_nm] + 110)o_x[i] = o_x[i] - 200;
            o_p1[i]++;
            if (o_p1[i] == 14 || Next_I_Random() % 4 == 0) {
              o_p1[i] = 0;
              o_p0[i] = 2 + (Next_I_Random() & 1);
              if (Next_I_Random() % (12 - dif) == 0)o_p0[i] = 1;
            }
            break;
        }
        Hchk_OBJ(1200, 1200, 25);
        Ene_c2(25, 800, 900);
        if (o_h[i] == 0)stoper = 0;
        break;

    }
  }
  return;
}

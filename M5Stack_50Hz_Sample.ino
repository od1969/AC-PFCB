//
// 電源周波数を測る基板サンプルコード
// @od_1969
// Version 0.01
//
// 以下のラジオペンチさんのBlogにあるコードを使用し作成しました。
// http://radiopench.blog96.fc2.com/blog-entry-1080.html
//


//#include <M5StickCPlus.h>
#include <M5Stack.h>

#define AC_pin       21          // AC信号入力ピン M5stack:21/M5StickC Plus:32
#define AC_FREQ      50          // 電源公称周波数（50 or 60)
#define LOG_INTERVAL 60          // ログ周期（秒）
#define K_Freq       1.0         // 周波数補正係数 (デフォルトは1.0)
 
volatile uint32_t t1;            // 50(60)サイクル分の周期（us単位、標準1000000us）
volatile boolean flag;
 
float f1;                        // 周波数の1秒平均値
float fMax = 0.0;                // 最大周波数
float fMin = 1000.0;             // 最小周波数
float fAve;                      // 平均周波数
float fSum = 0.0;
 
void IRAM_ATTR acIrq() {         // ピン割込み(電源1サイクル毎に割込み）
  static uint16_t n;
  static uint32_t lastMicros = 0;
  static uint32_t lastMicros2 = 0;
  uint32_t x;

  x = micros();
  t1 = x - lastMicros2;
  if (t1 > 15000){
    n++;                         // サイクルカウンタをインクリメント
    lastMicros2 = x;
  }
  
  if (n >= AC_FREQ) {            // 指定回数(50 or 60)なら
    x = micros();                // 現在の時刻を記録
    t1 = x - lastMicros;         // 経過時間を計算(この値はほぼ100000になる）
    lastMicros = x;
    n = 0;
    flag = true;                 // loopに通知するためにフラグをセット
  }
}
 
void setup() {
  M5.begin();
  
  Serial.begin(115200);
  
  pinMode(AC_pin, INPUT_PULLUP);                  // 電源交流信号入力ピン
  attachInterrupt(AC_pin, acIrq, RISING);

  M5.Lcd.init();
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextWrap(false);
  M5.Lcd.setTextSize(1);

  M5.Lcd.setCursor(4, 8);                         // テキストのカーソル位置指定
  M5.Lcd.setTextSize(4);
  M5.Lcd.println("Setup OK");
  M5.Lcd.println("Waite...");
  waitData();                                     // 異常値排除のため最初の1回(1秒)読み飛ばす
  M5.Lcd.fillScreen(BLACK);
}
 
void loop() {
  static uint16_t logCount = 0;
  waitData();                                     // データーが準備できるまで待つ

  f1 = K_Freq * 1.0E6 * AC_FREQ / t1;             // 1秒間の平均周波数を計算
  logCount++;
  if (f1 < fMin) fMin = f1;
  if (f1 > fMax) fMax = f1;
  fSum = fSum + f1;
   
  M5.Lcd.setCursor(8, 8); // テキストのカーソル位置指定
  M5.Lcd.setTextColor(WHITE,BLACK);
  M5.Lcd.setTextSize(4);
  M5.Lcd.print("Now ");
  M5.Lcd.print(f1, 3);
  M5.Lcd.print("Hz");


  if (logCount >= LOG_INTERVAL) {                 // 指定サイクル数割込みが入ったら、
    fAve = fSum / LOG_INTERVAL;                   // 平均周波数計算

    M5.Lcd.setCursor(8, 8+32); // テキストのカーソル位置指定
    M5.Lcd.setTextColor(ORANGE,BLACK);
    M5.Lcd.setTextSize(4);
    M5.Lcd.print("Ave ");
    M5.Lcd.print(fAve, 3);
    M5.Lcd.print("Hz");
    
    M5.Lcd.setCursor(8, 8+32+32);                    // テキストのカーソル位置指定
    M5.Lcd.setTextColor(RED,BLACK);
    M5.Lcd.setTextSize(4);
    M5.Lcd.print("Max ");
    M5.Lcd.print(fMax,3);
    M5.Lcd.print("Hz");

    M5.Lcd.setCursor(8, 8+32+32+32);                    // テキストのカーソル位置指定
    M5.Lcd.setTextColor(BLUE,BLACK);
    M5.Lcd.setTextSize(4);
    M5.Lcd.print("Min ");
    M5.Lcd.print(fMin,3);
    M5.Lcd.print("Hz");

    fMin = 1000.0;                                // 次回の測定用に変数を初期化
    fMax = 0.0;
    fSum = 0.0;
    logCount = 0;
  }
}
 
void waitData() {                                 // データーが揃うまで待つ
  while (flag == false) {                         // フラグをポーリング
  }
  flag = false;
}

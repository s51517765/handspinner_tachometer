#include <M5Stack.h>
#include "arduinoFFT.h"

const int bitSize = 8;
const uint16_t FFTsamples = 1 << bitSize; // サンプル数は2のべき乗

// 光センサーのポート番号。アナログ入力ポートは35が使える
int CdS_Port = 35;

void setup()
{
  Serial.begin(115200);
  M5.begin(true, false, true);
  M5.Power.begin();
  pinMode(CdS_Port, INPUT);

  // LCDに表示
  m5.Lcd.fillScreen(BLACK);
  m5.Lcd.setTextColor(YELLOW);
  m5.Lcd.setTextSize(5);
  m5.Lcd.setCursor(0, 0);
}

void printLCD(double Freq, double Rpm)
{
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(5);
  m5.Lcd.clear();
  if (Freq > 1)
  {
    M5.Lcd.println("rpm : ");
    M5.Lcd.setTextSize(8);
    M5.Lcd.println(Rpm);
  }
  else
  {
    M5.Lcd.print("Let's start spinner!");
  }
}

void loop()
{
  double inputRe[FFTsamples]; // サンプリングしたデーターを入れる
  double inputIm[FFTsamples];

  double SAMPLING_FREQUENCY = 200;

  double ave = 0;
  double max = 0;
  double min = 4095;
  for (int i = 0; i < FFTsamples; i++)
  {
    inputRe[i] = analogRead(CdS_Port); //0 - 4095
    if (inputRe[i] > max)
      max = inputRe[i];
    if (inputRe[i] < min)
      min = inputRe[i];
    ave += inputRe[i];
    delay(1000 / SAMPLING_FREQUENCY); //ms
  }

  if (max - min > 1000)
  {
    //光量の変化があったら
    ave /= FFTsamples;

    for (int i = 0; i < FFTsamples; i++)
    {
      inputRe[i] -= ave; //平均値を0にする
      inputIm[i] = 0;    //虚数部の初期化
    }
    arduinoFFT FFT = arduinoFFT(inputRe, inputIm, FFTsamples, SAMPLING_FREQUENCY); // FFTオブジェクトを作る
    FFT.Compute(inputRe, inputIm, FFTsamples, FFT_FORWARD);
    FFT.ComplexToMagnitude(inputRe, inputIm, FFTsamples);
    double Freq = FFT.MajorPeak(inputRe, FFTsamples, SAMPLING_FREQUENCY);

    //Serial.print("Raw data -> ");
    //for (int i = 0; i < FFTsamples; i++)
    //  Serial.println(String(inputRe[i]) + "," + String(inputIm[i]));
    double Rpm = Freq / 3 * 60;
    Serial.println("Freqency : " + String(Freq) + "Hz" + " rpm : " + String(Rpm));
    printLCD(Freq, Rpm);
    delay(2000);
  }
  else
  {
    printLCD(0, 0);
  }
}
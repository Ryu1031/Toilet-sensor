#include <EasyWebSocket.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <Wire.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <UTF8toSJIS.h>
#include <MisakiFNT_read.h>
#include <I2C_Adafruit_8x8_LED_matrix.h>
#include <OLED_SSD1306.h>


const char* ssid = "";
const char* password = "";

const char* UTF8SJIS_file = "/Utf8Sjis.tbl"; //UTF8 Shift_JIS 変換テーブルファイル名を記載しておく
const char* Zen_Font_file = "/MSKG13KU.FNT"; //全角フォントファイル名を定義
const char* Half_Font_file = "/mgotec48.FNT"; //半角フォントファイル名を定義

const uint8_t OLED_Adress = 0x3C; //OLED address 製品に記載の数値は7bitなので、8bitに変換して1bit右へずらした値(78>>3c)

EasyWebSocket ews;
UTF8toSJIS u8ts;
MisakiFNT_read MFR;
I2C_Adafruit_8x8_LED_matrix adaLED;
OLED_SSD1306 oled;

String html_str1, html_str2, html_str3, html_str4, html_str5, html_str6, html_str7;

uint16_t PingSendTime = 30000;

String ret_str = "";
String scl_txt = "";

uint8_t scl_cnt = 0, scl_cnt2 = 0;
uint8_t fnt_cnt = 0, fnt_cnt2 = 0;
bool FntReadOK = true, FntReadOK2 = true;

uint8_t LedDotOut[8][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
uint8_t Next_buf[8][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
uint8_t tmp_buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t tmp_buf_cnv[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t tmp_buf_cnv_2[8] = {0, 0, 0, 0, 0, 0, 0, 0};

uint8_t LedDotOut2[8][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
uint8_t Next_buf2[8][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
uint8_t tmp_buf2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t tmp_buf_cnv2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t tmp_buf_cnv2_2[8] = {0, 0, 0, 0, 0, 0, 0, 0};

uint8_t sj_txt[2048], sj_txt2[2048];
uint16_t sj_cnt = 0, sj_cnt2 = 0;
uint16_t sj_length, sj_length2;
uint32_t SclTime, SclTime2;
uint16_t SclSpeed = 50, SclSpeed2 = 30;
boolean Scl_Stop = false;
uint8_t Direction = 0;
uint8_t brightness = 0;
int16_t Angle = 0;

boolean sjis_txt_in = true, sjis_txt_in2 = false;
boolean BW_reverse = false, BW_reverse2 = false;

//-------NTPサーバー定義----------------
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov; //(129, 6, 15, 28)time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
const int timeZone = 9;     // Tokyo
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
time_t prevDisplay = 0; // when the digital clock was displayed
uint32_t LastTime = 0;

String time_str;
String ymd_str;
String dummy_ymd_str;
uint8_t time_sj_txt[25], ymd_sj_txt[25];
uint16_t time_sj_length, ymd_sj_length;

struct TimeStrData
{
  String week_jp;
  String week_en3;
  String week_en2;
};

struct TimeStrData TSD[7] =
{
  {"日", "Sun", "Su"}, {"月", "Mon", "Mo"}, {"火", "Tue", "Tu"}, {"水", "Wed", "We"}, {"木", "Thr", "Th"}, {"金", "Fri", "Fr"}, {"土", "Sat", "Sa"}
};

//-----News & Weather-----------------------------------------------
uint8_t web_get = 0, web_get2 = 0;
uint32_t Web_time = 0;
boolean first_get = false;
String mLocalIP;

//-----OLED-----------------------------------------------
uint8_t OLED_tmp_buf_cnv2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t OLED_DotOut[8][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
uint8_t OLED_DotOut2[8][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
uint8_t OLED_DotOut3[8][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
uint8_t OLED_Next_buf[8][8] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
uint8_t tmp_OLED_buf_cnv2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t tmp_OLED_buf_cnv3[8] = {0, 0, 0, 0, 0, 0, 0, 0};



void initialize() {
  //セットアップで全角を一旦表示させることが重要。半角だとなぜかSPIFFSファイル読み込みエラーになってしまうため。
  u8ts.UTF8_to_SJIS_str_cnv(UTF8SJIS_file, "ＷＡＩＴ・・・・", sj_txt, &sj_length);
  for (uint8_t i = 0; i < 8; i++) MFR.Sjis_To_MisakiFNT_DotRead(Zen_Font_file, Half_Font_file, 0, 0, sj_txt[i * 2], sj_txt[i * 2 + 1], LedDotOut[i]);
  for (uint8_t i = 0; i < 8; i++) adaLED.LED_Dot_Rotation(90, LedDotOut[i], OLED_DotOut2[i]);
  oled.OLED_2X2_Display_Out_16x127(OLED_Adress, 0, 2, OLED_DotOut2[7], OLED_DotOut2[6], OLED_DotOut2[5], OLED_DotOut2[4], OLED_DotOut2[3], OLED_DotOut2[2], OLED_DotOut2[1], OLED_DotOut2[0]);
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      OLED_DotOut2[i][j] = 0; LedDotOut[i][j] = 0; //一旦初期化しておく
    }
  }

  html_str1 = "<body style='background:#ddddff; color:#000;'>\r\n";
  html_str1 += ews.EWS_TextBox_Send("txt1", "１行目任意テキスト送信、半角ｶﾀｶﾅもOK!", "送信");
  html_str1 += "<br>\r\n";
  html_str1 += ews.EWS_TextBox_Send("Txt2", "２行目任意text送信、半角ｶﾀｶﾅもOK!", "送信");
  html_str1 += "<br><br>\r\n";
  html_str2 = "Scrolle Speed 1\r\n";
  html_str2 += ews.EWS_Canvas_Slider_T("Speed", 200, 40, "#777777", "#77aaff");
  html_str2 += "<br><br>\r\n";
  html_str2 += "Scrolle Speed 2\r\n";
  html_str2 += ews.EWS_Canvas_Slider_T("speed2", 200, 40, "#777777", "#77aaff");
  html_str3 = "<br><br>\r\n";
  html_str3 += ews.EWS_On_Momentary_Button("_SclStop", "Scrole Stop", 100, 25, 15, "#ffffff", "#777777");
  html_str3 += "<br><br>１行目白黒反転　\r\n";
  html_str4 = ews.EWS_On_Momentary_Button("(rev", "Normal", 70, 25, 15, "#000000", "#ffffff");
  html_str4 += "<span>  </span>\r\n";
  html_str4 += ews.EWS_On_Momentary_Button(")rev", "Reverse", 70, 25, 15, "#ffffff", "#0000ff");
  html_str4 += "<br>２行目白黒反転　\r\n";
  html_str4 += ews.EWS_On_Momentary_Button("[rev", "Normal", 70, 25, 15, "#000000", "#ffffff");
  html_str4 += "<span>  </span>\r\n";
  html_str4 += ews.EWS_On_Momentary_Button("]rev", "Reverse", 70, 25, 15, "#ffffff", "#0000ff");
  html_str4 += "<br><br>\r\n";
  html_str5 = "<div style='width:330px; height:140px; text-align:center; padding:10px; background:#ccccff; border:#0000ff solid 1px;'>\r\n";
  html_str5 += ews.EWS_On_Momentary_Button("webON", "Web GET ON", 110, 25, 15, "#000000", "#ff77ff");
  html_str5 += "※ここボタンを押すとWebSocketは切断されます。再操作する場合はWS-Reconnectボタンを押してください<br>１行目　\r\n";
  html_str5 += ews.EWS_On_Momentary_Button("News", "Top NEWS", 100, 25, 15, "#000000", "#ffffff");
  html_str5 += "<span>  </span>\r\n";
  html_str5 += ews.EWS_On_Momentary_Button("Cnews", "PC NEWS", 110, 25, 15, "#000000", "#ffffff");
  html_str5 += "<br><br>２行目　\r\n";
  html_str5 += ews.EWS_On_Momentary_Button("ENews", "芸能News", 110, 25, 15, "#000000", "#ffffff");
  html_str5 += "<span>  </span>\r\n";
  html_str5 += ews.EWS_On_Momentary_Button("Weather", "Weather", 110, 25, 15, "#000000", "#ffffff");
  html_str5 += "</div><br><br>\r\n";
  html_str6 = ews.EWS_Status_Text(20, "RED");
  html_str6 += "<br>\r\n";
  html_str6 += "<br><br>\r\n";
  html_str6 += ews.EWS_WebSocket_Reconnection_Button("WS-Reconnect", 200, 40, 17);
  html_str6 += "<br><br>\r\n";
  html_str6 += ews.EWS_Close_Button("WS CLOSE", 150, 40, 17);
  html_str6 += ews.EWS_Window_ReLoad_Button("ReLoad", 150, 40, 17);
  html_str6 += "</body></html>\r\n";
  html_str7 = "";
}

//***********セットアップ***************************************************
void setup()
{
  Wire.begin(); // I2C initialise the connection
  Wire.setClock(400000L); //クロックはMax 400kHz

  oled.setup_OLED_SSD1306(OLED_Adress);
  delay(300);
  initialize();

  ews.AP_Connect(ssid, password);

  //NTPサーバーでタイムを取得
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  delay(1000);

  mLocalIP = "     [" + WiFi.localIP().toString() + "]     ";
  u8ts.UTF8_to_SJIS_str_cnv(UTF8SJIS_file, mLocalIP, sj_txt, &sj_length);
  SclTime = millis();
  LastTime = millis();
  prevDisplay = now();
}

//OLED1行目電光掲示板スクロール
void LCDDisplayLineOne(uint8_t xsj_txt[])
{
  uint8_t i;
  if (millis() - SclTime > SclSpeed) {
    if (sj_cnt >= sj_length) {
      sj_cnt = 0;
    }
    if (FntReadOK == true) {
      uint8_t cp = MFR.Sjis_To_MisakiFNT_DotRead(Zen_Font_file, Half_Font_file, Direction, Angle, xsj_txt[sj_cnt], xsj_txt[sj_cnt + 1], tmp_buf_cnv);
      FntReadOK = false;
      sj_cnt = sj_cnt + cp;
      adaLED.LED_Black_White_Reversal(BW_reverse, tmp_buf_cnv, tmp_buf_cnv);
      adaLED.LED_Dot_Rotation(Angle, tmp_buf_cnv, tmp_buf_cnv_2);
    }

    if (Scl_Stop == false) {
      for (i = 0; i < 8; i++) adaLED.LED_Dot_Rotation(90, LedDotOut[i], OLED_DotOut2[i]);
      oled.OLED_2X2_Display_Out_16x127(OLED_Adress, 0, 6, OLED_DotOut2[0], OLED_DotOut2[1], OLED_DotOut2[2], OLED_DotOut2[3], OLED_DotOut2[4], OLED_DotOut2[5], OLED_DotOut2[6], OLED_DotOut2[7]);
      adaLED.Scroller_Dot_Replace( Direction, Next_buf[0], LedDotOut[0], tmp_buf_cnv_2);
      for (i = 0; i < 7; i++) adaLED.Scroller_Dot_Replace( Direction, Next_buf[i + 1], LedDotOut[i + 1], Next_buf[i]);
      scl_cnt++;
    }

    if (scl_cnt == 8) {
      scl_cnt = 0;
      FntReadOK = true;
    }
    SclTime = millis();
  }

}

//OLED2行目電光掲示板スクロール
void LCDDisplayLineTwo(uint8_t xsj_txt2[])
{
  uint8_t i;
  if (millis() - SclTime2 > SclSpeed2) {
    if (sj_cnt2 >= sj_length2) {
      sj_cnt2 = 0;
    }
    if (FntReadOK2 == true) {
      uint8_t cp2 = MFR.Sjis_To_MisakiFNT_DotRead(Zen_Font_file, Half_Font_file, Direction, Angle, xsj_txt2[sj_cnt2], xsj_txt2[sj_cnt2 + 1], tmp_buf_cnv2);
      FntReadOK2 = false;
      sj_cnt2 = sj_cnt2 + cp2;
      adaLED.LED_Black_White_Reversal(BW_reverse2, tmp_buf_cnv2, tmp_buf_cnv2);
    }

    if (Scl_Stop == false) {
      for (i = 0; i < 8; i++) adaLED.LED_Dot_Rotation(90, LedDotOut2[i], OLED_DotOut3[i]);
      oled.OLED_2X2_Display_Out_16x127(OLED_Adress, 0, 4, OLED_DotOut3[0], OLED_DotOut3[1], OLED_DotOut3[2], OLED_DotOut3[3], OLED_DotOut3[4], OLED_DotOut3[5], OLED_DotOut3[6], OLED_DotOut3[7]);
      adaLED.Scroller_Dot_Replace( Direction, Next_buf2[0], LedDotOut2[0], tmp_buf_cnv2);
      for (i = 0; i < 7; i++) adaLED.Scroller_Dot_Replace( Direction, Next_buf2[i + 1], LedDotOut2[i + 1], Next_buf2[i]);
      scl_cnt2++;
    }

    if (scl_cnt2 == 8) {
      scl_cnt2 = 0;
      FntReadOK2 = true;
    }
    SclTime2 = millis();
  }

}

//**************メインループ*************************************************
void loop() {
  //WebSocket ハンドシェイク関数
  ews.EWS_HandShake(html_str1, html_str2, html_str3, html_str4, html_str5, html_str6, html_str7);

  uint8_t i, br;

  ret_str = ews.EWS_ESP8266CharReceive(PingSendTime); //ブラウザからのWebSocketデータ受信

  if (ret_str != "_close") {
    if (ret_str != "\0") {
      if (ret_str != "Ping") {
        if (ret_str[0] != 't' && ret_str[0] != 'T') {
          switch (ret_str[4]) {
            case 'S':
              SclSpeed = (ret_str[0] - 0x30) * 100 + (ret_str[1] - 0x30) * 10 + (ret_str[2] - 0x30);
              SclSpeed = 200 - SclSpeed;
              Scl_Stop = false;
              break;
            case 's':
              SclSpeed2 = (ret_str[0] - 0x30) * 100 + (ret_str[1] - 0x30) * 10 + (ret_str[2] - 0x30);
              SclSpeed2 = 200 - SclSpeed2;
              Scl_Stop = false;
              break;
            case '_':
              Scl_Stop = true;
              break;
            case '(':
              BW_reverse = false;
              break;
            case ')':
              BW_reverse = true;
              break;
            case '[':
              BW_reverse2 = false;
              break;
            case ']':
              BW_reverse2 = true;
              break;
            case 'w':
              web_get = 1; web_get2 = 1; first_get = true;
              break;
            case 'N':
              web_get = 1;
              sjis_txt_in = false; first_get = true;
              break;
            case 'C':
              web_get = 2;
              sjis_txt_in = false; first_get = true;
              break;
            case 'E':
              web_get2 = 1;
              sjis_txt_in2 = false; first_get = true;
              break;
            case 'W':
              web_get2 = 2;
              sjis_txt_in2 = false; first_get = true;
              break;
          }
        } else if (ret_str[0] == 't' || ret_str[0] == 'T' ) {
          scl_txt = ret_str.substring(ret_str.indexOf('|') + 1, ret_str.length() - 1);
          scl_txt += String("　") + String("\0");
          switch (ret_str[0]) {
            case 't':
              u8ts.UTF8_to_SJIS_str_cnv(UTF8SJIS_file, scl_txt, sj_txt, &sj_length);
              fnt_cnt = 0; scl_cnt = 0; sj_cnt = 0; web_get = 0;
              FntReadOK = true; sjis_txt_in = true; Scl_Stop == false;
              break;
            case 'T':
              u8ts.UTF8_to_SJIS_str_cnv(UTF8SJIS_file, scl_txt, sj_txt2, &sj_length2);
              fnt_cnt2 = 0; scl_cnt2 = 0; sj_cnt2 = 0; web_get = 0;
              FntReadOK2 = true; sjis_txt_in2 = true; Scl_Stop == false;
              break;
          }
          Serial.println();
          for (int iiii = 0; iiii < sj_length; iiii++) Serial.write(sj_txt[iiii]);
          Serial.println();
        }
        ret_str = "";
      } else {
        ret_str = "";
      }
    }

    if (sjis_txt_in == true) { //OLED1行目電光掲示板スクロール
      LCDDisplayLineOne(sj_txt);
    }

    if (sjis_txt_in2 == true) {  //OLED2行目電光掲示板スクロール
      LCDDisplayLineTwo(sj_txt2);
    }

  } else if (ret_str == "_close") {
    ret_str = "";
    scl_txt = "";
  }

  if (now() != prevDisplay) { //ここから時刻表示設定
    char month_chr[3], day_chr[3], h_chr[3], m_chr[3], s_chr[3];
    sprintf(h_chr, "%2d", hour());//ゼロを空白で埋める場合は%2dとすれば良い
    sprintf(m_chr, "%02d", minute());
    sprintf(s_chr, "%02d", second());
    sprintf(month_chr, "%2d", month());
    sprintf(day_chr, "%02d", day());

    time_str = String(h_chr) + ':' + String(m_chr) + ':' + String(s_chr);
    ymd_str = String(year()) + '/' + String(month_chr) + '/' + String(day_chr) + "（" + String(TSD[weekday() - 1].week_jp) + "）";

    uint8_t time_dot[4][8];
    u8ts.UTF8_to_SJIS_str_cnv(UTF8SJIS_file, time_str, time_sj_txt, &time_sj_length);
    if (Direction != 0) Direction = 0;
    for (i = 0; i < 4; i++) MFR.Sjis_To_MisakiFNT_DotRead(Zen_Font_file, Half_Font_file, Direction, 0, time_sj_txt[i * 2], time_sj_txt[i * 2 + 1], time_dot[i]);
    uint8_t dummy_time_dot[4][8];
    uint8_t OLED_time_dot[4][8];
    for (i = 0; i < 4; i++) adaLED.LED_Dot_Rotation(Angle, time_dot[i], dummy_time_dot[i]);
    for (i = 0; i < 4; i++) adaLED.LED_Dot_Rotation(90, time_dot[i], OLED_time_dot[i]);

    uint8_t OLED_ymd_dot[8][8];
    if (dummy_ymd_str != ymd_str) {
      u8ts.UTF8_to_SJIS_str_cnv(UTF8SJIS_file, ymd_str, ymd_sj_txt, &ymd_sj_length);
      uint8_t ymd_dot[8][8];
      if (Direction != 0) Direction = 0;
      for (i = 0; i < 8; i++) MFR.Sjis_To_MisakiFNT_DotRead(Zen_Font_file, Half_Font_file, Direction, 0, ymd_sj_txt[i * 2], ymd_sj_txt[i * 2 + 1], ymd_dot[i]);
      uint8_t dummy_ymd_dot[8][8];
      for (i = 0; i < 8; i++) adaLED.LED_Dot_Rotation(Angle, ymd_dot[i], dummy_ymd_dot[i]);
      for (i = 0; i < 8; i++) adaLED.LED_Dot_Rotation(90, ymd_dot[i], OLED_ymd_dot[i]);
    }

    if (Angle == 0 || Angle == 90) {
      oled.OLED_2X2_Display_Out_16x64(OLED_Adress, 30, 0, OLED_time_dot[3], OLED_time_dot[2], OLED_time_dot[1], OLED_time_dot[0]);
      if (dummy_ymd_str != ymd_str) {
        oled.OLED_2X2_Display_Out_16x127(OLED_Adress, 0, 2, OLED_ymd_dot[7], OLED_ymd_dot[6], OLED_ymd_dot[5], OLED_ymd_dot[4], OLED_ymd_dot[3], OLED_ymd_dot[2], OLED_ymd_dot[1], OLED_ymd_dot[0]);
        dummy_ymd_str = ymd_str;
      }
    } else {
      oled.OLED_2X2_Display_Out_16x64(OLED_Adress, 30, 0, OLED_time_dot[0], OLED_time_dot[1], OLED_time_dot[2], OLED_time_dot[3]);
    }
    prevDisplay = now();
  }

  if (web_get == 0) {
    if (millis() - LastTime >= 300000L) { //5分毎にNTPサーバーからタイム取得
      setSyncProvider(getNtpTime);
      Serial.println(time_str);
      LastTime = millis();
    }
  } else {
    if (millis() - LastTime >= 240000L) { //Web記事取得時でもNTPサーバからタイムを補正しておく
      setSyncProvider(getNtpTime);
      LastTime = millis();
    }
  }

  if (web_get > 0 || web_get2 > 0) { //Web記事取得
    if (first_get == true || millis() - Web_time > 600000L) { //Web記事を１０分毎に取得
      char* news1_1_host = "news.yahoo.co.jp";
      String news1_1_target_ip = "/pickup/rss.xml"; // トップニュースタイトルページ
      char* news1_2_host = "news.yahoo.co.jp";
      String news1_2_target_ip = "/pickup/computer/rss.xml"; //コンピューター系ニュースページ
      String news_str = "";
      char Web_h[3], Web_m[3];
      sprintf(Web_h, "%02d", hour());//ゼロを空白で埋める場合は%2dとする
      sprintf(Web_m, "%02d", minute());
      news_str = String("◆") + String(Web_h) + ":" + String(Web_m) + " ";
      switch (web_get) {
        case 1:
          news_str += ews.EWS_Web_Get(news1_1_host, news1_1_target_ip, '\n', "</rss>", "<title>", "</title>", "◆");
          break;
        case 2:
          news_str += ews.EWS_Web_Get(news1_2_host, news1_2_target_ip, '\n', "</rss>", "<title>", "</title>", "◆");
          break;
      }
      news_str.replace("&amp;", "＆");
      Serial.println();
      u8ts.UTF8_to_SJIS_str_cnv(UTF8SJIS_file, news_str, sj_txt, &sj_length);
      news_str = "";
      for (int iy = 0; iy < sj_length; iy++) {
        Serial.write(sj_txt[iy]);
      }
      Serial.println();
      fnt_cnt = 0; scl_cnt = 0; sj_cnt = 0;
      sjis_txt_in = true;  FntReadOK = true;
      //２つめの記事取得
      char* news2_1_host = "news.yahoo.co.jp";
      String news2_1_target_ip = "/pickup/entertainment/rss.xml"; // 芸能トップニュースタイトルページ
      char* news2_2_host = "rss.weather.yahoo.co.jp";
      String news2_2_target_ip = "/rss/days/4620.xml"; //小田原の天気予報ページ
      String news2_str = "";

      news2_str = String("◆") + String(Web_h) + ":" + String(Web_m) + " ";
      switch (web_get2) {
        case 1:
          news2_str += ews.EWS_Web_Get(news2_1_host, news2_1_target_ip, '\n', "</rss>", "<title>", "</title>", "◆");
          break;
        case 2:
          news2_str += ews.EWS_Web_Get(news2_2_host, news2_2_target_ip, '>', "</rss>", "【", " - ", "【");
          break;
      }
      news2_str.replace("&amp;", "＆");
      Serial.println(); Serial.print("WebGet2=");
      u8ts.UTF8_to_SJIS_str_cnv(UTF8SJIS_file, news2_str, sj_txt2, &sj_length2);
      news2_str = "";
      for (int iy = 0; iy < sj_length2; iy++) {
        Serial.write(sj_txt2[iy]);
      }
      Serial.println();
      fnt_cnt2 = 0; scl_cnt2 = 0; sj_cnt2 = 0;
      sjis_txt_in2 = true;  FntReadOK2 = true;
      Web_time = millis();
      first_get = false;
    }
  }
}

//*************************NTP Time**************************************
time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

//*************************NTP Time**************************************
void sendNTPpacket(IPAddress &address)
{ // send an NTP request to the time server at the given address
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


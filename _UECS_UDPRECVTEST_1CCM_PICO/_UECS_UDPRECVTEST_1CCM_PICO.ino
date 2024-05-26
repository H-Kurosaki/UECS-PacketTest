//UECS パケット処理能力テスト受信側
/*
実験の概要
２つのマイコンで送受信速度のテストを行います
このプログラムは受信側のラズパイPICOです。
0から順に+1ずつ加算した数値が格納されたUECS-CCMパケットが送られてきます。
もしパケットロスが無ければ数値は+1ずつ順番に受信されますが
パケットロスがあると間の数字が飛んで差が+1ではなくなります。
これを検出するとLEDを点灯して通知します。
*/

#include <Uardecs_pico.h>
#define DOUT_LED_RECV   2
#define DOUT_LED_LOSS   4


/*
#define NONE -1
#define A_1S_0 0
#define A_1S_1 1
#define A_10S_0 2
#define A_10S_1 3
#define A_1M_0 4
#define A_1M_1 5
#define S_1S_0 6
#define S_1M_0 7
#define B_0 8
#define B_1 9

#define UECSSHOWDATA 0
#define UECSINPUTDATA 1
#define UECSSELECTDATA 2
#define UECSSHOWSTRING 3
*/

/////////////////////////////////////
//IP reset jupmer pin setting
//IPアドレスリセット用ジャンパーピン設定
/////////////////////////////////////
//Pin ID. This pin is pull-upped automatically.
//ピンIDを入力、このピンは自動的にプルアップされます
//ピンIDは変更可能です
const byte U_InitPin = 3;
const byte U_InitPin_Sense=LOW;//ソフトウェア上でSafeModeを抜けたい場合はLOWにしてください

//When U_InitPin status equals this value,IP address is set "192.168.1.7".
//U_InitPinに指定したピンがこの値になった時、IPアドレスが"192.168.1.7"に初期化されます。
//購入直後のArduinoは初期化が必要です。

////////////////////////////////////
//Node basic infomation
//ノードの基本情報
///////////////////////////////////
const char U_name[] PROGMEM= "UARDECS Node v.1.1";//MAX 20 chars
const char U_vender[] PROGMEM= "XXXXXXXX Co.";//MAX 20 chars
const char U_uecsid[] PROGMEM= "000000000000";//12 chars fixed
const char U_footnote[] PROGMEM= "UARDECS Sample Program Thermostat";
char U_nodename[20] = "Sample";//MAX 19chars (This value enabled in safemode)
UECSOriginalAttribute U_orgAttribute;//この定義は弄らないで下さい


const int U_HtmlLine = 2;

//////////////////////////////////
// html page1 setting
//////////////////////////////////

PROGMEM const char NONES[] = "";

const char PROGMEM NAME0[] = "recv packet";
PROGMEM const char UNIT0[] = "";

const char PROGMEM NAME1[] = "drop packet";
PROGMEM const char UNIT1[] = "";

static signed long recvpacket=0;
static signed long droppacket=0;
static signed long recvpacket_=0;
static signed long droppacket_=0;


struct UECSUserHtml U_html[U_HtmlLine]={
{NAME0, UECSSHOWDATA, UNIT0, NONES, NULL, 0, &(recvpacket), 0, 0, 0},
{NAME1, UECSSHOWDATA, UNIT1, NONES, NULL, 0, &(droppacket), 0, 0, 0},
};

//////////////////////////////////
// UserCCM setting
// CCM用の素材
//////////////////////////////////


enum {
CCMID_test1,
CCMID_dummy,
};


//CCM格納変数の宣言
//ここはこのままにして下さい
const int U_MAX_CCM = CCMID_dummy;
UECSCCM U_ccmList[U_MAX_CCM];

const char PROGMEM ccmName1[] = "TEST1";
const char PROGMEM ccmType1[] = "test00000000000.xXX";
const char PROGMEM ccmUnit1[] = "";

void UserInit(){
U_orgAttribute.mac[0] = 0x11;
U_orgAttribute.mac[1] = 0x11;
U_orgAttribute.mac[2] = 0x22;
U_orgAttribute.mac[3] = 0x22;
U_orgAttribute.mac[4] = 0x33;
U_orgAttribute.mac[5] = 0x33;
UECSsetCCM(false, CCMID_test1      , ccmName1, ccmType1,ccmUnit1, 30, 0, A_1S_0);
}


//---------------------------------------------------------
//Webページから入力が行われ各種値を取得後以下の関数が呼び出される。
//この関数呼び出し後にEEPROMへの値の保存とWebページの再描画が行われる
//---------------------------------------------------------
void OnWebFormRecieved(){

}


//---------------------------------------------------------
//毎秒１回呼び出される関数
//関数の終了後に自動的にCCMが送信される
//---------------------------------------------------------
void UserEverySecond(){

}

//---------------------------------------------------------
//１分に１回呼び出される関数
//---------------------------------------------------------
void UserEveryMinute(){
}

//---------------------------------------------------------
//メインループ
//システムのタイマカウント，httpサーバーの処理，
//UDP16520番ポートと16529番ポートの通信文をチェックした後，呼び出さされる関数。
//呼び出される頻度が高いため，重い処理を記述しないこと。
//---------------------------------------------------------
void UserEveryLoop(){
static signed long lastVal=-1;
  if(U_ccmList[CCMID_test1].validity)
      {
      if(U_ccmList[CCMID_test1].value-lastVal==1)
        {
        recvpacket_++;
        lastVal=U_ccmList[CCMID_test1].value;
        }
      else if(U_ccmList[CCMID_test1].value-lastVal>1)
        {
        droppacket_+=U_ccmList[CCMID_test1].value-lastVal-1;
        recvpacket_++;
        digitalWrite(DOUT_LED_LOSS,HIGH);
        lastVal=U_ccmList[CCMID_test1].value;
        }
      }
     else
     {
     }
     
    if((U_ccmList[CCMID_test1].value/100)%2)
        {digitalWrite(DOUT_LED_RECV,HIGH);}
    else
        {digitalWrite(DOUT_LED_RECV,LOW);}
     recvpacket=recvpacket_;
     droppacket=droppacket_;
}

//---------------------------------------------------------
//setup()実行後に呼び出されるメインループ
//この関数内ではUECSloop()関数を呼び出さなくてはならない。
//UserEveryLoop()に似ているがネットワーク関係の処理を行う前に呼び出される。
//必要に応じて処理を記述してもかまわない。
//呼び出される頻度が高いため,重い処理を記述しないこと。
//---------------------------------------------------------
void loop(){
UECSloop();
}

//---------------------------------------------------------
//起動直後に１回呼び出される関数。
//様々な初期化処理を記述できる。
//この関数内ではUECSsetup()関数を呼び出さなくてはならない。
//必要に応じて処理を記述してもかまわない。
//---------------------------------------------------------
void setup(){
pinMode(20, OUTPUT);//W5500 Reset
digitalWrite(20,LOW);
delay(100);
digitalWrite(20,HIGH);

pinMode(DOUT_LED_LOSS, OUTPUT);//LED
digitalWrite(DOUT_LED_LOSS,LOW);

pinMode(DOUT_LED_RECV, OUTPUT);//LED
digitalWrite(DOUT_LED_RECV,LOW);


UECSsetup();
}

//UECS パケット処理能力テスト送信側(若干危険)
/*
実験の概要
２つのマイコンで送受信速度のテストを行います
このプログラムは送信側のラズパイPICOです。
0から順に+1ずつ加算した数値が格納されたUECS-CCMパケットを生成してブロードキャストします。
パケットを送る頻度を徐々に早くしていきます。
最初は600usecのウェイトを入れてありますが、5秒ごとに-5usecされ、
最後にはノーウェイトで大量のパケットが送信されます。
(だいたい1000packet/secぐらいまで行きます)
DDOS攻撃になるので絶対にLAN外で使用しないこと。
*/

//Arduinoのノーウェイト最高値:712packet/sec
//RSPI_PICOのノーウェイト最高値:1170packet/sec
#include <SPI.h>    
#include <Ethernet2.h>
#include <EthernetUdp2.h>

byte mac[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};

byte ip[] = { 192,168, 1, 117 };
byte gateway[] = { 192,168, 1, 1 };
byte subnet[] = { 255, 255, 255, 0 };
byte dnssrv[] = { 0, 0, 0, 0 };
byte defdummy[] = {0,0,0,0};

IPAddress sendip(255, 255,255,255);
unsigned int localPort = 16520;
EthernetUDP UECS_UDP16520;


void EthernetSetup()
{
  Ethernet.init(17);//RSPI_PICO
  //Ethernet.init(10);//Arduino UNO or MEGA
  Ethernet.begin(mac, ip, dnssrv, gateway,subnet);
  UECS_UDP16520.begin(16520);
}

void loop(){
  static unsigned long wait=3000;
  static unsigned long lastms=millis()/1000;


  static unsigned long count=0;
  static unsigned long countps=0;

  UECS_UDP16520.beginPacket(sendip, 16520);
  UECS_UDP16520.write("<?xml version=\"1.0\"?><UECS ver=\"1.00-E10\"><DATA type=\"test00000000000.xXX\" room=\"1\" region=\"1\" order=\"1\" priority=\"30\">");
  char s[8];
  sprintf(s,"%d",count);
  UECS_UDP16520.write(s);
  UECS_UDP16520.write("</DATA><IP>192.168.1.101</IP></UECS>");

  UECS_UDP16520.endPacket();
  count++;
  countps++;
  delayMicroseconds(wait);
  
  if(lastms!=millis()/1000)
    {
      lastms=millis()/1000;
      if(lastms%2==0&& wait>0)
        {
          wait-=100;
        }
      Serial.print(countps);
      Serial.println("packets/sec");
      countps=0;
    }



 

}


void setup(){
delay(5000);
Serial.begin(9600);
 EthernetSetup();
}

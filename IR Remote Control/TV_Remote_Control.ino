/*
처음 작성 일자: 2019년 3월 30일 KST
마지막 편집 일자: 2019년 3월 30일 KST
작성자: Sean (nadane1708)

테스트한 보드: 아두이노 UNO R3 정품 보드 (ATmega328P)
테스트한 스케치 IDE 버전: 1.8.9

소스코드 설명: IR Receiver 모듈을 이용하여 브루트포스 공격(brute-force attack)으로 알아낸 제주 KCTV TV 리모컨 적외선 신호를 4x4 버튼 모듈과 IR Transmitter 모듈을 이용하여 KCTV 컨버터로 보낸다.(TV전원 신호 제외)
*/

#include <Keypad.h>
#include <IRremote.h>

IRsend irsend;

const byte ROWS = 4;    // 행(rows) 개수
const byte COLS = 4;    // 열(columns) 개수

// 제주 KCTV 케이블 TV용 리모컨 정보는 주소 참고: https://www.kctvjeju.com/product/analog-tv.kctv
// 제주 KCTV TV 리모컨용 적외선 신호 HEX값 (순서대로 TV전원, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 음량 올리기, 음량 내리기, 채널 올리기, 채널 내리기, 이전)
unsigned long hex_keys[] = {0x20DF10EF,0x12A4807F,0x12A440BF,0x12A4C03F,0x12A420DF,0x12A4A05F,0x12A4609F,0x12A4E01F,0x12A410EF,0x12A4906F,0x12A400FF,0x12A4C837,0x12A428D7,0x12A4E817,0x12A418E7,0x12A49867};
String alphabet = "APONMLKJIHGFEDCB"; // 4x4 키패드 순서에 맞게 배열

char keys[ROWS][COLS] = {
  {'A','B','C','D'},
  {'E','F','G','H'},
  {'I','J','K','L'},
  {'M','N','O','P'}
};

byte rowPins[ROWS] = {13, 12, 11, 10};   // R1, R2, R3, R4 단자가 연결된 아두이노 핀 번호
byte colPins[COLS] = {9, 8, 7, 5};   // C1, C2, C3, C4 단자가 연결된 아두이노 핀 번호
 
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
 
void setup() {
  Serial.begin(9600);
}
   
void loop() {
  char key = keypad.getKey();
   
  if (key) {
    Serial.println(key);
    Serial.println(alphabet.indexOf(key));
    Serial.println(hex_keys[alphabet.indexOf(key)]);
    irsend.sendNEC(hex_keys[alphabet.indexOf(key)], 32); // 적외선 HEX값 신호 보내기
    delay(50);
  }
}

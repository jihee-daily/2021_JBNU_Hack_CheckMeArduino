#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MLX90614.h>
#include <qrcode.h>
#include <U8glib.h>
 
#define TRIG 9
#define ECHO 8
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define OLED_RESET 4
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0);

int limit = 20;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(11, OUTPUT);
  mlx.begin();

  lcd.init();
  lcd.backlight(); 
}

void loop() {
  if(checkDistance() <= limit){
    unsigned long t = millis(); //스톱워치
    bool mode = false;
    while(1){
      if(checkDistance() > limit){
        break;
      }
      if(millis() - t > 1000){
        //1초이상 차이가 발생한 지점
        mode = true;
        break;
      }
    }

    if(mode){
      lcd.setCursor(0,0);
      lcd.print("Don't move!");
      digitalWrite(11, HIGH);
      t = millis(); //스톱워치작동
      double temp = 0; //온도값을 누적할 변수
      int count = 0; //샘플의 갯수를 누적할 변수
      while(1){
        if(millis() - t > 3000){
          //3초이상
          digitalWrite(11, LOW);
          break;
        }
        temp += mlx.readObjectTempC();
        count++;
        delay(10);
      }
      //측정종료
      lcd.setCursor(0,1);
      lcd.print(temp/count,1);
      lcd.print(" C");
      int value = temp/count*10;
      
      Serial.print(value);
      Serial.print("#10115@");
      printQRCode(value+"#10115@");

      delay(5000);
      lcd.clear();
    }   
  }
}

float checkDistance(){
  float duration = 0;
  
  digitalWrite(TRIG,LOW);
  delayMicroseconds(TRIG);
  digitalWrite(TRIG,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG,LOW);
  duration = (pulseIn(ECHO, HIGH)/29.0)/2; //cm
  
  return duration;
}

void printQRCode(const char *text){
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(4)];;
  qrcode_initText (&qrcode, qrcodeData, 4, ECC_LOW, text);
  
  u8g.firstPage();
  do {
      u8g.setColorIndex(0);
      u8g.drawBox(0, 0, 128, 64);
      u8g.setColorIndex(1);
      for (uint8_t y = 0; y < qrcode.size; y++) {
          for (uint8_t x = 0; x < qrcode.size; x++) {
              if (qrcode_getModule(&qrcode, x, y))
                  u8g.drawBox(30+2*x, 2*y, 2, 2);
          }
      }
  } while ( u8g.nextPage() ); 
}

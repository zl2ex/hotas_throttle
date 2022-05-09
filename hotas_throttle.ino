#include <Joystick.h>
#include <Adafruit_ADS1X15.h>
#include <PCF8575.h>

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK , 32, 0,
  true, true, true, true, true, true,
  true, true, false, false, false);
  
Adafruit_ADS1115 ads1115;

PCF8575 pcf8575(0x39); // i2c adress of PCF8575

// pins 0 to 99 native arduino digitalRead
// pins 100 to 255 external PCF8575 io board
// pin 100 = pin 0 on PCF8575 io board
bool __digitalRead(char pin) {
  if(pin < 100) {
    return digitalRead(pin);
  }
  else {
    pcf8575.read(pin - 100);
  }
}

class sw3p {
  public:
    void init(char pin1, char pin2, char btn1, char btn2, char btn3);
    void poll();
    char pins[2];
    char btns[3];
};

void sw3p::init(char pin1, char pin2, char btn1, char btn2, char btn3) {
  sw3p::pins[0] = pin1;
  sw3p::pins[1] = pin2;
  if(pin1 < 100 && pin2 < 100) { // only need to configure arduino pins not pcf8575 pins
    for(int i = 0; i < 2; i++) {
      pinMode(sw3p::pins[i], INPUT_PULLUP);
    }
  }
  sw3p::btns[0] = btn1;
  sw3p::btns[1] = btn2;
  sw3p::btns[2] = btn3;
}

void sw3p::poll() {
  char state = !__digitalRead(sw3p::pins[0]) | (!__digitalRead(sw3p::pins[1]) << 1);
  Joystick.setButton(sw3p::btns[0], (state==1)?1:0);
  Joystick.setButton(sw3p::btns[1], (state==0)?1:0);
  Joystick.setButton(sw3p::btns[2], (state==2)?1:0);
}

class sw2p {
  public:
    void init(char pin, char btn1, char btn2);
    void poll();
    char pin;
    char btns[2];
};

void sw2p::init(char pin, char btn1, char btn2) {
  sw2p::pin = pin;
  if(pin < 100) { // only need to configure arduino pins not pcf8575 pins
    pinMode(sw2p::pin, INPUT_PULLUP);
  }
  sw2p::btns[0] = btn1;
  sw2p::btns[1] = btn2;
}

void sw2p::poll() {
  int state = __digitalRead(sw2p::pin);
  Joystick.setButton(sw2p::btns[0], state);
  Joystick.setButton(sw2p::btns[1], !state);
}

class btn {
  public:
    void init(char pin, char btn);
    void poll();
    char pin;
    char btn;
    bool invert;
};

void btn::init(char pin, char btn) {
  btn::pin = pin;
  btn::invert = invert;
  if(pin < 100) { // only need to configure arduino pins not pcf8575 pins
    pinMode(btn::pin, INPUT_PULLUP);
  }
  btn::btn = btn;
}

void btn::poll() {
  int state = __digitalRead(btn::pin);
  if(invert) state = !state;
  Joystick.setButton(btn::btn, state);
}

// switches count
#define SW_3_POS_COUNT 1
sw3p sw3pos[SW_3_POS_COUNT];

#define SW_2_POS_COUNT 1
sw2p sw2pos[SW_2_POS_COUNT];

#define BTN_COUNT 7
btn button[BTN_COUNT];

class hat {
  public:
    void init(char pin1, char pin2, char pin3, char pin4, char hat_sw);
    void poll();
    char hat_sw;
    char pins[7];
};

void hat::init(char pin1, char pin2, char pin3, char pin4, char hat_sw) {
  hat::hat_sw = hat_sw;
  hat::pins[0] = pin1;
  hat::pins[1] = pin2;
  hat::pins[2] = pin3;
  hat::pins[3] = pin4;
  if(pin < 100) { // only need to configure arduino pins not pcf8575 pins
    for(int i = 0; i < 8; i++) {
      pinMode(hat::pins[i], INPUT_PULLUP);
    }
  }
}

void hat::poll() {
  int deg;
  uint8_t buff = 0;
  for(int i = 0; i < 4; i++) {
   buff |= __digitalRead(hat::pins[i]);
   buff >> 1;
  }
  switch(buff) {
    case 0b0001:
      deg = 0;
    break;
    case 0b0011:
      deg = 45;
    break;
    case 0b0010:
      deg = 90;
    break;
    case 0b0110:
      deg = 135;
    break;
    case 0b0100:
      deg = 180;
    break;
    case 0b1100:
      deg = 225;
    break;
    case 0b1000:
      deg = 270;
    break;
    case 0b1001:
      deg = 315;
    break;
    default:
      deg = 0;
    break;
  }
  Joystick.setHatSwitch(hat::hat_sw, deg);
}

//JOYSTICK_HATSWITCH_RELEASE 


void setup() {
  sw3pos[0].init(5, 6, 0, 1, 2); // digital in pins 5 and 6, representeing 3 positions on joystick buttons 0, 1, 2
  sw2pos[0].init(4, 3, 4); // digital in pin 4, representeing 2 positions on joystick buttons 3, 4
  button[0].init(7, 5); // button 5 on pin 7
  button[1].init(8, 6);
  button[2].init(9, 7);
  button[3].init(10, 8);
  button[4].init(14, 9);
  button[5].init(15, 10);
  button[6].init(100, 11); // button 11 on pin 100 (or 0 of pcf8575)
  Joystick.setXAxisRange(-32767, 32767);
  Joystick.setYAxisRange(-32767, 32767);
  Joystick.setZAxisRange(-32767, 32767);
  Joystick.setThrottleRange(-32767, 32767);
  Joystick.setRxAxisRange(0, 1024);
  Joystick.setRyAxisRange(0, 1024);
  Joystick.begin();

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  ads1115.setGain(GAIN_TWO);
  ads1115.begin(0x48);  // Initialize ads1115 at address 0x48 ADDR=GND
  
}

void loop() {

  for(int i = 0; i < SW_3_POS_COUNT; i++) {
    sw3pos[i].poll();
  }
  for(int i = 0; i < SW_2_POS_COUNT; i++) {
    sw2pos[i].poll();
  }
  for(int i = 0; i < BTN_COUNT; i++) {
    button[i].poll();
  }
  Joystick.setZAxis(ads1115.readADC_SingleEnded(0));
  Joystick.setThrottle(ads1115.readADC_SingleEnded(1));
  Joystick.setXAxis(ads1115.readADC_SingleEnded(2));
  Joystick.setYAxis(ads1115.readADC_SingleEnded(3));
  Joystick.setRxAxis(analogRead(A0));
  Joystick.setRyAxis(analogRead(A1));
  delay(100);
}

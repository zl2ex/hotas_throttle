#include <Joystick.h>
#include <Adafruit_ADS1X15.h>

// analog inputs
#define BL_INPUT A3

// outputs
#define BL_PWM 10

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK , 32, 0,
  true, true, true, true, true, true,
  true, true, false, false, false);
  
Adafruit_ADS1115 ads1115;

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
  for(int i = 0; i < 2; i++) {
    pinMode(sw3p::pins[i], INPUT_PULLUP);
  }
  sw3p::btns[0] = btn1;
  sw3p::btns[1] = btn2;
  sw3p::btns[2] = btn3;
}

void sw3p::poll() {
  char state = !digitalRead(sw3p::pins[0]) | (!digitalRead(sw3p::pins[1]) << 1);
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
  pinMode(sw2p::pin, INPUT_PULLUP);
  sw2p::btns[0] = btn1;
  sw2p::btns[1] = btn2;
}

void sw2p::poll() {
  int state = digitalRead(sw2p::pin);
  Joystick.setButton(sw2p::btns[0], state);
  Joystick.setButton(sw2p::btns[1], !state);
}

class btn {
  public:
    void init(char pin, char btn, bool invert);
    void poll();
    char pin;
    char btn;
    bool invert;
};

void btn::init(char pin, char btn, bool invert) {
  btn::pin = pin;
  btn::invert = invert;
  if(btn::invert) {
    pinMode(btn::pin, INPUT_PULLUP);
  }
  else {
    pinMode(btn::pin, INPUT);
  }
  btn::btn = btn;
}

void btn::poll() {
  int state = digitalRead(btn::pin);
  if(invert) state = !state;
  Joystick.setButton(btn::btn, state);
}

// switches count
#define SW_3_POS_COUNT 1
sw3p sw3pos[SW_3_POS_COUNT];

#define SW_2_POS_COUNT 1
sw2p sw2pos[SW_2_POS_COUNT];

#define BTN_COUNT 6
btn button[BTN_COUNT];

class hat {
  public:
    void init(char hat_sw, char pin1, char pin2, char pin3, char pin4, char pin5, char pin6, char pin7, char pin8);
    void poll();
    char hat_sw;
    char pins[7];
};

void hat::init(char hat_sw, char pin1, char pin2, char pin3, char pin4, char pin5, char pin6, char pin7, char pin8) {
  hat::hat_sw = hat_sw;
  hat::pins[0] = pin1;
  hat::pins[1] = pin2;
  hat::pins[2] = pin3;
  hat::pins[3] = pin4;
  hat::pins[4] = pin5;
  hat::pins[5] = pin6;
  hat::pins[6] = pin7;
  hat::pins[7] = pin8;
  for(int i = 0; i < 8; i++) {
    pinMode(hat::pins[i], INPUT_PULLUP);
  }
}

void hat::poll() {
  int deg;
  uint8_t buff = 0;
  for(int i = 0; i < 8; i++) {
   buff |= digitalRead(hat::pins[i]);
   buff >> 1;
  }
  deg = buff; // wip !
  Joystick.setHatSwitch(hat::hat_sw, deg);
}

//JOYSTICK_HATSWITCH_RELEASE 

void backLight() {
  int bl = analogRead(BL_INPUT);
  int out = map(bl, 0, 1023, 10, 255);
  analogWrite(BL_PWM, out);
}

void setup() {
  pinMode(BL_PWM, OUTPUT);
  sw3pos[0].init(5, 6, 0, 1, 2); // digital in pins 5 and 6, representeing 3 positions on joystick buttons 0, 1, 2
  sw2pos[0].init(4, 3, 4);
  button[0].init(7, 5, 1); // button 5 on pin 7 inverted (0v = on)
  button[1].init(8, 6, 1);
  button[2].init(9, 7, 1);
  button[3].init(10, 8, 1);
  button[4].init(14, 9, 0);
  button[5].init(15, 10, 0);
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

  backLight();

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

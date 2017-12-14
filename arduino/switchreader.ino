#define DEBOUNCE_TIME 10
#define DIGITAL_INPUT_PINS  54
#define START_PIN  2
#define MS_LS_PIN  31           // Most significant least-significant-pin. 
#define BIT_RATE 115200

#define PACKET_TYPE__KEY_STATE 1

unsigned long lsPins;  // Pins 0 through 31 - though 0 and 1 can't be used
unsigned long msPins;  // Pins 32 through 53
bool lasttime[DIGITAL_INPUT_PINS];

unsigned long lsPinsLastValue;     // To detect state changes 
unsigned long msPinsLastValue;

byte longBuf[8];

void setup()
{

  for (int pin = START_PIN; pin <= DIGITAL_INPUT_PINS; pin++) {
    pinMode(pin, INPUT_PULLUP);        
  }

  lsPins = 0;
  msPins = 0; 

  Serial.begin(BIT_RATE);
}

void loop()
{

  // Scan
  for (int pin = START_PIN; pin < DIGITAL_INPUT_PINS; pin++) {
    checkbutton(pin);       
  }
  
  // State change?  May want to bucket these
  if ((lsPins != lsPinsLastValue)||(msPins != msPinsLastValue)) {
    reportStateChange();
    msPinsLastValue = msPins;
    lsPinsLastValue = lsPins;
  }
  
}

void checkbutton(int pin) 
{
  //debounce
  if ((lasttime[pin] + DEBOUNCE_TIME) > millis()) {
    return;
  }
  lasttime[pin] = millis();

  int val = digitalRead(pin);     // read the input pin
  if (val == 0) {
    // On, since it is on a pullup resistor
    setPin(pin, true);

  } else {
    // Off
    setPin(pin, false);
  }
  
}

void setPin(int pin, bool state) {
  if (pin <= MS_LS_PIN) {
    setPins(&lsPins, pin, state); 
  } else {
    setPins(&msPins, pin - MS_LS_PIN, state); 
  }
}

void setPins(unsigned long *pins, int pin, bool state) {
  if (state) {
    *pins |= 1 << (pin);
  } else {
    *pins  &= ~(1UL << (pin));
  }
}

void reportStateChange() {
  unpackPins();
  Serial.write(PACKET_TYPE__KEY_STATE);
  Serial.write(longBuf, 8);
}

void unpackPins() {
  longBuf[0] = lsPins & 255;
  longBuf[1] = (lsPins >> 8)  & 255;
  longBuf[2] = (lsPins >> 16) & 255;
  longBuf[3] = (lsPins >> 24) & 255;
  longBuf[4] = msPins & 255;
  longBuf[5] = (msPins >> 8)  & 255;
  longBuf[6] = (msPins >> 16) & 255;
  longBuf[7] = (msPins >> 24) & 255;
}



#define DEBOUNCE_TIME 50
#define DIGITAL_INPUT_PINS  54
#define START_PIN  2
#define MS_LS_PIN  31           // Most significant least-significant-pin. 
#define BIT_RATE 115200
#define FRAME_START 0x66

#define PACKET_TYPE__KEY_STATE 1

bool lasttime[DIGITAL_INPUT_PINS];

// To detect state changes 
unsigned long lsPinsValue = 0;     
unsigned long msPinsValue = 0;
unsigned long lsPinsLastValue = 0;     
unsigned long msPinsLastValue = 0;

byte pins[8];

void setup()
{

  //pinMode(13, OUTPUT);

  for (int pin = START_PIN; pin <= DIGITAL_INPUT_PINS; pin++) {
    if (pin != 13) pinMode(pin, INPUT_PULLUP);       
    //pinMode(pin, INPUT_PULLUP);   
  }

  Serial.begin(BIT_RATE);
  while (!Serial);
}

void loop()
{

  // Scan
  for (int pin = START_PIN; pin < DIGITAL_INPUT_PINS; pin++) {
    checkbutton(pin);       
  }

  // Get state values
  lsPinsValue = pinValue(24, pins[3]);
  lsPinsValue += pinValue(16, pins[2]);
  lsPinsValue += pinValue(8, pins[1]); 
  lsPinsValue += pinValue(0, pins[0]);
  msPinsValue = pinValue(24, pins[7]);
  msPinsValue += pinValue(16, pins[6]);
  msPinsValue += pinValue(8, pins[5]); 
  msPinsValue += pinValue(0, pins[4]);
 
  // State change?  May want to bucket these
  if ((lsPinsValue != lsPinsLastValue)||(msPinsValue != msPinsLastValue)) {
    reportStateChange();
    msPinsLastValue = msPinsValue;
    lsPinsLastValue = lsPinsValue;
  }
  
}

unsigned long pinValue(int shift, unsigned long value)
{
  return value << shift; 
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
  unsigned block = pin / 8;
  if (state) {
    pins[block] |= 1 << (pin % 8);
  } else {
    pins[block]  &= ~(1UL << (pin % 8));
  }
}

void reportStateChange() {
  Serial.write(FRAME_START);
  delay(10); 
  Serial.write(PACKET_TYPE__KEY_STATE);
  delay(10);
  Serial.write(pins[0]);
  delay(10);
  Serial.write(pins[1]);
  delay(10);
  Serial.write(pins[2]);
  delay(10);
  Serial.write(pins[3]);
  delay(10);
  Serial.write(pins[4]);
  delay(10);
  Serial.write(pins[5]);
  delay(10);
  Serial.write(pins[6]);
  delay(10);
  Serial.write(pins[7]);
  delay(10);  
  Serial.flush();

}




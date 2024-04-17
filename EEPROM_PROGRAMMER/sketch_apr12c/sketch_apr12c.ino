#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4

#define EEPROM_D0 5
#define EEPROM_D7 12

#define WRITE_EN 13

#define DATA_MAX_BYTE_SIZE 512


void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

byte readByte(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, INPUT);
  }
  setAddress(address, /*outputEnable*/ true);

  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}

unsigned int readEEPROM() {
  byte start[2];
  byte end[2];

  unsigned int k = 0;

  // Reading 2 args of 2 bytes size from Serial
  while(Serial.available() && k < 4){
    byte b = Serial.read();
    if(k < 2){
      start[k % 2] = b;
    }
    else{
      end[k % 2] = b;
    }
    k++;
  }
  if (k < 4){
    return 1;
  }

  unsigned int startAddress = start[0] * 0x100 + start[1];
  unsigned int endAddress = end[0] * 0x100 + end[1];

  for (unsigned int address = startAddress; address < endAddress; address += 1) {
        Serial.write(readByte(address));
  }

  return 0;
}


/*
 * Write a byte to the EEPROM at the specified address.
 */
void writeByte(int address, byte data) {
  setAddress(address, /*outputEnable*/ false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, OUTPUT);
  }

  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, data & 1);
    data = data >> 1;
  }
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);
  delay(10);
}

void writeBytes(unsigned int address, byte data[256], unsigned int n) {
  for (unsigned int i = 0; i < n && address + i < 32768; i += 1) {
    writeByte(address + i, data[i]);
  }
}

unsigned int writeEEPROM(){
  byte start[2];
  unsigned int k = 0;
  
  while(Serial.available() && k < 2){
    byte b = Serial.read();
    start[k++] = b;
  }
  if (k < 2){
    return 1;
  }

  unsigned int startAddress = start[0] * 0x100 + start[1];

  byte data[DATA_MAX_BYTE_SIZE];
  unsigned int n = 0;
  while(Serial.available() && n < DATA_MAX_BYTE_SIZE){
    data[n++] = Serial.read();
  }
  writeBytes(startAddress, data, n);

  return 0;
}

void eraseEEPROM(){
  for (unsigned int address = 0; address < 32768; address += 1) {
    writeBytes(address, 0xff, 32768);

    if (address % 12048 == 0) {
      Serial.print(".");
    }
  }
  delay(10);
}

void sendBuffer(byte arr[DATA_MAX_BYTE_SIZE], unsigned int n){
  for (unsigned int i = 0; i < n; i += 1) {
    Serial.write(arr[i]);
  }
}

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);
  
  Serial.begin(115200);
}

bool dataRead = false;
byte opCode = 0x00;

void loop() {
  opCode = 0x00;
  if (!dataRead){
    while (Serial.available() == 0){ 
    }
    byte opCode = Serial.read();
    if (opCode == 0x01){
      unsigned int res = readEEPROM();
      delay(100);
    }
    else if (opCode == 0x02){
      unsigned int res = writeEEPROM();
      Serial.write(0x0b);
      delay(100);
    }
  }
}
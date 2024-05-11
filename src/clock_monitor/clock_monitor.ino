#include <CommandParser.h>

const char ADDR[] = {22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52};
const char DATA[] = {39, 41, 43, 45, 47, 49, 51, 53};

#define CLK_PIN LED_BUILTIN
#define READ_WRITE 3

typedef CommandParser<> MyCommandParser;

MyCommandParser parser;
bool monitorOn = false;
bool clockOn = false;
bool stepOn = false;
bool breakOn = false;
int breakAddr = 0;
int stepCount = 0;
int clockToggleMS = 250; // clock half period in milliseconds

byte incomingByte = 0; // for incoming serial data
char inputBuffer[40]; // input buffer
char lastCmd[40];
byte bufferLen = 0;
char resp[64]; // response buffer

void setClockFrequency(MyCommandParser::Argument *args, char *response) {
  clockToggleMS = 1.0 / args[0].asUInt64 * 500; // convert Hz to half period in ms
  
  char freq[10];
  dtostrf(500.0 / clockToggleMS, 0, 0, freq); // convert freq to string
  sprintf(resp, "new clock frequency: %s hz", freq); // format response
  strlcpy(response, resp, MyCommandParser::MAX_RESPONSE_SIZE);
}

void stepClock(MyCommandParser::Argument *args, char *response) {

  stepCount = args[0].asUInt64;
  stepOn = true;
  clockOn = true;
  
  sprintf(resp, "stepping clock for %d cycle(s)", stepCount); // format response
  strlcpy(response, resp, MyCommandParser::MAX_RESPONSE_SIZE);
}

void setBreakpoint(MyCommandParser::Argument *args, char *response) {

  sscanf(args[0].asString, "%x", &breakAddr);
  if (breakAddr == 0) {
    breakOn = false;
    sprintf(resp, "breakpoint cleared"); // format response
    strlcpy(response, resp, MyCommandParser::MAX_RESPONSE_SIZE);
  } else {
    breakOn = true;
    sprintf(resp, "breakpoint set to %04x", breakAddr); // format response
    strlcpy(response, resp, MyCommandParser::MAX_RESPONSE_SIZE);
  }
  
}

// source: Ben Eater's 6502 Arduino Monitor
// https://eater.net/6502
void readBus() {
  char output[15];

  unsigned int address = 0;
  for (int n = 0; n < 16; n += 1) {
    int bit = digitalRead(ADDR[n]) ? 1 : 0;
    Serial.print(bit);
    address = (address << 1) + bit;
  }
  
  Serial.print("   ");
  
  unsigned int data = 0;
  for (int n = 0; n < 8; n += 1) {
    int bit = digitalRead(DATA[n]) ? 1 : 0;
    Serial.print(bit);
    data = (data << 1) + bit;
  }

  sprintf(output, "   %04x  %c %02x", address, digitalRead(READ_WRITE) ? 'r' : 'W', data);
  Serial.println(output);  

  if (breakOn && address == breakAddr) {
    clockOn = false;
    Serial.println("breakpoint reached");
  }
}

void runClock() {
  static unsigned long clkMillis = millis();

  if (clockOn) {
    if (millis() - clkMillis > clockToggleMS) {
      int curClk = digitalRead(CLK_PIN);

      // read bus if monitor is on
      if (curClk == LOW && monitorOn) readBus();

      // manual step processing
      if (curClk == HIGH && stepOn) {
        stepCount -= 1;
        if (stepCount <= 0) {
          stepOn = false;
          clockOn = false;
        }
      }

      // toggle clock
      digitalWrite(CLK_PIN, !curClk);
      clkMillis = millis();
    }
  } else {
    digitalWrite(CLK_PIN, LOW);
  }
}

void toggleClock(MyCommandParser::Argument *args, char *response) {
  clockOn = !clockOn;
  sprintf(resp, "clock state: %s", clockOn ? "on" : "off");
  strlcpy(response, resp, MyCommandParser::MAX_RESPONSE_SIZE);
}

void toggleMonitor(MyCommandParser::Argument *args, char *response) {
  monitorOn = !monitorOn;
  sprintf(resp, "monitor state: %s", monitorOn ? "on" : "off");
  strlcpy(response, resp, MyCommandParser::MAX_RESPONSE_SIZE);
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println(F("    h          : help - prints the command set"));
  Serial.println(F("    f <hz>     : sets the clock frequency to <hz>"));
  Serial.println(F("    c          : toggles the clock on/off"));
  Serial.println(F("    s <cycles> : step the clock for <cycles>"));
  Serial.println(F("    m          : toggles the 6502 monitor on/off"));
  Serial.println(F("    b <addrHEX>: sets breakpoint at <addrHex> (in hex format, e.g. FF10). 0000 clears breakpoint"));
}

void setup() {

  // set up pins
  pinMode(CLK_PIN, OUTPUT);
  for (int n = 0; n < 16; n += 1) {
    pinMode(ADDR[n], INPUT);
  }
  for (int n = 0; n < 8; n += 1) {
    pinMode(DATA[n], INPUT);
  }
  pinMode(READ_WRITE, INPUT);

  Serial.begin(115200); // open serial port, set data rate to 115,200 bps
  while (!Serial); // allow serial port to complete initialization

  // register commands
  // arguments types: s (string), u (unsigned int), i (int), (d) double
  // for more details: https://github.com/Uberi/Arduino-CommandParser/tree/master
  parser.registerCommand("h", "", &printHelp);
  parser.registerCommand("f", "u", &setClockFrequency);
  parser.registerCommand("c", "", &toggleClock);
  parser.registerCommand("s", "u", &stepClock);
  parser.registerCommand("m", "", &toggleMonitor);
  parser.registerCommand("b", "s", &setBreakpoint);

  printHelp();

}

void loop() {
// send data only when you receive data:
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    Serial.write(incomingByte);

    if (incomingByte == 13 || incomingByte == 10) {
      
      if (bufferLen == 0) {
        // if buffer is empty, repeat last command
        strcpy(inputBuffer, lastCmd);
      } else {
        inputBuffer[bufferLen] = '\0';
        // save last command
        strcpy(lastCmd, inputBuffer);
      }

      char response[MyCommandParser::MAX_RESPONSE_SIZE];
      parser.processCommand(inputBuffer, response);
      if (!(response == "")) Serial.println(response);
      bufferLen = 0;
    } else
    {
      inputBuffer[bufferLen++] = incomingByte;
    }
  }

  runClock();
}

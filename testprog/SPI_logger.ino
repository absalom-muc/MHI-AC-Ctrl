// SPI logger
// logs the timing of the SCK and MOSI SPI signal edges

// Please use the serial terminal to create a log file
// baud rate should be 115200, timestamp disabled!
// run the program with 160MHz (tools -> CPU frequency -> 160MHz)
// Please copy the log content to a file and use the file name format "AC-model.txt" (e.g. SRK35ZS-S.txt)
// and upload it
// Thank you!

// Background:
// You can easily read the log file by Excel and create a graphical view (diagram) similar to a logic analyzer/oscilloscope
// This helps to understand the SPI protocol

#define version "v1.0"
#define SCK_PIN  14
#define MOSI_PIN 13
#define edge_cnt_MAX 2000

volatile unsigned long sck_time[edge_cnt_MAX];
volatile byte sck_value[edge_cnt_MAX];
volatile unsigned long mosi_time[edge_cnt_MAX];
volatile byte mosi_value[edge_cnt_MAX];
volatile uint sck_edge_cnt = 0;;
volatile uint mosi_edge_cnt = 0;;

void setup()  {  // measure the frequency on the pins
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  pinMode(SCK_PIN, INPUT);
  pinMode(MOSI_PIN, INPUT);
  Serial.println("SPI logger " version);
  Serial.print("CPU frequency [Hz] : ");
  Serial.println(F_CPU);
}

ICACHE_RAM_ATTR void handleInterrupt_SCK() {
  sck_time[sck_edge_cnt] = micros();
  sck_value[sck_edge_cnt] = digitalRead(SCK_PIN);
  sck_edge_cnt++;
}

ICACHE_RAM_ATTR void handleInterrupt_MOSI() {
  mosi_time[mosi_edge_cnt] = micros();
  mosi_value[mosi_edge_cnt] = digitalRead(MOSI_PIN);
  mosi_edge_cnt++;
}

void loop() {
  Serial.println();
  Serial.print("acquisition - please wait, ");
  for (int edge_cnt = 0; edge_cnt < edge_cnt_MAX; edge_cnt++) {
    sck_time[edge_cnt] = 0;
    mosi_time[edge_cnt] = 0;
  }

  attachInterrupt(digitalPinToInterrupt(SCK_PIN), handleInterrupt_SCK, CHANGE);
  attachInterrupt(digitalPinToInterrupt(MOSI_PIN), handleInterrupt_MOSI, CHANGE);
  while (sck_edge_cnt < edge_cnt_MAX);
  detachInterrupt(SCK_PIN);
  detachInterrupt(MOSI_PIN);

  Serial.println("ready, output data:");
  Serial.println("time;SCK;MOSI");
  for (int edge_cnt = 1; edge_cnt < edge_cnt_MAX; edge_cnt++) {
    Serial.printf("%lu;%i;\n", sck_time[edge_cnt] - sck_time[0], sck_value[edge_cnt] ^ 1);
    Serial.printf("%lu;%i;\n", sck_time[edge_cnt] - sck_time[0], sck_value[edge_cnt]);
    delay(0);
  }
  Serial.println();
  for (int edge_cnt = 1; edge_cnt < edge_cnt_MAX; edge_cnt++) {
    if ((mosi_time[edge_cnt] == 0) | (mosi_time[edge_cnt] > sck_time[edge_cnt_MAX - 1]))
      break;
    Serial.printf("%lu; ;%i\n", mosi_time[edge_cnt] - sck_time[0], 2 + mosi_value[edge_cnt] ^ 1);
    Serial.printf("%lu; ;%i\n", mosi_time[edge_cnt] - sck_time[0], 2 + mosi_value[edge_cnt]);
    delay(0);
  }
  Serial.println();
  Serial.println("finished");
  while (1)
    delay(0);
}

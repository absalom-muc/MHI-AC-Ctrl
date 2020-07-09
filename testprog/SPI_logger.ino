// SPI logger
// logs the timing of the SCK, MOSI and MISO signal edges
// and creates a .vcd (Value change dump) format.
// This vcd format is supported by many waveform viewers, e.g. PulseView

// Please use the serial terminal to generate a log file
// baud rate should be 115200, timestamp disabled!
// Please copy the log content to a file and use the file name format "AC-model.txt" (e.g. SRK35ZS-S.txt)
// and upload it as an attachement to your issue.

#define version "v2.1"
#define SCK_PIN  14
#define MOSI_PIN 13
#define MISO_PIN 12

const char SCK_CHAR =  '!';
const char MOSI_CHAR =  '#';
const char MISO_CHAR =  '$';

const int N_SAMPLES = 1400;
const uint32_t STARTTIME = 1000;

static constexpr uint32_t MASK = (1 << SCK_PIN) | (1 << MOSI_PIN) | (1 << MISO_PIN);

void setup()  {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  pinMode(SCK_PIN, INPUT);
  pinMode(MOSI_PIN, INPUT);
  pinMode(MISO_PIN, INPUT);
  Serial.println("SPI logger " version);
  Serial.print("CPU frequency [Hz] : ");
  Serial.println(F_CPU);
  Serial.print(N_SAMPLES);
  Serial.println(" samples");
  Serial.println();
}

unsigned long times[N_SAMPLES]; // when did change happen
uint32_t values[N_SAMPLES];     // GPI value at time

extern void ICACHE_RAM_ATTR collect() {
  // based on https://github.com/aster94/logic-analyzer
  times[0] = micros();
  values[0] = GPI & MASK;
  for (int i = 1; i < N_SAMPLES; ++i) {
    uint32_t value;
    do {
      value = GPI & MASK;
    } while (value == values[i - 1]);
    times[i] = micros();
    values[i] = value;
  }
}

void loop() {
  do {
    collect();
    delay(0);
  } while ((uint32_t)(times[1] - times[0]) < 5000);  // wait for 5ms stable signal

  uint32_t time_offset = times[1] - STARTTIME;
  Serial.println("$timescale 1 us $end");
  Serial.printf("$var wire 1 %c sck $end\n", SCK_CHAR);
  Serial.printf("$var wire 1 %c mosi $end\n", MOSI_CHAR);
  Serial.printf("$var wire 1 %c miso $end\n", MISO_CHAR);
  Serial.println("$enddefinitions $end");
  Serial.printf("#0 %i%c %i%c %i%c\n", ((values[0] & (1 << SCK_PIN)) > 0), SCK_CHAR, ((values[0] & (1 << MOSI_PIN)) > 0), MOSI_CHAR, ((values[0] & (1 << MISO_PIN)) > 0), MISO_CHAR);
  for (int i = 1; i < N_SAMPLES; ++i) {
    char val_and_char[10];
    val_and_char[0] = 0;
    int strcnt = 0;
    if ((values[i] & (1 << SCK_PIN)) != (values[i - 1] & (1 << SCK_PIN))) {
      val_and_char[strcnt++] = ' ';
      val_and_char[strcnt++] = 48 + ((values[i] & (1 << SCK_PIN)) > 0);
      val_and_char[strcnt++] = SCK_CHAR;
    }
    if ((values[i] & (1 << MOSI_PIN)) != (values[i - 1] & (1 << MOSI_PIN))) {
      val_and_char[strcnt++] = ' ';
      val_and_char[strcnt++] = 48 + ((values[i] & (1 << MOSI_PIN)) > 0);
      val_and_char[strcnt++] = MOSI_CHAR;
    }
    if ((values[i] & (1 << MISO_PIN)) != (values[i - 1] & (1 << MISO_PIN))) {
      val_and_char[strcnt++] = ' ';
      val_and_char[strcnt++] = 48 + ((values[i] & (1 << MISO_PIN)) > 0);
      val_and_char[strcnt++] = MISO_CHAR;
    }
    val_and_char[strcnt] = 0;
    Serial.printf("#%u%s\n", ((uint32_t)(times[i] - time_offset)), val_and_char);
  }

  Serial.println();
  Serial.println("finished");

  while (1)
    delay(0);
}
// SPI logger
// logs the timing of the SCK, MOSI and MISO signal edges
// and creates a .vcd (Value change dump) format. 
// This vcd format is supported by many waveform viewers, e.g. PulseView

// Please use the serial terminal to generate a log file
// baud rate should be 115200, timestamp disabled!
// Please copy the log content to a file and use the file name format "AC-model.txt" (e.g. SRK35ZS-S.txt)
// and upload it as an attachement to your item.

#define version "v2.0"
#define SCK_PIN  14
#define MOSI_PIN 13
#define MISO_PIN 12

const char SCK_CHAR =  '!';
const char MOSI_CHAR =  '#';
const char MISO_CHAR =  '$';

static const int N_SAMPLES = 1400;

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
  Serial.println("$timescale 1 us $end");
  Serial.printf("$var wire 1 %c sck $end\n", SCK_CHAR);
  Serial.printf("$var wire 1 %c mosi $end\n", MOSI_CHAR);
  Serial.printf("$var wire 1 %c miso $end\n", MISO_CHAR);
  Serial.println("$enddefinitions $end");
  collect();
  char sig_char;
  byte sig_val;
  for (int i = 1; i < N_SAMPLES; ++i) {
    if ((values[i] & (1 << SCK_PIN)) != (values[i - 1] & (1 << SCK_PIN))) {
      sig_char = SCK_CHAR;
      sig_val = ((values[i] & (1 << SCK_PIN)) > 0);
    }
    else if ((values[i] & (1 << MOSI_PIN)) != (values[i - 1] & (1 << MOSI_PIN))) {
      sig_char = MOSI_CHAR;
      sig_val = ((values[i] & (1 << MOSI_PIN)) > 0);
    }
    else if ((values[i] & (1 << MISO_PIN)) != (values[i - 1] & (1 << MISO_PIN))) {
      sig_char = MISO_CHAR;
      sig_val = ((values[i] & (1 << MISO_PIN)) > 0);
    }
    else {
      Serial.printf("ERROR\n");
      break;
    }
    Serial.printf("#%u %i%c\n", ((uint32_t)(times[i] - times[0])), sig_val, sig_char);
  }

  Serial.println();
  Serial.println("finished");

  //Serial.printf("MISO cnt=%i\n", tmp);
  while (1)
    delay(0);
}

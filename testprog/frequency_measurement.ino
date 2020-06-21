// frequency_measurement v1.0
// check of the SCK timing

// Please use the serial terminal to create a log file
// baud rate should be 115200, timestamp enabled
// Program should run aprox. 1 minute 
// Please copy the log content to a file and use the file name format "AC-model.txt" (e.g. SRK35ZS-S.txt)
// and upload it
// Thank you!

#define version "v1.0"
#define SCK 14
#define cnt_MAX 8000
volatile unsigned long falledge_time[cnt_MAX];
volatile uint edge_cnt;

void setup()  {  // measure the frequency on the pins
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  pinMode(SCK, INPUT);
  Serial.println("Frequency measurement " version);
  Serial.print("CPU frequency [Hz] : ");
  Serial.println(F_CPU);
}

ICACHE_RAM_ATTR void handleInterrupt_SCK() {
  falledge_time[edge_cnt] = micros();
  edge_cnt++;
}

void loop() {
  static unsigned long delta_cycle_time_max[] = {0, 0, 0};
  static uint acq_fails = 0;
  unsigned long dt = 0;  // time from start to first falling edge
  unsigned long start_micros;

  Serial.println("");
  Serial.print("acquisition-please wait, ");
  edge_cnt = 0;
  start_micros = micros();
  attachInterrupt(digitalPinToInterrupt(SCK), handleInterrupt_SCK, FALLING);
  while (((uint)(micros() - start_micros) < 1000000));
  detachInterrupt(SCK);

  //Serial.println("ready");
  //Serial.printf("start_micros=%luµs\n", start_micros);
  //Serial.printf("start time=%luµs end time=%luµs\n", falledge_time[0], falledge_time[edge_cnt - 1]);
  Serial.printf("ready: acq. time=%lums f=%i time to 1st falling edge=%iµs\n", (falledge_time[edge_cnt - 1] - falledge_time[0]) / 1000, edge_cnt, falledge_time[0] - start_micros);

  unsigned long cycle_time[] = {0, 0, 0};
  unsigned long cycle_time_min[] = {999999, 999999, 999999};
  unsigned long cycle_time_max[] = {0, 0, 0};
  unsigned long cycle_time_cnt[] = {0, 0, 0};
  unsigned long diff;
  uint cy;

  for (int cnt = 1; cnt < edge_cnt; cnt++) {
    diff = falledge_time[cnt] - falledge_time[cnt - 1];
    for (cy = 0; cy < 3; cy++) {
      if ((diff >= cycle_time[cy] * 0.8f) & (diff <= cycle_time[cy] * 1.2f)) { // known range?
        if (diff < cycle_time_min[cy])
          cycle_time_min[cy] = diff;
        if (diff > cycle_time_max[cy])
          cycle_time_max[cy] = diff;
        cycle_time_cnt[cy]++;
        break;
      }
    }
    if (cy >= 3) { // if not in range of used cycle_time
      for (cy = 0; cy < 3; cy++) { // find next unused cycle_time
        if (cycle_time[cy] == 0) {
          cycle_time[cy] = diff;
          cycle_time_min[cy] = diff;
          cycle_time_max[cy] = diff;
          cycle_time_cnt[cy] = 1;
          break;
        }
      }
      if (cy >= 3)
        break;
    }
  }

  if (cy >= 3){
    acq_fails++;
    Serial.printf("more than 3 ranges detected, skip measurement, sum acq_fails=%i\n", acq_fails);
  }
  else {
    for (cy = 0; cy < 3; cy++) {
      Serial.printf("cycle_time_min[%i]=%luµs, delta_time[%i]=%luµs, cycle_time_cnt=%lu\n", cy, cycle_time_min[cy], cy, cycle_time_max[cy] - cycle_time_min[cy], cycle_time_cnt[cy]);
      if ((cycle_time_max[cy] - cycle_time_min[cy]) > delta_cycle_time_max[cy])
        delta_cycle_time_max[cy] = cycle_time_max[cy] - cycle_time_min[cy];
    }
    Serial.printf("delta_cycle_time_max %i %i %i\n", delta_cycle_time_max[0], delta_cycle_time_max[1], delta_cycle_time_max[2]);
  }
}

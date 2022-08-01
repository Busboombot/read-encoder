#include "const.h"
#include <Encoder.h>
#include <PacketSerial.h>

#include "messages.h"
#include "limit_encoder.h"

IntervalTimer clearTimer; // Timer for clearing the LIMIT_INTR pin
IntervalTimer clearLedTimer; // Timer for clearing the LED
PacketSerial ps;
extern EncoderReport encoder_report;
extern volatile int limits_changed;


void printReport(){
  Serial1.printf("    ER idx=%d code=%d ", limits_changed, encoder_report.axis_index, encoder_report.code);
  for (int i = 0 ; i < N_AXES; i++){
    Serial1.printf(" (%d,%d) ", encoder_report.limit_states[i], encoder_report.positions[i]);

  }
  Serial1.println(' ');
  
}
void segmentCompleted(){
  updateReportSegCompleted();
  ps.send((const uint8_t*)&encoder_report, sizeof(encoder_report));
  //Serial1.printf("Seg complete idx=%d code=%d ", limits_changed, encoder_report.axis_index, encoder_report.code);
  //printReport();
}

void sendReportOnChange(){
  if ( limits_changed > 0){
    ps.send((const uint8_t*)&encoder_report, sizeof(encoder_report));
    //Serial1.printf("%d limits_changed idx=%d code=%d ", limits_changed, encoder_report.axis_index, encoder_report.code);
    //printReport();
    limits_changed =0;
  }

}

void loopTick()    {
    static unsigned long last = millis();
    static bool ledToggle = true;
    
    if( millis() - last > 1000  ){
      digitalWrite(LED_BUILTIN, (ledToggle = !ledToggle));
      last = millis();
    }
}

void calcDirection()    {
    static unsigned long last = millis();
    
    if( millis() - last > 100  ){
      for ( int i = 0; i < N_AXES; i++) {
        getEncoder(i).calcDirection();
      } 
      
      last = millis();
    }
}

void updateSerial(){
  if (Serial.available() > 0) {
    // read the incoming byte:
    char cmd = Serial.read();
    
    switch (cmd) {
      case 'z': // Zero
        Serial1.println("Zeroing");
        for ( int i = 0; i < N_AXES; i++) {

          getEncoder(i).setPosition(0);
          getEncoder(i).limitUnChanged();
        }
        updateReport(MC_ZERO,0);
        break;
      case 'p': // Poll the encoders
        Serial1.println("Poll requested");
        for ( int i = 0; i < N_AXES; i++) {
          getEncoder(i).limitUnChanged();
        }
        updateReport(MC_POLL,0);
        break;
      case 's': // Set Values
        Serial1.println("Set Values");
        for (int i = 0 ; i <  N_AXES ; i++){
          int v = Serial.parseInt();
          getEncoder(i).setPosition(v);
          getEncoder(i).limitUnChanged();
          Serial1.printf("Set %d to %d",i,v );
        }
        break;
    }
  }
}

void setup() {

  ps.begin(SERIAL_BAUD);

  Serial1.begin(DEBUG_BAUD);
  Serial1.println("start");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LIMIT_INTR_PIN, OUTPUT);
  pinMode(SEG_TRIGGER_PIN, INPUT);
  
  attachInterrupt(SEG_TRIGGER_PIN, segmentCompleted, RISING);

  attachLimitInterrupts();

  setupEncoderTest(2);

}

void loop() {

  loopTick();

  updateSerial();

  calcDirection();

  sendReportOnChange();

}

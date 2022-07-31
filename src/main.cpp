#include <Encoder.h>
#include <PacketSerial.h>

#include "const.h"
#include "messages.h"
#include "limit_encoder.h"

IntervalTimer clearTimer; // Timer for clearing the LIMIT_INTR pin
IntervalTimer clearLedTimer; // Timer for clearing the LED
PacketSerial ps;

volatile int limits_changed = 0; // Number of limits that have changed since last limit interrupt


void limit_change(int n);

LimitEncoder encoders[] = {
  //  A B I E
  LimitEncoder(0, 30, 32, 31, 29), // Bottom Left
  LimitEncoder(1, 6, 7, 8, 9),
  LimitEncoder(2, 10, 11, 12, 24),
  LimitEncoder(3, 13, 14, 15, 16),
  LimitEncoder(4, 17, 18, 19, 20),
  LimitEncoder(5, 25, 26, 27, 28)
};

int n_encoders = sizeof encoders / sizeof encoders[0];


void updateReport(enum message_code code, uint8_t axis_idx){
  for ( int i = 0; i < n_encoders; i++) {
    LimitEncoder& enc = encoders[i];
    encoder_report.code = code;
    encoder_report.axis_index = axis_idx;
    encoder_report.limit_states[i] = enc.getLimitState() | (enc.getDirection() <<2 ) ;
    encoder_report.positions[i] = enc.getPosition();
  }

  ps.send((const uint8_t*)&encoder_report, sizeof(encoder_report));
}

void limit_change(int n) {
  limits_changed += 1;
  
  for ( int i = 0; i < n_encoders; i++) {
    if (i == n) {
      encoders[i].limitChanged();
    } else {
      encoders[i].limitUnChanged();
    }
  }

  updateReport(MC_LIMIT, n+1); // +1 b/c 0 means no limit, so actual limits are 1 to 6
  
}

void limit_change_1() {
  limit_change(0);
}
void limit_change_2() {
  limit_change(1);
}
void limit_change_3() {
  limit_change(2);
}
void limit_change_4() {
  limit_change(3);
}
void limit_change_5() {
  limit_change(4);
}
void limit_change_6() {
  limit_change(5);
}


voidfp limit_changes[] = {
  limit_change_1,
  limit_change_2,
  limit_change_3,
  limit_change_4,
  limit_change_5,
  limit_change_6
};

int n_limit_changes = sizeof limit_changes / sizeof limit_changes[0];


void segmentCompleted(){
  for ( int i = 0; i < n_encoders; i++) {
    encoders[i].limitUnChanged();
  }
  updateReport(MC_SEGDONE,0);

}

void setup() {

  ps.begin(115200);


  pinMode(LIMIT_INTR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SEG_TRIGGER_PIN, INPUT);
  
  attachInterrupt(SEG_TRIGGER_PIN, segmentCompleted, RISING);

  // Attach the interrupts for limit changes
  for ( int i = 0; i < n_encoders; i++) {
    LimitEncoder& enc = encoders[i];
    enc.attach(limit_changes[enc.getAxisNumber()]);
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

void loop() {

  loopTick();

  if (Serial.available() > 0) {
    // read the incoming byte:
    char cmd = Serial.read();
    
    switch (cmd) {
      case 'z': // Zero
        for ( int i = 0; i < n_encoders; i++) {
          encoders[i].setPosition(0);
          encoders[i].limitUnChanged();
        }
        updateReport(MC_ZERO,0);
        break;
      case 'p': // Poll the encoders
        for ( int i = 0; i < n_encoders; i++) {
          encoders[i].limitUnChanged();
        }
        updateReport(MC_POLL,0);
        break;
      case 's': // Set Values
        for (int i = 0 ; i <  n_encoders ; i++){
          encoders[i].setPosition(Serial.parseInt());
          encoders[i].limitUnChanged();
        }
        break;
    }
  }

  for ( int i = 0; i < n_encoders; i++) {
    encoders[i].calcDirection();
  }




}

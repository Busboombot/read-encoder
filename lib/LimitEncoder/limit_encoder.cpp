#include "messages.h"
#include "limit_encoder.h"

volatile int limits_changed = 0; // Number of limits that have changed since last limit interrupt

EncoderReport encoder_report;


LimitEncoder encoders[N_AXES] = {
  //  A B I E
  LimitEncoder(0, 32, 30, 31, 29), // Bottom Left. A&B may be reversed to change direction
  LimitEncoder(1, 33, 36, 34, 36),
  LimitEncoder(2, 10, 11, 12, 24),
  LimitEncoder(3, 13, 14, 15, 16),
  LimitEncoder(4, 17, 18, 19, 20),
  LimitEncoder(5, 25, 26, 27, 28)
};

LimitEncoder& getEncoder(int i){
    return encoders[i];
}

void limit_change(int n) {
  limits_changed += 1;
  
  for ( int i = 0; i < N_AXES; i++) {
    if (i == n) {
      encoders[i].limitChanged();
    } else {
      encoders[i].limitUnChanged();
    }
  }

  // It looks like we may be running through this rundundantly if many limits changed all at once
  // but that's probably very rare. 
  updateReport(MC_LIMIT, n+1); // +1 b/c 0 means no limit, so actual limits are 1 to 6
  
}

void limit_change_1() {limit_change(0);}
void limit_change_2() {limit_change(1);}
void limit_change_3() {limit_change(2);}
void limit_change_4() {limit_change(3);}
void limit_change_5() {limit_change(4);}
void limit_change_6() {limit_change(5);}

void index_change(int n){

}
void index_change_1() {index_change(0);}
void index_change_2() {index_change(1);}
void index_change_3() {index_change(2);}
void index_change_4() {index_change(3);}
void index_change_5() {index_change(4);}
void index_change_6() {index_change(5);}

void attachLimitInterrupts(){
    // Attach the interrupts for limit changes

  voidfp limit_changes[N_AXES] = {
    limit_change_1,limit_change_2,limit_change_3,
    limit_change_4,limit_change_5,limit_change_6
  };

  voidfp index_changes[N_AXES] = {
    index_change_1,index_change_2,index_change_3,
    index_change_4,index_change_5,index_change_6
  };

  for ( int i = 0; i < N_AXES; i++) {
    LimitEncoder& enc = encoders[i];
    enc.attach(limit_changes[enc.getAxisNumber()], index_changes[enc.getAxisNumber()]);
  }
}

void updateReport(enum message_code code, uint8_t axis_idx){
  for ( int i = 0; i < N_AXES; i++) {
    LimitEncoder& enc = encoders[i];
    encoder_report.code = code;
    encoder_report.axis_index = axis_idx;
    encoder_report.limit_states[i] = enc.getLimitState() | (enc.getDirection() <<2 ) ;
    encoder_report.positions[i] = enc.getPosition();
  }
}

/**
 * @brief Update the report to indicate that no limits were changes, but a segment 
 * has been signaled from the step controller as completed. 
 * 
 */
void updateReportSegCompleted(){
  for ( int i = 0; i < N_AXES; i++) {
    encoders[i].limitUnChanged();
  }
  updateReport(MC_SEGDONE,0);

}



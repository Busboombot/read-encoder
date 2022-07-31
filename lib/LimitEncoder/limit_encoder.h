
#include "const.h"
#include "messages.h"
#include "Encoder.h"

typedef void (*voidfp)();


void updateReport(enum message_code code, uint8_t axis_idx);
void updateReportSegCompleted();
void attachLimitInterrupts();

class LimitEncoder {

  private:
    int axis_n; // Axis Number
    int pin_a; // Encoder A
    int pin_b; // Encoder B
    int pin_i; // Encoder Index
    int pin_l; // Limit pin

    long last_limit_pos; // Position at last limit changed
    long pen_limit_pos; // Position at penultimate limit change
    int last_limit; // Most recent limit state after a change
    long last_pos;

    uint8_t limit_state;

    int dir;
    
    Encoder encoder;

  public:

    LimitEncoder(int axis_n, int pin_a, int pin_b, int pin_i, int pin_l):
      axis_n(axis_n), pin_a(pin_a), pin_b(pin_b), pin_i(pin_i), pin_l(pin_l), encoder(pin_a, pin_b) {
      pinMode(pin_l, INPUT_PULLUP);
      pinMode(pin_i, INPUT_PULLUP);
    }

    // Attach the interrupt handler to the limit pin
    void attach(voidfp limit_f, voidfp index_f) {
      attachInterrupt(digitalPinToInterrupt(pin_l), limit_f, CHANGE);
      attachInterrupt(digitalPinToInterrupt(pin_i), index_f, CHANGE);
    }

    int getAxisNumber() {
      return axis_n;
    }

    long getPosition() {
      last_pos = encoder.read();
      return last_pos;
    }

    long getLastPosition() {
      return last_pos;
    }

    int calcDirection(){
      long l = getLastPosition();
      long d = getPosition()-l; // distance changed
      d = (int)(d/abs(d)); // -1 or 1
      if(d != 0){
        dir = (d+1)/2; // 0 or 1
      }
      return dir;
    }

    int getDirection(){
      return dir;
    }
    
    void setPosition(long p) {
      encoder.write(p);
    }

    int getLimit() {
      return digitalRead(pin_l);
    }

    int getLastLimit() {
      return last_limit;
    }

    void limitChanged() {
      pen_limit_pos = last_limit_pos;
      last_limit_pos = getPosition();
 
      last_limit = getLimit();

      if(last_limit){
        if(dir){
          limit_state = LH;
        } else {
          limit_state = HL;
        }
        
      } else {
        if(dir){
          limit_state = HL;
        } else {
          limit_state = LH;
        }
        
      }
    }

    void limitUnChanged(){

      if(getLimit()){
        limit_state = HH;
      } else {
        limit_state = LL;
      }
      
    }

    int getLimitState(){
      return limit_state;
    }
 
    long getLimitDiff() {
      return last_limit_pos - pen_limit_pos;
    }

    long getLastLimitPos() {
      return last_limit;
    }
};

LimitEncoder& getEncoder(int i);


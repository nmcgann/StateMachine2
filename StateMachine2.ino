#include "StateMachine2.h"
//table driven state machine with event queue test

#define NUM_ELEMENTS(x) (sizeof(x)/sizeof(x[0]))

const int LED = LED_BUILTIN;

//forward decs
void Led_Init(uint8_t data);
void Led_On(uint8_t data);
void Led_Off(uint8_t data);
void Led_Idle(uint8_t data);

//state table class
enum class state_t : int {
    ST_START = 0, //must be 0 and first
    ST_IDLE,
    ST_LED_ON,
    ST_LED_OFF,
};

//event table class
enum class event_t : int {
    EV_ANY = 0, //must be 0 and first
    EV_NONE,
    EV_BUTTON_PUSHED,
    EV_TIME_OUT,
};


typedef struct {
    state_t currState;      //current state
    event_t event;          //event to match to cause a state transition
    state_t nextState;      //next state
    void (*func)(uint8_t);  //pointer to the state transition function - void fn(void)
    uint8_t data;           //passed to transition functions (can be any type)
} stateTransMatrixRow_t;

//main state transition table
const stateTransMatrixRow_t stateTransMatrix[] PROGMEM = {
    // CURR STATE           // EVENT                       // NEXT STATE          //TRANSITION FN   //DATA
    { state_t::ST_START,    event_t::EV_ANY,               state_t::ST_IDLE,      &Led_Idle,        0     },
    { state_t::ST_IDLE,     event_t::EV_BUTTON_PUSHED,     state_t::ST_LED_ON,    &Led_On,          0     },
    { state_t::ST_LED_ON,   event_t::EV_TIME_OUT,          state_t::ST_LED_OFF,   &Led_Off,         0     },
    { state_t::ST_LED_ON,   event_t::EV_BUTTON_PUSHED,     state_t::ST_IDLE,      &Led_Idle,        0     },
    { state_t::ST_LED_OFF,  event_t::EV_TIME_OUT,          state_t::ST_LED_ON,    &Led_On,          0     },
    { state_t::ST_LED_OFF,  event_t::EV_BUTTON_PUSHED,     state_t::ST_IDLE,      &Led_Idle,        0     },

};

// State transition functions

void Led_Init(uint8_t data) {
    Serial.println(F("Led_Init() called."));
}

void Led_On(uint8_t data) {
    Serial.println(F("Led_On() called - LED turned on."));
}

void Led_Off(uint8_t data) {
    Serial.println(F("Led_Off() called - LED turned off."));
}

void Led_Idle(uint8_t data) {
    Serial.println(F("Led_Idle() called - LED in idle state."));
}

//event-driven state machine

StateMachine<state_t, event_t, stateTransMatrixRow_t, NUM_ELEMENTS(stateTransMatrix)> sm2(stateTransMatrix);

//event queue
EventQueue<event_t, 16> evQueue;

//#################################################################################################


void setup() {
  Serial.begin(115200);
   Serial.println(F("Test SM"));
   Serial.println(F("======="));
   
   Serial.print(F("Transition table size = "));Serial.print(sizeof(stateTransMatrix));Serial.println(F(" bytes"));
 
  //get ready
  sm2.init();
  //do the transition into the idle state
  sm2.runIteration(event_t::EV_ANY);

}

void loop() {
  // put your main code here, to run repeatedly:
  
  //simulate the events
  if(Serial.available()){
    char c = Serial.read();

    switch(c){
      case 't': //timeout
        //sm2.runIteration(event_t::EV_TIME_OUT);
        evQueue.addToQueue(event_t::EV_TIME_OUT);
        break;
      case 'b': //button
        //sm2.runIteration(event_t::EV_BUTTON_PUSHED);
        evQueue.addToQueue(event_t::EV_BUTTON_PUSHED);
        break;
      default:
        break;

    }
  }

  if(!evQueue.isQueueEmpty()){
    //Serial.println("Q not empty");
    event_t ev = evQueue.removeFromQueue();
    //Serial.print("ev = ");Serial.println((int) ev);
    //evQueue.isQueueEmpty() ? Serial.println("empty") : Serial.println("not empty");
    sm2.runIteration(ev);
    //debug - show state number
    Serial.print(F("state = "));Serial.println((int) sm2.getState());
  }

}

#include "StateMachine2.h"
//table driven state machine with event queue. Test program

#define NUM_ELEMENTS(x) (sizeof(x)/sizeof(x[0]))

const uint8_t LED = LED_BUILTIN;

//state table class
enum class state_t : uint8_t {
    ST_START = 0, //must be 0 and first
    ST_IDLE,
    ST_LED_ON,
    ST_LED_OFF,
};

//event table class
enum class event_t : uint8_t {
    EV_ANY = 0, //must be 0 and first
    EV_NONE,
    EV_BUTTON_PUSHED,
    EV_TIME_OUT,
};


typedef struct {
    state_t currState;      //current state
    event_t event;          //event to match to cause a state transition
    state_t nextState;      //next state
    void (*func)(event_t);  //pointer to the state transition function - void fn(event_t)
} stateTransMatrixRow_t;

//forward decs
void Led_Init(event_t);
void Led_On(event_t);
void Led_Off(event_t);
void Led_Idle(event_t);


//main state transition table
const stateTransMatrixRow_t stateTransMatrix[] PROGMEM = {
    // CURR STATE           // EVENT                       // NEXT STATE          //TRANSITION FN
    { state_t::ST_START,    event_t::EV_ANY,               state_t::ST_IDLE,      &Led_Idle,        },
    { state_t::ST_IDLE,     event_t::EV_BUTTON_PUSHED,     state_t::ST_LED_ON,    &Led_On,          },
    { state_t::ST_LED_ON,   event_t::EV_TIME_OUT,          state_t::ST_LED_OFF,   &Led_Off,         },
    { state_t::ST_LED_ON,   event_t::EV_BUTTON_PUSHED,     state_t::ST_IDLE,      &Led_Idle,        },
    { state_t::ST_LED_OFF,  event_t::EV_TIME_OUT,          state_t::ST_LED_ON,    &Led_On,          },
    { state_t::ST_LED_OFF,  event_t::EV_BUTTON_PUSHED,     state_t::ST_IDLE,      &Led_Idle,        },

};

// State transition functions

void Led_Init(event_t e) {
    Serial.println(F("Led_Init() called."));
}

void Led_On(event_t e) {
    Serial.println(F("Led_On() called - LED turned on."));
    digitalWrite(LED, HIGH);
}

void Led_Off(event_t e) {
    Serial.println(F("Led_Off() called - LED turned off."));
    digitalWrite(LED, LOW);
}

void Led_Idle(event_t e) {
    Serial.println(F("Led_Idle() called - LED in idle state."));
}

//event-driven state machine

StateMachine<state_t, event_t, stateTransMatrixRow_t, NUM_ELEMENTS(stateTransMatrix)> theSM(stateTransMatrix);

//event queue
EventQueue<event_t, 16> evQueue;

//#################################################################################################


void setup() {
  Serial.begin(115200);
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    
   Serial.println(F("Test SM"));
   Serial.println(F("======="));
   
   Serial.print(F("Transition table size = "));Serial.print(sizeof(stateTransMatrix));Serial.println(F(" bytes"));
 
  //get ready
  theSM.init();
  //do the transition into the idle state
  theSM.runIteration(event_t::EV_ANY);

}

void loop() {
  // put your main code here, to run repeatedly:
  
  //simulate the events
  if(Serial.available()){
    char c = Serial.read();

    switch(c){
      case 't': //timeout
        //theSM.runIteration(event_t::EV_TIME_OUT);
        evQueue.addToQueue(event_t::EV_TIME_OUT);
        break;
      case 'b': //button
        //theSM.runIteration(event_t::EV_BUTTON_PUSHED);
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
    theSM.runIteration(ev);
    //debug - show state number
    Serial.print(F("state = "));Serial.println((int) theSM.getState());
  }

}

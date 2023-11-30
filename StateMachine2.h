#pragma once
#include <Arduino.h>
//#################################################################################################
//
// Statemachine (type 2) - Table driven with events causing state transitions.
// when current state and event match the transition function is called and the machine
// advances to the next state.
//
//#################################################################################################
//

/*

Data structures for state machine:

//states can be in any order, but START must be =0 and first as this is the initial state run
enum class state_t : uint8_t {
  START = 0, 
  SPLASH, 
  RUN,
};

//events can be in any order, but ANY must be first and =0.
// ANY is an unconditional transition actioned by running the runIteration() function immediately.
enum class event_t : uint8_t {
  ANY = 0, 
  UI_TIMEOUT, 
};

//Structure of the rows of the state table:
//
typedef struct {
    state_t currState;      //current state
    event_t event;          //event to match to cause a state transition
    state_t nextState;      //next state
    void (*func)(event_t);  //pointer to the state transition function - void fn(event_t)
} stateTransMatrixRow_t;

//UI state machine forward decl of transition functions. Called when current state and event match.
void StartState(event_t);
void RunState(event_t);

//STATE MACHINE
//Transition state matrix. (In PROGMEM as this table can be large.)
//Note that transition functions needn't correspond 1:1 with the state, they can be
//different for different paths into a state. Data passes extra info about the event

//Simple example (no way out of RUN state shown)
const stateTransMatrixRow_t stateTransMatrix[] PROGMEM = {
    // CURR STATE               // EVENT                      // NEXT STATE             // TRANSITION FUNC
    { UIStates::START,          UIEvents::ANY,                UIStates::SPLASH,         &StartState,       },  //init
    { UIStates::SPLASH,         UIEvents::UI_TIMEOUT,         UIStates::RUN,            &RunState,         },  //show the splash screen
};

//The instantiation of the SM would look like:

StateMachine<UIStates, 
             UIEvents,
             stateTransMatrixRow_t, 
             sizeof(stateTransMatrix)/sizeof(stateTransMatrix[0])
             > UIStateMC(stateTransMatrix);

//Running it looks like:

//in setup():
  //run once
  
  UIStateMC.init();
  //kick off the sm (runs the START state to init anthing that needs it)
  UIStateMC.runIteration(UIEvents::ANY);

//in loop():
  //run frequently and UIStateMC.runIteration called if any event happens
  
  if(...an event...){

    UIStateMC.runIteration(...the event...);

  }

//See simple FIFO event queue class for a way of collecting events from wherever (e.g. button presses)
//and getting them into the state machine in one place.

*/

//#################################################################################################
//S=state enum class, E=event enum class, T=transition matrix row type (N is num elements), 

template <class S, class E, typename T, size_t N>
class StateMachine{
  static_assert((N > 0),"Transition table size must be >0");
  public:
    StateMachine(const T (&transMatrix)[N]){
      m_stateTransMatrix = transMatrix;
    }
    
//-------------------------------------------------------------------------------------------------
  
    void init() {
        //DEBUG_PRINT("Initialising UI state machine.", "");
        m_state = static_cast<S>(0); //ST_START
    }
    
//-------------------------------------------------------------------------------------------------

    S getState() { //debug function
        return m_state;
    }
    
//-------------------------------------------------------------------------------------------------

    void runIteration(const E event) {
      // Iterate through the state transition matrix, checking if there is both a match with the current state
      // and the event
      T localData;

      for(int i = 0; i < N; i++) {
        //fetch table row from PROGMEM
        memcpy_P(&localData, &m_stateTransMatrix[i], sizeof(T));

        if(localData.currState == m_state) {
          if((localData.event == event) || (localData.event == static_cast<E>(0) )) { //::EV_ANY = 0

            // Transition to the next state
            m_state =  localData.nextState;

            // Call the function associated with transition - if not null
            if(localData.func) localData.func(event); //passes event
            break;
          }
        }
        
      } //end for
    }
    
//-------------------------------------------------------------------------------------------------

  private:
    T* m_stateTransMatrix;
    S m_state;
};

//#################################################################################################

template <class T, int16_t EVENT_QUEUE_SIZE = 8>
class EventQueue{
  static_assert((EVENT_QUEUE_SIZE > 0),"EVENT_QUEUE_SIZE must be >0");
  public:
    EventQueue() {
      //clear queue
        m_head = m_tail = 0;
    }
    
//-------------------------------------------------------------------------------------------------

    bool addToQueue(T ev){ //push
      int16_t next = m_head + 1;
      if(next >= EVENT_QUEUE_SIZE) next = 0;

      if(next == m_tail) {
        //DEBUG_PRINT("buff full ", index);
        return false; //full, ignore
      }
      m_data[m_head] = ev;
      m_head = next;

      return true;
    }
    
//-------------------------------------------------------------------------------------------------
    
    //returns the data
    T removeFromQueue(){ //pop
      if(m_head == m_tail){
        //empty
        //DEBUG_PRINT("buff empty", index);
        return static_cast<T>(-1);
      }

      int16_t next = m_tail + 1;
      if(next >= EVENT_QUEUE_SIZE) next = 0;

      T data = m_data[m_tail];

      m_tail = next; 

      return data;
    }
    
//-------------------------------------------------------------------------------------------------
    
    //empty/full?
    bool isQueueEmpty() const{ return (m_head == m_tail);}
    
//-------------------------------------------------------------------------------------------------

    bool isQueueFull() const{
      int16_t next = m_head + 1;
      if(next >= EVENT_QUEUE_SIZE) next = 0;

      return (next == m_tail);
    }
    
//-------------------------------------------------------------------------------------------------
    
    //make queue empty
    void flushQueue(){m_head = m_tail;}
    
//-------------------------------------------------------------------------------------------------
 
  private:
    //fifo queue
      T m_data[EVENT_QUEUE_SIZE];
      int16_t m_head;
      int16_t m_tail;

};

//#################################################################################################

# Arduino Table Driven State Machine
## Event-driven state machine for Arduino

Based on the ideas from: https://blog.mbedded.ninja/programming/general/control-methodology/a-function-pointer-based-state-machine/

This type of state machine works really well in practice as it is very easy to modify and the logic is clear.

The current version was turned into a C++ template class so that the specific implementation of the machine could live outside the class definition.
This makes it easy to re-use, and/or have several state machines working together.

The state transition table is designed to live in PROGMEM as it can get quite large for a complex machine.

A FIFO event queueing system is also included to be used alongside the SM. This is optional, but means that the calls to run
the machine can just be in the `loop()` and done whenever there is an event waiting in the queue. The SM is typically called only when there is an event
waiting, but it can also be called whenever required.

This is not the fastest way to implement a state machine as the entire transition table may need to be searched looking for a state-event match, but
this is only significant for complex machines with large transition tables that must react to events in sub-millisecond timeframes. 

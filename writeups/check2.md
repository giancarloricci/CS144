Checkpoint 2 Writeup
====================

My name: Giancarlo Ricci

My SUNet ID: gricci

I collaborated with: N/A

I would like to thank/reward these classmates for their help: N/A

This lab took me about 2 hours to do. I did attend the lab session.

Describe Wrap32 and TCPReceiver structure and design. [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

For Wrap32, I initially thought of finding some potential candidates and iterating over them.
However, I realized that this logic could be simplified by first computing an offset to find the absolute
sequence. This helped prevent any iteration and optimized the code.

In terms of the TCPReciever, I added two private variables:
a. the initial_sequence number, which is specified by SYN
b. a bool specifying whether syn has been set. 

This is useful, because we need to keep track of our initial_sequence once it has been set.
Furthermore, it is useful to know whether we've recieved SYN for logic in both send and recieve.
These fields could be consolidated by setting a default value of initial_sequence value, which would 
determine whether SYN has been set. However, this seemed less intuitive and readable to me

Implementation Challenges:
The initial logic for unwrapped posed some challenges for me.

Remaining Bugs:
There are no bugs I'm currently aware of 

- Optional: I had unexpected difficulty with: N/A

- Optional: I think you could make this lab better by: N/A

- Optional: I was surprised by: The length of this lab. I found Lab1 much more difficult in comparison to this lab.

- Optional: I'm not sure about: N/A

- Optional: I made an extra test I think will be helpful in catching bugs: N/A
Checkpoint 3 Writeup
====================

My name: Giancarlo Ricci

My SUNet ID: gricci

I collaborated with: NA

I would like to thank/reward these classmates for their help: NA

This checkpoint took me about 8 hours to do. I did not attend the lab session.

Program Structure and Design of the TCPSender [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]: []

Data Structures:

As mentioned by the spec, the outstanding segments need to be stored.
I choose to represent these as queue, because it already imposes a natural
notion of order based on when they were entered. 

I stored other private variables to implement functionality. 
Below I will motivate why each variable is stored.

a. bytes_pushed_: This keeps track of the current amount of bytes transmitted.
This is useful for edge-case handling (e.g. bytes_pushed == 0) and checking if the 
received ackno is within bounds. 

b. no_retransmissions_. This is necessary to store for handling retransmission logic.

c. ack_no_. This stores the first unassembled index, which is necessary for unwrapping the seqno.
 
d. highest_ack_no_. This is useful, because several functionality relies on knowing the 
highest sent ackno (both push and receive).

e. window_size. This is needed in tick, as well as to determine the number of bytes available to send 
i.e. capacity. 

f. FIN_sent_. Becasue FIN takes up a sequence number, its best to only send this once.
As such, this flag prevents sending multiple FINS. 

Furthermore, I choose to implement the timer as it own separate class.
This made sense to me, as it has its own internal variables and functions that 
are called.

Alternate Approaches: 

One alternate design approach is to place the timer functionality in TCPSender;
as mentioned, this seems quite messy. In my private variables, I tried not to store 
redundant state; in my mind, all of these variables are necessary and independent (i.e. 
they can't be used to compute one another. However, perhaps there is an alternate approach 
which is able to store less information).

Another design decision is the use of a queue. This is nice because it already stores the elems
based on when they were added. However, I supposed another collection be used if it also kept 
track of the element time, which would allow the oldest element to be removed. However, I think
queue is probably the simplest approach for this. 

Implementation Challenges:

I struggled with a couple bugs. I failed with transmit the correct payload, due to 
not casting string_view to a literal string, which caused some memory issues. This was 
a little annoying (and could be avoided by maybe having a hint to be careful about how
string_views are used!)

Remaining Bugs: There are no current bugs (to my understanding)

- Optional: I had unexpected difficulty with: N/A

- Optional: I think you could make this lab better by: N/A

- Optional: I was surprised by: N/A

- Optional: I'm not sure about: N/A

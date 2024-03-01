Checkpoint 6 Writeup
====================

My name: Giancarlo Ricci

My SUNet ID: gricci

I collaborated with: NA

I would like to thank/reward these classmates for their help: NA

This checkpoint took me about 5 hours to do. I did not attend the lab session.

Program Structure and Design of the Router [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]: 

add_route is incredibly simple; it just adds the relevant information to the routing_table.

route is a little more complicated. The function will go through each interface and process 
each received datagram. The bulk of the logic for sending a single datagram (besides the extra logic
needed for TTL) is to find the the best route match.

This is achieved through a helper "find match" function, which will search over each route.
First, we will find any route that "matches" (i.e. the most-significant prefix length bits of
the destination address are identical to the most-significant prefix length bits of the
route prefix). Then, we will remember the longest of such matches. 

Note that for my routing table implementation, I choose to use a vector (of structs with the necessary 
routing info) or my routing table. As mentioned in the spec "It’s perfectly acceptable for each datagram
to require O(N) work, where N is the number of entries in the routing table." As such, I focused on 
getting a simple working implementation with this.


Implementation Challenges: The biggest challenge for me was completing the mask logic to ensure
the MSB matched.

Remaining Bugs: To my knowledge, there are no remaining bugs.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

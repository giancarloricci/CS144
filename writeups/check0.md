Checkpoint 0 Writeup
====================

My name: Giancarlo Ricci

My SUNet ID: gricci

I collaborated with: yousifm, ifreitas, 

I would like to credit/thank these classmates for their help: yousifm, ifreitas, 

This lab took me about 5 hours to do. I did attend the lab session.

My secret code from section 2.1 was: 603239

I was surprised by or edified to learn that: The entire web communication could occur with just a single socket. In the beginning I overcomplicated part 3, using multiple sockets - it was suprising to me that a singular socket could provide the entire communication channel between server and client. THis made me realize how powerful of an tool sockets could be.

Describe ByteStream implementation. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

I decided to use a string as my data structure to store the buffer. 

I originally considered using a queue of characters; however, I realized that the atomic (e.g. per character) nature of this data structure would mean that push / pop operations would act on a by-character basis, which is quite inefficient. After discussing with Kamran the TA during lab, I realized that certain operations would be easier using a string. 

To begin, this datastructure makes the peek operation quite easy, as we are able to return the entire string. Push operations are performed by appending to the end of the first string; I believe this has time-complexity that is linear in the length of the appended string. The main drawback of this approach is the pop operation; after the first elements in the string are popped, the remaining elements must be shifted over.

Despite this limitation, one benefit of using this datastructure is its simplicity - the C++ string library includes many functions (e.g. erase, append) that make these operations incredibly simple. 

One alternate design decision I considered was something that more easily maintains our first-in first-out (FIFO) property. In particular, recall that a string has to reshuffle elements in pop; this drawback could be avoided with an alternate data structure, such as a circular buffer. 

However, due to the simplicity in implementation of string, and the fact that it exceeded the necessary throughput (i.e. the limitation of string pop was not too severe), I decided to keep the string implementation.

- Optional: I had unexpected difficulty with: N/A

- Optional: I think you could make this lab better by: N/A

- Optional: I'm not sure about: Certain socket operations, like should I shutdown / close the socket in webget.cc after use? 

- Optional: I contributed a new test case that catches a plausible bug
  not otherwise caught: N/A
Checkpoint 5 Writeup
====================

My name: Giancarlo Ricci

My SUNet ID: gricci

I collaborated with: NA

I would like to thank/reward these classmates for their help: NA

This checkpoint took me about 6 hours to do. I did not attend the lab session.

Program Structure and Design of the NetworkInterface [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]: 


The assignment spec details the main algorithmic component of each function.
One main contribution needed is to add new datastructures. I added the following:

1. 

struct CachedEthernetAddress
  {
    size_t time_cached;
    EthernetAddress ethernet_address;
  };

std::map<uint32_t, CachedEthernetAddress> cache_ {};

This datastructures keeps track of our cache from ip_addresses to internet addresses.
The struct (or equivalent std::pair) is necessary so we can keep track of how long 
the address has been cached for (so it can be removed when necessary).

2.

struct WaitingDatagrams
  {
    size_t time_ARP = 0;
    std::queue<InternetDatagram> waiting_datagrams {};
  };

std::map<uint32_t, WaitingDatagrams> ip_waiting_ {};

Similarly, we keep track of the ip_addresses which are waiting to send datagrams,
and the last time they sent an ARP request (so we known when to resend it).
This DS is used in both recv_frame and send_datagram to ensure that if we can't 
send immediately, we will send soon.


Implementation Challenges:

The main challenge was the choice of datastructure. The actual logic was pretty clear
following the assignment handout. 


Remaining Bugs:

There are no bugs as far as I'm aware of.

- Optional: I had unexpected difficulty with: NA

- Optional: I think you could make this lab better by: NA

- Optional: I was surprised by: NA

- Optional: I'm not sure about: NA

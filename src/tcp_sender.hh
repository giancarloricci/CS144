#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>

class Timer
{
  uint64_t time_;
  uint64_t initial_RTO_;
  uint64_t RTO_;
  bool running_;
  void reset();

public:
  explicit Timer( uint64_t initial_RTO );
  uint64_t get();
  void elapse( uint64_t time_elapsed );
  bool expired();
  void stop();
  void double_RTO();
  void start();
  void restart();
  void restore_RTO();
};

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms ), timer_ { initial_RTO_ms }
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
  Writer& writer() { return input_.writer(); }
  const Writer& writer() const { return input_.writer(); }

  // Access input stream reader, but const-only (can't read from outside)
  const Reader& reader() const { return input_.reader(); }

private:
  // Variables initialized in constructor
  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  Timer timer_;
  uint64_t bytes_pushed_ { 0 };
  uint64_t retransmission_no { 0 };
  uint64_t ack_no_ { 0 };
  uint64_t highest_ack_no_ { 0 };
  uint64_t window_size_ { 1 };
  bool FIN_sent_ { false };

  // keep track of which segments have been sent but not yet acknowledged by the receiver
  std::queue<TCPSenderMessage> outstanding_segments_ {};
  void send_data( const TransmitFunction& transmit, TCPSenderMessage msg );
  void try_set_FIN( TCPSenderMessage& msg, uint64_t size );
};

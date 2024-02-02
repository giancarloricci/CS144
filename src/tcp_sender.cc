#include "tcp_sender.hh"
#include "tcp_config.hh"
#include <algorithm>
#include <iostream>

using namespace std;

Timer::Timer( uint64_t init_RTO ) : time_( init_RTO ), initial_RTO_( init_RTO ), RTO_( init_RTO ), running_( false )
{}

// uint64_t Timer::get()
// {
//   return time_;
// }

void Timer::elapse( uint64_t time_elapsed )
{
  if ( running_ )
    time_ = time_elapsed > time_ ? 0 : time_ - time_elapsed;
  // cerr << "timer is: " << time_ << endl;
}
bool Timer::expired()
{
  return time_ == 0;
}

void Timer::stop()
{
  running_ = false;
}

void Timer::double_RTO()
{
  // cerr << "doubling" << RTO_ << endl;
  RTO_ = std::min( RTO_ * 2, UINT64_MAX );
  // cerr << "RTO is now" << RTO_ << endl;
}

void Timer::reset()
{
  // cerr << "reset called with time" << time_ << endl;
  time_ = RTO_;
  // cerr << "time now" << time_ << endl;
}

void Timer::start()
{
  // cerr << "start called" << endl;
  running_ = true;
}

void Timer::restore_RTO()
{
  // cerr << "restore called with RTO as " << RTO_ << endl;
  RTO_ = initial_RTO_;
  // cerr << "RTO now" << RTO_ << endl;
}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return bytes_pushed_ - ack_no_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return no_retransmissions_;
}

void TCPSender::send_data( const TransmitFunction& transmit, TCPSenderMessage msg )
{
  transmit( msg );
  bytes_pushed_ += msg.sequence_length();
  outstanding_segments_.push( msg );
  timer_.start();
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // special case for initial push
  if ( bytes_pushed_ == 0 ) {
    TCPSenderMessage msg = make_empty_message();
    msg.SYN = true;
    send_data( transmit, msg );
  }

  uint64_t capacity = ack_no_ + std::max( window_size_, uint64_t { 1 } );
  while ( input_.reader().bytes_buffered() > 0 && capacity > bytes_pushed_ ) {
    uint64_t msg_len
      = std::min( { input_.reader().bytes_buffered(), capacity - bytes_pushed_, TCPConfig::MAX_PAYLOAD_SIZE } );
    std::string buffer = std::string( input_.reader().peek().substr( 0, msg_len ) );
    input_.reader().pop( msg_len );

    TCPSenderMessage msg = make_empty_message();
    msg.payload = buffer;
    msg.FIN = input_.reader().is_finished();
    send_data( transmit, msg );
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage message;
  message.seqno = Wrap32::wrap( bytes_pushed_, isn_ );
  return message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  window_size_ = msg.window_size;

  if ( msg.ackno.has_value() ) {
    uint64_t received_ackno = msg.ackno.value().unwrap( isn_, ack_no_ );

    while ( !outstanding_segments_.empty() ) {

      TCPSenderMessage front_msg = outstanding_segments_.front();
      if ( received_ackno < ack_no_ + front_msg.sequence_length() )
        break;
      ack_no_ += front_msg.sequence_length();
      outstanding_segments_.pop();
    }

    if ( received_ackno > highest_ack_no_ ) {
      no_retransmissions_ = 0;
      timer_.restore_RTO();
      timer_.start();
      timer_.reset();
      highest_ack_no_ = received_ackno;
    }

    if ( outstanding_segments_.empty() ) {
      timer_.stop();
    }
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // cerr << "RUNNING TICK" << endl;
  // cerr << "before" << timer_.get() << "and ms" << ms_since_last_tick << endl;
  timer_.elapse( ms_since_last_tick );
  // cerr << "after" << timer_.get() << endl;
  if ( timer_.expired() ) {
    // cerr << "inside expired window is " << window_size_ << endl;
    transmit( outstanding_segments_.front() );
    if ( window_size_ ) {
      no_retransmissions_ += 1;
      timer_.double_RTO();
      // cerr << "doubled rto" << endl;
    }
    timer_.reset();
    timer_.start();
  }
}

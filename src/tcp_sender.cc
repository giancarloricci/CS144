#include "tcp_sender.hh"
#include "tcp_config.hh"
#include <algorithm>
#include <iostream>

using namespace std;

Timer::Timer( uint64_t init_RTO ) : time_( init_RTO ), initial_RTO_( init_RTO ), RTO_( init_RTO ), running_( false )
{}

void Timer::elapse( uint64_t time_elapsed )
{
  if ( running_ ) {
    //time_ = std::max( uint64_t { 0 }, time_ - time_elapsed );
    time_ -= std::min( time_, time_elapsed);
  }
  cerr << "timer is: " << time_ << endl;
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
  RTO_ = std::min( RTO_ * 2, UINT64_MAX );
}

void Timer::reset()
{
  time_ = RTO_;
}

void Timer::start()
{
  running_ = true;
}

void Timer::restore_RTO()
{
  RTO_ = initial_RTO_;
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
      // TODO prevent this from happening more than once
      timer_.restore_RTO();
      no_retransmissions_ = 0;
      if ( !outstanding_segments_.empty() ) {
        timer_.reset();
        timer_.start();
      }
    }
    if ( outstanding_segments_.empty() )
      timer_.stop();
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  timer_.elapse( ms_since_last_tick );
  if ( timer_.expired() ) {
    cerr << "inside expired " << window_size_ << endl;
    transmit( outstanding_segments_.front() );
    if ( window_size_ ) {
      no_retransmissions_ += 1;
      timer_.double_RTO();
    }
    timer_.reset();
    timer_.start();
  }
}

#include "tcp_sender.hh"
#include "tcp_config.hh"
#include <algorithm>

using namespace std;

Timer::Timer( uint64_t init_RTO ) : time_( init_RTO ), initial_RTO_( init_RTO ), RTO_( init_RTO ), running_( false )
{}

void Timer::elapse( uint64_t time_elapsed )
{
  if ( running_ )
    time_ = time_elapsed > time_ ? 0 : time_ - time_elapsed;
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

void Timer::restart()
{
  start();
  reset();
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
  return retransmission_no;
}

void TCPSender::send_data( const TransmitFunction& transmit, TCPSenderMessage msg )
{
  bytes_pushed_ += msg.sequence_length();
  outstanding_segments_.push( msg );
  timer_.start();
  transmit( msg );
}

void TCPSender::try_set_FIN( TCPSenderMessage& msg, uint64_t size )
{
  uint64_t capacity = highest_ack_no_ + std::max( window_size_, uint64_t { 1 } );
  if ( input_.reader().is_finished() && !FIN_sent_ && capacity - bytes_pushed_ > size ) {
    msg.FIN = true;
    FIN_sent_ = true;
  }
}

void TCPSender::push( const TransmitFunction& transmit )
{
  uint64_t capacity = highest_ack_no_ + std::max( window_size_, uint64_t { 1 } );
  if ( bytes_pushed_ == 0 ) {
    TCPSenderMessage msg = make_empty_message();
    msg.SYN = true;
    try_set_FIN( msg, 1 );
    send_data( transmit, msg );
  }

  while ( ( input_.reader().bytes_buffered() > 0 || ( input_.reader().is_finished() && !FIN_sent_ ) )
          && capacity > bytes_pushed_ ) {
    uint64_t msg_len
      = std::min( { input_.reader().bytes_buffered(), capacity - bytes_pushed_, TCPConfig::MAX_PAYLOAD_SIZE } );
    std::string buffer = std::string( input_.reader().peek().substr( 0, msg_len ) );
    input_.reader().pop( msg_len );

    TCPSenderMessage msg = make_empty_message();
    msg.payload = buffer;
    try_set_FIN( msg, msg_len );
    send_data( transmit, msg );
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage message;
  message.seqno = Wrap32::wrap( bytes_pushed_, isn_ );
  if ( input_.writer().has_error() ) {
    message.RST = true;
  }
  return message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if ( msg.RST ) {
    input_.writer().set_error();
  }

  window_size_ = msg.window_size;

  if ( msg.ackno.has_value() ) {
    uint64_t received_ackno = msg.ackno.value().unwrap( isn_, ack_no_ );

    if ( received_ackno > bytes_pushed_ )
      return;

    while ( !outstanding_segments_.empty() ) {

      TCPSenderMessage front_msg = outstanding_segments_.front();
      if ( received_ackno < ack_no_ + front_msg.sequence_length() )
        break;
      ack_no_ += front_msg.sequence_length();
      outstanding_segments_.pop();
    }

    if ( received_ackno > highest_ack_no_ ) {
      retransmission_no = 0;
      timer_.restore_RTO();
      timer_.restart();
      highest_ack_no_ = received_ackno;
    }

    if ( outstanding_segments_.empty() )
      timer_.stop();
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  timer_.elapse( ms_since_last_tick );
  if ( timer_.expired() ) {
    transmit( outstanding_segments_.front() );
    if ( window_size_ ) {
      retransmission_no += 1;
      timer_.double_RTO();
    }
    timer_.restart();
  }
}

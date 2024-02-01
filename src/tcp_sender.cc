#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

Timer::Timer( uint64_t init_RTO ) : time( init_RTO ), initial_RTO( init_RTO ), RTO( init_RTO ), running( false ) {}

void Timer::elapse( uint64_t time_elapsed )
{
  time -= time_elapsed;
}

bool Timer::expired()
{
  return time == 0;
}

void Timer::stop()
{
  running = false;
}

void Timer::double_RTO()
{
  RTO *= 2;
}

void Timer::reset()
{
  time = RTO;
}

void Timer::start()
{
  running = true;
}

void Timer::restore_RTO()
{
  RTO = initial_RTO;
}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return bytes_pushed_ - ack_no_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return no_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  (void)transmit;
  // TODO 
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage message;

  message.seqno = Wrap32::wrap( bytes_pushed_, isn_ );
  message.SYN = bytes_pushed_ == 0;
  message.payload = "";
  message.FIN = false;
  message.RST = false;

  return message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if (msg.ackno.has_value()) {
  
    // TODO
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  timer_.elapse( ms_since_last_tick );
  if ( timer_.expired() ) {

    // TODO: re-transmit

    timer_.reset();
    timer_.start();
  }
}

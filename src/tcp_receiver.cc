#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  auto [seqno, SYN, payload, FIN, RST] = message;

  if ( RST ) {
    reassembler_.reader().set_error();
  }

  if ( SYN ) {
    initial_seqno = seqno;
    syn_ = true;
  }

  // Can't proceed without initial_seqno
  if ( !syn_ )
    return;

  // + 1 for SYN, - 1 for stream index
  uint64_t first_index = seqno.unwrap( initial_seqno, reassembler_.writer().bytes_pushed() ) + SYN - 1;
  reassembler_.insert( first_index, payload, FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage message {};

  if ( reassembler_.writer().has_error() )
    message.RST = true;

  if ( syn_ )
    // include SYN and FIN to ackno
    message.ackno
      = Wrap32::wrap( reassembler_.writer().bytes_pushed(), initial_seqno + 1 + reassembler_.writer().is_closed() );

  uint64_t capacity = reassembler_.writer().available_capacity();
  message.window_size = capacity < UINT16_MAX ? capacity : UINT16_MAX; // truncate if necessary

  return message;
}

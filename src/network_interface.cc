#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

void send_ARP_request( const uint32_t target_ip_address )
{
  EthernetFrame frame;
  frame.header.type = EthernetHeader::TYPE_ARP;
  frame.header.src = _ethernet_address;
  frame.header.dst = ETHERNET_BROADCAST;
  ARPMessage arp;
  arp.opcode = ARPMessage::OPCODE_REQUEST;
  arp.sender_ethernet_address = _ethernet_address;
  arp.sender_ip_address = _ip_address.ipv4_numeric();
  arp.target_ip_address = target_ip_address;
  frame.payload = arp.serialize();
  transmit( frame );
}

void send_ARP_reply( const uint32_t target_ip_address, const EthernetAddress& target_ethernet_addr )
{
  EthernetFrame frame;
  frame.header.type = EthernetHeader::TYPE_ARP;
  frame.header.src = _ethernet_address;
  frame.header.dst = target_ethernet_addr;
  ARPMessage arp;
  arp.opcode = ARPMessage::OPCODE_REPLY;
  arp.sender_ethernet_address = _ethernet_address;
  arp.sender_ip_address = _ip_address.ipv4_numeric();
  arp.target_ethernet_address = destination_ethernet_addr;
  arp.target_ip_address = target_ip_address;
  frame.payload = arp.serialize();
  transmit( frame );
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  const uint32_t next_hop_ip = next_hop.ipv4_numeric();
  EthernetAddress destination_ethernet; // TODO: get this

  // if the destination Ethernet address is already known
  if ( destinationAddress ) {
    EthernetFrame frame;
    frame.header.type = EthernetHeader::TYPE_IPv4;
    frame.header.src = _ethernet_address;
    frame.header.dst = destination_ethernet;
    frame.payload = dgram.serialize();
    transmit( frame );
  } else {
    // If the network interface already sent an ARP request about the same IP address
    // in the last five seconds, don’t send a second request
    // just wait for a reply to the first one.
    bool should_send_arp_request = true; // TODO replace
    if ( should_send_arp_request ) {
      send_ARP_request( next_hop_ip );
    }
    // TODO: queue the IP datagram so it can be sent after the ARP reply is received.
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // ignore any frames not destined for the network interface
  if ( frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST ) {
    return
  }

  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram dgram;
    if ( parse( dgram, frame.payload ) == ParseResult::NoError ) {
      // push the resulting datagram on to the datagrams received queue.
      datagrams_received_.push( dgram );
    }
  } else if ( frame.header.type == EthernetHeader::TYPE_ARP ) {
    ARPMessage arp;
    if ( parse( arp, frame.payload ) == ParseResult::NoError ) {
      // TODO
      // remember the mapping between the sender’s IP address and Ethernet address for 30 seconds.
      // (Learn mappings from both requests and replies.)

      // if it’s an ARP request asking for our IP address, send an appropriate ARP reply
      if ( arp.opcode == ARPMessage::OPCODE_REQUEST && arp.target_ip_address == _ip_address.ipv4_numeric() ) {
        send_ARP_reply( arp.sender_ip_address, arp.sender_ethernet_address );
      }
    }
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  (void)ms_since_last_tick;

  // TODO Expire any IP-to-Ethernet mappings that have expired
}

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

void NetworkInterface::create_and_send_frame( const InternetDatagram& dgram,
                                              const EthernetAddress& target_ethernet_addr )
{
  EthernetFrame frame;
  frame.header.type = EthernetHeader::TYPE_IPv4;
  frame.header.src = ethernet_address_;
  frame.header.dst = target_ethernet_addr;
  frame.payload = serialize( dgram );
  transmit( frame );
}

void NetworkInterface::send_ARP_request( const uint32_t target_ip_address )
{
  EthernetFrame frame;
  frame.header.type = EthernetHeader::TYPE_ARP;
  frame.header.src = ethernet_address_;
  frame.header.dst = ETHERNET_BROADCAST;
  ARPMessage arp;
  arp.opcode = ARPMessage::OPCODE_REQUEST;
  arp.sender_ethernet_address = ethernet_address_;
  arp.sender_ip_address = ip_address_.ipv4_numeric();
  arp.target_ip_address = target_ip_address;
  frame.payload = serialize( arp );
  transmit( frame );
}

void NetworkInterface::send_ARP_reply( const uint32_t target_ip_address,
                                       const EthernetAddress& target_ethernet_addr )
{
  EthernetFrame frame;
  frame.header.type = EthernetHeader::TYPE_ARP;
  frame.header.src = ethernet_address_;
  frame.header.dst = target_ethernet_addr;
  ARPMessage arp;
  arp.opcode = ARPMessage::OPCODE_REPLY;
  arp.sender_ethernet_address = ethernet_address_;
  arp.sender_ip_address = ip_address_.ipv4_numeric();
  arp.target_ethernet_address = target_ethernet_addr;
  arp.target_ip_address = target_ip_address;
  frame.payload = serialize( arp );
  transmit( frame );
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  const uint32_t next_hop_ip = next_hop.ipv4_numeric();
  auto it = cache_.find( next_hop_ip );

  // if the destination Ethernet address is already known
  if ( it != cache_.end() ) {
    create_and_send_frame( dgram, it->second.ethernet_address );
  } else {
    bool should_send_ARP = true;

    auto iter = ip_waiting_.find( next_hop_ip );
    if ( iter != ip_waiting_.end() ) {
      should_send_ARP = ip_waiting_[next_hop_ip].time_ARP >= ARP_WINDOW_MS;
      ip_waiting_[next_hop_ip].waiting_datagrams.push( dgram );
    } else {
      WaitingDatagrams waiting;
      waiting.waiting_datagrams.push( dgram );
      ip_waiting_[next_hop_ip] = waiting;
    }
    if ( should_send_ARP ) {
      send_ARP_request( next_hop_ip );
    }
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // ignore any frames not destined for the network interface
  if ( frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST ) {
    return;
  }

  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram dgram;
    if ( parse( dgram, frame.payload ) ) {
      // push the resulting datagram on to the datagrams received queue.
      datagrams_received_.push( dgram );
    }
  } else if ( frame.header.type == EthernetHeader::TYPE_ARP ) {
    ARPMessage arp;
    if ( parse( arp, frame.payload ) ) {

      // remember mapping between the sender’s IP address and Ethernet address for 30 seconds.
      CachedEthernetAddress cached_entry;
      cached_entry.time_cached = 0; // TODO when is the write time?
      cached_entry.ethernet_address = arp.sender_ethernet_address;
      cache_[arp.sender_ip_address] = cached_entry;

      // send waiting
      auto it = ip_waiting_.find( arp.sender_ip_address );
      if ( it != ip_waiting_.end() ) {

        while ( !it->second.waiting_datagrams.empty() ) {
          InternetDatagram dgram = it->second.waiting_datagrams.front();
          it->second.waiting_datagrams.pop();
          create_and_send_frame( dgram, arp.sender_ethernet_address );
        }
        ip_waiting_.erase( arp.sender_ip_address );
      }

      // if it’s an ARP request asking for our IP address, send an appropriate ARP reply
      if ( arp.opcode == ARPMessage::OPCODE_REQUEST && arp.target_ip_address == ip_address_.ipv4_numeric() ) {
        send_ARP_reply( arp.sender_ip_address, arp.sender_ethernet_address );
      }
    }
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // update the time since last ARP request
  for ( auto it = ip_waiting_.begin(); it != ip_waiting_.end(); ) {
    it->second.time_ARP += ms_since_last_tick;
    if ( it->second.time_ARP >= ARP_WINDOW_MS ) {
      it = ip_waiting_.erase( it );
    } else {
      ++it;
    }
  }
  
  // expire any IP-to-Ethernet mappings that have expired
  for ( auto it = cache_.begin(); it != cache_.end(); ) {
    it->second.time_cached += ms_since_last_tick;
    if ( it->second.time_cached >= MAX_CACHE_TIME_MS ) {
      it = cache_.erase( it );
    } else {
      ++it;
    }
  }
}

#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  routing_table_.emplace_back( route_prefix, prefix_length, next_hop, interface_num );
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  for ( auto& interface : _interfaces ) {
    auto& dgrams = interface->datagrams_received();
    while ( !dgrams.empty() ) {
      InternetDatagram dgram = dgrams.front();
      route_datagram( dgram );
      dgrams.pop();
    }
  }
}

void Router::route_datagram( InternetDatagram dgram )
{
  if ( dgram.header.ttl <= 1 )
    return;

  dgram.header.ttl -= 1;
  dgram.header.compute_checksum();
  auto dst = dgram.header.dst;

  auto match = find_match( dst );
  if ( !match.has_value() )
    return;

  RouteEntry entry = match.value();
  auto target = interface( entry.interface_num );
  target->send_datagram( dgram, entry.next_hop.value_or( Address::from_ipv4_numeric( dst ) ) );
}

std::optional<Router::RouteEntry> Router::find_match( uint32_t dst )
{
  std::optional<RouteEntry> longest_match;
  size_t longest_prefix_length = 0;

  for ( auto& entry : routing_table_ ) {

    uint32_t mask = entry.prefix_length == 0 ? 0 : numeric_limits<int>::min() >> ( entry.prefix_length - 1 );
    if ( ( dst & mask ) == entry.route_prefix && entry.prefix_length >= longest_prefix_length ) {
      longest_match = entry;
      longest_prefix_length = entry.prefix_length;
    }
  }
  return longest_match;
}
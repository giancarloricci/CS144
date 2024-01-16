#include "byte_stream.hh"
#include <iostream>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  // Your code here.
  return is_closed_;
}

void Writer::push( string data )
{
  if ( data == "" )
    return;
  uint64_t bytes_to_push = available_capacity() > data.length() ? data.length() : available_capacity();
  string available_data = data.substr( 0, bytes_to_push );
  buf_.append( available_data );
  bytes_pushed_ += bytes_to_push;
  return;
}

void Writer::close()
{
  // Your code here.
  is_closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - buf_.length();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_pushed_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return is_closed_ && !buf_.length();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_popped_;
}

string_view Reader::peek() const
{
  return buf_;
}

void Reader::pop( uint64_t len )
{
  // Your code here
  size_t bytes_to_pop = len;
  buf_.erase( buf_.begin(), buf_.begin() + bytes_to_pop );
  bytes_popped_ += bytes_to_pop;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return bytes_pushed_ - bytes_popped_;
}

#include "reassembler.hh"

using namespace std;

void Reassembler::storage_insert( const string& data, uint64_t index )
{
  uint64_t insertion_index = index;
  uint64_t end_index = index + data.size();

  for ( auto it = storage_.begin(); it != storage_.end(); ) {

    // EDGE CASE: Data cannot be inserted
    if ( insertion_index >= end_index )
      return;

    // CASE 1: Insert as much data as possible return
    uint64_t storage_index = it->first;
    if ( end_index <= storage_index ) {
      uint64_t len = end_index - insertion_index;
      pending_ += len;
      storage_.emplace( it, make_pair( insertion_index, data.substr( insertion_index - index, len ) ) );
      return;
    }

    // CASE 2: Continue searching for space in storage
    if ( insertion_index >= storage_index ) {
      insertion_index = std::max( insertion_index, storage_index + it->second.size() );
      it++;
      continue;
    }

    // CASE 3: Store some data and continue searching
    uint64_t len = storage_index - insertion_index;
    pending_ += len;
    storage_.emplace( it, make_pair( insertion_index, data.substr( insertion_index - index, len ) ) );
    insertion_index = storage_index;
  }

  // CASE 4: We've reached end, and there's still some space
  if ( insertion_index < end_index ) {
    uint64_t len = end_index - insertion_index;
    pending_ += len;
    storage_.emplace_back( make_pair( insertion_index, data.substr( insertion_index - index, len ) ) );
    return;
  }
}

void Reassembler::remove_from_storage()
{
  // CASE 1: Storage is empty
  if ( storage_.empty() )
    return;

  for ( auto it = storage_.begin(); it != storage_.end(); ) {
    // CASE 2: Can't push to byte stream
    uint64_t first_index = it->first;
    if ( first_index > first_unassembled_ )
      break;

    // CASE 3: Already pushed all of string
    string data = it->second;
    uint64_t end_index = first_index + data.size();
    if ( end_index <= first_unassembled_ )
      pending_ -= data.size();

    // CASE 4: Remove data and push to stream
    else {
      pending_ -= data.size();
      // Truncate any data that was already written
      if ( first_index < first_unassembled_ )
        data = data.substr( first_unassembled_ - first_index );

      first_unassembled_ += data.size();
      output_.writer().push( data );
    }
    it = storage_.erase( it );
  }
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if ( is_last_substring )
    final_index_ = first_index + data.size();

  // EDGE CASE 1: Nothing to add
  if ( data.empty() ) {

    // Close stream if possible
    if ( storage_.empty() && final_index_ == first_unassembled_ ) {
      output_.writer().close();
    }
    return;
  }

  // EDGE CASE 2: No available capacity
  if ( writer().available_capacity() == 0 )
    return;

  // EDGE CASE 3: The entire string has alredy been pushed
  uint64_t end_index = first_index + data.size();
  if ( end_index < first_unassembled_ )
    return;

  // EDGE CASE 4: Unable to store any of the string
  uint64_t first_unacceptable = first_unassembled_ + writer().available_capacity();
  if ( first_index >= first_unacceptable )
    return;

  // TRUNCATION 1: Unable to store end of string
  if ( end_index > first_unacceptable )
    data = data.substr( 0, first_unacceptable - first_index );

  // TRUNCATION 2: Beginning of string has already been pushed
  if ( first_index < first_unassembled_ )
    data = data.substr( first_unassembled_ - first_index );

  // CASE 1: Current index should be pushed to stream
  if ( first_index <= first_unassembled_ ) {
    first_unassembled_ += data.size();
    output_.writer().push( data );
    remove_from_storage();
  }
  // CASE 2: Add as much as possible to re-assembler
  else
    storage_insert( data, first_index );

  // Once everything assembled, close stream
  if ( first_unassembled_ == final_index_ && storage_.empty() )
    output_.writer().close();
}

uint64_t Reassembler::bytes_pending() const
{
  return pending_;
}

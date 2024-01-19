#include "reassembler.hh"
#include <iostream>

using namespace std;

void Reassembler::storage_insert(const string& data, uint64_t index) {
  // TODO fix overlapping 
  storage.insert(make_pair(index, data));
}

size_t Reassembler::write( string data ) 
{
  uint64_t before_write = output_.writer().bytes_pushed();
  output_.writer().push(data);
  uint64_t total_written =  output_.writer().bytes_pushed() - before_write;
  first_unassembled_ += total_written;
  return total_written;
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{

  if (is_last_substring) final_index_ = first_index + data.size();


  // CASE 1: Current index should be pushed to stream 
  if (first_index == first_unassembled_) {    
   
    // TODO: check that write hasnt occured before. 
    size_t total_written = write(data);
    if(total_written < data.size()) return; 
    
    while (true) {
      auto it = storage.find(first_unassembled_);
      if (it == storage.end()) break;
      total_written = write(data);
      storage.erase(it);
    }
  }
  else { 
    // uint64_t first_unacceptable = first_unassembled_ + writer().bytes_pushed() + writer().available_capacity();
    // (void)first_unacceptable;
    storage_insert(data, first_index);
  }
        
  // Once everything assembled, close stream 
  if (first_unassembled_ == final_index_ && storage.empty()) {
        output_.writer().close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  size_t count = 0;
  for (auto &elem : storage) count += elem.second.size();
  return count;
}

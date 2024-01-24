#include "wrapping_integers.hh"

using namespace std;
#define OFFSET ( 1UL << 32 ) // 2 ^ 32

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { zero_point + uint32_t( n ) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // STEP 1: Convert to wrapped representation
  uint32_t wrapped = wrap( checkpoint, zero_point ).raw_value_;

  // STEP 2: Since wrapped will truncate, this represents the meaningful byte offset
  int32_t offset = raw_value_ - wrapped;

  // STEP 3: Find absolute sequence using offset
  int64_t abs_seq = checkpoint + offset;

  // STEP 4: Account for overflow
  return abs_seq < 0 ? abs_seq + OFFSET : abs_seq;
}

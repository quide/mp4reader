#pragma once	// this should be accepted by all compilers // TODO: test...

#include <cstdint>


class box
{
public:
	box(uint64_t  box_address,  uint32_t box_size,  uint32_t box_type) :
		address(box_address), size(box_size), type(box_type)
	{}
	
private:
	uint64_t  address;	// first position at the file (an MP4 file can have a max of 18,446,744,073,709,551,615 bytes)
	uint32_t  size;		// size of this box (a box can have a max of 4,294,967,295 bytes)
	uint32_t  type;		// type (name) of this box
};


/***********************************
 *     ****** MP4 Reader ******
 *  lists mp4 boxes and extracts MDAT box 
 *  (Supports: Linux/OS X/Windows)
 *   
 *  Usage: mp4Reader <file.mp4>
 *
 *  NOTES on C++14:
 *		. VS2015 supports C++14
 *		. g++-4.9 has -std=c++14
 *		. Xcode (use Clang in C++14 mode with the -std=c++14)
 *  
 *  TODO: . add unit tests
 *        . test at Linux
 *        . test at MacOS
 *        . unified schema to generate multiplatform projects? (like MPC)
 *		  . Prevent: 
 *				    . system endianness
 *					. UTF-8 named files and data
 *					. check unsigned char and unsigned int (uint32_t) convertions and portability
 ***********************************/ 

#include <fstream> 
#include <iostream>
#include <string>
#include <memory>

using namespace std; 

#include "box.h"


#define BLOCK2INT(c)       \
   static_cast<uint32_t>(((static_cast<unsigned char>(c[0]))<<24) | \
						 ((static_cast<unsigned char>(c[1]))<<16) | \
						 ((static_cast<unsigned char>(c[2]))<< 8) | \
						 ((static_cast<unsigned char>(c[3]))    )     )


void usage( const string&  app_name ){
	cout << "Usage:" << endl << "\t" << app_name << " <file.mp4>" << endl;
}



int main(int argc, char* argv[]) {
	string app_name(argv[0]);

	if (argc != 2) {
		cout << "File not correctly inserted" << ((argc == 1) ? " (lacking)." : ".") << endl;
		usage(app_name);
		return -1;
	}

	ifstream file;
	string filename{ argv[1] };
	file.open(filename, ios::binary | ios::ate);	// open file at the end of it (to determine file size)

	if( file.is_open() ){ 
		cout << "Successfully loaded file " << filename << endl;
	}
	else{
		cout << "File " << filename << " not possible to be open." << endl;
		usage(app_name);
		return -2;
	}

	uint64_t file_size = file.tellg(); 	// detect full file size
	file.seekg(0, std::ios::beg);		// get ready to read from the beginning

	///// TESTING ...

	bool work_to_do = true;

	const int BLOCKS_SIZE{ 4 };
	char memory_block[BLOCKS_SIZE];

	while( work_to_do ){
		uint64_t box_address = file.tellg();

		if( file.eof()  ||  box_address >= file_size ){
			cout << "All file was read." << endl;
			work_to_do = false;
			break;
		}

		file.read(memory_block, BLOCKS_SIZE);
		uint32_t box_size = BLOCK2INT(memory_block);	// we are supporting boxes with 4,294,967,295 bytes max (32 bit representation)

		file.read(memory_block, BLOCKS_SIZE);
		uint32_t box_type = BLOCK2INT(memory_block);
		string box_type_s(memory_block);

		box new_box(box_address, box_size, box_type);
		cout << "Found box of type " << box_type_s << " and size " << box_size << endl;

		switch (box_type) {
			case BLOCK2INT("moof"):	// these only contain sub-boxes,
			case BLOCK2INT("traf"):	// just go to the next sub-box
				break;
			case BLOCK2INT("mdat"):
				break;
			default:				// all other box types don't contain sub-boxes
				// skip their content
				break;
		}
	}

	/////////////////////
	
	file.close();
  	return 0;
}
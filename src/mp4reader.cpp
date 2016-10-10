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

#define BOX_TYPE(c1,c2,c3,c4)       \
   (((static_cast<uint32_t>(c1))<<24) |  \
    ((static_cast<uint32_t>(c2))<<16) |  \
    ((static_cast<uint32_t>(c3))<< 8) |  \
    ((static_cast<uint32_t>(c4))    ))


void usage( const string&  app_name ){
	cout << "Usage:" << endl << "\t" << app_name << " <file.mp4>" << endl;
}

int main( int argc ,  char* argv[] ){
	string app_name( argv[0] );

	if( argc != 2 ){
		cout << "File to be openned not correctly inserted" << ( (argc==1)? " (lacking)." : "." ) << endl;
		usage( app_name );
		return -1; 
	}

	ifstream file;  
	string filename{ argv[1] };
	file.open(filename, ios::binary);
	
	if( file.is_open() ) { cout << "File " << filename << " correctly open." << endl;  }
	else{ 
		cout << "File " << filename << " not possible to open." << endl;
		usage( app_name );
		return -2; 
	}
	
	///// TESTING ...

	const int BLOCKS_SIZE{ 4 };
	union {
		uint32_t i;
		char c[BLOCKS_SIZE];
	} memory_block;

	//unique_ptr<char[]> memory_block(new char[BLOCKS_SIZE]);
	file.read(memory_block.c, BLOCKS_SIZE);
	
	//int block_size = 0;
	//block_size = static_cast<int>( *memory_block );
	/*int block_size = static_cast<int>(	static_cast<unsigned char>(memory_block[0]) << 24 |
										static_cast<unsigned char>(memory_block[1]) << 16 |
										static_cast<unsigned char>(memory_block[2]) << 8 |
										static_cast<unsigned char>(memory_block[3]));
										*/
	string block_name;

	//eof()

	/////////////////////
	
	file.close();
  	return 0;
}
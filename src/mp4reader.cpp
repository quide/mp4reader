/***********************************
 *     ****** MP4 Reader ******
 *  lists mp4 boxes and extracts MDAT box 
 *  (Supports: Linux/OS X/Windows)
 *   
 *  Usage: mp4Reader <file.mp4>
 *
 *  NOTES:
 *		. VS2015 supports C++14
 *		. g++-4.9 has -std=c++14
 *		. Xcode 
 *  
 *  TODO: . add unit tests
 *        . test at Linux
 *        . test at MacOS
 *        . unified schema to generate multiplatform projects? (like MPC)
 *		  . Prevent: 
 *				    . system endianness
 *					. UTF-8 named files and data
 *					. unsigned char and unsigned int?
 ***********************************/ 

#include <fstream> 
#include <iostream>
#include <string>
#include <memory>

using namespace std; 

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

	const int blocks_size{ 4 };
	unique_ptr<char[]> memory_block(new char[blocks_size]);
	file.read(memory_block.get(), blocks_size);
	
	//int block_size = 0;
	//block_size = static_cast<int>( *memory_block );
	int block_size = static_cast<int>(	static_cast<unsigned char>(memory_block[0]) << 24 |
										static_cast<unsigned char>(memory_block[1]) << 16 |
										static_cast<unsigned char>(memory_block[2]) << 8 |
										static_cast<unsigned char>(memory_block[3]));

	string block_name;

	//eof()

	/////////////////////
	
	file.close();
  	return 0;
}
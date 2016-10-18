/***********************************
 *     ****** MP4 Reader ******
 *  lists mp4 boxes and extracts MDAT box 
 *  (Supports: Linux/OS X/Windows)
 *   
 *  Usage: mp4Reader <file.mp4>
 *
 *  NOTES on C++14:
 *		. Windows:  VS2015 supports C++14
 *		. Linux  :  g++-5 has -std=c++14 ( use g++-5 instead of g++ in order to support put_time()) )
 *		. MacOS  :  e.g.: Xcode (use Clang in C++14 mode with the -std=c++14).	 
 *					In my case I did:
 *                  	    ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
 *                          brew install gcc --without-multilib
 *                          make 
 *  
 *  TODO: . create a controller class and remove logic from main (this also helps for creating unit tests)
 *        . add unit tests
 *        . test at MacOS
 *        . unified schema to generate multiplatform projects? (like MPC)
 *		  . Prevent problems related to: 
 *				    . system endianness
 *					. UTF-8 named files and data
 *					. check unsigned char and unsigned int (uint32_t) convertions and portability
 ***********************************/ 
#define _CRT_SECURE_NO_WARNINGS		// in order to use localtime() // TODO: remove this line; use strftime() instead
#include <fstream> 
#include <iostream>
#include <string>
#include <memory>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "rapidxml-1.13/rapidxml.hpp"
#include <vector>

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

#include "base64/base64.h"

using namespace std; 

//#include "box.h"


#define BLOCK2INT(c)       \
   static_cast<uint32_t>(((static_cast<unsigned char>(c[0]))<<24) | \
						 ((static_cast<unsigned char>(c[1]))<<16) | \
						 ((static_cast<unsigned char>(c[2]))<< 8) | \
						 ((static_cast<unsigned char>(c[3]))    )     )


void usage( const string&  app_name ){
	cout << "Usage:" << endl << "\t" << app_name << " <file.mp4>" << endl;
}


string current_timestamp() {
	auto now = chrono::system_clock::now();
	time_t now_t = chrono::system_clock::to_time_t(now);
	auto now_round = chrono::system_clock::from_time_t(now_t);
	chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(now - now_round);

	ostringstream os;
	os << put_time(localtime(&now_t), "%F %T.") << setfill('0') << setw(3) << ms.count() << " ";	// TODO: use strftime() instead: http://www.cplusplus.com/reference/ctime/strftime/ | http://en.cppreference.com/w/cpp/chrono/c/strftime
	string ret(os.str());
	return ret;
}

void extract_images(const string& content) {
	cout << current_timestamp() << "Extracting images from XML." << endl;
	// Format: https://www.smpte.org/sites/default/files/st2052-1-2010.pdf

	// make a safe-to-modify copy of input_xml
	vector<char> xml_copy(content.begin(), content.end());
	xml_copy.push_back('\0');
	
	rapidxml::xml_document<> doc;    // character type defaults to char
	doc.parse<rapidxml::parse_validate_xmlns /*rapidxml::parse_declaration_node | rapidxml::parse_no_data_nodes*/>( &xml_copy[0] /*const_cast<char*>(content.c_str())*/ );    // 0 means default parse flags

	rapidxml::xml_node<> *node = doc.first_node("tt", R"(http://www.w3.org/ns/ttml)");
	if (node) {
		node = node->first_node("head");
		if (node) {
			node = node->first_node("metadata");
			if (node) {
				string filename{ "extracted_image" };
				int img_counter = 1;

				node = node->first_node("image", R"(http://www.smpte-ra.org/schemas/2052-1/2010/smpte-tt)");
				while (node) {
					if (node) {
						ostringstream os;

						rapidxml::xml_attribute<>* attr = node->first_attribute("imagetype");
						string image_type{ "png" };	// default
						if (attr) { image_type = attr->value(); }

						attr = node->first_attribute("xml:id");

						if (attr) {
							os << attr->value() << "." << image_type;
						}
						else {
							os << filename << img_counter << "." << image_type;
							++img_counter;
						}

						// TODO: avoid duplicate names (by XML mistake) (eg: add (counter) to filename when duplicate found)

						string final_filename{ os.str() };
						ofstream file(final_filename, ios::binary);

						if (file  &&  file.is_open()) {
							string image{ node->value() };
							// clean the data
							image.erase(image.begin(), find_if(image.begin(), image.end(), not1(ptr_fun<int, int>(std::isspace))));
							image.erase(image.find_last_not_of(" \n\r\t") + 1);

							attr = node->first_attribute("encoding");
							if( attr  &&  string(attr->value()) == "Base64" ){
								image = base64_decode(image);
							}

							file << image;
							cout << current_timestamp() << "File " << final_filename << " created." << endl;
						}
						else {
							cout << current_timestamp() << "File " << final_filename << " not possible to be created." << endl;
						}
					}
					
					node = node->next_sibling("image");
				}
			}
		}
	}
}


int main(int argc, char* argv[]) {
	string app_name(argv[0]);
	int ret_code = 0;

	if (argc != 2) {
		cout << current_timestamp() << "File not correctly inserted" << ((argc == 1) ? " (lacking)." : ".") << endl;
		usage(app_name);
		return -1;
	}

	string filename{ argv[1] };
	ifstream file(filename, ios::binary | ios::ate);	// open file at the end of it (to determine file size)

	if( file  &&  file.is_open() ){ 
		cout << current_timestamp() << "Successfully loaded file " << filename << endl;
	}
	else{
		cout << current_timestamp() << "File " << filename << " not possible to be open." << endl;
		usage(app_name);
		return -2;
	}

	uint64_t file_size = file.tellg(); 	// detect full file size
	file.seekg(0, ios::beg);		// get ready to read from the beginning


	bool work_to_do = true;

	const int BLOCKS_SIZE{ 4 };
	char memory_block[BLOCKS_SIZE];

	uint64_t box_address = 0;
	uint32_t box_size = 0;
	uint32_t box_type = 0;

	const int CHUNK_SIZE{ 50 * 1024 * 1024 };
	unique_ptr<char[]>  chunk_of_content(new char[CHUNK_SIZE]);	// read content in maximum chunks of 50MB (prevents big data at once into RAM)
	string content;

	while( work_to_do ){
		box_address = file.tellg();

		if( file.eof()  ||										// terminate if we are at the end of the file 
			box_address >= file_size - (2 * BLOCKS_SIZE + 1) ){	// or it lacks less than a min box size for the end of the file
			cout << current_timestamp() << "All file was read." << endl;
			work_to_do = false;
			break;
		}
		
		if (file.fail()) {
			cout << current_timestamp() << u8R"("fail" error occurred.)" << endl;
			work_to_do = false;
			ret_code = -1;
			break;
		}
		if (file.bad()) {
			cout << current_timestamp() << u8R"("bad" error occurred.)" << endl;
			work_to_do = false;
			ret_code = -2;
			break;
		}

		file.read(memory_block, BLOCKS_SIZE);
		box_size = BLOCK2INT(memory_block);	// we are supporting boxes with 4,294,967,295 bytes max (32 bit representation)

		file.read(memory_block, BLOCKS_SIZE);
		box_type = BLOCK2INT(memory_block);
		string box_type_s(memory_block, BLOCKS_SIZE);

		//box new_box(box_address, box_size, box_type);
		cout << current_timestamp() << "Found box of type " << box_type_s << " and size " << box_size << endl;

		switch (box_type) {
			case BLOCK2INT("moof"):	// these only contain sub-boxes,
			case BLOCK2INT("traf"):	// just go to the next sub-box
				break;
			case BLOCK2INT("mdat"): {
				cout << current_timestamp() << "Content of mdat box is: " << endl;
				uint32_t l = box_size - 2 * BLOCKS_SIZE;
				uint32_t chunk_size = ((l > CHUNK_SIZE) ? CHUNK_SIZE : l);
				for (; l > 0; l -= chunk_size) {
					chunk_size = ((l > CHUNK_SIZE) ? CHUNK_SIZE : l);
					file.read(chunk_of_content.get(), chunk_size);
					string content_chunk(chunk_of_content.get(), chunk_size);
					cout << content_chunk;
					extract_images(content_chunk);	// TODO: glue up images if divided by several chunks; implement XML chunks reading (eg. extract valid sub XMLs to be parsed)
					// TODO: check intermediate reading file errors
				}
				cout << endl;
				break; }
			default:				// all other box types don't contain sub-boxes
				file.seekg( box_size - 2 * BLOCKS_SIZE,  ios::cur ); // skip their content
				// TODO: detect when reading was badly done because of wrong box size info
				break;
		}
	}

  	return ret_code;
}

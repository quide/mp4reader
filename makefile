mp4reader: 
	mkdir -p bin && \
	g++-5 -std=c++14 -o bin/mp4reader src/mp4reader.cpp src/base64/base64.cpp -I src/ -I src/base64 -I src/rapidxml-1.13

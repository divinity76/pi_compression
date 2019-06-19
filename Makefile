# i don't know how to make makefiles.

pi_compressor: ./compressor/compressor.cpp
	g++ $< -std=c++17 -lstdc++fs -Ofast -o $@
	g++ ./compressor/decompressor.cpp -std=c++17 -lstdc++fs -Ofast -o pi_decompressor


clean:
	rm -rfv pi_compressor pi_decompressor

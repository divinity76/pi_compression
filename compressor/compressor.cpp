#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <algorithm>
#include <assert.h>
#include <limits>
#include <cstring>
#include <sstream>
#if !defined(HETOBE16)
#if !defined(__BYTE_ORDER__)
#error Failed to detect byte order!
#endif
#if __BYTE_ORDER__ == __BIG_ENDIAN
#define HETOBE64(x) (x)
#define HETOLE64(x) __bswap_constant_64(x)
#define HETOBE32(x) (x)
#define HETOLE32(x) __bswap_constant_32(x)
#define HETOBE16(x) (x)
#define HETOLE16(x) __bswap_constant_16(x)
//
#define LETOHE64(x) __bswap_constant_64(x)
#define BETOHE64(x) (x)
#define LETOHE32(x) __bswap_constant_32(x)
#define BETOHE32(x) (x)
#define LETOHE16(x) __bswap_constant_16(x)
#define BETOHE16(x) (x)
#else
#if __BYTE_ORDER__ == __LITTLE_ENDIAN
#define HETOBE64(x) __bswap_constant_64(x)
#define HETOLE64(x) (x)
#define HETOBE32(x) __bswap_constant_32(x)
#define HETOLE32(x) (x)
#define HETOBE16(x) __bswap_constant_16(x)
#define HETOLE16(x) (x)
//
#define LETOHE64(x) (x)
#define BETOHE64(x) __bswap_constant_64(x)
#define LETOHE32(x) (x)
#define BETOHE32(x) __bswap_constant_32(x)
#define LETOHE16(x) (x)
#define BETOHE16(x) __bswap_constant_16(x)
#else
#error Failed to detect byte order! appears to be neither big endian nor little endian..
#endif
#endif
#endif


static std::string file_get_contents(const std::string &$filename)
{
    std::ostringstream ss;
    std::ifstream ifs($filename,std::ifstream::binary);
    ifs.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
    ss << ifs.rdbuf();
    return ss.str();
}
int main(int argc, char *argv[])
{
    if(argc !=4)
    {
        std::cerr << "usage: " << argv[0] << " \"pi_file.WTF\" \"file_to_compress\" \"save_path\"" << std::endl;
        std::cerr << "(" << (argc-1) << " arguments provided but exactly 3 arguments required.)" << std::endl;
        exit(EXIT_FAILURE);
    }
    const std::string pi_file=argv[1];
    const uint_fast32_t max_loop=(uint_fast32_t)std::min(uint64_t(UINT32_MAX),uint64_t(std::filesystem::file_size(pi_file)));
    if(max_loop < UINT32_MAX)
    {
        std::cerr << "Warning: pi file is small, it should be slightly larger than "<< UINT32_MAX << " bytes for better compression! (UINT32_MAX)" << std::endl;
    }
    const std::string pi_binary=file_get_contents(pi_file);
    assert(max_loop <= pi_binary.size());
    const std::string file_to_compress=argv[2];
    const size_t file_to_compress_size=std::filesystem::file_size(file_to_compress);
    std::string compress_binary=file_get_contents(file_to_compress);
    const std::string save_path=argv[3];
    if(std::filesystem::exists(save_path))
    {
        std::cerr << "Error: save file already exists! delete it first" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::ofstream outfile (save_path,std::ofstream::binary);
    outfile.exceptions ( std::ofstream::failbit | std::ofstream::badbit );
    char packed[sizeof(uint32_t)+sizeof(uint8_t)];
    while(compress_binary.size()>0)
    {
        uint_fast32_t best_offset=0;
        uint_fast8_t best_offset_matches=0;
        for(uint_fast32_t pi_offset=0; pi_offset<max_loop; ++pi_offset)
        {
            uint_fast8_t matches=0;
            for(uint_fast32_t binary_offset=0; binary_offset<compress_binary.size(); ++binary_offset)
            {
                if(compress_binary[binary_offset]!=pi_binary[pi_offset+binary_offset] || matches>=UINT8_MAX)
                {
                    break;
                }
                ++matches;
            }
            if(matches>best_offset_matches)
            {
                best_offset=pi_offset;
                best_offset_matches=matches;
            }
        }
        std::cout << "offset: " << best_offset << " matches: " << uint_fast16_t(best_offset_matches) << std::endl;
        // optimized for little-endian...
        const uint32_t pack32_offset=HETOLE32(uint32_t(best_offset));
        memcpy(&packed[0],&pack32_offset,sizeof(pack32_offset));
        const uint8_t pack8_matches=uint8_t(best_offset_matches);
        memcpy(&packed[sizeof(pack32_offset)],&pack8_matches,sizeof(pack8_matches));
        outfile.write(packed, sizeof(packed));
        compress_binary.erase(0,best_offset_matches);
    }
}

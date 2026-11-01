#include "core.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace rescateusb {
void debug_log(bool enabled,const std::string& message){if(enabled)std::cerr<<"debug: "<<message<<'\n';}
Report analyze_image(const std::string& path,bool debug){debug_log(debug,"opening input read-only: "+path);std::ifstream file(path,std::ios::binary);if(!file)throw std::runtime_error("Cannot open input image");file.seekg(0,std::ios::end);auto end=file.tellg();if(end<0)throw std::runtime_error("Cannot determine image size");Report report;report.size_bytes=uint64_t(end);file.seekg(0);Sha256 sha;std::vector<uint8_t> buffer(1024*1024);uint64_t processed=0;while(file){file.read(reinterpret_cast<char*>(buffer.data()),std::streamsize(buffer.size()));auto count=file.gcount();if(count>0){sha.update(buffer.data(),size_t(count));processed+=uint64_t(count);}}if(!file.eof())throw std::runtime_error("Error while reading image");report.sha256=sha.finish();debug_log(debug,"hashed "+std::to_string(processed)+" bytes");if(report.size_bytes<kSectorSize){report.detail="Image is smaller than one 512-byte sector";return report;}std::array<uint8_t,512> sector{};if(!read_at(file,0,sector.data(),sector.size()))throw std::runtime_error("Cannot read first sector");analyze_partition_tables(file,sector,report,debug);return report;}
}

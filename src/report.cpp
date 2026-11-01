#include "core.hpp"
#include <iomanip>
#include <sstream>

namespace rescateusb {
namespace {
std::string escape_json(const std::string& value){std::ostringstream out;for(unsigned char c:value){switch(c){case '\\':out<<"\\\\";break;case '"':out<<"\\\"";break;case '\n':out<<"\\n";break;case '\r':out<<"\\r";break;case '\t':out<<"\\t";break;default:if(c<32)out<<"\\u"<<std::hex<<std::setw(4)<<std::setfill('0')<<unsigned(c)<<std::dec;else out<<char(c);}}return out.str();}
}
std::string report_json(const std::string& path,const Report& report,bool has_expected,bool hash_match){std::ostringstream out;out<<"{\"file\":\""<<escape_json(path)<<"\",\"size_bytes\":"<<report.size_bytes<<",\"sha256\":\""<<report.sha256<<"\",\"partition_table\":\""<<escape_json(report.format)<<"\",\"partitions\":"<<report.partitions<<",\"valid\":"<<(report.valid?"true":"false")<<",\"detail\":\""<<escape_json(report.detail)<<"\",\"expected_sha256_match\":";if(!has_expected)out<<"null";else out<<(hash_match?"true":"false");return out.str()+"}";}
}

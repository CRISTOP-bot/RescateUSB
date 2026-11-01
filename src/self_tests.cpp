#include "core.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace rescateusb {
namespace {
void put32(uint8_t* p,uint32_t v){for(int i=0;i<4;i++)p[i]=uint8_t(v>>(8*i));}
void put64(uint8_t* p,uint64_t v){for(int i=0;i<8;i++)p[i]=uint8_t(v>>(8*i));}
void expect(bool condition,const char* message){if(!condition)throw std::runtime_error(std::string("Self-test failed: ")+message);}
void save_image(const std::string& path,const std::vector<uint8_t>& image){std::ofstream out(path,std::ios::binary);out.write(reinterpret_cast<const char*>(image.data()),std::streamsize(image.size()));if(!out)throw std::runtime_error("Cannot create self-test image");}
}
void run_self_tests(){const std::string path="rescateusb-selftest.img";Sha256 empty,abc;expect(empty.finish()=="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855","SHA-256 empty vector");std::string text="abc";abc.update(reinterpret_cast<const uint8_t*>(text.data()),text.size());expect(abc.finish()=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad","SHA-256 abc vector");std::vector<uint8_t> mbr(200*512);mbr[510]=0x55;mbr[511]=0xaa;mbr[450]=0x83;put32(mbr.data()+454,1);put32(mbr.data()+458,100);save_image(path,mbr);Report result=analyze_image(path,false);if(!(result.valid&&result.format=="MBR"&&result.partitions==1))throw std::runtime_error("Self-test failed: valid MBR; format="+result.format+" partitions="+std::to_string(result.partitions)+" size="+std::to_string(result.size_bytes)+" detail="+result.detail);mbr[466]=0x83;put32(mbr.data()+470,50);put32(mbr.data()+474,100);save_image(path,mbr);result=analyze_image(path,false);expect(!result.valid&&result.detail.find("overlap")!=std::string::npos,"overlapping MBR rejection");std::vector<uint8_t> image(100*512);image[510]=0x55;image[511]=0xaa;image[450]=0xee;put32(image.data()+454,1);put32(image.data()+458,99);std::vector<uint8_t> entries(128*128);entries[0]=0x28;put64(entries.data()+32,40);put64(entries.data()+40,50);uint32_t entries_crc=crc32(entries.data(),entries.size());std::copy(entries.begin(),entries.end(),image.begin()+2*512);std::copy(entries.begin(),entries.end(),image.begin()+67*512);auto write_header=[&](uint64_t current,uint64_t backup,uint64_t table){uint8_t* h=image.data()+current*512;std::memcpy(h,"EFI PART",8);put32(h+8,0x00010000);put32(h+12,92);put64(h+24,current);put64(h+32,backup);put64(h+40,34);put64(h+48,66);put64(h+72,table);put32(h+80,128);put32(h+84,128);put32(h+88,entries_crc);put32(h+16,crc32(h,92));};write_header(1,99,2);write_header(99,1,67);save_image(path,image);result=analyze_image(path,false);expect(result.valid&&result.format=="GPT"&&result.partitions==1,"valid GPT and backup");auto corrupted=image;corrupted[99*512+16]^=1;save_image(path,corrupted);result=analyze_image(path,false);expect(!result.valid,"corrupt GPT backup CRC");corrupted=image;corrupted[99*512+32]=2;put32(corrupted.data()+99*512+16,0);put32(corrupted.data()+99*512+16,crc32(corrupted.data()+99*512,92));save_image(path,corrupted);result=analyze_image(path,false);expect(!result.valid,"GPT primary/backup disagreement");std::remove(path.c_str());std::cout<<"Self-test passed: SHA-256 vectors; valid, overlapping MBR; valid GPT, backup CRC and metadata mismatch\n";}
}

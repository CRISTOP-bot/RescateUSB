#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {
uint32_t crc32(const uint8_t* data, size_t size) {
    uint32_t crc = 0xffffffffu;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}
uint32_t le32(const uint8_t* p) { return uint32_t(p[0]) | (uint32_t(p[1])<<8) | (uint32_t(p[2])<<16) | (uint32_t(p[3])<<24); }
uint64_t le64(const uint8_t* p) { return uint64_t(le32(p)) | (uint64_t(le32(p+4))<<32); }

class Sha256 {
    static constexpr uint32_t K[64] = {
      0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
      0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
      0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
      0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
      0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,
      0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,
      0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,
      0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,
      0xc67178f2};
    std::array<uint32_t,8> h{{0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19}};
    std::array<uint8_t,64> block{}; size_t used=0; uint64_t bytes=0;
    static uint32_t rotr(uint32_t x,int n){return (x>>n)|(x<<(32-n));}
    void transform(const uint8_t* b) {
        uint32_t w[64]; for(int i=0;i<16;i++) w[i]=(uint32_t(b[i*4])<<24)|(uint32_t(b[i*4+1])<<16)|(uint32_t(b[i*4+2])<<8)|b[i*4+3];
        for(int i=16;i<64;i++){uint32_t s0=rotr(w[i-15],7)^rotr(w[i-15],18)^(w[i-15]>>3);uint32_t s1=rotr(w[i-2],17)^rotr(w[i-2],19)^(w[i-2]>>10);w[i]=w[i-16]+s0+w[i-7]+s1;}
        auto [a0,b0,c0,d0,e0,f0,g0,h0]=h; uint32_t a=a0,bv=b0,c=c0,d=d0,e=e0,f=f0,g=g0,hh=h0;
        for(int i=0;i<64;i++){uint32_t S1=rotr(e,6)^rotr(e,11)^rotr(e,25), ch=(e&f)^(~e&g), t1=hh+S1+ch+K[i]+w[i];uint32_t S0=rotr(a,2)^rotr(a,13)^rotr(a,22), maj=(a&bv)^(a&c)^(bv&c),t2=S0+maj;hh=g;g=f;f=e;e=d+t1;d=c;c=bv;bv=a;a=t1+t2;}
        h[0]+=a;h[1]+=bv;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=hh;
    }
public:
    void update(const uint8_t* data,size_t n){bytes+=n;while(n){size_t take=std::min(n,64-used);std::memcpy(block.data()+used,data,take);used+=take;data+=take;n-=take;if(used==64){transform(block.data());used=0;}}}
    std::string finish(){uint64_t bits=bytes*8;block[used++]=0x80;if(used>56){while(used<64)block[used++]=0;transform(block.data());used=0;}while(used<56)block[used++]=0;for(int i=7;i>=0;i--)block[used++]=uint8_t(bits>>(i*8));transform(block.data());std::ostringstream out;out<<std::hex<<std::setfill('0');for(uint32_t v:h)out<<std::setw(8)<<v;return out.str();}
};

struct Report { std::string format="Unknown", detail="No recognized partition table", sha; uint64_t size=0; bool valid=false; };
Report analyze(const std::string& path) {
    std::ifstream f(path,std::ios::binary); if(!f) throw std::runtime_error("Cannot open input image");
    f.seekg(0,std::ios::end); auto end=f.tellg(); if(end<0) throw std::runtime_error("Cannot determine image size");
    Report r; r.size=static_cast<uint64_t>(end); f.seekg(0);
    Sha256 sha; std::vector<uint8_t> buf(1024*1024); while(f){f.read(reinterpret_cast<char*>(buf.data()),buf.size());auto got=f.gcount();if(got>0)sha.update(buf.data(),static_cast<size_t>(got));}
    if(!f.eof()) throw std::runtime_error("Error while reading image"); r.sha=sha.finish();
    if(r.size<512) {r.detail="Image is smaller than one 512-byte sector";return r;}
    std::array<uint8_t,512> mbr{}; f.clear();f.seekg(0);f.read(reinterpret_cast<char*>(mbr.data()),mbr.size()); if(f.gcount()!=512)throw std::runtime_error("Cannot read first sector");
    bool mbrsig=mbr[510]==0x55&&mbr[511]==0xaa;
    if(r.size>=1024){std::array<uint8_t,512> gh{};f.clear();f.seekg(512);f.read(reinterpret_cast<char*>(gh.data()),gh.size()); if(f.gcount()==512&&std::memcmp(gh.data(),"EFI PART",8)==0){
        r.format="GPT";uint32_t hsize=le32(&gh[12]), saved=le32(&gh[16]);
        if(hsize<92||hsize>512){r.detail="Invalid GPT header size";return r;}
        auto header=gh;std::fill(header.begin()+16,header.begin()+20,0);
        if(crc32(header.data(),hsize)!=saved){r.detail="GPT header checksum mismatch";return r;}
        uint64_t first=le64(&gh[40]),last=le64(&gh[48]),entries_lba=le64(&gh[72]);uint32_t count=le32(&gh[80]),entry_size=le32(&gh[84]),entries_crc=le32(&gh[88]);
        if(entry_size<128||entry_size%8||count==0||count>16384){r.detail="Invalid GPT partition-entry geometry";return r;}
        if(first>last||last>=r.size/512){r.detail="GPT partition bounds exceed image size";return r;}
        uint64_t bytes=uint64_t(count)*entry_size, offset=entries_lba*512;
        if(entries_lba>std::numeric_limits<uint64_t>::max()/512||offset>r.size||bytes>r.size-offset||bytes>64*1024*1024){r.detail="GPT partition-entry table is outside image or too large";return r;}
        std::vector<uint8_t> entries(static_cast<size_t>(bytes));f.clear();f.seekg(static_cast<std::streamoff>(offset));f.read(reinterpret_cast<char*>(entries.data()),static_cast<std::streamsize>(entries.size()));if(static_cast<size_t>(f.gcount())!=entries.size()){r.detail="Could not read GPT partition-entry table";return r;}
        if(crc32(entries.data(),entries.size())!=entries_crc){r.detail="GPT partition-entry checksum mismatch";return r;}
        r.valid=true;r.detail="GPT header and partition-entry checksums are valid";return r;
    }}
    if(mbrsig){r.format="MBR";unsigned active=0,parts=0;for(int i=0;i<4;i++){const uint8_t* e=&mbr[446+i*16];if(e[0]!=0&&e[0]!=0x80){r.detail="Invalid MBR boot flag";return r;}if(e[0]==0x80)active++;uint32_t start=le32(e+8),len=le32(e+12);if(len){parts++;if(uint64_t(start)+len>r.size/512){r.detail="MBR partition exceeds image size";return r;}}}if(active>1){r.detail="Multiple active MBR partitions";return r;}r.valid=true;r.detail=std::to_string(parts)+" MBR partition(s); signature and bounds are valid";return r;}
    r.detail="No valid MBR signature or GPT header found";return r;
}

void self_test(){
    const std::string p="rescateusb-selftest.img";std::array<uint8_t,512> m{};m[510]=0x55;m[511]=0xaa;{std::ofstream o(p,std::ios::binary);o.write(reinterpret_cast<const char*>(m.data()),m.size());}
    auto r=analyze(p);std::remove(p.c_str());if(!r.valid||r.format!="MBR"||r.sha.size()!=64)throw std::runtime_error("Self-test failed: valid MBR image");
    m[510]=0;{std::ofstream o(p,std::ios::binary);o.write(reinterpret_cast<const char*>(m.data()),m.size());}r=analyze(p);std::remove(p.c_str());if(r.valid)throw std::runtime_error("Self-test failed: invalid image was accepted");
    std::cout<<"Self-test passed: MBR detection, invalid-image rejection, SHA-256 output\n";
}
void usage(){std::cerr<<"Rescate USB 0.1.0 — read-only disk image diagnostics\nUsage: rescateusb <image-file> [--expected-sha256 HEX]\n       rescateusb --self-test\n";}
}
int main(int argc,char** argv){try{if(argc==2&&std::string(argv[1])=="--self-test"){self_test();return 0;}if(argc!=2&&argc!=4){usage();return 2;}if(argc==4&&std::string(argv[2])!="--expected-sha256"){usage();return 2;}Report r=analyze(argv[1]);std::cout<<"Rescate USB — diagnóstico de solo lectura\nArchivo: "<<argv[1]<<"\nTamaño: "<<r.size<<" bytes\nSHA-256: "<<r.sha<<"\nTabla: "<<r.format<<"\nResultado: "<<(r.valid?"VÁLIDO":"REVISAR")<<" — "<<r.detail<<"\n";if(argc==4){std::string expected=argv[3];for(char& c:expected)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));bool ok=expected.size()==64&&expected==r.sha;std::cout<<"Hash esperado: "<<(ok?"coincide":"NO coincide")<<"\n";if(!ok)return 3;}return r.valid?0:1;}catch(const std::exception& e){std::cerr<<"Error: "<<e.what()<<"\n";return 2;}}

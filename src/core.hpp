#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace rescateusb {
constexpr uint64_t kSectorSize = 512;
struct Report {
    std::string format = "Unknown";
    std::string detail = "No recognized partition table";
    std::string sha256;
    uint64_t size_bytes = 0;
    bool valid = false;
    unsigned partitions = 0;
};

class Sha256 {
public:
    void update(const uint8_t* data, size_t size);
    std::string finish();
private:
    void transform(const uint8_t* block);
    uint32_t state_[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    uint8_t block_[64]{};
    size_t used_ = 0;
    uint64_t bytes_ = 0;
};

uint32_t crc32(const uint8_t* data, size_t size);
bool read_at(std::ifstream& file, uint64_t offset, uint8_t* destination, size_t size);
void analyze_partition_tables(std::ifstream& file, const std::array<uint8_t, 512>& first_sector, Report& report, bool debug);
Report analyze_image(const std::string& path, bool debug);
std::string report_json(const std::string& path, const Report& report, bool has_expected, bool hash_match);
void run_self_tests();
void debug_log(bool enabled, const std::string& message);
}

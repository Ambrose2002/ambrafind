
#include <cstdint>

typedef struct {
    uint32_t token_offset;
    uint32_t postings_offset;
    uint32_t postings_count;
} DictEntry;

typedef struct {
    uint32_t filename_offset;
    uint32_t path_offset;
    uint32_t ext_offset;
    uint64_t size;
    int64_t mtime;
} FileRecord;

typedef struct {
    uint32_t file_id;
    uint8_t field_mask;
} Posting;
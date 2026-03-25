#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <stdint.h>

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

typedef struct {
    char magic[4];
    uint32_t version;
    uint32_t file_count;
    uint32_t dict_count;
    uint32_t postings_count;
    uint32_t file_records_offset;
    uint32_t string_blob_offset;
    uint32_t postings_offset;
} IndexHeader;


typedef struct {
    IndexHeader header;
    FileRecord *files;
    char *string_blob;
    DictEntry *dict;
    Posting *postings;
} LoadedIndex;

void getFilePaths(const char *base);
const char *get_output(void);
long get_file_count(void);
void clear_file_paths(void);

#endif // MY_FUNCTIONS_H

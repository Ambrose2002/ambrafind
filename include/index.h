#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <stddef.h>
#include <stdint.h>

#ifdef DEBUG
	#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
	#define DEBUG_PRINT(...) // Does nothing in release mode
#endif

typedef struct {
	uint64_t file_id;
	char *path;
	char *filename;
	char *ext;
	uint64_t size;
	int64_t mtime;
} IndexedFile;

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
	Posting *data;
	size_t len;
	size_t cap;
} PostingVec;

typedef struct {
	char *token;
	PostingVec postings;
} TokenEntry;

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

typedef struct {
	uint64_t file_id;
	char* filepath;
} TokenValue;

typedef struct {
	char* key;
	TokenValue* value;
} TokenMap;

void build_blob(const char *base);
const char *get_output(void);
const FileRecord *get_file_records(void);
long get_file_count(void);
void clear_file_paths(void);
size_t get_output_len(void);
void print_blob_and_records(const char *blob);
void print_my_map(TokenMap *map);
void build_path_map(IndexedFile ifile);
TokenMap* get_token_map(void);

#endif // MY_FUNCTIONS_H

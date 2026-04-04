#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

#include <stddef.h>
#include <stdint.h>

#ifdef DEBUG
	#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
	#define DEBUG_PRINT(...)
#endif

/**
 * Lightweight file metadata used while building token postings.
 */
typedef struct {
	uint64_t file_id;
	char *path;
	char *filename;
	char *ext;
	uint64_t size;
	int64_t mtime;
} IndexedFile;

/**
 * Dictionary entry for an indexed token.
 * Offsets point into the token string blob and postings array.
 */
typedef struct {
	uint32_t token_offset;
	uint32_t postings_offset;
	uint32_t postings_count;
} DictEntry;

/**
 * Serialized metadata for a single indexed file.
 * String fields are offsets into the string blob.
 */
typedef struct {
	uint32_t filename_offset;
	uint32_t path_offset;
	uint32_t ext_offset;
	uint64_t size;
	int64_t mtime;
} FileRecord;

/**
 * Posting entry mapping a token to a file and the matched field.
 *
 * field_mask bits currently used:
 * - 0b001: extension match
 * - 0b010: filename stem match
 * - 0b100: path segment match
 */
typedef struct {
	uint32_t file_id;
	uint8_t field_mask;
} Posting;

/**
 * Hash table entry for token -> postings list.
 */
typedef struct {
	char *key;
	Posting* value;
} TokenEntry;

/**
 * Header for the on-disk index format.
 */
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

/**
 * In-memory representation of a loaded index file.
 */
typedef struct {
	IndexHeader header;
	FileRecord *files;
	char *string_blob;
	DictEntry *dict;
	Posting *postings;
} LoadedIndex;

/**
 * Recursively walks the directory rooted at `base` and appends discovered file
 * strings and records to internal buffers.
 */
void build_blob(const char *base);

/**
 * Returns the internal string blob buffer built by `build_blob`/`build_index`.
 * The pointer remains valid until `clear_file_paths` is called.
 */
const char *get_output(void);

/**
 * Returns the internal file record array.
 * The pointer remains valid until `clear_file_paths` is called.
 */
const FileRecord *get_file_records(void);

/**
 * Returns the number of entries currently stored in the file record array.
 */
long get_file_count(void);

/**
 * Releases all internal index-building allocations and resets global state.
 */
void clear_file_paths(void);

/**
 * Returns the current size in bytes of the internal string blob buffer.
 */
size_t get_output_len(void);

/**
 * Debug helper that prints the string blob and all file records to stdout.
 */
void print_blob_and_records(const char *blob);

/**
 * Debug helper that prints token map keys and postings to stdout.
 */
void print_my_map(TokenEntry *map);

/**
 * Tokenizes an indexed file path and updates the global token map postings.
 */
void build_path_map(IndexedFile ifile);

/**
 * Returns the global token map pointer used during index construction.
 */
TokenEntry *get_token_map(void);

/**
 * Builds index state for `base` and sorts token entries lexicographically.
 */
void build_index(const char *base);

#endif // MY_FUNCTIONS_H

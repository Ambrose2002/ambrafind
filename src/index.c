
#include <dirent.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syslimits.h>

#include "../include/index.h"
#include "../lib/stb_ds.h"

static char *output = NULL;
static size_t output_len = 0;
static size_t output_cap = 0;

static FileRecord *file_records = NULL;
static size_t file_records_len = 0;
static size_t file_records_cap = 0;

static Posting *postings = NULL;
static size_t postings_len = 0;
static size_t postings_cap = 0;

static DictEntry *dict_entries = NULL;
static size_t dict_entry_len = 0;
static size_t dict_entry_cap = 0;

static uint64_t curr_file_id = 0;

TokenEntry *token_map = NULL;

static char *duplicate_string(const char *src) {
	size_t len = strlen(src) + 1;
	char *copy = malloc(len);
	if (copy != NULL) {
		memcpy(copy, src, len);
	}
	return copy;
}

void print_my_map(TokenEntry *map) {
	if (map == NULL) {
		printf("(token map is empty)\n");
		return;
	}

	for (ptrdiff_t i = 0; i < shlen(map); ++i) {
		printf("Key: %s\n", map[i].key);

		ptrdiff_t count = arrlen(map[i].value);
		for (ptrdiff_t j = 0; j < count; ++j) {
			printf("  [%td] ID: %d, Field_Mask: %d\n", j,
			       map[i].value[j].file_id, map[i].value[j].field_mask);
		}
	}
}

void print_blob_and_records(const char *blob) {
	size_t blob_len = get_output_len();
	const FileRecord *recs = get_file_records();
	long n = get_file_count();

	printf("string_blob (%zu bytes):\n", blob_len);
	for (size_t i = 0; i < blob_len; i++) {
		unsigned char c = (unsigned char)blob[i];
		if (c == '\0')
			printf("\\0\n");
		else
			putchar(c);
	}

	printf("\nfile_records (%ld):\n", n);
	for (long i = 0; i < n; i++) {
		const FileRecord *r = &recs[i];
		printf("[%ld] fn@%u=\"%s\" path@%u=\"%s\" ext@%u=\"%s\" size=%" PRIu64
		       " mtime=%" PRId64 "\n",
		       i, r->filename_offset, blob + r->filename_offset, r->path_offset,
		       blob + r->path_offset, r->ext_offset, blob + r->ext_offset,
		       r->size, r->mtime);
	}
}

int token_cmpr(const void *a, const void *b) {
	const TokenEntry *t_a = (const TokenEntry *)a;
	const TokenEntry *t_b = (const TokenEntry *)b;

	return strcmp(t_a->key, t_b->key);
}

static int append_string(const char *value, uint32_t *offset) {
	size_t value_len = strlen(value);
	size_t needed = output_len + value_len + 1; // terminator

	if (needed > output_cap) {
		size_t new_cap = output_cap == 0 ? 1024 : output_cap;
		while (new_cap < needed) {
			new_cap *= 2;
		}

		char *new_output = realloc(output, new_cap);
		if (new_output == NULL) {
			return -1;
		}

		output = new_output;
		output_cap = new_cap;
	}

	if (offset != NULL) {
		if (output_len > UINT32_MAX) {
			return -1;
		}
		*offset = (uint32_t)output_len;
	}

	memcpy(output + output_len, value, value_len + 1);
	output_len += value_len + 1;
	return 0;
}

static int append_file_record(const FileRecord *record) {
	if (file_records_len == file_records_cap) {
		size_t new_cap = file_records_cap == 0 ? 128 : file_records_cap * 2;
		FileRecord *new_records =
		    realloc(file_records, new_cap * sizeof(FileRecord));
		if (new_records == NULL) {
			return -1;
		}

		file_records = new_records;
		file_records_cap = new_cap;
	}

	file_records[file_records_len++] = *record;
	return 0;
}

void build_path_map(IndexedFile ifile) {
	char path[PATH_MAX];
	strcpy(path, ifile.path);
	const char delimeter[] = "/.";

	if (token_map == NULL) {
		sh_new_strdup(token_map);
	}

	char *token;

	token = strtok(path, delimeter);

	while (token != NULL) {
		TokenEntry *entry = shgetp_null(token_map, token);
		if (!entry) {
			shput(token_map, token, NULL);
			entry = shgetp(token_map, token);
		}

		Posting value = {ifile.file_id};
		uint8_t field_mask = 0b000;

		if (strcmp(token, ifile.ext) == 0) {
			field_mask |= 0b1;
		} else {
			char filename[PATH_MAX];
			snprintf(filename, sizeof(filename), "%s.%s", token, ifile.ext);
			if (strcmp(filename, ifile.filename) == 0) {
				field_mask |= 0b110;
			} else {
				field_mask |= 0b100;
			}
		}

		value.field_mask = field_mask;

		arrput(entry->value, value);

		token = strtok(NULL, delimeter);
	}
}

void build_blob(const char *base) {
	DIR *dir = opendir(base);
	struct stat st;

	if (dir == NULL) {
		perror("opendir failed");
		return;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 ||
		    strcmp(entry->d_name, "..") == 0 ||
		    strcmp(entry->d_name, ".git") == 0 ||
		    strcmp(entry->d_name, "build") == 0) {
			continue;
		}

		char path[PATH_MAX];
		int written =
		    snprintf(path, sizeof(path), "%s/%s", base, entry->d_name);
		if (written < 0 || (size_t)written >= sizeof(path)) {
			continue;
		}

		if (stat(path, &st) != 0) {
			continue;
		}

		if (S_ISDIR(st.st_mode)) {
			build_blob(path);
		} else {
			const char *filename = entry->d_name;
			const char *dot = strrchr(filename, '.');
			const char *ext = (dot != NULL && dot != filename && dot[1] != '\0')
			                      ? dot + 1
			                      : "";

			FileRecord fr;
			int code = append_string(filename, &fr.filename_offset);
			if (code != 0) {
				perror("realloc failed");
				closedir(dir);
				return;
			}

			code = append_string(path, &fr.path_offset);
			if (code != 0) {
				perror("realloc failed");
				closedir(dir);
				return;
			}

			code = append_string(ext, &fr.ext_offset);
			if (code != 0) {
				perror("realloc failed");
				closedir(dir);
				return;
			}

			fr.size = (uint64_t)st.st_size;
			fr.mtime = (int64_t)st.st_mtime;
			code = append_file_record(&fr);
			if (code != 0) {
				perror("realloc failed");
				return;
			}

			IndexedFile ifile = {curr_file_id++, (char *)path, (char *)filename,
			                     (char *)ext,    st.st_size,   st.st_mtime};
			build_path_map(ifile);
		}
	}

	closedir(dir);
}

static int append_posting(const Posting *posting) {
	if (postings_len == postings_cap) {
		size_t new_cap = postings_cap == 0 ? 128 : postings_cap * 2;
		Posting *new_postings = realloc(postings, new_cap * sizeof(Posting));
		if (new_postings == NULL) {
			return -1;
		}

		postings = new_postings;
		postings_cap = new_cap;
	}

	postings[postings_len++] = *posting;
	return 0;
}

static int append_dict_entry(const DictEntry *dict_entry) {
	if (dict_entry_len == dict_entry_cap) {
		size_t new_cap = dict_entry_cap == 0 ? 128 : dict_entry_cap * 2;
		DictEntry *new_entries =
		    realloc(dict_entries, new_cap * sizeof(DictEntry));
		if (new_entries == NULL) {
			return -1;
		}
		dict_entries = new_entries;
		dict_entry_cap = new_cap;
	}

	dict_entries[dict_entry_len++] = *dict_entry;
	return 0;
}

void build_postings_and_dict(TokenEntry *token_map) {

	uint32_t postings_offset = 0;
	int code;

	if (token_map != NULL) {
		for (ptrdiff_t i = 0; i < shlen(token_map); ++i) {
			Posting *values = token_map[i].value;
			ptrdiff_t n = arrlen(values);

			for (ptrdiff_t j = 0; j < n; ++i) {
				code = append_posting(&values[i]);

				if (code != 0) {
					perror("realloc failed");
					return;
				}
			}
			DictEntry entry;
			code = append_string(token_map[i].key, &entry.token_offset);
			if (code != 0) {
				perror("realloc failed");
				return;
			}
			entry.postings_offset = postings_offset;
			entry.postings_count = n;
			code = append_dict_entry(&entry);
			if (code != 0) {
				perror("realloc failed");
				return;
			}
		}
	}
}

void build_index(const char *base) {
	build_blob(base);
	qsort(token_map, shlen(token_map), sizeof(TokenEntry), token_cmpr);
	build_postings_and_dict(token_map);
}

const char *get_output(void) { return output; }

const FileRecord *get_file_records(void) { return file_records; }

long get_file_count(void) { return (long)file_records_len; }

size_t get_output_len(void) { return output_len; }

TokenEntry *get_token_map(void) { return token_map; }

void clear_file_paths(void) {
	if (token_map != NULL) {
		for (ptrdiff_t i = 0; i < shlen(token_map); ++i) {
			Posting *values = token_map[i].value;
			arrfree(values);
		}
		shfree(token_map);
		token_map = NULL;
	}

	free(output);
	free(file_records);
	output = NULL;
	file_records = NULL;
	output_len = 0;
	output_cap = 0;
	file_records_len = 0;
	file_records_cap = 0;
	curr_file_id = 0;
}

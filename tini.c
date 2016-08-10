/*=======================================================================================

TinyINI - small and simple open-source library for loading, saving and 
managing INI file data structures in the memory.

TinyINI is distributed under following terms and conditions:

Copyright (c) 2015-2016, Ivan Pizhenko.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ''AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL BEN HOYT BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

SPECIAL NOTICE
TinyINI library relies on the open-source INIH library 
(https://github.com/benhoyt/inih) for parsing text of INI file.
Source code of INIH library and information about it, including 
licensing conditions, is included in the subfolder inih.

=======================================================================================*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <tini/tini.h>
#include "inih/ini.h"

struct _ini_section {
	char* name;
	char** keys;
	char** values;
	size_t parameter_count;
	size_t max_parameter_count;	
};

struct _ini_file {
	ini_section** sections;
	size_t section_count;
	size_t max_section_count;
};

static size_t find_section_index(const ini_file* ini, const char* section) {
	size_t i;
	for (i = 0; i < ini->section_count; ++i) {
		if (strcmp(ini->sections[i]->name, section) == 0)
			return i + 1;
	}
	return 0;
}

static size_t find_parameter_index_in_section(const ini_section* section, const char* key) {
	size_t i;
	for (i = 0; i < section->parameter_count; ++i) {
		if(strcmp(section->keys[i], key) == 0)
			return i + 1;
	}
	return 0;
}

#ifdef TINI_FEATURE_EDIT_INI_FILE

static void remove_section_by_index(ini_file* ini, size_t index) {
	tini_free_section(ini->sections[index]);
	if(index < ini->section_count - 1) {
		memmove(ini->sections + index, ini->sections + index + 1, 
			sizeof(ini_section*) * (ini->section_count - index - 1));
	}
	--ini->section_count;	
}

#endif

static int ini_file_handler(void* user, const char* section, const char* name, 
			    const char* value)
{
	return tini_add_parameter((ini_file*)user, section, name, value, 1) == 0 
		? 1 : 0;
}

static int grow_section_storage(ini_file* ini) {
	size_t new_max_section_count = ini->max_section_count + TINI_SECTION_STORAGE_SIZE_INCREMENT;
	ini_section** new_sections = realloc(ini->sections, sizeof(ini_section*) * new_max_section_count);
	if (new_sections) {
		ini->sections = new_sections;
		ini->max_section_count = new_max_section_count;
		return 0;
	}
	return -1;
}

static void cleanup_section(ini_section* section) {
	size_t i;
	for (i = 0; i < section->parameter_count; ++i) {
		free(section->values[i]);
		free(section->keys[i]);
	}
	free(section->keys);
	free(section->name);
}

static int initialize_section(ini_section* section, const char* name) {
	int saved_errno = 0;
	section->name = strdup(name);
	if (!section->name) {
		saved_errno = errno;
		goto exit_error;
	}
	
	section->keys = malloc(sizeof(const char*) * (TINI_PARAMETER_STORAGE_INITIAL_SIZE + 1) * 2);
	if (!section->keys) {
		saved_errno = errno;
		goto cleanup_name;
	}

	section->values = section->keys + TINI_PARAMETER_STORAGE_INITIAL_SIZE + 1;
	*(section->keys) = NULL;
	*(section->values) = NULL;
	section->parameter_count = 0;
	section->max_parameter_count = TINI_PARAMETER_STORAGE_INITIAL_SIZE;
	return 0;
	
cleanup_name:
	free(section->name);
	
exit_error:
	errno = saved_errno;
	return -1;
}

static int initialize_ini(ini_file* ini) {
	ini->sections = malloc(sizeof(ini_section*) * TINI_SECTION_STORAGE_INITIAL_SIZE);
	if (ini->sections) {
		ini->section_count = 0;
		ini->max_section_count = TINI_SECTION_STORAGE_INITIAL_SIZE;
		return 0;
	}
	return -1;
}

static void cleanup_ini(ini_file* ini) {
	size_t i;
	for (i = 0; i < ini->section_count; ++i) {
		cleanup_section(ini->sections[i]);
		free(ini->sections[i]);
	}
	free(ini->sections);
}

int grow_parameter_storage(ini_section* section) {
	size_t new_max_parameter_count = section->max_parameter_count + TINI_PARAMETER_STORAGE_SIZE_INCREMENT;
	char** new_keys = realloc(section->keys, sizeof(char*) * (new_max_parameter_count + 1) * 2);
	if (new_keys) {
		char** old_values = new_keys + section->max_parameter_count + 1;
		char** new_values = new_keys + new_max_parameter_count + 1;
		memmove(new_values, old_values, sizeof(char*) * (section->parameter_count + 1));
		section->keys = new_keys;
		section->values = new_values;
		section->max_parameter_count = new_max_parameter_count;
		return 0;
	}
	return -1;	
}


ini_file* tini_create_ini(void) {
	ini_file* ini = malloc(sizeof(ini_file));
	if (ini && initialize_ini(ini) != 0) {
		int saved_errno = errno;
		free(ini);
		ini = NULL;
		errno = saved_errno;
	}
	return ini;
}

void tini_free_ini(ini_file* ini) {
	if (ini) {
		cleanup_ini(ini);
		free(ini);
	}
}

ini_file* tini_load_ini(const char* file_path) {
	ini_file* ini = tini_create_ini();
	if (ini) {
		if (ini_parse(file_path, &ini_file_handler, ini) < 0) {
			int saved_errno = errno;
			free(ini);
			ini = NULL;
			errno = saved_errno;
		}
	}
	return ini;
}

#ifdef TINI_FEATURE_SAVE_INI

int tini_save_ini(const ini_file* ini, const char* file_path) {
	int res = -1;
	FILE* f = fopen(file_path, "w");
	if (f) {
		int saved_errno;
		res = tini_dump_ini(ini, f);
		saved_errno = errno;
		fclose(f);
		errno = saved_errno;
	}
	return res;
}

#endif

#if defined(TINI_FEATURE_SAVE_INI) || defined(TINI_FEATURE_SAVE_INI)

int tini_dump_ini(const ini_file* ini, FILE* f) {
	size_t i, j;
	for (i = 0; i < ini->section_count; ++i) {
		const ini_section* s = ini->sections[i];
		if(fprintf(f, "[%s]\n", s->name) < 0)
			return -1;
		for (j = 0; j < s->parameter_count; ++j) {
			if(fprintf(f, "%s=%s\n", s->keys[j], s->values[j]) < 0)
				return -1;
		}
		if(fputc('\n', f) == EOF)
			return -1;
	}
	return 0;
}

#endif

ini_section* tini_new_section(const char* name) {
	ini_section* section = malloc(sizeof(ini_section));
	if (section) {
		if (initialize_section(section, name) != 0) {
			int saved_errno = errno;
			free(section);
			section = NULL;
			errno = saved_errno;
		}
	}
	return section;
}

void tini_free_section(ini_section* section) {
	if (section) {
		cleanup_section(section);
		free(section);
	}
}

int tini_add_parameter(ini_file* ini, const char* section, const char* key, const char* value, int replace) {
	ini_section* s = (ini_section*) tini_find_section(ini, section);
	if (s)
		return tini_add_parameter_to_section(s, key, value, replace);
	else {
		s = tini_new_section(section);
		if (s) {
			if(tini_add_parameter_to_section(s, key, value, replace) == 0) {
				if(ini->section_count < ini->max_section_count
					|| (ini->section_count == ini->max_section_count
					&& grow_section_storage(ini) == 0)) {
					ini->sections[ini->section_count++] = s;
					return 0;
				} else
					return -1;
			} else {
				int saved_errno = errno;
				tini_free_section(s);
				errno = saved_errno;
				return -1;
			}
		} else
			return -1;
	}
}

int tini_add_parameter_to_section(ini_section* section, const char* key, const char* value, int replace) {
	size_t i = find_parameter_index_in_section(section, key);
	if (i == 0) {
		char *new_key, *new_value;
		new_key = strdup(key);
		if (!new_key)
			return -1;
		new_value = strdup(value);
		if (!new_value) {
			free(new_key);
			return -1;
		}
		if (section->parameter_count < section->max_parameter_count
			|| (section->parameter_count == section->max_parameter_count
			&& grow_parameter_storage(section) == 0)) {
			section->keys[section->parameter_count] = new_key;
			section->values[section->parameter_count] = new_value;
			++section->parameter_count;
			section->keys[section->parameter_count] = NULL;
			section->values[section->parameter_count] = NULL;
			return 0;
		} else {
			free(new_value);
			free(new_key);
			return -1;
		}
	} else if (replace) {
		char* new_value = strdup(value);
		if (new_value) {
			--i;
			free(section->values[i]);
			section->values[i] = new_value;
			return 0;
		} else
			return -1;
	} else {
		errno = EEXIST;
		return -1;
	}
}

#ifdef TINI_FEATURE_EDIT_INI

int tini_remove_section(ini_file* ini, const char* section) {
	size_t i = find_section_index(ini, section);
	if (i == 0) {
		errno = ESRCH;
		return -1;
	} else {
		--i;
		remove_section_by_index(ini, i);
		return 0;
	}
}

int tini_remove_parameter(ini_file* ini, const char* section, const char* key) {	
	size_t i, j;
	i = find_section_index(ini, section);
	if (i == 0) {
		errno = ESRCH;
		return -1;
	} else {
		ini_section* s = ini->sections[--i];
		j = find_parameter_index_in_section(s, key);
		if(j == 0) {
			errno = ESRCH;
			return -1;
		} else {
			--j;
			free(s->values[j]);
			free(s->keys[j]);
			if(i < s->parameter_count - 1) {
				memmove(s->keys + i, s->keys + i + 1, 
					sizeof(char*) * (s->parameter_count - i - 1));
				memmove(s->values + i, s->values + i + 1, 
					sizeof(char*) * (s->parameter_count - i - 1));
			}
			s->keys[s->parameter_count] = NULL;
			s->values[s->parameter_count] = NULL;
			--s->parameter_count;
			return 0;
		}
	}
}

#endif

const ini_section* tini_find_section(const ini_file* ini, const char* section) {
	size_t i = find_section_index(ini, section);
	return i == 0 ? NULL : ini->sections[i - 1];
}

const char* tini_find_parameter_in_section(const ini_section* section, const char* key, const char* default_value) {
	size_t i = find_parameter_index_in_section(section, key);
	return i == 0 ? default_value : section->values[i - 1];
}

const char* tini_find_parameter(const ini_file* ini, const char* section, const char* key, const char* default_value) {
	const ini_section* s = tini_find_section(ini, section);
	return s ? tini_find_parameter_in_section(s, key, default_value) : default_value;
}

#ifdef TINI_FEATURE_GET_ELEMENT_COUNT

size_t tini_get_parameter_count(const ini_section* section) {
	return section->parameter_count;
}

size_t tini_get_section_count(const ini_file* ini) {
	return ini->section_count;
}

#endif

#ifdef TINI_FEATURE_GET_PARAMETERS_STORAGE

const char* const* tini_get_keys(const ini_section* section) {
	return (const char* const*)section->keys;
}

const char* const* tini_get_values(const ini_section* section) {
	return (const char* const*)section->values;
}

#endif

#ifdef TINI_FEATURE_GET_SECTIONS_STORAGE

const ini_section* const* tini_get_sections(const ini_file* ini) {
	return (const ini_section* const*)ini->sections;
}

#endif

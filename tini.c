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

/* INI section data structure */
struct _ini_section {
	char* name; /* section name */
	char** keys; /* array of parameter names */
	char** values; /* array of parameter values */
	size_t parameter_count; /* current number of parameters */
	size_t max_parameter_count; /* maximum number of parameters for which memory is currently allocated */
};

struct _ini_file {
	ini_section** sections; /* array of INI file sections */
	size_t section_count; /* number  of sections */
	size_t max_section_count; /* maximum number of parameters for which memory is currently allocated */
};

static size_t find_section_index(const ini_file* ini, const char* section) {
	size_t i;
	
	/* Enumerate all sections, compare section name to the given input, 
	 * return section index + 1 if match found, otherwise return zero.
	 */
	for (i = 0; i < ini->section_count; ++i) {
		if (strcmp(ini->sections[i]->name, section) == 0)
			return i + 1;
	}
	
	return 0;
}

static size_t find_parameter_index_in_section(const ini_section* section, const char* key) {
	size_t i;

	/* Enumerate all parameter names, compare parameter name to the given input, 
	 * return parameter index + 1 if match found, otherwise return zero.
	 */
	for (i = 0; i < section->parameter_count; ++i) {
		if(strcmp(section->keys[i], key) == 0)
			return i + 1;
	}

		return 0;
}

#ifdef TINI_FEATURE_EDIT_INI_FILE

static void remove_section_by_index(ini_file* ini, size_t index) {
	/* Destroy section object */
	tini_free_section(ini->sections[index]);
	
	/* Pack array of section pointers if removed section was in the beginning or middle */
	if(index < ini->section_count - 1) {
		memmove(ini->sections + index, ini->sections + index + 1, 
			sizeof(ini_section*) * (ini->section_count - index - 1));
	}
	
	/* Decrease current number of sections */
	--ini->section_count;	
}

#endif

/* INI file parsing handler for included INIH library. */
static int ini_file_handler(void* user, const char* section, const char* name, 
			    const char* value)
{
	/* Attempt to parameter to the INI file object, replace duplicates */
	return tini_add_parameter((ini_file*)user, section, name, value, 1) == 0 
		? 1 : 0;
}

static int grow_section_storage(ini_file* ini) {
	/* Find new storage size */
	size_t new_max_section_count = ini->max_section_count + TINI_SECTION_STORAGE_SIZE_INCREMENT;
	
	/* Reallocate memory */
	ini_section** new_sections = realloc(ini->sections, sizeof(ini_section*) * new_max_section_count);
	
	/* Update INI file object or indicate failure */
	if (new_sections) {
		ini->sections = new_sections;
		ini->max_section_count = new_max_section_count;
		return 0;
	}
	else return -1;	
}

static int grow_parameter_storage(ini_section* section) {
	/* Find new storage size */
	size_t new_max_parameter_count = section->max_parameter_count + TINI_PARAMETER_STORAGE_SIZE_INCREMENT;

	/* Reallocate memory */
	char** new_keys = realloc(section->keys, sizeof(char*) * (new_max_parameter_count + 1) * 2);

	/* Update INI file section object or indicate failure */
	if (new_keys) {
		char** old_values = new_keys + section->max_parameter_count + 1;
		char** new_values = new_keys + new_max_parameter_count + 1;
		memmove(new_values, old_values, sizeof(char*) * (section->parameter_count + 1));
		section->keys = new_keys;
		section->values = new_values;
		section->max_parameter_count = new_max_parameter_count;
		return 0;
	}
	else return -1;	
}

static void cleanup_section(ini_section* section) {
	/* Free memory consumed by parameter names and values */
	size_t i;
	for (i = 0; i < section->parameter_count; ++i) {
		free(section->values[i]);
		free(section->keys[i]);
	}
	
	/* Free memory consumed by parameter names and values storage */
	free(section->keys);

	/* Free memory consumed by section name */
	free(section->name);
}

static int initialize_section(ini_section* section, const char* name) {
	/* Save previous errno */
	int saved_errno = errno;

	/* Create section name*/
	section->name = strdup(name);
	if (!section->name) {
		saved_errno = errno;
		goto exit_error;
	}
	
	/* Allocate initial storage for parameter names and values */
	section->keys = malloc(sizeof(const char*) * (TINI_PARAMETER_STORAGE_INITIAL_SIZE + 1) * 2);
	if (!section->keys) {
		saved_errno = errno;
		goto cleanup_name;
	}

	/* Initialize storage */
	section->values = section->keys + TINI_PARAMETER_STORAGE_INITIAL_SIZE + 1;
	*(section->keys) = NULL;
	*(section->values) = NULL;
	
	/* Initialize counts of parameters */
	section->parameter_count = 0;
	section->max_parameter_count = TINI_PARAMETER_STORAGE_INITIAL_SIZE;
	
	return 0;
	
cleanup_name:
	/* Free memory on error */
	free(section->name);
	
exit_error:
	/* restore saved errno */
	errno = saved_errno;
	return -1;
}

static int initialize_ini(ini_file* ini) {
	/* Allocate storage for sections */
	ini->sections = malloc(sizeof(ini_section*) * TINI_SECTION_STORAGE_INITIAL_SIZE);
	
	/* Initialize storage of sections or indicate error */
	if (ini->sections) {
		ini->section_count = 0;
		ini->max_section_count = TINI_SECTION_STORAGE_INITIAL_SIZE;
		return 0;
	}
	else return -1;
}

static void cleanup_ini(ini_file* ini) {
	/* Cleanup all section objects */
	size_t i;
	for (i = 0; i < ini->section_count; ++i) {
		cleanup_section(ini->sections[i]);
		free(ini->sections[i]);
	}
	
	/* Free sections storage */
	free(ini->sections);
}

ini_file* tini_create_ini(void) {
	/* Allocate memory for INI file object */
	ini_file* ini = malloc(sizeof(ini_file));
	
	/* Initialize object, check result, indicate error if necessary */
	if (ini && initialize_ini(ini) != 0) {
		int saved_errno = errno;
		free(ini);
		ini = NULL;
		errno = saved_errno;
	}
	
	/* Returns pointer to object (or NULL) */
	return ini;
}

void tini_free_ini(ini_file* ini) {
	/* By convention, free()-like functions accept NULL input */
	if (ini) {
		/* Cleanup object */
		cleanup_ini(ini);
		
		/* free memory */
		free(ini);
	}
}

ini_file* tini_load_ini(const char* file_path) {
	/* Create INI file object */
	ini_file* ini = tini_create_ini();
	
	if (ini) {
		/* Parse INI file using INIH library into INI file object, check result, 
		 * indicate error if necessary.
		 */
		if (ini_parse(file_path, &ini_file_handler, ini) < 0) {
			int saved_errno = errno;
			free(ini);
			ini = NULL;
			errno = saved_errno;
		}
	}
	
	/* Returns resulting object */
	return ini;
}

#ifdef TINI_FEATURE_SAVE_INI

int tini_save_ini(const ini_file* ini, const char* file_path) {
	/* Initialize result variable */
	int res = -1;
	
	/* Open file */
	FILE* f = fopen(file_path, "w");
	if (f) {
		/* Dump INI file object into that file */
		int saved_errno;
		res = tini_dump_ini(ini, f);
		
		/* Close file */
		saved_errno = errno;
		if(fclose(f) != 0);
			res = -1;
		else
			errno = saved_errno;
	}
	
	/* Return result */
	return res;
}

#endif

#if defined(TINI_FEATURE_SAVE_INI) || defined(TINI_FEATURE_SAVE_INI)

int tini_dump_ini(const ini_file* ini, FILE* f) {
	size_t i, j;
	
	/* Enumerate all section */
	for (i = 0; i < ini->section_count; ++i) {
		const ini_section* s = ini->sections[i];
		
		/* Write section header */
		if(fprintf(f, "[%s]\n", s->name) < 0)
			return -1;
		
		/* Enumerate and write all parameters and values */
		for (j = 0; j < s->parameter_count; ++j) {
			if(fprintf(f, "%s=%s\n", s->keys[j], s->values[j]) < 0)
				return -1;
		}
		
		/* Put empty line between sections */
		if(fputc('\n', f) == EOF)
			return -1;
	}
	
	/* Indicate success. */
	return 0;
}

#endif

ini_section* tini_new_section(const char* name) {
	/* Allocate memory for section object */
	ini_section* section = malloc(sizeof(ini_section));
	
	if (section) {
		/* Initialize section object, check result, indicate error if necessary */
		if (initialize_section(section, name) != 0) {
			int saved_errno = errno;
			free(section);
			section = NULL;
			errno = saved_errno;
		}
	}
	
	/* Return resulting object or NULL */
	return section;
}

void tini_free_section(ini_section* section) {
	/* By convention, free()-like functions can accept NULL values */
	if (section) {
		/* Cleanup section object */
		cleanup_section(section);
		
		/* Free memory */
		free(section);
	}
}

int tini_add_parameter(ini_file* ini, const char* section, const char* key, const char* value, int replace) {
	/* Attempt to find section with given name */
	ini_section* sectionObj = (ini_section*) tini_find_section(ini, section);
	if (sectionObj) {
		/* Section found - attempt adding parameter into it */
		return tini_add_parameter_to_section(sectionObj, key, value, replace);
	} else {
		/* Otherwise create new section */
		sectionObj = tini_new_section(section);
		if (sectionObj) {
			/* Attempt adding parameter to it */
			if(tini_add_parameter_to_section(sectionObj, key, value, replace) == 0) {
				/* Attempt to add section to sections storage */
				if(ini->section_count < ini->max_section_count
					|| (ini->section_count == ini->max_section_count
					&& grow_section_storage(ini) == 0)) {
					ini->sections[ini->section_count++] = sectionObj;
					return 0;
				} else
					return -1;
			} else {
				/* Indicate error */
				int saved_errno = errno;
				tini_free_section(sectionObj);
				errno = saved_errno;
				return -1;
			}
		} else {
			/* Indicate error */
			return -1;
		}
	}
}

int tini_add_parameter_to_section(ini_section* section, const char* key, const char* value, int replace) {
	/* Check whether parameter with given name already exists */
	size_t i = find_parameter_index_in_section(section, key);
	if (i == 0) {
		/* Create parameter name string */
		char *new_key, *new_value;
		new_key = strdup(key);
		if (!new_key)
			return -1;
		
		/* Create parameter value string */
		new_value = strdup(value);
		if (!new_value) {
			free(new_key);
			return -1;
		}
		
		/* Attempt to add parameter to section, resize parameters storage if necessary */
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
			/* Free memory and indicate error */
			free(new_value);
			free(new_key);
			return -1;
		}
	} else if (replace) { /* Parameter exists and we can replace it */
		/* Create new parameter value string */
		char* new_value = strdup(value);
		if (new_value) {
			--i;
			/* Free old parameter value string */
			free(section->values[i]);
			/* Put new one in place */
			section->values[i] = new_value;
			return 0;
		} else
			return -1;
	} else { /* Parameter exists but we can't replace it */
		errno = EEXIST;
		return -1;
	}
}

#ifdef TINI_FEATURE_EDIT_INI

int tini_remove_section(ini_file* ini, const char* section) {
	/* Search for section with given name */
	size_t i = find_section_index(ini, section);
	if (i == 0) {
		/* Section not found, indicate error */
		errno = ESRCH;
		return -1;
	} else {
		/* Section found, remove it */
		--i;
		remove_section_by_index(ini, i);
		return 0;
	}
}

int tini_remove_parameter(ini_file* ini, const char* section, const char* key) {	
	size_t i, j;

	/* Find section with given name */
	i = find_section_index(ini, section);
	if (i == 0) {
		/* Section not found, indicate error */
		errno = ESRCH;
		return -1;
	} else {
		/* Section found, find parameter */
		ini_section* sectionObj = ini->sections[--i];
		j = find_parameter_index_in_section(s, key);
		if(j == 0) {
			/* Parameter not found, indicate error */
			errno = ESRCH;
			return -1;
		} else {
			/* Parameter found */
			--j;
			
			/* Free parameter name and value strings */
			free(sectionObj->values[j]);
			free(sectionObj->keys[j]);
			
			/* Pack parameters array if parameter was in the beginning or middle */
			if(i < s->parameter_count - 1) {
				memmove(sectionObj->keys + i, sectionObj->keys + i + 1, 
					sizeof(char*) * (sectionObj->parameter_count - i - 1));
				memmove(sectionObj->values + i, sectionObj->values + i + 1, 
					sizeof(char*) * (sectionObj->parameter_count - i - 1));
			}
			
			/* Clear last parameters storage entries, so storage always finishes with NULLs */
			s->keys[sectionObj->parameter_count] = NULL;
			s->values[sectionObj->parameter_count] = NULL;
			
			/* Decreate parameter number */
			--sectionObj->parameter_count;
			
			/* Indicate success */
			return 0;
		}
	}
}

#endif

const ini_section* tini_find_section(const ini_file* ini, const char* section) {
	/* Find section, retrieve its index */
	size_t i = find_section_index(ini, section);
	
	/* If index is valid, return section object, otherwise return NULL */
	return i == 0 ? NULL : ini->sections[i - 1];
}

const char* tini_find_parameter_in_section(const ini_section* section, const char* key, const char* default_value) {
	/* Find parameter, retrieve its index */
	size_t i = find_parameter_index_in_section(section, key);

	/* If index is valid, return parameter value, otherwise return default value */
	return i == 0 ? default_value : section->values[i - 1];
}

const char* tini_find_parameter(const ini_file* ini, const char* section, const char* key, const char* default_value) {
	/* Find section*/
	const ini_section* sectionObj = tini_find_section(ini, section);
	
	/* If section found, attempt find parameter, otherwise return default value */
	return sectionObj ? tini_find_parameter_in_section(sectionObj, key, default_value) : default_value;
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

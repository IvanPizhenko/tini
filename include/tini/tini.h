/*=======================================================================================

TinyINI - small and simple open-source library for loading, saving and 
managing INI file data structures in the memory.

TinyINI is distributed under the following terms and conditions:

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

#pragma once

#ifndef TINYINI_TINI_H__
#define TINYINI_TINI_H__

#include <stddef.h>

/* Initial size of the storage for section objects */
#ifndef TINI_SECTION_STORAGE_INITIAL_SIZE
#define TINI_SECTION_STORAGE_INITIAL_SIZE 4
#endif

/* Automatic size increment of the storage for section objects */
#ifndef TINI_SECTION_STORAGE_SIZE_INCREMENT
#define TINI_SECTION_STORAGE_SIZE_INCREMENT 4
#endif

/* Initial size of the storage for parameter objects in the each section */
#ifndef TINI_PARAMETER_STORAGE_INITIAL_SIZE
#define TINI_PARAMETER_STORAGE_INITIAL_SIZE 8
#endif

/* Automatic size increment of the storage for parameter objects in the each section */
#ifndef TINI_PARAMETER_STORAGE_SIZE_INCREMENT
#define TINI_PARAMETER_STORAGE_SIZE_INCREMENT 8
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* User-visible INI file section handle */
struct _ini_section;
typedef struct _ini_section ini_section;

/* User-visible INI file handle */
struct _ini_file;
typedef struct _ini_file ini_file;

/* Create empty INI file objects */
ini_file* tini_create_ini(void);

/* Destroy INI file object */
void tini_free_ini(ini_file* ini);

/* Parse given INI file into new INI file object */
ini_file* tini_load_ini(const char* file_path);

#ifdef TINI_FEATURE_SAVE_INI
/* Create INI file from the given INI file object */
int tini_save_ini(const ini_file* ini, const char* file_path);
#endif

#if defined(TINI_FEATURE_SAVE_INI) || defined(TINI_FEATURE_DUMP_INI)
/* Dump content of the given INI file object into given file descriptor. */
int tini_dump_ini(const ini_file* ini, FILE* f);
#endif

/* Initialize INI file object */
int tini_initialize_ini(ini_file* ini);

/* Cleanup INI file object */
void tini_cleanup_ini(ini_file* ini);

/* Create INI file section object */
ini_section* tini_new_section(const char* name);

/* Destroy INI file section object */
void tini_free_section(ini_section* section);

/* Add given parameter to INI file object. If parameter exists, it may be replaced if replace is nonzero. 
 * Returns zero on success, nonzero on failure. Check errno for error details.
 */
int tini_add_parameter(ini_file* ini, const char* section, const char* key, const char* value, int replace);

/* Add given parameter to INI file section object. If parameter exists, it may be replaced if replace is nonzero. 
 * Returns zero on success, nonzero on failure. Check errno for error details.
 */
int tini_add_parameter_to_section(ini_section* section, const char* key, const char* value, int replace);

#ifdef TINI_FEATURE_EDIT_INI_FILE
/* Remove given parameter from INI file section object.
 * Returns zero on success, nonzero on failure. Check errno for error details.
 */
int tini_remove_section(ini_file* ini, const char* section);

/* Removes given section from INI file object.
 * Returns zero on success, nonzero on failure. Check errno for error details.
 */
int tini_remove_parameter(ini_file* ini, const char* section, const char* key);
#endif

/* Find given section by name. Returns section handle if section found, or NULL otherwise. */
const ini_section* tini_find_section(const ini_file* ini, const char* section);

/* Find given parameter by name. Returns parameter value] if parameter found, or NULL otherwise. */
const char* tini_find_parameter_in_section(const ini_section* section, const char* key, const char* default_value);

/* Find given parameter by section name and parameter name. Returns parameter value if parameter found, or NULL otherwise. */
const char* tini_find_parameter(const ini_file* ini, const char* section, const char* key, const char* default_value);

#ifdef TINI_FEATURE_GET_ELEMENT_COUNT
/* Returns count of sections in the given INI file object */
size_t tini_get_section_count(const ini_file* ini);

/* Returns count of parameters in the given INI file section object */
size_t tini_get_parameter_count(const ini_section* section);
#endif

#ifdef TINI_FEATURE_GET_PARAMETERS_STORAGE
/* Returns array of parameter names in the given section */
const char* const* tini_get_keys(const ini_section* section);

/* Returns array of parameter values in the given section */
const char* const* tini_get_values(const ini_section* section);
#endif

#ifdef TINI_FEATURE_GET_SECTIONS_STORAGE
/* Returns array of section objects in the given INI file object */
const ini_section* const* tini_get_sections(const ini_file* ini);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif /* TINYINI_TINI_H__ */

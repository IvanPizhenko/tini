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

#ifndef TINI_SECTION_STORAGE_INITIAL_SIZE
#define TINI_SECTION_STORAGE_INITIAL_SIZE 4
#endif

#ifndef TINI_SECTION_STORAGE_SIZE_INCREMENT
#define TINI_SECTION_STORAGE_SIZE_INCREMENT 4
#endif

#ifndef TINI_PARAMETER_STORAGE_INITIAL_SIZE
#define TINI_PARAMETER_STORAGE_INITIAL_SIZE 8
#endif

#ifndef TINI_PARAMETER_STORAGE_SIZE_INCREMENT
#define TINI_PARAMETER_STORAGE_SIZE_INCREMENT 8
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct _ini_section;
typedef struct _ini_section ini_section;

struct _ini_file;
typedef struct _ini_file ini_file;

ini_file* tini_create_ini(void);
void tini_free_ini(ini_file* ini);

ini_file* tini_load_ini(const char* file_path);
int tini_save_ini(const ini_file* ini, const char* file_path);
int tini_dump_ini(const ini_file* ini, FILE* f);

int tini_initialize_ini(ini_file* ini);
void tini_cleanup_ini(ini_file* ini);

ini_section* tini_new_section(const char* name);
void tini_free_section(ini_section* section);

int tini_add_parameter(ini_file* ini, const char* section, const char* key, const char* value, int replace);
int tini_add_parameter_to_section(ini_section* section, const char* key, const char* value, int replace);

int tini_remove_section(ini_file* ini, const char* section);
int tini_remove_parameter(ini_file* ini, const char* section, const char* key);

const ini_section* tini_find_section(const ini_file* ini, const char* section);
const char* tini_find_parameter_in_section(const ini_section* section, const char* key, const char* default_value);
const char* tini_find_parameter(const ini_file* ini, const char* section, const char* key, const char* default_value);

size_t tini_get_parameter_count(const ini_section* section);
const char* const* tini_get_keys(const ini_section* section);
const char* const* tini_get_values(const ini_section* section);

size_t tini_get_section_count(const ini_file* ini);
const ini_section* const* tini_get_sections(const ini_file* ini);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


#endif // PERCEPTUALHASHES__LIBPHTOOL__INI_FILE_H__

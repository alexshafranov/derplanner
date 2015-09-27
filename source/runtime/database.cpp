//
// Copyright (c) 2015 Alexander Shafranov shafranov@gmail.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <string.h> // memset, strlen
#include "derplanner/runtime/database.h"

using namespace plnnr;

static size_t get_blob_size(const Fact_Type* format, uint32_t max_entries)
{
    size_t size = 0;
    // data columns
    for (uint8_t i = 0; i < format->num_params; ++i)
    {
        size += get_type_alignment(format->types[i]) + max_entries * get_type_size(format->types[i]);
    }

    return size;
}

void plnnr::init(Fact_Table* self, Memory* mem, const Fact_Type* format, uint32_t max_entries)
{
    memset(self, 0, sizeof(Fact_Table));

    self->format = *format;
    self->num_entries = 0;
    self->max_entries = max_entries;

    const size_t size = get_blob_size(format, max_entries);
    void* blob = mem->allocate(size);
    memset(blob, 0, size);

    self->blob = blob;
    uint8_t* bytes = static_cast<uint8_t*>(blob);

    for (uint8_t i = 0; i < format->num_params; ++i)
    {
        Type param_type = format->types[i];
        size_t param_align = get_type_alignment(param_type);
        uint8_t* column = static_cast<uint8_t*>(plnnr::align(bytes, param_align));
        bytes = column + max_entries * get_type_size(param_type);
        self->columns[i] = column;
    }

    self->memory = mem;
}

void plnnr::destroy(Fact_Table* self)
{
    Memory* mem = self->memory;
    mem->deallocate(self->blob);
    memset(self, 0, sizeof(Fact_Table));
}

void plnnr::set_max_entries(Fact_Table* self, uint32_t max_entries)
{
    plnnr_assert(max_entries >= self->num_entries);

    Memory* mem = self->memory;
    void* old_blob = self->blob;
    const Fact_Type* format = &self->format;

    const size_t size = get_blob_size(format, max_entries);
    void* new_blob = mem->allocate(size);
    memset(new_blob, 0, size);

    self->blob = new_blob;
    uint8_t* bytes = static_cast<uint8_t*>(new_blob);

    for (uint8_t i = 0; i < format->num_params; ++i)
    {
        const Type param_type = format->types[i];
        size_t param_align = get_type_alignment(param_type);
        uint8_t* new_column = static_cast<uint8_t*>(plnnr::align(bytes, param_align));
        bytes = new_column + max_entries * get_type_size(param_type);
        const uint8_t* old_column = (const uint8_t*)(self->columns[i]);
        memcpy(new_column, old_column, self->num_entries * get_type_size(param_type));
        self->columns[i] = new_column;
    }

    self->max_entries = max_entries;

    mem->deallocate(old_blob);
}

void plnnr::init(Fact_Database* self, Memory* mem, const Database_Format* format)
{
    memset(self, 0, sizeof(Fact_Database));

    self->num_tables = format->num_tables;
    self->max_tables = format->num_tables;
    self->hash_seed = format->hash_seed;

    const uint32_t max_tables = format->num_tables;
    // hashes
    size_t size = plnnr_alignof(uint32_t) + sizeof(uint32_t) * max_tables;
    // names
    size = align(size, plnnr_alignof(void*));
    size += sizeof(void*) * max_tables;
    // tables
    size = align(size, plnnr_alignof(Fact_Table));
    size += sizeof(Fact_Table) * max_tables;

    void* blob = mem->allocate(size);
    self->blob = blob;
    uint8_t* bytes = static_cast<uint8_t*>(blob);

    self->hashes = plnnr::align<uint32_t>(bytes);
    bytes += sizeof(uint32_t) * max_tables;
    self->names = plnnr::align<const char*>(bytes);
    bytes += sizeof(void*) * max_tables;
    self->tables = plnnr::align<Fact_Table>(bytes);

    for (uint32_t i = 0; i < format->num_tables; ++i)
    {
        uint32_t size_hint = format->size_hints[i];
        uint32_t max_entries = ( size_hint > 0 ) ? size_hint : 128;
        self->hashes[i] = format->hashes[i];
        self->names[i] = format->names[i];
        init(self->tables + i, mem, format->types + i, max_entries);
    }

    self->memory = mem;
}

void plnnr::destroy(Fact_Database* self)
{
    Memory* mem = self->memory;

    for (uint32_t i = 0; i < self->num_tables; ++i)
    {
        Fact_Table* table = self->tables + i;
        destroy(table);
    }

    mem->deallocate(self->blob);
    memset(self, 0, sizeof(Fact_Database));
}

const Fact_Table* plnnr::find_table(const Fact_Database* self, const char* fact_name)
{
    const uint32_t seed = self->hash_seed;
    const uint32_t hash = murmur2_32(fact_name, (uint32_t)strlen(fact_name), seed);
    const uint32_t* db_hashes = self->hashes;
    const uint32_t num_hashes = self->num_tables;
    const Fact_Table* db_tables = self->tables;

    for (uint32_t i = 0; i < num_hashes; ++i)
        if (hash == db_hashes[i])
            return &db_tables[i];

    return 0;
}

Fact_Table* plnnr::find_table(Fact_Database* self, const char* fact_name)
{
    const uint32_t seed = self->hash_seed;
    const uint32_t hash = murmur2_32(fact_name, (uint32_t)strlen(fact_name), seed);
    const uint32_t* db_hashes = self->hashes;
    const uint32_t num_hashes = self->num_tables;
    Fact_Table* db_tables = self->tables;

    for (uint32_t i = 0; i < num_hashes; ++i)
        if (hash == db_hashes[i])
            return &db_tables[i];

    return 0;
}

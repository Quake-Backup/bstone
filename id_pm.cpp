/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
============================================================== */


#include "id_pm.h"

#include "id_heads.h"


// File specific variables
std::string PageFileName = "VSWAP.";

int ChunksInFile = 0;
int PMSpriteStart = 0;
int PMSoundStart = 0;


namespace {


bstone::FileStream PageFile;
uint8_t* raw_data = NULL;
uint32_t* chunks_offsets = NULL;


} // namespace


static void open_page_file(
    const std::string& file_name)
{
    PageFile.open(file_name);

    if (!PageFile.is_open()) {
        PM_ERROR(PML_OPENPAGEFILE_OPEN);
    }

    int64_t file_length = PageFile.get_size();

    if (file_length > 4 * 1024 * 1024) {
        ::Quit("Page file is too large.");
    }

    int32_t file_length_32 = static_cast<int32_t>(file_length);

    raw_data = new uint8_t[file_length_32 + PMPageSize];
    std::uninitialized_fill_n(&raw_data[file_length], PMPageSize, 0);

    if (PageFile.read(raw_data, file_length_32) != file_length_32) {
        PM_ERROR(PML_READFROMFILE_READ);
    }

    bstone::MemoryBinaryReader reader(raw_data, file_length);

    ChunksInFile = bstone::Endian::le(reader.read_u16());
    PMSpriteStart = bstone::Endian::le(reader.read_u16());
    PMSoundStart = bstone::Endian::le(reader.read_u16());

    chunks_offsets = reinterpret_cast<uint32_t*>(&raw_data[6]);
    bstone::Endian::lei(chunks_offsets, ChunksInFile + 1);
}

void PM_Startup()
{
    ::PM_Shutdown();
    ::open_page_file(PageFileName);
}

void PM_Shutdown()
{
    PageFile.close();

    ChunksInFile = 0;
    PMSpriteStart = 0;
    PMSoundStart = 0;

    delete [] raw_data;
    raw_data = NULL;

    chunks_offsets = NULL;
}

void* PM_GetPage(
    int page_number)
{
    if (page_number >= ChunksInFile) {
        PM_ERROR(PM_GETPAGE_BAD_PAGE);
    }

    uint32_t offset = chunks_offsets[page_number];

    if (offset == 0) {
        PM_ERROR(PM_GETPAGE_SPARSE_PAGE);
    }

    return &raw_data[offset];
}

void* PM_GetSoundPage(
    int page_number)
{
    return PM_GetPage(PMSoundStart + page_number);
}

void* PM_GetSpritePage(
    int page_number)
{
    return PM_GetPage(PMSpriteStart + page_number);
}

/* Z_Zone.c */

//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//      Zone Memory Allocation. Neat.
//
// Based on the doom64 Ex code by Samuel Villarreal
// https://github.com/svkaiser/Doom64EX/blob/master/src/engine/zone/z_zone.cc
//-----------------------------------------------------------------------------

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <zlib.h>

#include "DoomRPG.h"
#include "Z_Zip.h"

zip_file_t zipFile;

static void* zip_alloc(void* ctx, unsigned int items, unsigned int size)
{
	return SDL_malloc(items * size);
}

static void zip_free(void* ctx, void* ptr)
{
	SDL_free(ptr);
}

void findAndReadZipDir(zip_file_t* zipFile, int startoffset)
{
	int sig, offset, count;
	int namesize, metasize, commentsize;
	int i;

	SDL_RWseek(zipFile->file, startoffset, SEEK_SET);

	sig = File_readLong(zipFile->file);
	if (sig != ZIP_END_OF_CENTRAL_DIRECTORY_SIG) {
		DoomRPG_Error("wrong zip end of central directory signature (0x%x)", sig);
	}

	File_readShort(zipFile->file); // this disk
	File_readShort(zipFile->file); // start disk
	File_readShort(zipFile->file); // entries in this disk
	count = File_readShort(zipFile->file); // entries in central directory disk
	File_readLong(zipFile->file); // size of central directory
	offset = File_readLong(zipFile->file); // offset to central directory

	zipFile->entry = SDL_calloc(count, sizeof(zip_entry_t));
	zipFile->entry_count = count;

	SDL_RWseek(zipFile->file, offset, SEEK_SET);

	for (i = 0; i < count; i++)
	{
		zip_entry_t* entry = zipFile->entry + i;

		sig = File_readLong(zipFile->file);
		if (sig != ZIP_CENTRAL_DIRECTORY_SIG) {
			DoomRPG_Error("wrong zip central directory signature (0x%x)", sig);
		}

		File_readShort(zipFile->file); // version made by
		File_readShort(zipFile->file); // version to extract
		File_readShort(zipFile->file); // general
		File_readShort(zipFile->file); // method
		File_readShort(zipFile->file); // last mod file time
		File_readShort(zipFile->file); // last mod file date
		File_readLong(zipFile->file); // crc-32
		entry->csize = File_readLong(zipFile->file); // csize
		entry->usize = File_readLong(zipFile->file); // usize
		namesize = File_readShort(zipFile->file);
		metasize = File_readShort(zipFile->file);
		commentsize = File_readShort(zipFile->file);
		File_readShort(zipFile->file); // disk number start
		File_readShort(zipFile->file); // int file atts
		File_readLong(zipFile->file); // ext file atts
		entry->offset = File_readLong(zipFile->file);

		entry->name = SDL_malloc(namesize + 1);
		SDL_RWread(zipFile->file, (unsigned char*)entry->name, sizeof(byte), namesize);
		entry->name[namesize] = 0;

		SDL_RWseek(zipFile->file, metasize, SEEK_CUR);
		SDL_RWseek(zipFile->file, commentsize, SEEK_CUR);
	}
}

void openZipFile(const char* name, zip_file_t* zipFile)
{
	byte buf[512];
	int filesize, back, maxback;
	int i, n;

	zipFile->file = SDL_RWFromFile(name, "r");
	if (zipFile->file == NULL) {
		DoomRPG_Error("openZipFile: cannot open file %s\n", name);
	}

	filesize = (int)SDL_RWsize(zipFile->file);

	maxback = MIN(filesize, 0xFFFF + sizeof(buf));
	back = MIN(maxback, sizeof(buf));

	while (back < maxback)
	{
		SDL_RWseek(zipFile->file, filesize - back, SEEK_SET);
		n = sizeof(buf);
		SDL_RWread(zipFile->file, buf, sizeof(byte), sizeof(buf));
		for (i = n - 4; i > 0; i--)
		{
			if (!SDL_memcmp(buf + i, "PK\5\6", 4)) {
				findAndReadZipDir(zipFile, filesize - back + i);
				return;
			}
		}
		back += sizeof(buf) - 4;
	}

	DoomRPG_Error("cannot find end of central directory\n");
}

void closeZipFile(zip_file_t* zipFile)
{
	if (zipFile->entry) {
		SDL_free(zipFile->entry);
	}
	if (zipFile->file) {
		SDL_RWclose(zipFile->file);
	}
}

unsigned char* readZipFileEntry(const char* name, zip_file_t* zipFile, int* sizep)
{
	zip_entry_t* entry = NULL;
	int i, sig, general, method, namelength, extralength;
	byte* cdata;
	int code;

	for (i = 0; i < zipFile->entry_count; i++)
	{
		zip_entry_t* entryTmp = zipFile->entry + i;

		if (!SDL_strcasecmp(name, entryTmp->name)) {
			entry = zipFile->entry + i;
			break;
		}
	}

	if (entry == NULL) {
		DoomRPG_Error("did not find the %s file in the zip file", name);
	}

	SDL_RWseek(zipFile->file, entry->offset, SEEK_SET);

	sig = File_readLong(zipFile->file);
	if (sig != ZIP_LOCAL_FILE_SIG) {
		DoomRPG_Error("wrong zip local file signature (0x%x)", sig);
	}

	File_readShort(zipFile->file); // version
	general = File_readShort(zipFile->file); // general
	if (general & ZIP_ENCRYPTED_FLAG) {
		DoomRPG_Error("zipfile content is encrypted");
	}

	method = File_readShort(zipFile->file);
	File_readShort(zipFile->file); // file time
	File_readShort(zipFile->file); // file date
	File_readLong(zipFile->file); // crc-32
	File_readLong(zipFile->file); // csize
	File_readLong(zipFile->file); // usize
	namelength = File_readShort(zipFile->file);
	extralength = File_readShort(zipFile->file);

	SDL_RWseek(zipFile->file, namelength + extralength, SEEK_CUR);

	cdata = SDL_malloc(entry->csize);
	SDL_RWread(zipFile->file, cdata, sizeof(byte), entry->csize);

	if (method == 0)
	{
		*sizep = entry->usize;
		return cdata;
	}
	else if (method == 8)
	{
		byte* udata = SDL_malloc(entry->usize);
		z_stream stream;

		SDL_memset(&stream, 0, sizeof stream);
		stream.zalloc = zip_alloc;
		stream.zfree = zip_free;
		stream.opaque = Z_NULL;
		stream.next_in = cdata;
		stream.avail_in = entry->csize;
		stream.next_out = udata;
		stream.avail_out = entry->usize;

		code = inflateInit2(&stream, -15);
		if (code != Z_OK) {
			DoomRPG_Error("zlib inflateInit2 error: %s", stream.msg);
		}

		code = inflate(&stream, Z_FINISH);
		if (code != Z_STREAM_END) {
			inflateEnd(&stream);
			DoomRPG_Error("zlib inflate error: %s", stream.msg);
		}

		code = inflateEnd(&stream);
		if (code != Z_OK) {
			inflateEnd(&stream);
			DoomRPG_Error("zlib inflateEnd error: %s", stream.msg);
		}

		SDL_free(cdata);

		*sizep = entry->usize;
		return udata;
	}
	else {
		DoomRPG_Error("unknown zip method: %d", method);
	}

	return NULL;
}

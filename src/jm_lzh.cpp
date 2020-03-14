/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
*/


// ===========================================================================
//
// LZHUFF COMPRESSION ROUTINES
// VERSION 1.0
//
// Compression algrythim by Haruhiko OKUMURA
// Implementation by Jim T. Row
//
//
// Copyright (c) 1992 -
//
// ===========================================================================
//
// Compiler #ifdef switches
//
// LZHUFF_COMPRESSION & LZHUFF_DECOMPRESSION - not yet functional!
//
// Usage Explanition :
//
//    if LZHUFF_COMPRESSION is defined then the compression code & data is
//    compiled and so-forth for the decompression code.
//
// ---------------------------------------------------------------------------


#include <cstring>

#include "jm_cio.h"
#include "jm_lzh.h"


// ===========================================================================
//
// SWITCHES
//
// NOTE : Make sure the appropriate switches are set in SOFT.c for Softlib
//                       archive support.
//
// ===========================================================================


#define INCLUDE_LZH_COMP 1
#define INCLUDE_LZH_DECOMP 1


#define LZH_DYNAMIC_ALLOCATION

#define LZH_ID_MEMORY_ALLOCATION


// ===========================================================================
//
// DEFINES
//
// ===========================================================================


#define EXIT_OK (0)
#define EXIT_FAILED (-1)

/* LZSS Parameters */

#define N (4096) /* Size of string buffer */
#define F (30) /* Size of look-ahead buffer */
#define THRESHOLD (2)
#define NIL (N) /* End of tree's node  */

/* Huffman coding parameters */

#define N_CHAR (256 - THRESHOLD + F) /* character code (= 0..N_CHAR-1) */
#define T ((N_CHAR * 2) - 1) /* Size of table */
#define R (T - 1) /* root position */
#define MAX_FREQ (0x8000) /* update when cumulative frequency */
/* reaches to this value */


// ==========================================================================
//
// LOCAL PROTOTYPES
//
// ==========================================================================


static void StartHuff();
static void reconst();

static void update(
	std::int16_t c);


static void DeleteNode(
	std::int16_t p); /* Deleting node from the tree */

static void InsertNode(
	std::int16_t r); /* Inserting node to the tree */

static void InitTree(); /* Initializing tree */

static void Putcode(
	void*& outfile_ptr,
	std::int16_t l,
	std::uint16_t c);

static void EncodeChar(
	void*& outfile_ptr,
	std::uint16_t c);

static void EncodePosition(
	void*& outfile_ptr,
	std::uint16_t c);

static void EncodeEnd(
	void*& outfile_ptr);

static std::int16_t GetByte(
	const void*& infile_ptr,
	std::uint32_t* CompressLength);

static std::int16_t GetBit(
	const void*& infile_ptr,
	std::uint32_t* CompressLength);

static std::int16_t DecodeChar(
	const void*& infile_ptr,
	std::uint32_t* CompressLength);

static std::int16_t DecodePosition(
	const void*& infile_ptr,
	std::uint32_t* CompressLength);


// ==========================================================================
//
// USER AVAILABLE VECTORS
//
// ==========================================================================

// ---------------------------------------------------------------------------
//
// LZHUFF DISPLAY VECTORS
//
// These vectors allow you to hook up any form of display you desire for
// displaying the compression/decompression status.
//
// These routines are called inside of the compression/decompression routines
// and pass the original size of data and current position within that
// data.  This allows for any kind of "% Done" messages.
//
// Your functions MUST have the following parameters in this order...
//
//   void VectorRoutine(unsigned long OriginalSize,unsigned long CurPosition)
//
//

#if INCLUDE_LZH_COMP
void(*LZH_CompressDisplayVector)(std::uint32_t, std::uint32_t) = nullptr;
#endif

#if INCLUDE_LZH_DECOMP
void(*LZH_DecompressDisplayVector)(std::uint32_t, std::uint32_t) = nullptr;
#endif


// ===========================================================================
//
// GLOBAL VARIABLES
//
// ===========================================================================
/* pointing children nodes (son[], son[] + 1)*/

std::uint16_t code, len;
std::uint32_t textsize = 0, codesize = 0, printcount = 0, datasize;

#ifdef LZH_DYNAMIC_ALLOCATION

std::int16_t* son = nullptr;

//
// pointing parent nodes.
// area [T..(T + N_CHAR - 1)] are pointers for leaves
//

std::int16_t* prnt;
std::uint16_t* freq; /* cumulative freq table */
std::uint8_t* text_buf;

#ifdef LZH_ID_MEMORY_ALLOCATION
std::int16_t* id_son;
std::int16_t* id_prnt;
std::uint16_t* id_freq;
std::uint8_t* id_text_buf;
#endif

#else

std::int16_t son[T];

//
// pointing parent nodes.
// area [T..(T + N_CHAR - 1)] are pointers for leaves
//

std::int16_t prnt[T + N_CHAR];

std::uint16_t freq[T + 1]; /* cumulative freq table */

std::uint8_t text_buf[N + F - 1];

#endif



//
// COMPRESSION VARIABLES
//

#if INCLUDE_LZH_COMP

#ifdef LZH_DYNAMIC_ALLOCATION

static std::int16_t* lson, *rson, *dad;

#ifdef LZH_ID_MEMORY_ALLOCATION
std::int16_t* id_lson;
std::int16_t* id_rson;
std::int16_t* id_dad;
#endif
#else

static std::int16_t lson[N + 1], rson[N + 257], dad[N + 1];

#endif

static std::int16_t match_position, match_length;
std::uint16_t putbuf = 0;
std::uint16_t putlen = 0;

//
// Tables for encoding/decoding upper 6 bits of
// sliding dictionary pointer
//

//
// encoder table
//

std::uint8_t p_len[64] = {
	0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

std::uint8_t p_code[64] = {
	0x00, 0x20, 0x30, 0x40, 0x50, 0x58, 0x60, 0x68,
	0x70, 0x78, 0x80, 0x88, 0x90, 0x94, 0x98, 0x9C,
	0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC,
	0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE,
	0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
	0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
	0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};
#endif


//
// DECOMPRESSION VARIABLES
//


//
// decoder table
//

#if INCLUDE_LZH_DECOMP

std::uint8_t d_code[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
	0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
	0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
	0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
	0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
	0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
	0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
	0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
	0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
	0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
	0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

std::uint8_t d_len[256] = {
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

std::uint16_t getbuf = 0;
std::uint16_t getlen = 0;

#endif


// ===========================================================================
//
// COMPRESSION & DECOMPRESSION ROUTINES
//
// ===========================================================================

bool LZH_Startup()
{
	if (son)
	{
		return true;
	}

#ifdef LZH_DYNAMIC_ALLOCATION
#ifdef LZH_ID_MEMORY_ALLOCATION
	id_son = new std::int16_t[T];
	id_prnt = new std::int16_t[T + N_CHAR];
	id_freq = new std::uint16_t[T + 1];
	id_text_buf = new std::uint8_t[N + F - 1];
#else
	if (!(son = farmalloc(T * sizeof(*son))))
	{
		return false;
	}

	if (!(prnt = farmalloc((T + N_CHAR) * sizeof(*prnt))))
	{
		return false;
	}

	if (!(freq = farmalloc((T + 1) * sizeof(*freq))))
	{
		return false;
	}

	if (!(text_buf = farmalloc((N + F - 1) * sizeof(*text_buf))))
	{
		return false;
	}
#endif

#if INCLUDE_LZH_COMP
#ifdef LZH_ID_MEMORY_ALLOCATION
	id_lson = new std::int16_t[N + 1];
	id_rson = new std::int16_t[N + 257];
	id_dad = new std::int16_t[N + 1];
#else
	if (!(lson = farmalloc((N + 1) * sizeof(*lson))))
	{
		return false;
	}

	if (!(rson = farmalloc((N + 257) * sizeof(*rson))))
	{
		return false;
	}

	if (!(dad = farmalloc((N + 1) * sizeof(*dad))))
	{
		return false;
	}
#endif
#endif
#endif

	return true;
}

void LZH_Shutdown()
{
#ifdef LZH_DYNAMIC_ALLOCATION
#ifdef LZH_ID_MEMORY_ALLOCATION
	delete[] id_son;
	id_son = nullptr;

	delete[] id_prnt;
	id_prnt = nullptr;

	delete[] id_freq;
	id_freq = nullptr;

	delete[] id_text_buf;
	id_text_buf = nullptr;
#else
	if (son)
	{
		farfree(son);
	}

	if (prnt)
	{
		farfree(prnt);
	}

	if (freq)
	{
		farfree(freq);
	}

	if (text_buf)
	{
		farfree(text_buf);
	}
#endif

#if INCLUDE_LZH_COMP
#ifdef LZH_ID_MEMORY_ALLOCATION
	delete[] id_lson;
	id_lson = nullptr;

	delete[] id_rson;
	id_rson = nullptr;

	delete[] id_dad;
	id_dad = nullptr;
#else
	if (lson)
	{
		farfree(lson);
	}

	if (rson)
	{
		farfree(rson);
	}

	if (dad)
	{
		farfree(dad);
	}
#endif
#endif

	son = nullptr; // Must be zeroed on shutdown!
#endif
}

/* initialize freq tree */
static void StartHuff()
{
	std::int16_t i, j;

#ifdef LZH_DYNAMIC_ALLOCATION
#ifdef LZH_ID_MEMORY_ALLOCATION

	// Assign _seg pointers to far pointers, always initialized here in case
	// the memory manager shifted things around after LZH_Startup() was called.
	//
	son = id_son;
	prnt = id_prnt;
	freq = id_freq;
	text_buf = id_text_buf;
	lson = id_lson;
	rson = id_rson;
	dad = id_dad;

#endif
#endif

	for (i = 0; i < N_CHAR; i++)
	{
		freq[i] = 1;
		son[i] = i + T;
		prnt[i + T] = i;
	}
	i = 0;
	j = N_CHAR;
	while (j <= R)
	{
		freq[j] = freq[i] + freq[i + 1];
		son[j] = i;
		prnt[i] = prnt[i + 1] = j;
		i += 2;
		j++;
	}
	freq[T] = 0xffff;
	prnt[R] = 0;

	printcount = 0;

	putbuf = putlen = match_position = match_length = 0;
}

/* reconstruct freq tree */
static void reconst()
{
	std::int16_t i, j, k;
	std::uint16_t f, l;

	/* halven cumulative freq for leaf nodes */

	j = 0;

	for (i = 0; i < T; i++)
	{
		if (son[i] >= T)
		{
			freq[j] = (freq[i] + 1) / 2;
			son[j] = son[i];
			j++;
		}
	}

	/* make a tree : first, connect children nodes */

	for (i = 0, j = N_CHAR; j < T; i += 2, j++)
	{
		k = i + 1;
		f = freq[j] = freq[i] + freq[k];

		for (k = j - 1; f < freq[k]; k--)
		{
		}

		k++;
		l = (j - k) * 2;

		memcpy(&freq[k + 1], &freq[k], l);
		freq[k] = f;

		memcpy(&son[k + 1], &son[k], l);
		son[k] = i;
	}

	/* connect parent nodes */

	for (i = 0; i < T; i++)
	{
		if ((k = son[i]) >= T)
		{
			prnt[k] = i;
		}
		else
		{
			prnt[k] = prnt[k + 1] = i;
		}
	}
}

// update freq tree
static void update(
	std::int16_t c)
{
	std::int16_t i, j, k, l;

	if (freq[R] == MAX_FREQ)
	{
		reconst();
	}

	c = prnt[c + T];

	do
	{
		k = ++freq[c];

		//
		// swap nodes to keep the tree freq-ordered
		//

		if (k > freq[l = c + 1])
		{
			while (k > freq[++l])
			{
			}

			l--;
			freq[c] = freq[l];
			freq[l] = k;

			i = son[c];
			prnt[i] = l;
			if (i < T)
			{
				prnt[i + 1] = l;
			}

			j = son[l];
			son[l] = i;

			prnt[j] = c;
			if (j < T)
			{
				prnt[j + 1] = c;
			}

			son[c] = j;

			c = l;
		}
	} while ((c = prnt[c]) != 0);       /* do it until reaching the root */
}


// ===========================================================================
//
// COMPRESSION ROUTINES
//
// ===========================================================================


#if INCLUDE_LZH_COMP
static void DeleteNode(
	std::int16_t p) /* Deleting node from the tree */
{
	std::int16_t q;

	if (dad[p] == NIL)
	{
		return; /* unregistered */

	}
	if (rson[p] == NIL)
	{
		q = lson[p];
	}
	else if (lson[p] == NIL)
	{
		q = rson[p];
	}
	else
	{
		q = lson[p];
		if (rson[q] != NIL)
		{
			do
			{
				q = rson[q];
			} while (rson[q] != NIL);

			rson[dad[q]] = lson[q];
			dad[lson[q]] = dad[q];
			lson[q] = lson[p];
			dad[lson[p]] = q;
		}

		rson[q] = rson[p];
		dad[rson[p]] = q;
	}

	dad[q] = dad[p];

	if (rson[dad[p]] == p)
	{
		rson[dad[p]] = q;
	}
	else
	{
		lson[dad[p]] = q;
	}

	dad[p] = NIL;
}

/* Inserting node to the tree */
static void InsertNode(
	std::int16_t r)
{
	std::int16_t i, p, cmp;
	std::uint8_t* key;
	std::uint16_t c;

	cmp = 1;
	key = &text_buf[r];
	p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;
	match_length = 0;
	for (;; )
	{
		if (cmp >= 0)
		{
			if (rson[p] != NIL)
			{
				p = rson[p];
			}
			else
			{
				rson[p] = r;
				dad[r] = p;
				return;
			}
		}
		else
		{
			if (lson[p] != NIL)
			{
				p = lson[p];
			}
			else
			{
				lson[p] = r;
				dad[r] = p;
				return;
			}
		}


		for (i = 1; i < F; i++)
		{
			if ((cmp = key[i] - text_buf[p + i]) != 0)
			{
				break;
			}
		}

		if (i > THRESHOLD)
		{
			if (i > match_length)
			{
				match_position = ((r - p) & (N - 1)) - 1;
				if ((match_length = i) >= F)
				{
					break;
				}
			}

			if (i == match_length)
			{
				if ((c = ((r - p) & (N - 1)) - 1) < match_position)
				{
					match_position = c;
				}
			}
		}
	}

	dad[r] = dad[p];
	lson[r] = lson[p];
	rson[r] = rson[p];
	dad[lson[p]] = r;
	dad[rson[p]] = r;

	if (rson[dad[p]] == p)
	{
		rson[dad[p]] = r;
	}
	else
	{
		lson[dad[p]] = r;
	}

	dad[p] = NIL; /* remove p */
}

/* Initializing tree */
static void InitTree()
{
	std::int16_t i;

	for (i = N + 1; i <= N + 256; i++)
	{
		rson[i] = NIL; /* root */

	}
	for (i = 0; i < N; i++)
	{
		dad[i] = NIL; /* node */
	}
}

// output c bits
static void Putcode(
	void*& outfile_ptr,
	std::int16_t l,
	std::uint16_t c)
{
	putbuf |= c >> putlen;

	putlen += l;

	if (putlen >= 8)
	{
		CIO_WritePtr(outfile_ptr, putbuf >> 8);
		++codesize;

		putlen -= 8;
		if (putlen >= 8)
		{
			CIO_WritePtr(outfile_ptr, static_cast<std::uint8_t>(putbuf));
			++codesize;

			putlen -= 8;
			putbuf = c << (l - putlen);
		}
		else
		{
			putbuf <<= 8;
		}
	}
}

static void EncodeChar(
	void*& outfile_ptr,
	std::uint16_t c)
{
	std::uint16_t i;
	std::int16_t j, k;

	i = 0;
	j = 0;
	k = prnt[c + T];

	/// search connections from leaf node to the root

	do
	{
		i >>= 1;

		//
		// if node's address is odd, output 1 else output 0
		//

		if ((k & 1) != 0)
		{
			i += 0x8000;
		}

		++j;
		k = prnt[k];
	} while (k != R);

	Putcode(outfile_ptr, j, i);

	code = i;
	len = j;
	update(c);
}

static void EncodePosition(
	void*& outfile_ptr,
	std::uint16_t c)
{
	std::uint16_t i;

	//
	// output upper 6 bits with encoding
	//

	i = c >> 6;
	Putcode(outfile_ptr, p_len[i], static_cast<std::uint16_t>(p_code[i]) << 8);

	//
	// output lower 6 bits directly
	//

	Putcode(outfile_ptr, 6, (c & 0x3F) << 10);
}

static void EncodeEnd(
	void*& outfile_ptr)
{
	if (putlen != 0)
	{
		CIO_WritePtr(outfile_ptr, putbuf >> 8);
		++codesize;
	}
}
#endif


// ===========================================================================
//
// DECOMPRESSION ROUTINES
//
// ===========================================================================


#if INCLUDE_LZH_DECOMP
// ---------------------------------------------------------------------------
// GetByte
// ---------------------------------------------------------------------------
static std::int16_t GetByte(
	const void*& infile_ptr,
	std::uint32_t* CompressLength)
{
	std::uint16_t i;

	while (getlen <= 8)
	{
		if (*CompressLength)
		{
			i = CIO_ReadPtr(infile_ptr);
			(*CompressLength)--;
		}
		else
		{
			i = 0;
		}

		getbuf |= i << (8 - getlen);
		getlen += 8;
	}

	i = getbuf;
	getbuf <<= 8;
	getlen -= 8;
	return i >> 8;
}

static std::int16_t GetBit(
	const void*& infile_ptr,
	std::uint32_t* CompressLength)
{
	std::int16_t i;

	while (getlen <= 8)
	{
		if (*CompressLength)
		{
			i = CIO_ReadPtr(infile_ptr);
			(*CompressLength)--;
		}
		else
		{
			i = 0;
		}

		getbuf |= i << (8 - getlen);
		getlen += 8;
	}

	i = getbuf;
	getbuf <<= 1;
	--getlen;
	return i < 0;
}

static std::int16_t DecodeChar(
	const void*& infile_ptr,
	std::uint32_t* CompressLength)
{
	std::uint16_t c;

	c = son[R];

	/*
	* start searching tree from the root to leaves.
	* choose node #(son[]) if input bit == 0
	* else choose #(son[]+1) (input bit == 1)
	*/

	while (c < T)
	{
		c += GetBit(infile_ptr, CompressLength);
		c = son[c];
	}

	c -= T;
	update(c);
	return c;
}

static std::int16_t DecodePosition(
	const void*& infile_ptr,
	std::uint32_t* CompressLength)
{
	std::uint16_t i;
	std::uint16_t j;
	std::uint16_t c;

	//
	// decode upper 6 bits from given table
	//

	i = GetByte(infile_ptr, CompressLength);
	c = static_cast<std::uint16_t>(d_code[i]) << 6;
	j = d_len[i];

	//
	// input lower 6 bits directly
	//

	j -= 2;
	while (j--)
	{
		i = (i << 1) + GetBit(infile_ptr, CompressLength);
	}

	return c | (i & 0x3F);
}
#endif


// ===========================================================================
//
// EXTERNAL REFERENCED
// COMPRESSION & DECOMPRESSION
// ROUTINES
//
// ===========================================================================

#if INCLUDE_LZH_DECOMP
int LZH_Decompress(
	const void* infile,
	void* outfile,
	std::uint32_t OriginalLength,
	std::uint32_t CompressLength)
{
	std::int16_t i, j, k, r, c;
	std::int32_t count;

	datasize = textsize = OriginalLength;
	getbuf = 0;
	getlen = 0;

	if (textsize == 0)
	{
		return 0;
	}

	StartHuff();
	for (i = 0; i < N - F; i++)
	{
		text_buf[i] = ' ';
	}

	r = N - F;

	for (count = 0; count < static_cast<std::int32_t>(textsize); )
	{
		c = DecodeChar(infile, &CompressLength);

		if (c < 256)
		{
			CIO_WritePtr(outfile, static_cast<std::uint8_t>(c));

			datasize--; // Dec # of bytes to write

			text_buf[r++] = static_cast<std::uint8_t>(c);
			r &= (N - 1);
			count++; // inc count of bytes written
		}
		else
		{
			i = (r - DecodePosition(infile, &CompressLength) - 1) & (N - 1);

			j = c - 255 + THRESHOLD;

			for (k = 0; k < j; k++)
			{
				c = text_buf[(i + k) & (N - 1)];

				CIO_WritePtr(outfile, static_cast<std::uint8_t>(c));

				datasize--; // dec count of bytes to write

				text_buf[r++] = static_cast<std::uint8_t>(c);
				r &= (N - 1);
				count++; // inc count of bytes written
			}
		}

		if (LZH_DecompressDisplayVector && (count > static_cast<std::int32_t>(printcount)))
		{
			LZH_DecompressDisplayVector(OriginalLength, OriginalLength - datasize);
			printcount += 1024;
		}
	}

	if (LZH_DecompressDisplayVector)
	{
		LZH_DecompressDisplayVector(OriginalLength, OriginalLength);
	}

	return count;
}
#endif

#if INCLUDE_LZH_COMP
int LZH_Compress(
	const void* infile,
	void* outfile,
	std::uint32_t DataLength)
{
	std::int16_t i;
	std::int16_t c;
	std::int16_t length;
	std::int16_t r;
	std::int16_t s;
	std::int16_t last_match_length;

	textsize = DataLength;

	if (textsize == 0)
	{
		return 0;
	}

	getbuf = 0;
	getlen = 0;
	textsize = 0; /* rewind and rescan */
	codesize = 0;
	datasize = 0; // Init our counter of ReadData...
	StartHuff();
	InitTree();

	s = 0;
	r = N - F;

	for (i = s; i < r; i++)
	{
		text_buf[i] = ' ';
	}

	for (length = 0; length < F && (DataLength > datasize); length++)
	{
		c = CIO_ReadPtr(infile);

		datasize++; // Dec num of bytes to compress
		text_buf[r + length] = static_cast<std::uint8_t>(c);
	}

	textsize = length;

	for (i = 1; i <= F; i++)
	{
		InsertNode(r - i);
	}

	InsertNode(r);

	do
	{
		if (match_length > length)
		{
			match_length = length;
		}

		if (match_length <= THRESHOLD)
		{
			match_length = 1;
			EncodeChar(outfile, text_buf[r]);
		}
		else
		{
			EncodeChar(outfile, 255 - THRESHOLD + match_length);
			EncodePosition(outfile, match_position);
		}

		last_match_length = match_length;

		for (i = 0; i < last_match_length && (DataLength > datasize); i++)
		{
			c = CIO_ReadPtr(infile);

			datasize++;

			DeleteNode(s);
			text_buf[s] = static_cast<std::uint8_t>(c);

			if (s < F - 1)
			{
				text_buf[s + N] = static_cast<std::uint8_t>(c);
			}

			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			InsertNode(r);
		}

		if (LZH_CompressDisplayVector && ((textsize += i) > printcount))
		{
			LZH_CompressDisplayVector(DataLength, datasize);
			printcount += 1024;
		}


		while (i++ < last_match_length)
		{
			DeleteNode(s);
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			if (--length)
			{
				InsertNode(r);
			}
		}

	} while (length > 0);

	EncodeEnd(outfile);

	if (LZH_CompressDisplayVector)
	{
		LZH_CompressDisplayVector(DataLength, DataLength);
	}

	return codesize;
}
#endif

/*
 * Copyright (c) 2003 Tony Bybell.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the   
 * Software is furnished to do so, subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,  
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL   
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING   
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef DEFS_LXTW_H
#define DEFS_LXTW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <zlib.h>

#define LXT2_WR_HDRID (0x1380)
#define LXT2_WR_VERSION (0x0001)

#define LXT2_WR_GRANULE_SIZE (64)
#define LXT2_WR_GRANULE_NUM (8)

#define LXT2_WR_GRAN_SECT_TIME 0
#define LXT2_WR_GRAN_SECT_DICT 1

#define LXT2_WR_GZWRITE_BUFFER 4096
#define LXT2_WR_SYMPRIME 65519

typedef unsigned long long lxttime_t;


#ifndef _MSC_VER
        #define LXT2_WR_LLD "%lld"
        #define LXT2_WR_LLDESC(x) x##LL
        #define LXT2_WR_ULLDESC(x) x##ULL
#else
        #define LXT2_WR_LLD "%I64d"
        #define LXT2_WR_LLDESC(x) x##i64
        #define LXT2_WR_ULLDESC(x) x##i64
#endif

#if LXT2_WR_GRANULE_SIZE > 32
typedef unsigned long long granmsk_t;
#define LXT2_WR_GRAN_0VAL (LXT2_WR_ULLDESC(0))
#define LXT2_WR_GRAN_1VAL (LXT2_WR_ULLDESC(1))
#else
typedef unsigned int granmsk_t;
#define LXT2_WR_GRAN_0VAL (0)   
#define LXT2_WR_GRAN_1VAL (1)
#endif


enum LXT2_WR_Encodings { 
	LXT2_WR_ENC_0,
	LXT2_WR_ENC_1,
	LXT2_WR_ENC_INV,
	LXT2_WR_ENC_LSH0,
	LXT2_WR_ENC_LSH1,
	LXT2_WR_ENC_RSH0,
	LXT2_WR_ENC_RSH1,

	LXT2_WR_ENC_ADD1,
	LXT2_WR_ENC_ADD2,
	LXT2_WR_ENC_ADD3,
	LXT2_WR_ENC_ADD4,

	LXT2_WR_ENC_SUB1,
	LXT2_WR_ENC_SUB2,
	LXT2_WR_ENC_SUB3,
	LXT2_WR_ENC_SUB4,

	LXT2_WR_ENC_X,
	LXT2_WR_ENC_Z,

	LXT2_WR_ENC_BLACKOUT,

	LXT2_WR_DICT_START,
	};

/*
 * integer splay
 */
typedef struct ds_tree_node ds_Tree;
struct ds_tree_node {
    ds_Tree * left, * right;
    granmsk_t item;
    int val;
    ds_Tree * next;
};


/*
 * string splay
 */
typedef struct dslxt_tree_node dslxt_Tree;
struct dslxt_tree_node {
    dslxt_Tree * left, * right;  
    char *item;
    unsigned int val;
    dslxt_Tree * next;
};
                                        

struct lxt2_wr_trace
{
FILE *handle;
gzFile zhandle;

dslxt_Tree *dict;		/* dictionary manipulation */
unsigned int num_dict_entries;
unsigned int dict_string_mem_required;
dslxt_Tree *dict_head;
dslxt_Tree *dict_curr;

ds_Tree *mapdict;		/* bitmap compression */
unsigned int num_map_entries;
ds_Tree *mapdict_head;
ds_Tree *mapdict_curr;

unsigned int position;
unsigned int zfacname_predec_size, zfacname_size, zfacgeometry_size;
unsigned int zpackcount;
unsigned int current_chunk, current_chunkz;

struct lxt2_wr_symbol *sym[LXT2_WR_SYMPRIME];
struct lxt2_wr_symbol **sorted_facs;
struct lxt2_wr_symbol *symchain;
int numfacs;
int numfacbytes;
int longestname;

int numsections;
unsigned int facname_offset, facgeometry_offset;

lxttime_t mintime, maxtime;
unsigned int timegranule;
int timescale;
int timepos;
unsigned int maxgranule;
lxttime_t firsttime, lasttime;
lxttime_t timetable[LXT2_WR_GRANULE_SIZE];

char *compress_fac_str;
int compress_fac_len;

lxttime_t flushtime;
unsigned flush_valid : 1;

unsigned do_strip_brackets : 1;
unsigned emitted : 1;			/* gate off change field zmode changes when set */
unsigned timeset : 1;			/* time has been modified from 0..0 */
unsigned bumptime : 1;			/* says that must go to next time position in granule as value change exists for current time */
unsigned granule_dirty : 1;		/* for flushing out final block */
unsigned blackout : 1;			/* blackout on/off */

char initial_value;

char zmode[4];				/* fills in with "wb0".."wb9" */
unsigned int gzbufpnt;
unsigned char gzdest[LXT2_WR_GZWRITE_BUFFER + 4];	/* enough for zlib buffering */
};


struct lxt2_wr_symbol
{
struct lxt2_wr_symbol *next;
struct lxt2_wr_symbol *symchain;
char *name;
int namlen;

int facnum;
struct lxt2_wr_symbol *aliased_to;

char *value;	/* fac's actual value */

unsigned int rows;
int msb, lsb;
int len;
int flags;

unsigned int chgpos;
granmsk_t msk;			/* must contain LXT2_WR_GRANULE_SIZE bits! */
unsigned int chg[LXT2_WR_GRANULE_SIZE];
};


#define LXT2_WR_SYM_F_BITS         (0)
#define LXT2_WR_SYM_F_INTEGER      (1<<0)
#define LXT2_WR_SYM_F_DOUBLE       (1<<1)
#define LXT2_WR_SYM_F_STRING       (1<<2)
#define LXT2_WR_SYM_F_TIME         (LXT2_WR_SYM_F_STRING)      /* user must correctly format this as a string */
#define LXT2_WR_SYM_F_ALIAS        (1<<3)

#define LXT2_WR_SYM_F_SIGNED       (1<<4)
#define LXT2_WR_SYM_F_BOOLEAN      (1<<5)
#define LXT2_WR_SYM_F_NATURAL      ((1<<6)|(LXT2_WR_SYM_F_INTEGER))
#define LXT2_WR_SYM_F_POSITIVE     ((1<<7)|(LXT2_WR_SYM_F_INTEGER))
#define LXT2_WR_SYM_F_CHARACTER    (1<<8)

#define LXT2_WR_SYM_F_CONSTANT     (1<<9)
#define LXT2_WR_SYM_F_VARIABLE     (1<<10)
#define LXT2_WR_SYM_F_SIGNAL       (1<<11)

#define LXT2_WR_SYM_F_IN           (1<<12)
#define LXT2_WR_SYM_F_OUT          (1<<13)
#define LXT2_WR_SYM_F_INOUT        (1<<14)

#define LXT2_WR_SYM_F_WIRE         (1<<15)
#define LXT2_WR_SYM_F_REG          (1<<16)


			/* file I/O */
struct lxt2_wr_trace *	lxt2_wr_init(const char *name);
void 			lxt2_wr_flush(struct lxt2_wr_trace *lt);
void 			lxt2_wr_close(struct lxt2_wr_trace *lt);


			/* 0 = no compression, 9 = best compression, 4 = default */
void			lxt2_wr_set_compression_depth(struct lxt2_wr_trace *lt, unsigned int depth);

			/* facility creation */
void                    lxt2_wr_set_initial_value(struct lxt2_wr_trace *lt, char value);
struct lxt2_wr_symbol *	lxt2_wr_symbol_find(struct lxt2_wr_trace *lt, const char *name);
struct lxt2_wr_symbol *	lxt2_wr_symbol_add(struct lxt2_wr_trace *lt, const char *name, unsigned int rows, int msb, int lsb, int flags);
struct lxt2_wr_symbol *	lxt2_wr_symbol_alias(struct lxt2_wr_trace *lt, const char *existing_name, const char *alias, int msb, int lsb);
void			lxt2_wr_symbol_bracket_stripping(struct lxt2_wr_trace *lt, int doit);

			/* each granule is LXT2_WR_GRANULE_SIZE (32 or 64) timesteps, default is 8 per section */
void 			lxt2_wr_set_maxgranule(struct lxt2_wr_trace *lt, unsigned int maxgranule);

			/* time ops */
void 			lxt2_wr_set_timescale(struct lxt2_wr_trace *lt, int timescale);
int 			lxt2_wr_set_time(struct lxt2_wr_trace *lt, unsigned int timeval);
int 			lxt2_wr_inc_time_by_delta(struct lxt2_wr_trace *lt, unsigned int timeval);
int 			lxt2_wr_set_time64(struct lxt2_wr_trace *lt, lxttime_t timeval);
int 			lxt2_wr_inc_time_by_delta64(struct lxt2_wr_trace *lt, lxttime_t timeval);

                        /* allows blackout regions in LXT files */
void                    lxt2_wr_set_dumpoff(struct lxt2_wr_trace *lt);
void                    lxt2_wr_set_dumpon(struct lxt2_wr_trace *lt);

			/* left fill on bit_string uses vcd semantics (left fill with value[0] unless value[0]=='1', then use '0') */
int 			lxt2_wr_emit_value_int(struct lxt2_wr_trace *lt, struct lxt2_wr_symbol *s, unsigned int row, int value);
int 			lxt2_wr_emit_value_double(struct lxt2_wr_trace *lt, struct lxt2_wr_symbol *s, unsigned int row, double value);
int 			lxt2_wr_emit_value_string(struct lxt2_wr_trace *lt, struct lxt2_wr_symbol *s, unsigned int row, char *value);
int 			lxt2_wr_emit_value_bit_string(struct lxt2_wr_trace *lt, struct lxt2_wr_symbol *s, unsigned int row, char *value);

			/* old lxt dummy functions (might be used later) */
void                    lxt2_wr_set_no_interlace(struct lxt2_wr_trace *lt);
void                    lxt2_wr_set_chg_compress(struct lxt2_wr_trace *lt);
void                    lxt2_wr_set_clock_compress(struct lxt2_wr_trace *lt);
void                    lxt2_wr_set_dict_compress(struct lxt2_wr_trace *lt, unsigned int minwidth);

#endif

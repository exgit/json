/* json.c - json reader/writer for C language
 *
 * (c) 2018 Oleg Alexeev <oleg.alexeev@inbox.ru> (https://github.com/exgit)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <limits.h>     // for constants
#include <float.h>      // for FLT_MAX
#include <stdlib.h>     // for strtod()
#include <stdio.h>      // for snprintf()
#include <string.h>     // for memcpy()
#include <math.h>
#include "json.h"


typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef long long int64;


//! character types
enum {
    CNV,    // invalid characters
    CBL,    // blank symbols ' ', '\t', '\n', '\r'
    CMN,    // minus '-'
    CPT,    // point
    CNM,    // numbers '0-9'
    CLT,    // letters '_', 'a-z', 'A-Z'
    CQT,    // quotes "'", '"'
    CCM,    // comma ','
    CCL,    // colon ':'
    CAS,    // array start '['
    CAE,    // array end ']'
    COS,    // object start '{'
    COE,    // object end '}'
    CSL     // slash '/'
};


//! token types
typedef enum {
    JINSTART,   // input start
    JINEND,     // input end
    JASTART,    // '[' - array start
    JAEND,      // ']' - array end
    JOSTART,    // '{' - object start
    JOEND,      // '}' - object end
    JCOMMA,     // ',' - comma
    JNUM,       // number
    JSTR,       // string
    JNAME,      // object attribute name
    JERROR      // none of the above
} jtt;


//! parsing context type
typedef enum {
    CTXVAL,     // parsing singular value
    CTXARR,     // parsing json array
    CTXOBJ      // parsing json object
} jctx;


//! token
typedef struct {
    jtt type;   // token type
    uint len;   // token length
    uint pos;   // position inside json reader
} jtok;


//! read function`s stacked data
struct jrstk {
    struct jrstk *prev; // ptr to previous stacked data in stack
    jtok tokp;          // previous token
    uint ms;            // mapping size
    uint mi;            // current mapping index
    jctx ctx;           // parsing context
    union {
        atr *poa;       // ptr to first mapping of object attributes
        elt *pae;       // ptr to first mapping of array elements
    };
    uint amis;          // array of mapping indexes size
    uint *ami;          // array of mapping indexes
};


//! read function`s all data
struct jrdat {
    const char *start;  // json string start
    size_t len;         // json string length
    uint pos;           // current position in json string
    jtok tokc;          // current token
    int ssize;          // stack size
    struct jrstk *s;    // ptr to top stacked data
    void *pmfree;       // ptr to free pool memory
    int pool[1000];     // memory pool for stacked data
};


//! write function`s all data
struct jwdat {
    char *start;
    size_t len;
    uint pos;
};


static int json_read_push(struct jrdat *d);
static int json_read_pop(struct jrdat *d);
static void *json_read_alloc(struct jrdat *d, uint size);
static void json_read_next(struct jrdat *d);
static void json_read_find_mi(struct jrdat *d);
static int json_read_str(struct jrdat *d, uint size, char *p);
static int json_read_int(struct jrdat *d, uint size, void *p);
static int json_read_float(struct jrdat *d, uint size, void *p);
static void json_write_bytes(struct jwdat *d, size_t size, const char *p);
static void json_write_int(struct jwdat *d, size_t size, void *p);
static void json_write_flt(struct jwdat *d, size_t size, void *p);
static void json_write_str(struct jwdat *d, size_t size, void *p);
static void json_write_arr(struct jwdat *d, size_t size, elt *pe);
static void json_write_obj(struct jwdat *d, size_t size, atr *pa);


//! character type translation table
static uchar ct[256] = {
//    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F	//
    CNV, CNV, CNV, CNV, CNV, CNV, CNV, CNV, CNV, CBL, CBL, CNV, CNV, CBL, CNV, CNV,	// 00
    CNV, CNV, CNV, CNV, CNV, CNV, CNV, CNV, CNV, CNV, CNV, CNV, CNV, CNV, CNV, CNV,	// 10
    CBL, CNV, CQT, CNV, CNV, CNV, CNV, CQT, CNV, CNV, CNV, CNV, CCM, CMN, CPT, CSL,	// 20
    CNM, CNM, CNM, CNM, CNM, CNM, CNM, CNM, CNM, CNM, CCL, CNV, CNV, CNV, CNV, CNV,	// 30
    CNV, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT,	// 40
    CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CAS, CNV, CAE, CNV, CLT,	// 50
    CNV, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT,	// 60
    CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, CLT, COS, CNV, COE, CNV, CNV	// 70
};


//! json reader
int json_read(elt *map, const char *json, size_t jsonsize) {
    struct jrdat d; // data accessible for all json_read_* family of functions
    jtt t;          // type of previous token

    if (!map || !json || !jsonsize) return 1;

    d.start = json;
    d.len = jsonsize;
    d.pos = 0;
    d.tokc.type = JINSTART;
    d.tokc.pos = 0;
    d.tokc.len = 0;

    d.ssize = 1;
    d.s = (struct jrstk *)d.pool;
    d.s->prev = 0;
    d.s->ctx = CTXVAL;
    d.s->pae = map;
    d.s->ms = 1;
    d.s->mi = 0;

    d.pmfree = d.s + 1;

    for (;;) {
        t = d.tokc.type;
        d.s->tokp = d.tokc;
        json_read_next(&d);
        switch (d.tokc.type) {
        case JINEND:
            if (d.s->ctx != CTXVAL) return 1;
            if (t==JINSTART) return 1;
            return 0;
        case JASTART:
            if (d.s->ctx == CTXVAL) {if (t!=JINSTART) return 1;}
            else if (d.s->ctx == CTXARR) {if (t!=JASTART && t!=JCOMMA) return 1;}
            else {if (t!=JNAME) return 1;}
            if (json_read_push(&d)) return 1;
            break;
        case JOSTART:
            if (d.s->ctx == CTXVAL) {if (t!=JINSTART) return 1;}
            else if (d.s->ctx == CTXARR) {if (t!=JASTART && t!=JCOMMA) return 1;}
            else {if (t!=JNAME) return 1;}
            if (json_read_push(&d)) return 1;
            break;
        case JAEND:
            if (d.s->ctx != CTXARR) return 1;
            if (t == JCOMMA) return 1;
            if (json_read_pop(&d)) return 1;
            break;
        case JOEND:
            if (d.s->ctx != CTXOBJ) return 1;
            if (t==JCOMMA || t==JNAME) return 1;
            if (json_read_pop(&d)) return 1;
            break;
        case JCOMMA:
            if (d.s->ctx == CTXVAL) return 1;
            else if (d.s->ctx == CTXARR) {if (t==JASTART) return 1; d.s->mi++;}
            else {if (t==JOSTART || t==JNAME) return 1;}
            break;
        case JNUM:
            if (d.s->ctx == CTXVAL) {if (t!=JINSTART) return 1;}
            else if (d.s->ctx == CTXARR) {if (t!=JASTART && t!=JCOMMA) return 1;}
            else {if (t!=JNAME) return 1;}
            if (d.s->mi < d.s->ms) {
                int res = 0;
                if (d.s->ctx == CTXOBJ) {
                    atr *oa = &d.s->poa[d.s->mi];
                    if (oa->type == MINT)
                        res = json_read_int(&d, oa->size, oa->ptr);
                    else if (oa->type == MFLT)
                        res = json_read_float(&d, oa->size, oa->ptr);
                } else {
                    elt *ae = &d.s->pae[d.s->mi];
                    if (ae->type == MINT)
                        res = json_read_int(&d, ae->size, ae->ptr);
                    else if (ae->type == MFLT)
                        res = json_read_float(&d, ae->size, ae->ptr);
                }
                if (res) return 1;
            }
            break;
        case JSTR:
            if (d.s->ctx == CTXVAL) {if (t!=JINSTART) return 1;}
            else if (d.s->ctx == CTXARR) {if (t!=JASTART && t!=JCOMMA) return 1;}
            else {if (t!=JNAME) return 1;}
            if (d.s->mi < d.s->ms) {
                int res = 0;
                if (d.s->ctx == CTXOBJ) {
                    atr *oa = &d.s->poa[d.s->mi];
                    if (oa->type == MSTR)
                        res = json_read_str(&d, oa->size, oa->ptr);
                } else {
                    elt *ae = &d.s->pae[d.s->mi];
                    if (ae->type == MSTR)
                        res = json_read_str(&d, ae->size, ae->ptr);
                }
                if (res) return 1;
            }
            break;
        case JNAME:
            if (d.s->ctx != CTXOBJ) return 1;
            if (t!=JOSTART && t!=JCOMMA) return 1;
            json_read_find_mi(&d);
            break;
        default:
            return 1;
        }
    }
}


//! push current state into stack
static int json_read_push(struct jrdat *d) {
    void *next = (struct jrstk*)d->pmfree + 1;  // next free memory block
    if (next >= (void*)((char*)d->pool + sizeof(d->pool))) {
        return 1;
    }
    struct jrstk *ps = d->s;    // previous state
    d->s = (struct jrstk*)d->pmfree;
    d->pmfree = next;
    d->ssize++;
    d->s->prev = ps;
    d->s->tokp = d->tokc;
    d->s->ctx = d->tokc.type==JOSTART ? CTXOBJ : CTXARR;
    d->s->ms = 0;
    d->s->mi = 0;
    d->s->pae = 0;
    d->s->amis = 0;
    d->s->ami = 0;

    if (ps->mi >= ps->ms) return 0;

    if (ps->ctx == CTXOBJ) {
        atr *oa = &ps->poa[ps->mi];
        if (oa->type == MARR && d->tokc.type == JASTART) {
            d->s->ms = oa->size;
            d->s->pae = oa->ptr;
        } else if (oa->type == MOBJ && d->tokc.type == JOSTART) {
            d->s->ms = oa->size;
            d->s->poa = oa->ptr;
        }
    } else {
        elt *ae = &ps->pae[ps->mi];
        if (ae->type == MARR && d->tokc.type == JASTART) {
            d->s->ms = ae->size;
            d->s->pae = ae->ptr;
        } else if (ae->type == MOBJ && d->tokc.type == JOSTART) {
            d->s->ms = ae->size;
            d->s->poa = ae->ptr;
        }
    }

    if (d->s->ctx==CTXOBJ && d->s->ms > 0) {
        d->s->amis = d->s->ms;
        d->s->ami = json_read_alloc(d, d->s->ms * sizeof(*d->s->ami));
        if (d->s->ami == 0) return 1;
        uint i = 0;
        for (; i < d->s->ms; i++) d->s->ami[i] = i;
    }

    return 0;
}


//! pop current state from stack
static int json_read_pop(struct jrdat *d) {
    if (d->ssize <= 1) {
        return 1;
    }
    d->pmfree = d->s;
    d->ssize--;
    d->s = d->s->prev;
    return 0;
}


//! allocate memory from available pool
static void *json_read_alloc(struct jrdat *d, uint size) {
    size += (~size + 1) & 3u;   // round up to 4 bytes
    void *next = (char*)d->pmfree + size;
    if (next >= (void*)((char*)d->pool + sizeof(d->pool))) {
        return 0;
    }
    void *ret = d->pmfree;
    d->pmfree = next;
    return ret;
}


//! read next token from json stream
static void json_read_next(struct jrdat *d) {
    const uchar *jsn = (const uchar *)d->start;
    const uint len = d->len;
    uint pos = d->pos;
    jtok *tok = &d->tokc;
    int t = CNV;

    // skip spaces
    for (; pos < len; pos++) {
        t = ct[jsn[pos]]; if (t != CBL) break;
    }

    // check for input end
    if (pos >= len) {tok->type = JINEND; goto exit;}

    // check for array start
    if (t == CAS) {tok->type = JASTART; goto exit1;}

    // check for array end
    if (t == CAE) {tok->type = JAEND; goto exit1;}

    // check for object start
    if (t == COS) {tok->type = JOSTART; goto exit1;}

    // check for object end
    if (t == COE) {tok->type = JOEND; goto exit1;}

    // check for comma
    if (t == CCM) {tok->type = JCOMMA; goto exit1;}

    // check for number
    if (t == CMN || t == CNM) {
        tok->type = JNUM;
        tok->pos = pos++;
        tok->len = 1;
        for (; pos < len; pos++) {
            t = ct[jsn[pos]]; if (t != CNM) break; tok->len++;
        }
        if (pos >= len) goto exit;
        if (jsn[pos] == '.') {
            tok->len++; if (++pos >= len) goto error;
            t = ct[jsn[pos]]; if (t != CNM) goto error;
            tok->len++; pos++;
            for (; pos < len; pos++) {
                t = ct[jsn[pos]]; if (t != CNM) break; tok->len++;
            }
            if (pos >= len) goto exit;
        }
        if (jsn[pos]=='e' || jsn[pos]=='E') {
            tok->len++; if (++pos >= len) goto error;
            if (jsn[pos]=='-' || jsn[pos]=='+') {
                tok->len++; if (++pos >= len) goto error;
            }
            t = ct[jsn[pos]]; if (t != CNM) goto error;
            tok->len++; pos++;
            for (; pos < len; pos++) {
                t = ct[jsn[pos]]; if (t != CNM) break; tok->len++;
            }
        }
        goto exit;
    }

    // check for string or an object attribute name with quotes
    if (t == CQT) {
        int c = jsn[pos];
        tok->type = JERROR;
        tok->pos = ++pos;
        tok->len = 0;
        for (; pos < len; pos++) {
            if (jsn[pos] == c && jsn[pos-1] != '\\') {
                tok->type = JSTR; pos++; break;
            }
            tok->len++;
        }
        for (; pos < len; pos++) {
            t = ct[jsn[pos]]; if (t != CBL) break;
        }
        if (pos >= len) goto exit;
        if (t == CCL) {
            tok->type = JNAME;
            pos++;
            if (ct[jsn[tok->pos]] != CLT) goto error;
            uint i = 1;
            for (; i < tok->len; i++) {
                t = ct[jsn[tok->pos+i]]; if (t != CLT && t != CNM) goto error;
            }
        }
        goto exit;
    }

    // check for object attribute name without quotes
    if (t == CLT) {
        tok->type = JNAME;
        tok->pos = pos++;
        tok->len = 1;
        for (; pos < len; pos++) {
            t = ct[jsn[pos]]; if (t != CLT && t != CNM) break; tok->len++;
        }
        for (; pos < len; pos++) {
            t = ct[jsn[pos]]; if (t != CBL) break;
        }
        if (pos >= len) goto error;
        if (t != CCL) goto error;
        goto exit1;
    }

    // if none of the above than error
    goto error;

exit1:
    pos++;
exit:
    d->pos = pos;
    return;

error:
    tok->type = JERROR;
    return;
}


//! find mapping index for attribute name
static void json_read_find_mi(struct jrdat *d) {
    const char *psz;
    const char *psc = d->start + d->tokc.pos;
    uint sclen = d->tokc.len;

    uint i, j, len = d->s->amis;
    for (i=0; i < len; i++) {
        psz = d->s->poa[d->s->ami[i]].name;
        for (j=0; psz[j] && psz[j]==psc[j] && j<sclen; j++) continue;
        if (psz[j]==0 && j==sclen) break;
    }
    if (i == len) {d->s->mi = d->s->ms; return;}

    d->s->mi = d->s->ami[i];
    if (i < len/2) {
        for (; i > 0; i--) d->s->ami[i] = d->s->ami[i-1];
        d->s->ami++;
    } else {
        for (; i < len-1; i++) d->s->ami[i] = d->s->ami[i+1];
    }
    d->s->amis--;
}


//! copy string from json to C
static int json_read_str(struct jrdat *d, uint size, char *p) {
    if (size == 0 || p == 0) return 0;
    if (size >= d->tokc.len + 1) size = d->tokc.len;
    else size--;
    uint i = 0;
    const char *s = d->start + d->tokc.pos;
    for (; i < size; i++) {
        char c = s[i];
        if (c == '\\' && i < size-1) {
            c = s[++i];
            if (c == '"') *p++ = '"';
            else if (c == '\\') *p++ = '\\';
            else if (c == '/') *p++ = '/';
            else if (c == 'b') *p++ = '\b';
            else if (c == 'f') *p++ = '\f';
            else if (c == 'n') *p++ = '\n';
            else if (c == 'r') *p++ = '\r';
            else if (c == 't') *p++ = '\t';
            else {*p++ = '\\'; *p++ = c;}
        }
        else {
            *p++ = c;
        }
    }
    *p = 0;
    return 0;
}


//! copy integer number from json to C
static int json_read_int(struct jrdat *d, uint size, void *p) {
    char buf[256];

    if (d->tokc.len >= sizeof(buf)) return 1;
    memcpy(buf, d->start + d->tokc.pos, d->tokc.len);
    buf[d->tokc.len] = 0;

    double val = strtod(buf, 0);
    if (size == sizeof(char)) {
        if (val > SCHAR_MAX) *(char*)p = SCHAR_MAX;
        else if (val < SCHAR_MIN) *(char*)p = SCHAR_MIN;
        else *(char*)p = (char)val;
    } else if (size == sizeof(short)) {
        if (val > SHRT_MAX) *(short*)p = SHRT_MAX;
        else if (val < SHRT_MIN) *(short*)p = SHRT_MIN;
        else *(short*)p = (short)val;
    } else if (size == sizeof(int)) {
        if (val > INT_MAX) *(int*)p = INT_MAX;
        else if (val < INT_MIN) *(int*)p = INT_MIN;
        else *(int*)p = (int)val;
    } else if (size == sizeof(long long)) {
        if (val > LLONG_MAX) *(long long*)p = LLONG_MAX;
        else if (val < LLONG_MIN) *(long long*)p = LLONG_MIN;
        else *(long long*)p = (long long)val;
    } else {
        return 1;
    }

    return 0;
}


//! copy floating point number from json to C
static int json_read_float(struct jrdat *d, uint size, void *p) {
    char buf[256];

    if (d->tokc.len >= sizeof(buf)) return 1;
    memcpy(buf, d->start + d->tokc.pos, d->tokc.len);
    buf[d->tokc.len] = 0;

    double val = strtod(buf, 0);
    if (size == sizeof(float)) {
        if (val > (double)FLT_MAX) val = (double)FLT_MAX;
        *(float*)p = (float)val;
    } else if (size == sizeof(double)) {
        *(double*)p = val;
    } else {
        return 1;
    }

    return 0;
}


//! json writer
int json_write(void *buf, size_t bufsize, elt *map) {
    if (!buf || !bufsize || !map) {
        return 1;
    }

    struct jwdat d;
    d.start = buf;
    d.len = bufsize - 1;
    d.pos = 0;

    switch (map->type) {
    case MINT: json_write_int(&d, map->size, map->ptr); break;
    case MFLT: json_write_flt(&d, map->size, map->ptr); break;
    case MSTR: json_write_str(&d, map->size, map->ptr); break;
    case MARR: json_write_arr(&d, map->size, map->ptr); break;
    case MOBJ: json_write_obj(&d, map->size, map->ptr); break;
    };
    d.start[d.pos] = 0;

    return 0;
}


//! raw bytes writer
static void json_write_bytes(struct jwdat *d, size_t size, const char *p) {
    for (; size && d->pos < d->len; size--) {
        d->start[d->pos++] = *p++;
    }
}


//! int value writer
static void json_write_int(struct jwdat *d, size_t size, void *p) {
    char buf[128];
    if (size == sizeof(char))
        snprintf(buf, sizeof(buf), "%d", *(char*)p);
    else if (size == sizeof(short))
        snprintf(buf, sizeof(buf), "%d", *(short*)p);
    else if (size == sizeof(int))
        snprintf(buf, sizeof(buf), "%d", *(int*)p);
    else if (size == sizeof(int64))
        snprintf(buf, sizeof(buf), "%lld", *(int64*)p);
    else
        return;
    json_write_bytes(d, strlen(buf), buf);
}


//! float value writer
static void json_write_flt(struct jwdat *d, size_t size, void *p) {
    char buf[128];
    if (size == sizeof(float))
        snprintf(buf, sizeof(buf), "%g", (double)*(float*)p);
    else if (size == sizeof(double))
        snprintf(buf, sizeof(buf), "%g", *(double*)p);
    else
        return;
    json_write_bytes(d, strlen(buf), buf);
}


//! string value writer
static void json_write_str(struct jwdat *d, size_t size, void *p) {
    json_write_bytes(d, 1, "\"");
    json_write_bytes(d, size, p);
    json_write_bytes(d, 1, "\"");
}


//! array writer
static void json_write_arr(struct jwdat *d, size_t size, elt *pe) {
    json_write_bytes(d, 1, "[");
    size_t i = 0;
    for (; i < size; i++,pe++) {
        switch (pe->type) {
        case MINT: json_write_int(d, pe->size, pe->ptr); break;
        case MFLT: json_write_flt(d, pe->size, pe->ptr); break;
        case MSTR: json_write_str(d, pe->size, pe->ptr); break;
        case MARR: json_write_arr(d, pe->size, pe->ptr); break;
        case MOBJ: json_write_obj(d, pe->size, pe->ptr); break;
        };
        if (i < size - 1) json_write_bytes(d, 1, ",");
    }
    json_write_bytes(d, 1, "]");
}


//! object writer
static void json_write_obj(struct jwdat *d, size_t size, atr *pa) {
    json_write_bytes(d, 1, "{");
    size_t i = 0;
    for (; i < size; i++,pa++) {
        json_write_bytes(d, strlen(pa->name), pa->name);
        json_write_bytes(d, 1, ":");
        switch (pa->type) {
        case MINT: json_write_int(d, pa->size, pa->ptr); break;
        case MFLT: json_write_flt(d, pa->size, pa->ptr); break;
        case MSTR: json_write_str(d, pa->size, pa->ptr); break;
        case MARR: json_write_arr(d, pa->size, pa->ptr); break;
        case MOBJ: json_write_obj(d, pa->size, pa->ptr); break;
        };
        if (i < size - 1) json_write_bytes(d, 1, ",");
    }
    json_write_bytes(d, 1, "}");
}

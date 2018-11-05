/* json.h - json reader/writer for C language
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


#include <stddef.h>


// mapping types
typedef enum {
	MINT,	// json number => C char, short or int
	MFLT,	// json number => C float or double
	MSTR,	// json string => C char[N]
	MARR,	// json array  => nested mapping
	MOBJ	// json object => nested mapping
} map_t;


// mapping between json array element and C data
typedef struct {
	map_t type;		// mapping type
	size_t size;	// size of object pointed by ptr
	void *ptr;		// ptr to C data or nested mapping
} elt;


// mapping between json object attribute and C data
typedef struct {
	const char *name;	// attribute name
	map_t type;			// mapping type
	size_t size;		// size of object pointed by ptr
	void *ptr;			// ptr to C data or nested mapping
} atr;


// parse json and store data to C variables
int json_read(elt *map, const char *json, size_t jsonsize);


// generate json from data in C variables
int json_write(void *buf, size_t bufsize, elt *map);

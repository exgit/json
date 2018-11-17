Here is a C language library for reading and writing data in json format.

It is designed for maximum json decoding speed. This goal is achieved by not doing memory allocations
during parsing process. Values encountered while parsing are immediately stored into C variables
using caller provided mapping.

Library understands strict json syntax and a relaxed variant of it where:
* strings can be quoted not only with double quotes (") but also with single quotes (')
* object attribute names can be written without quotes

## usage examples

#### Read single integer value encoded in json:

```C
#include <stdio.h>
#include <string.h>
#include "json.h"

int main()
{
  int res, n = 0;
  elt map = {MINT, sizeof(n), &n};
  const char *j = "123";
  res = json_read(&map, j, strlen(j));
  if (res) return 1;
  printf("%d\n", n);
  return 0;
}
```

#### Read single float value encoded in json:

```C
double f = 0.0;
elt map = {MFLT, sizeof(f), &f};
const char *j = "3.141";
res = json_read(&map, j, strlen(j));
if (res) return 1;
printf("%g\n", f);
```

#### Read single string value encoded in json:

```C
char buf[256];
elt map = {MSTR, sizeof(buf), &buf};
const char *j = "'Hello json!'";
res = json_read(&map, j, strlen(j));
if (res) return 1;
printf("%s\n", buf);
```

#### Read json array

```C
int n = 0;
double f = 0.0;
char buf[256];
elt arr[] = {
    {MINT, sizeof(n), &n},
    {MFLT, sizeof(f), &f},
    {MSTR, sizeof(buf), &buf}
};
elt map = {MARR, 3, &arr};
const char *j = "[77, 7.77, 'json array example']";
res = json_read(&map, j, strlen(j));
if (res) return 1;
printf("%d\n", n);
printf("%g\n", f);
printf("%s\n", buf);
```

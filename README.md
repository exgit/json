# json
C language library for json data format.

It is designed for maximum json decoding speed. This goal is achieved by not doing memory allocations
during parsing process. Values encountered while parsing are immediately stored into C variables
using caller provided mapping.

## usage examples

#### 1. Read single integer value encoded in json:

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

#### 2. Read single float value encoded in json:

```C
double f = 0.0;
elt map = {MFLT, sizeof(f), &f};
const char *j = "3.141";
res = json_read(&map, j, strlen(j));
if (res) return 1;
printf("%g\n", f);
```

#### 3. Read single string value encoded in json:

```C
char buf[256];
elt map = {MSTR, sizeof(buf), &buf};
const char *j = "'Hello json!'";
res = json_read(&map, j, strlen(j));
if (res) return 1;
printf("%s\n", buf);
```

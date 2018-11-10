/* test.c - json library test suit
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


#include <stdio.h>
#include <string.h>
#include "json.h"


static int ValidSingleValues(void);
static int InvalidSingleValues(void);
static int ValidArrays(void);
static int InvalidArrays(void);
static int ValidObjects(void);
static int InvalidObjects(void);
static int ValidStrings(void);
static int InvalidStrings(void);
static int SomeComplexExamples(void);


typedef int (*testfunc)(void);


static testfunc tests[] = {
    ValidSingleValues, InvalidSingleValues,
    ValidArrays, InvalidArrays,
    ValidObjects, InvalidObjects,
    ValidStrings, InvalidStrings,
    SomeComplexExamples,
    0
};


int main()
{
    // test suite
    printf("Testing...\n\n");
    int i, res;
    for (i=0; tests[i]; i++) {
        res = (tests[i])();
        if (res == 0) printf("success.\n");
        else printf("FAILURE.\n");
    }

    // example of json writing
    int n1 = 5;
    int n2 = 7;
    char *str1 = "Hello";
    char *str2 = "json";
    float f1 = 3.141f;
    char buf[256];
    json_write(buf, sizeof(buf), &(elt){MARR, 4, &(elt[]){
        {MINT, sizeof(n1), &n1},
        {MINT, sizeof(n2), &n2},
        {MOBJ, 2, &(atr[]){
            {"word1", MSTR, strlen(str1), str1},
            {"word2", MSTR, strlen(str2), str2}
        }},
        {MFLT, sizeof(f1), &f1}
    }});
    printf("\n\nJson writing example:\n%s\n\n", buf);

    printf("\n");
    return 0;
}


static int ValidSingleValues(void) {
    char cnum = 0;
    short snum = 0;
    int inum = 0;
    float fnum = 0;
    float epsf = 0.000001f;
    double dnum = 0;
    double epsd = 0.000000001;
    char str[256] = {0};
    const char *j;
    int res;

    printf("%-24s - ", __func__);

    j = "1";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res || cnum!=1) return 1;
    j = "64";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res || cnum!=64) return 1;
    j = "127";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res || cnum!=127) return 1;
    j = "128";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res || cnum!=127) return 1;
    j = "-1";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res || cnum != -1) return 1;
    j = "-64";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res || cnum != -64) return 1;
    j = "-128";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res || cnum != -128) return 1;
    j = "-129";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res || cnum != -128) return 1;

    j = "1337";
    res = json_read(&(elt){MINT, sizeof(snum), &snum}, j, strlen(j));
    if (res || snum != 1337) return 1;
    j = "32767";
    res = json_read(&(elt){MINT, sizeof(snum), &snum}, j, strlen(j));
    if (res || snum != 32767) return 1;
    j = "32768";
    res = json_read(&(elt){MINT, sizeof(snum), &snum}, j, strlen(j));
    if (res || snum != 32767) return 1;
    j = "-1337";
    res = json_read(&(elt){MINT, sizeof(snum), &snum}, j, strlen(j));
    if (res || snum != -1337) return 1;
    j = "-32768";
    res = json_read(&(elt){MINT, sizeof(snum), &snum}, j, strlen(j));
    if (res || snum != -32768) return 1;
    j = "-32769";
    res = json_read(&(elt){MINT, sizeof(snum), &snum}, j, strlen(j));
    if (res || snum != -32768) return 1;

    j = "13371337";
    res = json_read(&(elt){MINT, sizeof(inum), &inum}, j, strlen(j));
    if (res || inum != 13371337) return 1;
    j = "2147483647";
    res = json_read(&(elt){MINT, sizeof(inum), &inum}, j, strlen(j));
    if (res || inum != 2147483647) return 1;
    j = "2147483648";
    res = json_read(&(elt){MINT, sizeof(inum), &inum}, j, strlen(j));
    if (res || inum != 2147483647) return 1;
    j = "-13371337";
    res = json_read(&(elt){MINT, sizeof(inum), &inum}, j, strlen(j));
    if (res || inum != -13371337) return 1;
    j = "-2147483648";
    res = json_read(&(elt){MINT, sizeof(inum), &inum}, j, strlen(j));
    if (res || inum != -2147483648) return 1;
    j = "-2147483649";
    res = json_read(&(elt){MINT, sizeof(inum), &inum}, j, strlen(j));
    if (res || inum != -2147483648) return 1;

    j = "3.1415926";
    res = json_read(&(elt){MFLT, sizeof(fnum), &fnum}, j, strlen(j));
    if (res || fnum < 3.1415926f-epsf || fnum > 3.1415926f+epsf) return 1;
    j = "3.1415926e1";
    res = json_read(&(elt){MFLT, sizeof(fnum), &fnum}, j, strlen(j));
    if (res || fnum < 31.415926f-epsf || fnum > 31.415926f+epsf) return 1;
    j = "3.1415926e+1";
    res = json_read(&(elt){MFLT, sizeof(fnum), &fnum}, j, strlen(j));
    if (res || fnum < 31.415926f-epsf || fnum > 31.415926f+epsf) return 1;
    j = "3.1415926e-1";
    res = json_read(&(elt){MFLT, sizeof(fnum), &fnum}, j, strlen(j));
    if (res || fnum < 0.31415926f-epsf || fnum > 0.31415926f+epsf) return 1;
    j = "-3.1415926";
    res = json_read(&(elt){MFLT, sizeof(fnum), &fnum}, j, strlen(j));
    if (res || fnum < -3.1415926f-epsf || fnum > -3.1415926f+epsf) return 1;
    j = "-3.1415926e1";
    res = json_read(&(elt){MFLT, sizeof(fnum), &fnum}, j, strlen(j));
    if (res || fnum < -31.415926f-epsf || fnum > -31.415926f+epsf) return 1;
    j = "-3.1415926e+1";
    res = json_read(&(elt){MFLT, sizeof(fnum), &fnum}, j, strlen(j));
    if (res || fnum < -31.415926f-epsf || fnum > -31.415926f+epsf) return 1;
    j = "-3.1415926e-1";
    res = json_read(&(elt){MFLT, sizeof(fnum), &fnum}, j, strlen(j));
    if (res || fnum < -0.31415926f-epsf || fnum > -0.31415926f+epsf) return 1;

    j = "2.718281828459";
    res = json_read(&(elt){MFLT, sizeof(dnum), &dnum}, j, strlen(j));
    if (res || dnum < 2.718281828459-epsd || dnum > 2.718281828459+epsd) return 1;
    j = "2.718281828459e1";
    res = json_read(&(elt){MFLT, sizeof(dnum), &dnum}, j, strlen(j));
    if (res || dnum < 27.18281828459-epsd || dnum > 27.18281828459+epsd) return 1;
    j = "2.718281828459e+1";
    res = json_read(&(elt){MFLT, sizeof(dnum), &dnum}, j, strlen(j));
    if (res || dnum < 27.18281828459-epsd || dnum > 27.18281828459+epsd) return 1;
    j = "2.718281828459e-1";
    res = json_read(&(elt){MFLT, sizeof(dnum), &dnum}, j, strlen(j));
    if (res || dnum < 0.2718281828459-epsd || dnum > 0.2718281828459+epsd) return 1;
    j = "-2.718281828459";
    res = json_read(&(elt){MFLT, sizeof(dnum), &dnum}, j, strlen(j));
    if (res || dnum < -2.718281828459-epsd || dnum > -2.718281828459+epsd) return 1;
    j = "-2.718281828459e1";
    res = json_read(&(elt){MFLT, sizeof(dnum), &dnum}, j, strlen(j));
    if (res || dnum < -27.18281828459-epsd || dnum > -27.18281828459+epsd) return 1;
    j = "-2.718281828459e+1";
    res = json_read(&(elt){MFLT, sizeof(dnum), &dnum}, j, strlen(j));
    if (res || dnum < -27.18281828459-epsd || dnum > -27.18281828459+epsd) return 1;
    j = "-2.718281828459e-1";
    res = json_read(&(elt){MFLT, sizeof(dnum), &dnum}, j, strlen(j));
    if (res || dnum < -0.2718281828459-epsd || dnum > -0.2718281828459+epsd) return 1;

    j = "'Hello Json!'";
    res = json_read(&(elt){MSTR, sizeof(str), &str}, j, strlen(j));
    if (res || strcmp(str, "Hello Json!")) return 1;
    j = "\"Hello Json!\"";
    res = json_read(&(elt){MSTR, sizeof(str), &str}, j, strlen(j));
    if (res || strcmp(str, "Hello Json!")) return 1;
    j = "'Hello\\nJson!'";
    res = json_read(&(elt){MSTR, sizeof(str), &str}, j, strlen(j));
    if (res || strcmp(str, "Hello\nJson!")) return 1;
    j = "'!'";
    res = json_read(&(elt){MSTR, sizeof(str), &str}, j, strlen(j));
    if (res || strcmp(str, "!")) return 1;

    return 0;
}


static int InvalidSingleValues(void) {
    char cnum = 0;
    const char *j;
    int res;

    printf("%-24s - ", __func__);

    j = "";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;
    j = ".";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;
    j = ",";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;
    j = "e";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;
    j = "e1";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;
    j = " ";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;
    j = "\n\t\r";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;
    j = "1.";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;
    j = "+1";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;
    j = "1.2.3";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;
    j = "1 2";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;
    j = "1, 2";
    res = json_read(&(elt){MINT, sizeof(cnum), &cnum}, j, strlen(j));
    if (res == 0) return 1;

    return 0;
}


static int ValidArrays(void) {
    int n1=0, n2=0, n3=0, n4=0, n5=0;
    const char *j;
    int res;

    printf("%-24s - ", __func__);

    j = "[]";
    res = json_read(
        &(elt){MARR, 0, 0}, j, strlen(j)
    );
    if (res) return 1;

    j = " [1]";
    res = json_read(
        &(elt){MARR, 2, &(elt[]){
            {MINT, sizeof(n1), &n1},
            {MINT, sizeof(n2), &n2}
        }}, j, strlen(j)
    );
    if (res || n1!=1) return 1;

    j = "[ 2,3]";
    res = json_read(
        &(elt){MARR, 2, &(elt[]){
            {MINT, sizeof(n1), &n1},
            {MINT, sizeof(n2), &n2}
        }}, j, strlen(j)
    );
    if (res || n1!=2 || n2!=3) return 1;

    j = "[4,[5  ,6,7],8]";
    res = json_read(
        &(elt){MARR, 3, &(elt[]){
            {MINT, sizeof(n1), &n1},
            {MARR, 3, &(elt[]){
                {MINT, sizeof(n3), &n3},
                {MINT, sizeof(n4), &n4},
                {MINT, sizeof(n5), &n5}
            }},
            {MINT, sizeof(n2), &n2}
        }}, j, strlen(j)
    );
    if (res || n1!=4 || n2!=8 || n3!=5 || n4!=6 || n5!=7) return 1;

    j = "[10,[11,12,[0,[ 0 ],0],13],14]  ";
    res = json_read(
        &(elt){MARR, 3, &(elt[]){
            {MINT, sizeof(n1), &n1},
            {MARR, 4, &(elt[]){
                {MINT, sizeof(n3), &n3},
                {MINT, sizeof(n4), &n4},
                {MINT, 0, 0}, // dummy
                {MINT, sizeof(n5), &n5}
            }},
            {MINT, sizeof(n2), &n2}
        }}, j, strlen(j)
    );
    if (res || n1!=10 || n2!=14 || n3!=11 || n4!=12 || n5!=13) return 1;

    return 0;
}


static int InvalidArrays(void) {
    const char *j;
    int res;

    printf("%-24s - ", __func__);

    j = "[";
    res = json_read(
        &(elt){MARR, 0, 0}, j, strlen(j)
    );
    if (res == 0) return 1;

    j = "]";
    res = json_read(
        &(elt){MARR, 0, 0}, j, strlen(j)
    );
    if (res == 0) return 1;

    j = "[,]";
    res = json_read(
        &(elt){MARR, 0, 0}, j, strlen(j)
    );
    if (res == 0) return 1;

    j = "[[]";
    res = json_read(
        &(elt){MARR, 0, 0}, j, strlen(j)
    );
    if (res == 0) return 1;

    j = "[:]";
    res = json_read(
        &(elt){MARR, 0, 0}, j, strlen(j)
    );
    if (res == 0) return 1;

    return 0;
}


static int ValidObjects(void) {
    int n1=0, n2=0, n3=0, n4=0, n5=0;
    int m1=0, m2=0, m3=0, m4=0, m5=0;
    const char *j;
    int res;

    printf("%-24s - ", __func__);

    // attribute name need not to be enclosed in quotes as in standard json
    j = "{n1:1}";
    res = json_read(
        &(elt){MOBJ, 1, &(atr[]){
            {"n1", MINT, sizeof(n1), &n1}
        }}, j, strlen(j)
    );
    if (res || n1!=1) return 1;

    // attribute name may be enclosed in double quotes as in standard json
    j = "{\"n1\":2}";
    res = json_read(
        &(elt){MOBJ, 1, &(atr[]){
            {"n1", MINT, sizeof(n1), &n1}
        }}, j, strlen(j)
    );
    if (res || n1!=2) return 1;

    // attribute name may be enclosed in single quotes
    j = "{'n1':3}";
    res = json_read(
        &(elt){MOBJ, 1, &(atr[]){
            {"n1", MINT, sizeof(n1), &n1}
        }}, j, strlen(j)
    );
    if (res || n1!=3) return 1;

    // test for more than one attribute and different spacing
    j = "  {n1:1,\tn2:2,n3:3   ,     n4:4,\nn5:5}  ";
    res = json_read(
        &(elt){MOBJ, 5, &(atr[]){
            {"n1", MINT, sizeof(n1), &n1},
            {"n2", MINT, sizeof(n2), &n2},
            {"n3", MINT, sizeof(n3), &n3},
            {"n4", MINT, sizeof(n4), &n4},
            {"n5", MINT, sizeof(n5), &n5}
        }}, j, strlen(j)
    );
    if (res || n1!=1 || n2!=2 || n3!=3 || n4!=4 || n5!=5) return 1;

    // order of attributes in json and in mapping may be different
    j = "{n3:7, n5:21, n1:17, n2:44, n4:39}";
    res = json_read(
        &(elt){MOBJ, 5, &(atr[]){
            {"n1", MINT, sizeof(n1), &n1},
            {"n2", MINT, sizeof(n2), &n2},
            {"n3", MINT, sizeof(n3), &n3},
            {"n4", MINT, sizeof(n4), &n4},
            {"n5", MINT, sizeof(n5), &n5}
        }}, j, strlen(j)
    );
    if (res || n1!=17 || n2!=44 || n3!=7 || n4!=39 || n5!=21) return 1;

    // test for nested objects
    j = "{n1:1, n2:2, n3:3, nestedobject:{m1:6,m2:7,m3:8,m4:9,m5:10}, n4:4, n5:5}";
    res = json_read(
        &(elt){MOBJ, 6, &(atr[]){
            {"n1", MINT, sizeof(n1), &n1},
            {"n2", MINT, sizeof(n2), &n2},
            {"n3", MINT, sizeof(n3), &n3},
            {"nestedobject", MOBJ, 5, &(atr[]){
                {"m1", MINT, sizeof(m1), &m1},
                {"m2", MINT, sizeof(m2), &m2},
                {"m3", MINT, sizeof(m3), &m3},
                {"m4", MINT, sizeof(m4), &m4},
                {"m5", MINT, sizeof(m5), &m5}
            }},
            {"n4", MINT, sizeof(n4), &n4},
            {"n5", MINT, sizeof(n5), &n5}
        }}, j, strlen(j)
    );
    if (res || n1!=1 || n2!=2 || n3!=3 || n4!=4 || n5!=5) return 1;
    if (m1!=6 || m2!=7 || m3!=8 || m4!=9 || m5!=10) return 1;

    return 0;
}


static int InvalidObjects(void) {
    const char *j;
    int res;

    printf("%-24s - ", __func__);

    j = "{";
    res = json_read(&(elt){MOBJ, 0, 0}, j, strlen(j));
    if (res == 0) return 1;
    j = "}";
    res = json_read(&(elt){MOBJ, 0, 0}, j, strlen(j));
    if (res == 0) return 1;
    j = "{1}";
    res = json_read(&(elt){MOBJ, 0, 0}, j, strlen(j));
    if (res == 0) return 1;
    j = "{,}";
    res = json_read(&(elt){MOBJ, 0, 0}, j, strlen(j));
    if (res == 0) return 1;
    j = "{a:1, b:{d:{1}}, c:2}";
    res = json_read(&(elt){MOBJ, 0, 0}, j, strlen(j));
    if (res == 0) return 1;

    return 0;
}


static int ValidStrings(void) {
    char str1[64] = {0};
    char str2[64] = {0};
    char str3[64] = {0};
    char str4[64] = {0};
    const char *j;
    int res;

    printf("%-24s - ", __func__);

    j = "'hello'";
    res = json_read(&(elt){MSTR, sizeof(str1), &str1}, j, strlen(j));
    if (res || strcmp(str1, "hello")) return 1;

    j = "\"world\"";
    res = json_read(&(elt){MSTR, sizeof(str1), &str1}, j, strlen(j));
    if (res || strcmp(str1, "world")) return 1;

    j = "['Hasta', 'la vista']";
    res = json_read(
        &(elt){MARR, 2, &(elt[]){
            {MSTR, sizeof(str1), &str1},
            {MSTR, sizeof(str2), &str2}
        }}, j, strlen(j)
    );
    if (res || strcmp(str1, "Hasta") || strcmp(str2, "la vista")) return 1;

    j = "{one:'aaa', two:'bbb'}";
    res = json_read(
        &(elt){MOBJ, 2, &(atr[]){
            {"one", MSTR, sizeof(str1), &str1},
            {"two", MSTR, sizeof(str2), &str2}
        }}, j, strlen(j)
    );
    if (res || strcmp(str1, "aaa") || strcmp(str2, "bbb")) return 1;

    j = "{one:'111', two:['333','444'], three:'222'}";
    res = json_read(
        &(elt){MOBJ, 3, &(atr[]){
            {"one", MSTR, sizeof(str1), &str1},
            {"two", MARR, 2, &(elt[]){
                {MSTR, sizeof(str3), &str3},
                {MSTR, sizeof(str4), &str4}
            }},
            {"three", MSTR, sizeof(str2), &str2}
        }}, j, strlen(j)
    );
    if (res || strcmp(str1, "111") || strcmp(str2, "222")) return 1;
    if (strcmp(str3, "333") || strcmp(str4, "444")) return 1;

    j = "['k', {one:'l', two:'m'}, 'n']";
    res = json_read(
        &(elt){MARR, 3, &(elt[]){
            {MSTR, sizeof(str1), &str1},
            {MOBJ, 2, &(atr[]){
                {"two", MSTR, sizeof(str3), &str3},
                {"one", MSTR, sizeof(str2), &str2}
            }},
            {MSTR, sizeof(str4), &str4}
        }}, j, strlen(j)
    );
    if (res || strcmp(str1, "k") || strcmp(str2, "l")) return 1;
    if (strcmp(str3, "m") || strcmp(str4, "n")) return 1;

    return 0;
}


static int InvalidStrings(void) {
    char str1[64] = {0};
    const char *j;
    int res;

    printf("%-24s - ", __func__);

    j = "'";
    res = json_read(&(elt){MSTR, sizeof(str1), &str1}, j, strlen(j));
    if (res == 0) return 1;
    j = "\"";
    res = json_read(&(elt){MSTR, sizeof(str1), &str1}, j, strlen(j));
    if (res == 0) return 1;
    j = " ' ";
    res = json_read(&(elt){MSTR, sizeof(str1), &str1}, j, strlen(j));
    if (res == 0) return 1;

    return 0;
}


static int SomeComplexExamples(void) {
    struct {
        char str1[64], str2[64], str3[64], str4[64];
        char c1, c2, c3, c4;
        short s1, s2, s3, s4;
        int n1, n2, n3, n4, n5;
        float f1, f2, f3, f4;
        double d1, d2, d3, d4;
    } s;
    float epsf = 0.000001f;
    double epsd = 0.000000001;
    const char *j;
    int res;

    printf("%-24s - ", __func__);

    j = "[1, 2, {a:3, b:4, c:['a','b',{}]}]";
    memset(&s, 0, sizeof(s));
    res = json_read(
        &(elt){MARR, 3, &(elt[]){
            {MINT, sizeof(s.c1), &s.c1},
            {MINT, sizeof(s.c2), &s.c2},
            {MOBJ, 3, &(atr[]){
                {"a", MINT, sizeof(s.c3), &s.c3},
                {"b", MINT, sizeof(s.c4), &s.c4},
                {"c", MARR, 2, &(elt[]){
                    {MSTR, sizeof(s.str1), &s.str1},
                    {MSTR, sizeof(s.str2), &s.str2}
                }},
            }}
        }}, j, strlen(j)
    );
    if (res) return 1;
    if (s.c1 != 1 || s.c2 != 2) return 1;
    if (s.c3 != 3 || s.c4 != 4) return 1;
    if (strcmp(s.str1, "a") || strcmp(s.str2, "b")) return 1;

    j= "  \
[  \
  100,  \
  {  \
    'aaa': 1000,  \
    'bbb': 2000,  \
    'ccc': [  \
      10,  \
      20,  \
      {'ddd': 111, 'eee': 222, 'fff': 333},  \
      30  \
    ]  \
  },  \
  200,  \
  300,  \
  [  \
    'abcdefg',  \
    'bcdefgh'  \
  ],  \
  {  \
    'ggg': 1.23,  \
    'hhh': 57.77e-1,  \
    'iii': 0.007e3  \
  }  \
]";
    atr arr_obj_arr_obj[] = {
        {"ddd", MINT, sizeof(s.s1), &s.s1},
        {"eee", MINT, sizeof(s.s2), &s.s2},
        {"fff", MINT, sizeof(s.s3), &s.s3}
    };
    elt arr_obj_arr[] = {
        {MINT, sizeof(s.c1), &s.c1},
        {MINT, sizeof(s.c2), &s.c2},
        {MOBJ, 3, &arr_obj_arr_obj},
        {MINT, sizeof(s.c3), &s.c3}
    };
    atr arr_obj1[] = {
        {"aaa", MINT, sizeof(s.n1), &s.n1},
        {"bbb", MINT, sizeof(s.n2), &s.n2},
        {"ccc", MARR, 4, arr_obj_arr},
    };
    elt arr_arr2[] = {
        {MSTR, sizeof(s.str1), &s.str1},
        {MSTR, sizeof(s.str2), &s.str2}
    };
    atr arr_obj3[] = {
        {"ggg", MFLT, sizeof(s.f1), &s.f1},
        {"hhh", MFLT, sizeof(s.f2), &s.f2},
        {"iii", MFLT, sizeof(s.d1), &s.d1}
    };
    elt arr[] = {
        {MINT, sizeof(s.n3), &s.n3},
        {MOBJ, 3, arr_obj1},
        {MINT, sizeof(s.n4), &s.n4},
        {MINT, sizeof(s.n5), &s.n5},
        {MARR, 2, &arr_arr2},
        {MOBJ, 3, &arr_obj3}
    };
    elt first = {MARR, 6, &arr};
    memset(&s, 0, sizeof(s));
    res = json_read(&first, j, strlen(j));
    if (res) return 1;
    if (s.c1 != 10 || s.c2 != 20 || s.c3 != 30) return 1;
    if (s.s1 != 111 || s.s2 != 222 || s.s3 != 333) return 1;
    if (s.n1 != 1000 || s.n2 != 2000 || s.n3 != 100 || s.n4 != 200 || s.n5 != 300) return 1;
    if (strcmp(s.str1, "abcdefg") || strcmp(s.str2, "bcdefgh")) return 1;
    if (!(1.23f - epsf <= s.f1 && s.f1 <= 1.23f + epsf)) return 1;
    if (!(5.777f - epsf <= s.f2 && s.f2 <= 5.777f + epsf)) return 1;
    if (!(7.0 - epsd <= s.d1 && s.d1 <= 7.0 + epsd)) return 1;

    return 0;
}

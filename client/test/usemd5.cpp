#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "md5.h"
#include "md5check.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cstdio>
using namespace std;

char filename[1005];

int main(int argc, char *argv[])
{
    cout << "파일의 이름을 입력하시오. 파일의 형식까지 입력하시오. ex) test.txt" << '\n';
    cin >> filename ;
    cout << FileHash(filename) <<'\n';
    return EXIT_SUCCESS;
}

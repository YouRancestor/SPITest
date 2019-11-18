// LSPUninstaller.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <WinSock2.h>
#include <WS2spi.h>
#include <stdio.h>
#include <tchar.h>

#define main _tmain
#define char TCHAR
#define printf(x, ...) _tprintf(L##x, ##__VA_ARGS__)
#define sscanf_s _stscanf_s


int main(int argc, char* argv[])
{
    printf("Deinstalling provider %s\n",argv[1]);

    GUID stGuid;

    sscanf_s(argv[1], _T("{%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx}"),
        & (stGuid.Data1), &(stGuid.Data2), &(stGuid.Data3),
        & (stGuid.Data4[0]), &(stGuid.Data4[1]), &(stGuid.Data4[2]), &(stGuid.Data4[3]),
        & (stGuid.Data4[4]), &(stGuid.Data4[5]), &(stGuid.Data4[6]), &(stGuid.Data4[7]));


    INT err=0;
    if (WSCDeinstallProvider(&stGuid, &err))
    {
        printf("Deinstall failed with error no. %d.\n", err);
        return 1;
    }

    return 0;
}

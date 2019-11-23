// https://blog.csdn.net/aaron133/article/details/78028942

#include <WinSock2.h>
#include <WS2spi.h>
#include <sporder.h>
#include <stdio.h>
#include <tchar.h>


#define main wmain
#define printf(x, ...) wprintf(L##x, ##__VA_ARGS__)
#define sscanf_s swscanf_s
#define char WCHAR
#define strcmp lstrcmpW
#define LPWSAPROTOCOL_INFO LPWSAPROTOCOL_INFOW
#define WSAPROTOCOL_INFO WSAPROTOCOL_INFOW

#define N(x) L##x

const char protocolName[] = N("Test TCP Protocol");
const char layeredProtocolName[] = N("Test LSP");
const char providerGuid[] = N("{BCFF0207-535B-4222-A871-DA1352E66CDD}");
const char providerGuid2[] = N("{4F2FFB50-30BA-4901-9997-E535740496D8}");


bool FindLayeredChain(GUID guid, LPWSAPROTOCOL_INFO info)
{
    bool ret = false;

    DWORD dwBufferLen = 16384;
    LPWSAPROTOCOL_INFO lpProtocolInfo = (LPWSAPROTOCOL_INFO)HeapAlloc(GetProcessHeap(),0,dwBufferLen);
    INT err;
    INT num = WSCEnumProtocols(NULL, lpProtocolInfo, &dwBufferLen, &err);

    for (INT i = 0; i < num; ++i)
    {
        if (lpProtocolInfo[i].ProviderId == guid)
        {
            ret = true;
            *info = lpProtocolInfo[i];
            break;
        }
    }

    HeapFree(GetProcessHeap(), 0, lpProtocolInfo);
    return ret;
}

bool FindBaseProtocol(LPWSAPROTOCOL_INFO info)
{
    bool ret = false;
    DWORD dwBufferLen = 16384;
    LPWSAPROTOCOL_INFO lpProtocolInfo = (LPWSAPROTOCOL_INFO)HeapAlloc(GetProcessHeap(), 0, dwBufferLen);
    INT err;
    INT num = WSCEnumProtocols(NULL, lpProtocolInfo, &dwBufferLen, &err);

    for (INT i = 0; i < num; ++i)
    {
        if (lpProtocolInfo[i].iProtocol == IPPROTO_TCP && lpProtocolInfo[i].iMaxSockAddr==16)
        {
            ret = true;
            *info = lpProtocolInfo[i];
            break;
        }
    }
    HeapFree(GetProcessHeap(), 0, lpProtocolInfo);
    return ret;
}

int ReOrderToFirst(LPWSAPROTOCOL_INFO info)
{
    DWORD dwBufferLen = 16384;
    LPWSAPROTOCOL_INFO lpProtocolInfo = (LPWSAPROTOCOL_INFO)HeapAlloc(GetProcessHeap(), 0, dwBufferLen);
    INT num;
    DWORD* index = NULL;
    LPWSAPROTOCOL_INFO p = NULL;
    int i = 0;
    INT err;

    if (!FindLayeredChain(info->ProviderId, info))
    {
        return -1;
    }

    num = WSCEnumProtocols(NULL, lpProtocolInfo, &dwBufferLen, &err);
    index = (DWORD*)HeapAlloc(GetProcessHeap(), 0, sizeof(DWORD) * num);
    index[0] = info->dwCatalogEntryId;
    for (i = 1, p = lpProtocolInfo; i <= num;)
    {
        if (p->dwCatalogEntryId == info->dwCatalogEntryId)
        {
            p++;
            continue;
        }
        index[i] = p->dwCatalogEntryId;
        i++;
        p++;
    }

    HeapFree(GetProcessHeap(), 0, lpProtocolInfo);

    WSCWriteProviderOrder(index, num);
    HeapFree(GetProcessHeap(), 0, index);

    return 0;
}

int main(int argc, char* argv[])
{
    system("pause");
    printf("Attempting to load dll %s.", argv[1]);
    HMODULE h = LoadLibrary(argv[1]);
    FreeLibrary(h);
    printf("Installing %s ...\n", argv[1]);

    GUID guid;
    GUID guid2;

    //CoCreateGuid(&guid);
    //StringFromGUID2(guid, providerGuid2, sizeof(providerGuid2));
    //printf("Generate GUID: %s.\n", providerGuid2);
    //return 0;

    sscanf_s(providerGuid, N("{%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx}"),
        &(guid.Data1), &(guid.Data2), &(guid.Data3),
        &(guid.Data4[0]), &(guid.Data4[1]), &(guid.Data4[2]), &(guid.Data4[3]),
        &(guid.Data4[4]), &(guid.Data4[5]), &(guid.Data4[6]), &(guid.Data4[7]));

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    //DWORD protocolBuffSize;
    //DWORD num = WSCEnumProtocols(0, 0, &protocolBuffSize, 0); // 查询数量

    //LPWSAPROTOCOL_INFO info = (LPWSAPROTOCOL_INFO)malloc(protocolBuffSize);
    //num = WSCEnumProtocols(0, info, &protocolBuffSize, 0);
    //if (num == SOCKET_ERROR)
    //{
    //    free(info);
    //    return 1;
    //}

    WSAPROTOCOL_INFO layeredProtocol;
    ZeroMemory(&layeredProtocol, sizeof(WSAPROTOCOL_INFO));

    layeredProtocol.ProviderId = guid;
    memcpy(layeredProtocol.szProtocol, layeredProtocolName, sizeof(layeredProtocolName));
    layeredProtocol.iMaxSockAddr = 28;
    layeredProtocol.iMinSockAddr = 16;
    layeredProtocol.dwProviderFlags = PFL_HIDDEN;
    layeredProtocol.ProtocolChain.ChainLen = LAYERED_PROTOCOL;

    INT err = 0;
    if (WSCInstallProvider(&guid, argv[1], &layeredProtocol, 1, &err) == SOCKET_ERROR)
    {
        printf("Install failed with errno %d.\n", err);
        return 2;
    }

    //ReOrderToFirst(&layeredProtocol);

    WSAPROTOCOL_INFO testProtocol[3]; //数组成员为TCP、UDP、原始的目录入口信息
    ZeroMemory(&testProtocol, sizeof(WSAPROTOCOL_INFO)*3);

    if (!FindLayeredChain(guid, &layeredProtocol))
    {
        return 3;
    }

    WSAPROTOCOL_INFO baseProtocol;

    if (!FindBaseProtocol(&baseProtocol))
    {
        return 4;
    }

    /*
        WSAPROTOCOL_INFO::dwServiceFlags1
 TCP UDP
     √ XP1_CONNECTIONLESS              // 无连接
 √     XP1_GUARANTEED_DELIVERY         // 提供可靠传输
 √     XP1_GUARANTEED_ORDER            // 保序
     √ XP1_MESSAGE_ORIENTED            // 面向消息（保护消息边界）

        XP1_PSEUDO_STREAM               // 伪流（面向消息但忽略消息边界）
 √     XP1_GRACEFUL_CLOSE              // 优雅关闭
 √     XP1_EXPEDITED_DATA              // 支持紧急数据
        XP1_CONNECT_DATA                // 

        XP1_DISCONNECT_DATA             // 
     √ XP1_SUPPORT_BROADCAST           // 支持广播
     √ XP1_SUPPORT_MULTIPOINT          // 支持多播
        XP1_MULTIPOINT_CONTROL_PLANE    // 

        XP1_MULTIPOINT_DATA_PLANE       //
        XP1_QOS_SUPPORTED               // 
        XP1_INTERRUPT                   // 支持中断（官方文档写的“保留”。但有其他资料显示，此位是对“中断环境中，
                                           由于无法调用WSAGetLastError()，故如果设置了MSG_INTERRUPT标识，WSARecv()
                                           函数直接返回错误码。”的支持。不过该资料中补充说明仅适用Win16环境，故猜
                                           想可能该标识位已弃用。ProxyCap将该标识位置true。）
        XP1_UNI_SEND                    // 单向协议的发送端

        XP1_UNI_RECV                    // 单向协议的接收端
 √  √ XP1_IFS_HANDLES                 // Socket描述符是IFS句柄
        XP1_PARTIAL_MESSAGE             // 调用WSASend与WSASendTo时是否支持MSG_PARTIAL标识位
        XP1_SAN_SUPPORT_SDP             // 

        Note: XP1_UNI_SEND和XP1_UNI_RECV只能使用其一，若协议为单向协议，但支持双端，应使用两个WSAPROTOCOL_INFO结构体。
        XP1_UNI_SEND和XP1_UNI_SEND同时为0表示协议为双向的。
    */

    sscanf_s(providerGuid2, N("{%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx}"),
        &(guid2.Data1), &(guid2.Data2), &(guid2.Data3),
        &(guid2.Data4[0]), &(guid2.Data4[1]), &(guid2.Data4[2]), &(guid2.Data4[3]),
        &(guid2.Data4[4]), &(guid2.Data4[5]), &(guid2.Data4[6]), &(guid2.Data4[7]));

    // TCP
    WSAPROTOCOL_INFO* tcp = &testProtocol[0];
    tcp->dwServiceFlags1 = XP1_GUARANTEED_DELIVERY | XP1_GUARANTEED_ORDER | XP1_GRACEFUL_CLOSE
        | XP1_EXPEDITED_DATA | XP1_IFS_HANDLES;
    tcp->iProtocol = IPPROTO_TCP;
    tcp->iAddressFamily = AF_INET;
    tcp->ProtocolChain.ChainLen = 2;
    tcp->ProtocolChain.ChainEntries[0] = layeredProtocol.dwCatalogEntryId;
    tcp->ProtocolChain.ChainEntries[1] = baseProtocol.dwCatalogEntryId;
    tcp->iMaxSockAddr = 16;
    tcp->iMinSockAddr = 16;
    tcp->ProviderId = guid2;
    memcpy(tcp->szProtocol, protocolName, sizeof(protocolName));
    tcp->dwProviderFlags = PFL_MATCHES_PROTOCOL_ZERO;
    tcp->iVersion = 2;
    tcp->iSocketType = SOCK_STREAM;

    // UDP
    //WSAPROTOCOL_INFO* udp = &testProtocol[1];
    //udp->dwServiceFlags1 = XP1_CONNECTIONLESS | XP1_CONNECT_DATA | XP1_MULTIPOINT_DATA_PLANE | XP1_INTERRUPT;

    // RAW


    if (WSCInstallProvider(&guid2, argv[1], tcp, 1, &err) == SOCKET_ERROR)
    {
        printf("Install TCP failed with errno %d.\n", err);
        goto uninstall;
    }

    // reorder
    ReOrderToFirst(tcp);

    printf("Installed\n");
    return 0;
    printf("Press any key to uninstall LSP DLL ...\n");

    getchar();

uninstall:
    printf("Uninstalling %s ...\n", argv[1]);

    WSCDeinstallProvider(&guid, &errno);
    WSCDeinstallProvider(&guid2, &errno);

    printf("Uninstalled\n");

    return 0;
}

#undef main
#undef printf
#undef system
#undef char

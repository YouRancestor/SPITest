// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsaenumprotocolsw

#ifndef UNICODE
#define UNICODE 1
#endif

#include <winsock2.h>
#include <WS2spi.h>
#include <ws2tcpip.h>
#include <objbase.h>
#include <stdio.h>

// Link with ws2_32.lib and ole32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "ole32.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
// Note: could also use malloc() and free()

int wmain()
{
    system("pause");
    //-----------------------------------------
    // Declare and initialize variables
    WSADATA wsaData;
    int iResult = 0;

    int iError = 0;
    INT iNuminfo = 0;

    int i;

    // Allocate a 16K buffer to retrieve all the protocol providers
    DWORD dwBufferLen = 16384;

    LPWSAPROTOCOL_INFO lpProtocolInfo = NULL;

    // variables needed for converting provider GUID to a string
    int iRet = 0;
    WCHAR GuidString[40] = { 0 };

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        wprintf(L"WSAStartup failed: %d\n", iResult);
        return 1;
    }

    lpProtocolInfo = (LPWSAPROTOCOL_INFO)MALLOC(dwBufferLen);
    if (lpProtocolInfo == NULL) {
        wprintf(L"Memory allocation for providers buffer failed\n");
        WSACleanup();
        return 1;
    }
    int err;
    iNuminfo = WSCEnumProtocols(NULL, lpProtocolInfo, &dwBufferLen, &err);
    if (iNuminfo == SOCKET_ERROR) {
        iError = WSAGetLastError();
        if (iError != WSAENOBUFS) {
            wprintf(L"WSAEnumProtocols failed with error: %d\n", iError);
            if (lpProtocolInfo) {
                FREE(lpProtocolInfo);
                lpProtocolInfo = NULL;
            }
            WSACleanup();
            return 1;
        }
        else {
            wprintf(L"WSAEnumProtocols failed with error: WSAENOBUFS (%d)\n",
                iError);
            wprintf(L"  Increasing buffer size to %d\n\n", dwBufferLen);
            if (lpProtocolInfo) {
                FREE(lpProtocolInfo);
                lpProtocolInfo = NULL;
            }
            lpProtocolInfo = (LPWSAPROTOCOL_INFO)MALLOC(dwBufferLen);
            if (lpProtocolInfo == NULL) {
                wprintf(L"Memory allocation increase for buffer failed\n");
                WSACleanup();
                return 1;
            }
            iNuminfo = WSAEnumProtocols(NULL, lpProtocolInfo, &dwBufferLen);
            if (iNuminfo == SOCKET_ERROR) {
                iError = WSAGetLastError();
                wprintf(L"WSAEnumProtocols failed with error: %d\n", iError);
                if (lpProtocolInfo) {
                    FREE(lpProtocolInfo);
                    lpProtocolInfo = NULL;
                }
                WSACleanup();
                return 1;
            }

        }
    }

    wprintf(L"WSAEnumProtocols succeeded with protocol count = %d\n\n",
        iNuminfo);
    for (i = 0; i < iNuminfo; i++) {
        wprintf(L"Winsock Catalog Provider Entry #%d\n", i);
        wprintf
        (L"----------------------------------------------------------\n");
        wprintf(L"Entry type:\t\t\t ");
        if (lpProtocolInfo[i].ProtocolChain.ChainLen == 1)
            wprintf(L"Base Service Provider\n");
        else
            wprintf(L"Layered Chain Entry\n");

        wprintf(L"Protocol:\t\t\t %ws\n", lpProtocolInfo[i].szProtocol);

        iRet =
            StringFromGUID2(lpProtocolInfo[i].ProviderId,
            (LPOLESTR)&GuidString, 39);
        if (iRet == 0)
            wprintf(L"StringFromGUID2 failed\n");
        else
            wprintf(L"Provider ID:\t\t\t %ws\n", GuidString);

        wprintf(L"Catalog Entry ID:\t\t %u\n",
            lpProtocolInfo[i].dwCatalogEntryId);

        wprintf(L"Version:\t\t\t %d\n", lpProtocolInfo[i].iVersion);

        wprintf(L"Address Family:\t\t\t %d\n",
            lpProtocolInfo[i].iAddressFamily);
        wprintf(L"Max Socket Address Length:\t %d\n",
            lpProtocolInfo[i].iMaxSockAddr);
        wprintf(L"Min Socket Address Length:\t %d\n",
            lpProtocolInfo[i].iMinSockAddr);

        wprintf(L"Socket Type:\t\t\t %d\n", lpProtocolInfo[i].iSocketType);
        wprintf(L"Socket Protocol:\t\t %d\n", lpProtocolInfo[i].iProtocol);
        wprintf(L"Socket Protocol Max Offset:\t %d\n",
            lpProtocolInfo[i].iProtocolMaxOffset);

        wprintf(L"Network Byte Order:\t\t %d\n",
            lpProtocolInfo[i].iNetworkByteOrder);
        wprintf(L"Security Scheme:\t\t %d\n",
            lpProtocolInfo[i].iSecurityScheme);
        wprintf(L"Max Message Size:\t\t %u\n", lpProtocolInfo[i].dwMessageSize);

        wprintf(L"ServiceFlags1:\t\t\t 0x%x\n",
            lpProtocolInfo[i].dwServiceFlags1);
        wprintf(L"ServiceFlags2:\t\t\t 0x%x\n",
            lpProtocolInfo[i].dwServiceFlags2);
        wprintf(L"ServiceFlags3:\t\t\t 0x%x\n",
            lpProtocolInfo[i].dwServiceFlags3);
        wprintf(L"ServiceFlags4:\t\t\t 0x%x\n",
            lpProtocolInfo[i].dwServiceFlags4);
        wprintf(L"ProviderFlags:\t\t\t 0x%x\n",
            lpProtocolInfo[i].dwProviderFlags);

        wprintf(L"Protocol Chain length:\t\t %d\n",
            lpProtocolInfo[i].ProtocolChain.ChainLen);

        wprintf(L"\n");
    }

    if (lpProtocolInfo) {
        FREE(lpProtocolInfo);
        lpProtocolInfo = NULL;
    }
    WSACleanup();

    return 0;
}
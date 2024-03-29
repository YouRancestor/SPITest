﻿// https://blog.csdn.net/aaron133/article/details/78028942

#include "pch.h"

#include <WinSock2.h>
#include <WS2spi.h>
#include <tchar.h>

WCHAR exepath[MAX_PATH] = { 0 };

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        GetModuleFileNameW(0, exepath, MAX_PATH * sizeof(wchar_t));
        WCHAR strTip[MAX_PATH + 100];
        wsprintf(strTip, L"Application %s loaded LSPDLL.dll！", exepath);
        MessageBoxW(0, strTip, L"Tip", MB_OK);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

WSPPROC_TABLE g_NextProcTable;

LPWSAPROTOCOL_INFOW GetProvider(LPINT lpnTotalProtocols)
{//遍历所有协议
    int nError = 0;
    DWORD dwSize = 0;
    LPWSAPROTOCOL_INFOW pProtoInfo = NULL;
    if (WSCEnumProtocols(NULL, pProtoInfo, &dwSize, &nError) == SOCKET_ERROR)
    {
        if (nError != WSAENOBUFS)
            return NULL;
    }
    pProtoInfo = (LPWSAPROTOCOL_INFOW)new WSAPROTOCOL_INFOW[dwSize / sizeof(WSAPROTOCOL_INFOW)];
    if (!pProtoInfo)
        return NULL;
    ZeroMemory(pProtoInfo, dwSize);
    *lpnTotalProtocols = WSAEnumProtocols(NULL, pProtoInfo, &dwSize);
    return pProtoInfo;
}

void FreeProvider(LPWSAPROTOCOL_INFOW pProtoInfo)
{
    delete[] pProtoInfo;
}

int WSPAPI WSPConnect(       //自定义的WSPConnect函数
    SOCKET s,
    const struct sockaddr FAR* name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    LPINT lpErrno
)
{
    WCHAR strTip[256];
    wsprintf(strTip, L"sorry,we shutdown you tcp connection!");
    MessageBoxW(0, strTip, exepath, MB_OK);
    *lpErrno = WSAECONNABORTED;
    return SOCKET_ERROR;

    sockaddr_in* info = (sockaddr_in*)name;
    USHORT port = ntohs(info->sin_port);
    if (port == 8888)   //如果是8888端口,那么拦截
    {
        int nError = 0;

        WCHAR strTip[256];
        wsprintf(strTip, L"sorry,we shutdown you tcp protocol port<8888>!");
        MessageBoxW(0, strTip, exepath, MB_OK);

        //因为整个dll已经加载进程序里,这里对我的控制台程序进行测试
        SetConsoleTitle(_T("sorry,we shutdown you tcp protocol port<8888>!"));
        g_NextProcTable.lpWSPShutdown(s, SD_BOTH, &nError);
        //设置错误信息
        *lpErrno = WSAECONNABORTED;
        return SOCKET_ERROR;
    }
    //如果不是,调用下层提供者的函数表中的WSPConnect函数
    return g_NextProcTable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
}
int WSPAPI WSPSendTo         //自定义的WSPSendTo函数
(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR* lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
)
{
    WCHAR strTip[256];
    wsprintf(strTip, L"The following content is intercepted: \"%s.\"", lpBuffers);
    MessageBoxW(0, strTip, exepath, MB_OK);
    *lpErrno = WSAECONNABORTED;
    return SOCKET_ERROR;

    sockaddr_in* info = (sockaddr_in*)lpTo;
    USHORT port = ntohs(info->sin_port);
    if (port == 8888)    //如果是8888端口,那么拦截
    {

        int nError = 0;
        WCHAR strTip[256];
        wsprintf(strTip, L"sorry,we shutdown you udp protocol port<8888>!");
        MessageBoxW(0, strTip, exepath, MB_OK);

        SetConsoleTitle(_T("sorry,we shutdown you udp protocol port<8888>!"));
        g_NextProcTable.lpWSPShutdown(s, SD_BOTH, &nError);
        //设置错误信息
        *lpErrno = WSAECONNABORTED;
        return SOCKET_ERROR;
    }
    //如果不是,调用下层提供者的函数表中的WSPSendTo函数
    return g_NextProcTable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags,
        lpTo, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}

int WSPAPI WSPSend(
    _In_ SOCKET s,
    _In_reads_(dwBufferCount) LPWSABUF lpBuffers,
    _In_ DWORD dwBufferCount,
    _Out_opt_ LPDWORD lpNumberOfBytesSent,
    _In_ DWORD dwFlags,
    _Inout_opt_ LPWSAOVERLAPPED lpOverlapped,
    _In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    _In_opt_ LPWSATHREADID lpThreadId,
    _Out_ LPINT lpErrno
)
{
    WCHAR strTip[256];
    wsprintf(strTip, L"The following content is intercepted: \"%s.\"", lpBuffers);
    MessageBoxW(0, strTip, exepath, MB_OK);
    *lpErrno = WSAECONNABORTED;
    return SOCKET_ERROR;

}

int WSPAPI WSPStartup(
    WORD wVersionRequested,                          //用户程序加载套接字库的版本号(in)
    LPWSPDATA lpWSPData,                               //用于取得Winsock服务的详细信息
    LPWSAPROTOCOL_INFO lpProtocolInfo,   //指定想得到的协议的特征
    WSPUPCALLTABLE UpcallTable,                 //Ws2_32.dll向上调用转发的函数表
    LPWSPPROC_TABLE lpProcTable                 //下层提供者的函数表（一般为基础协议,共30个服务函数）
)
{
    WCHAR strTip[MAX_PATH +100];
    wsprintf(strTip, L"LSPDLL.dll: Function WSPStartup is invoked.");
    MessageBoxW(0, strTip, exepath, MB_OK);
    //如果协议位分层协议或基础协议,那么返回错误
    if (lpProtocolInfo->ProtocolChain.ChainLen <= 1)
    {   //无法加载或初始化请求的服务提供程序
        return WSAEPROVIDERFAILEDINIT;
    }
    //找到下层协议的WSAPROTOCOL_INFOW结构体
    WSAPROTOCOL_INFOW NextProtocolInfo;
    int nTotalProtols;
    LPWSAPROTOCOL_INFOW pProtoInfo = GetProvider(&nTotalProtols);
    //下层提供者的入口ID
    DWORD dwBaseEntryId = lpProtocolInfo->ProtocolChain.ChainEntries[1];
    //遍历所有协议
    int i = 0;
    for (; i < nTotalProtols; i++)
    {//找到下层提供者协议
        if (pProtoInfo[i].dwCatalogEntryId == dwBaseEntryId)
        {
            memcpy(&NextProtocolInfo, &pProtoInfo[i], sizeof(WSAPROTOCOL_INFOW));
            break;
        }
    }
    //如果没找到
    if (i >= nTotalProtols)
        return WSAEPROVIDERFAILEDINIT;
    //加载下层协议的Dll
    int nError = 0;
    TCHAR szBaseProviderDll[MAX_PATH];
    int nLen = MAX_PATH;
    //取得下层提供者的DLL路径（可能包含坏境变量）
    if (WSCGetProviderPath(&NextProtocolInfo.ProviderId, szBaseProviderDll, &nLen, &nError) == SOCKET_ERROR)
        return WSAEPROVIDERFAILEDINIT;
    //坏境变量转换字符串
    if (!ExpandEnvironmentStrings(szBaseProviderDll, szBaseProviderDll, MAX_PATH))
        return WSAEPROVIDERFAILEDINIT;
    //加载dll
    HMODULE hModdule = LoadLibrary(szBaseProviderDll);
    if (hModdule == NULL)
        return WSAEPROVIDERFAILEDINIT;
    //取出下层提供者的WSPStartup函数
    LPWSPSTARTUP pfnWSPStartup = (LPWSPSTARTUP)GetProcAddress(hModdule, "WSPStartup");
    if (NULL == pfnWSPStartup)
        return WSAEPROVIDERFAILEDINIT;
    LPWSAPROTOCOL_INFOW pInfo = lpProtocolInfo;
    if (NextProtocolInfo.ProtocolChain.ChainLen == BASE_PROTOCOL)//如果下层提供者是基础协议
        pInfo = &NextProtocolInfo;                               //赋给pInfo指针
        //调用下层提供者的初始化函数
    int nRet = pfnWSPStartup(wVersionRequested, lpWSPData, lpProtocolInfo, UpcallTable, lpProcTable);
    //初始化失败
    if (nRet != ERROR_SUCCESS)
        return nRet;

    //初始化完成后,复制下层提供者(基础协议)的整个函数表
    g_NextProcTable = *lpProcTable;
    //将基础协议的SendTo函数指针,指向我们的WSPSendTo函数,在我们的函数内,再确定要不要调用回基础协议的Sendto函数
    lpProcTable->lpWSPSendTo = WSPSendTo;
    lpProcTable->lpWSPConnect = WSPConnect;
    lpProcTable->lpWSPSend = WSPSend;
    FreeProvider(pProtoInfo);
    return nRet;
}
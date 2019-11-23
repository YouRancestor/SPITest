# WinSock SPI(Service Provider Interface)调研

- 分层协议提供者（LSP）通过该接口将自己的协议（dll）安装到WinSock协议目录，来修改网络通信流程，对网络数据包进行监控、拦截、修改等操作。

- 当应用程序创建套接字时，系统会遍历WinSock协议目录，按顺序找到并使用第一个符合条件的协议，并加载此提供者的DLL，调用提供者的WSPStartup初始化函数。

- 协议类型：基础协议、分层协议、协议链。基础协议：实际与远程端点进行数据交换的协议，如TCP、UDP；分层协议：依靠基础协议进行通信，位于其上层，如安全层对数据进行加密；协议链：将若干分层协议与一个基础协议串联起来。

- 协议链安装流程

  1. 填充WSAPROTOCOL_INFO结构体，调用WSCInstallProvider指定DLL路径（路径字符串支持环境变量），安装一个分层协议。

  1. 通过WSCEnumProtocols枚举出所有已安装协议，遍历根据提供者GUID获取上述安装的分层协议在目录中的入口ID，用以填充协议链的WSAPROTOCOL_INFO。安装协议链。

  1. 再次调用WSCEnumProtocols获取所有已安装协议，记录所有协议的入口ID，通过WSCWriteProviderOrder将入口ID重新排序将上述协议链置于目录首位。

  1. 使用WSCDeinstallProvider卸载协议。

- 注意
  
  * 修改WinSock协议目录需要管理员权限。

  * 32位和64位的WinSock目录是独立的，需要分别安装，并分别提供动态库。

  * 系统通过提供者注册的DLL路径为任何使用网络的应用程序加载提供者的DLL，故注册时所使用的DLL路径需要在任何目录都能查找到（使用绝对路径或PATH）。

  * 安装/卸载协议即时生效，但不影响已建立的Socket。系统会通知通过WSAProviderConfigChange注册了回调的进程协议目录发生变化。

- 协议链DLL

  * 需要导出一个初始化函数WSPStartup。

  * 在初始化时通过自身协议链记录的下层协议入口ID，以及WSCEnumProtocols找到下层协议，通过WSCGetProviderPath获取其DLL路径并加载。然后调用下层协议的WSPStartup对下层协议进行初始化。下层协议初始化后记录其函数表留用，并用自己的函数替换掉函数表中对应的函数。

  * 实现函数表中需要的函数，在实现中调用函数表中记录的下层协议的实现函数，以完成实际的数据传输。

  * 反初始化函数在函数表中注册，而不是导出。
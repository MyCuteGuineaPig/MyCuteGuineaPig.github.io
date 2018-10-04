---
layout:     post
title:      "QuickFIX —— C++ 配置"
subtitle:   "FIX Engine C++ library —— QuickFix"
date:       2018-10-03 19:00:00
author:     "Becks"
header-img: "img/post/Deep_Learning-Sequence_Model_note/bg.jpg"
catalog:    true
tags:
    - Fix Engine
    - C++ library
  
---

## install QuickFix

1. Git Clone [Quick FIX Library](https://github.com/quickfix/quickfix)
2. Open Quickfix_vs15, then right click build solution, it will generate include and lib directory

![](/img/post/quickfixpic1.PNG)

## include in project


> C/C++ | Code Generation | Enable C++ Exceptions, should be Yes.
> C/C++ | Code Generation | Runtime Library, should be set to Multithreaded DLL or Debug Multithreaded DLL.
> C/C++ | General | Additional Include Directories add the root quickfix directory.
> Linker | Input | Additional Dependencies, must contain quickfix.lib and ws2_32.lib.
> Linker | General | Additional Library Directories, add the quickfix/lib directory.

## Settings File
Settings File是用于FIX::SessionSettings settings读取用, DataDictionary 是用来规定incoming FIX messages的xml file, datadictionary在下载的quickfix spec的文件夹中

client端
```
[DEFAULT]
ConnectionType=initiator
ReconnectInterval=60
SenderCompID=**
TargetCompID=**
FileLogPath=log
FileStorePath=log2
TimeZone=America/New_York
[SESSION]
BeginString=FIX.4.4
StartTime=17:05:30
EndTime=17:05:00
HeartBtInt=30
SocketConnectPort=**
SocketConnectHost=***
DataDictionary=\spec\FIX44.xml
```
## Customized DataDic

对方来的Message的要满足一定格式; 比如使用默认datadictionary, 当subscribe massquote, 对方的message特殊, 对方send的message并没有tag = 304(TotQuoteEntrie), 但是默认的datadictionary 默认必须有这个tag, 发显示(Message 2 Rejected: Required tag missing:304), client端就会发送(35=3\|58=Required tag missing\|371=304). 而实际上没必要发送, 需要改变的是我们data dictionary 

需要更改的datadictionary: FIX44.xml
![](/img/post/quickfixpic2.PNG)

更改后的datadictionary: 去掉不需要的field(去掉TotNoQuoteEntries), 加上需要的field
![](/img/post/quickfixpic3.PNG)


## Logon

client端用toAdmin()  callback, server 端用fromAdmin() callback
```C++
void YourMessageCracker::toAdmin( FIX::Message& message, const FIX::SessionID& sessionID)
{
    if (FIX::MsgType_Logon == message.getHeader().getField(FIX::FIELD::MsgType))
    {
        //FIX44::Logon& logon_message = dynamic_cast<FIX44::Logon&>(message);
        message.setField(FIX::Username("my_username"));
        message.setField(FIX::Password("my_password"));
    }
}
```

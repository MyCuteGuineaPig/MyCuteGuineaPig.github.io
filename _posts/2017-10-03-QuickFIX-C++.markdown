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

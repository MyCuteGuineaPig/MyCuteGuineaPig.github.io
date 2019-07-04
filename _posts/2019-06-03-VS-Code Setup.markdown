---
layout:     post
title:      "VSCode - Set-up"
subtitle:   "Visual Studio Code 配置"
date:       2019-07-03 20:00:00
author:     "Becks"
header-img: "img/post-bg3.jpg"
catalog:    true
tags:
    - Visual Studio Code
    - VS
    - 配置
    - Setup
---

## C++ Windows

#### 下载MINGW

1. 下载MinGW Installation Manager
2. click install
3. 右键所有的Package -> mark for installation, 再点Installation ->  ApplyChanges

![](/img/post/VSCode/MinGW1.png)

![](/img/post/VSCode/MinGW2.png)


#### add MINGW to system path

1. 控制面板--> 系统和安全 --> 系统  --> 高级系统设置
2. 高级 --> 环境变量
3. 系统变量内找到Path, 双击编辑, 把;C:\MinGW\bin 加在变量值的后面

![](/img/post/VSCode/MinGW3.png)

![](/img/post/VSCode/MinGW4.png)

![](/img/post/VSCode/MinGW5.png)

#### Build C++

1. 建一个folder, folder 里面有main.cpp,  需要build的程序
2. Ctrl + shift + p , search tasks, 选择 Tasks: Configure Task -->  选择create task.json file from template --> 选择 Others (Example to run an arbitrary external command）, 会生成一个tasks.json文件， 我们把 "command" 改成 "g++ -g main.cpp"   (-g is debug) or 写成如下图格式
3. Ctrl + Shift + B 可以build project (如果出现permission defined 错误，关掉VS Code, 重新打开再Ctrl+Shift +B, build)
4. 可以在VS Code中新建terminal, 点plus button. run  a.exe

![](/img/post/VSCode/task0.png)

![](/img/post/VSCode/task1.png)

![](/img/post/VSCode/task2.png)

![](/img/post/VSCode/task3.png)

注：不用Ctrl + shift + p, search C\Cpp: Edit Configurations, 在新的VS Code的configuration中自动设置好了MinGW include path



查看是不是把 MinGW 放进Environment variable,打开prompt, 输入 

```shell

g++ --version #如果MinGW 放进Environment variable，会显示信息

```

#### Debug in VS Code

1. 点击左侧 Debug Panel, 点击绿色箭头 --> 因为我们用MinGW, 选择C++ (GDB//LLDB) 
2. 修改launch.json
	- 修改 "miDebuggerPath": "C:\\MinGW\\bin\\gdb.exe"  (gdb.exe 是debugger)
	- add "preLaunchTask": "echo",  这个preLaunchTask 需要与tasks.json task 命名一样, (是为了build code first, then start debugging)
	- 修改"program": "${workspaceFolder}/a.exe", 这是告诉什么exe 用来debug的
3. 可以再点击绿箭头, 开始Debug, 可以设置Break point,来方便debug
	- 当开始Debug,可以控制到下一个step over break point,step into, step out

![](/img/post/VSCode/win_Debug1.png)

![](/img/post/VSCode/win_Debug2.png)

![](/img/post/VSCode/win_Debug3.png)



## C++ Mac

#### 下载code runner

1. 到extension 中找到code runner，下载
2. 到user/用户名/⁨ .vscode⁩ / ⁨extensions⁩ / ⁨formulahendry.code-runner-0.9.10⁩ / out⁩ /,codeManager.js, comment 掉line 12 和 line 225~236, 如图
3. 重新加载，或关掉VS Code 再打开，点击右上角箭头，run

![](/img/post/VSCode/coderunner1.png)

![](/img/post/VSCode/coderunner2.png)

![](/img/post/VSCode/coderunner3.png)



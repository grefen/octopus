# octopus
基于TCP分布式网络库，主要应用包括作为常见server，或作为分布式计算框架；
本网络库的初衷是作为小型的分布式计算框架使用；
主要应用场景是:使用局域网中的多台机器，对复杂的数学模型参数优化；

分三个阶段开发：
阶段一：针对单进程，完成基本的模块开发；
阶段二：在阶段一基础上，针对多进程进行扩展；
阶段三：针对具体的应用，完成应用实例；

环境安装
总体思路是：
0 采用vs2015开发，用linux虚拟机远程调试；单元测试采用gtest；
1 vs2015在build时会把本地代码copy到虚拟机中编译，所以需要在linux中安装gtest； 
2 在vs2015的测试工程中，需要在linker->library Dependencies中 添加： gtest pthread

（使用静态库的方法gcc -o test test.c /usr/lib/libm.a 或 gcc -o test test.c -lm）

gtest的安装
安装gtest分三步:
1.安装源代码
在ubuntu的桌面上,右键选择打开终端,在终端中输入如下命令:
$sudo apt-get install libgtest-dev

下载源码后,apt将会在目录/usr/src/生成gtest文件夹来存放源码.
2. 编译源代码
接着刚才的命令,我们继续再刚才打开的终端中输入:
$cd /usr/src/gtest

来进入源码存放的地方,里面的文件很简单(使用ls查看),一个源码 
文件夹,一个cmake文件夹和一个cmake的配置文件(CMakeLists.txt). 继续输入命令:
$sudo cmake .

这个命令会根据当前目录下cmake的配置文件生成对应的makefile 
文件.有了makefile文件后,只要编译就好, 
输入如下命令:
$sudo make

等编译完成,就可以看到生成的库文件和一堆没有用的东西,这些没 
用的东西可以删掉.(ps:在一开始的cd命令没有加sudo是因为根目 
录只对普通用户开放读的权限,而没有写的权限,后面的两个命令都 
会有新的文件生成,所以得使用超级用户权限.)
3.将编译生成好的库拷贝到系统目录下
将生成的libgtest.a 和 libgtest_main.a 拷贝到系统的lib路径下. 
具体命令如下:
$sudo cp libgtest*.a /usr/local/lib

OK,这样google gtest的库全部安装好了!

虚拟机中没有ip的解决方法：
sudo dhclient
重新获取ip

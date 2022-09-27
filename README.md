# 文件服务器/客户端
## 一、概要
个人学习tcp socket网络编程demo。基于Linux socket接口实现了多客户端上传下载、大文件传输和断点续传。
## 二、环境搭建
### 1. 安装cmake
~~~ bash
sudo apt install cmake
cmake -v
~~~
### 2. 安装ncurses
~~~bash
sudo apt-get install libncurses5-dev
~~~
### 3. 编译
~~~ bash
git clone https://github.com/cyicz123/file-server-client.git
cd file-server-client
mkdir build && cd build
cmake ..
make
sudo make install
~~~
在项目根目录就会生成bin文件夹，内部存放在ser和cli的服务端、客户端程序。
## 三、功能展示
## 3.1 启动服务端
![服务端启动](https://cdn.jsdelivr.net/gh/cyicz123/Picture@main/img/202209171551149.png)
## 3.2 客户端使用
**1. 帮助信息**
~~~bash
./cli -h # 或者 ./cli --help
# 打印输出
Usage: file-client [OPTION]... [FILE]...
A file transfer tool, support upload and download.

Mandatory arguments to long options are mandatory for short options too.
        -s, --ser_addr IP:Port                  Set the server address to IP :port. The default address is 127.0.0.1:8080.
        -l, --ls [FILE-DIRECTORY]                       View remote server downloadable files.
        -d, --download FILE                     Download the file.
        -u, --upload FILE                       Upload the file.

        -h, --help                      Print the help info.
        -v, --version                   Print the version.
~~~
**2. 测试上传**

生成10G大小测试文件
~~~ bash
dd if=/dev/zero of=test_data_10G bs=1G count=10
~~~

~~~ bash
./cli -u test_data_10G
~~~
![上传效果](https://cdn.jsdelivr.net/gh/cyicz123/Picture@main/img/202209171608685.png)
**P.S.** 当超过GB大小的文件上传，大概率会出错，最后的若干字节会丢失。

**3. 查看服务器文件列表**
~~~bash
./cli -l
~~~
**4. 测试下载**
~~~ bash
mv test_data_10G storage/ # 因为现在上传功能存在问题。所以需要手动将待下载文件移至服务器目录下，即storage目录
./cli -l # 查看服务器文件列表
./cli -d test_data_10G
~~~
![客户端上传](https://cdn.jsdelivr.net/gh/cyicz123/Picture@main/img/202209171623052.png)
**5. 断点续传**

所有上传下载均支持断点续传，按`ctrl c`取消传输后，重新执行命令即可断点续传
## 四、TODO
- [x] 解决大文件上传卡死、错误的问题
- [ ] 进度条加载支持多线程
- [ ] 日志记录支持多线程
- [ ] 日志记录增加客户端连接ip信息 
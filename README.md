# yunjian'chatroom

## 介绍

## 项目依赖
```
-  hiredis
-  nlohmann
```

## 使用方法

1. 下载源码

~~~ shell
git clone https://github.com/yunjianshijie/ccc.git chatroom

cd chatroom
~~~
2. 安装依赖

~~~shell 
sudo pacman -S hiredis nlohmann-json 
~~~

3. 运行
服务器
~~~ shell 
cd src/server
mkdir build && cd build
cmake .. && make -j8
./server
~~~
客户端
~~~shell 
cd src/client
mkdir build && cd build
cmake .. && make -j8
./client
~~~

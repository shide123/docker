************配置运行环境************
0. sudo apt-get install build-essential安装g++
1. sudo apt-get install default-jdk
2. sudo apt-get install mysql-server(可选）
3. sudo apt-get install mysql-client
4. sudo apt-get install libmysqlclient-dev
5. sudo apt-get install redis-server 
6. 将sdk.tar.bz2解压到 /home/boost_1_58_0/sdk中，如果是cp,记得加上-ar参数
#7. sudo cp /home/boost_1_58_0/sdk/lib/libboost_system.so.1.61.0 /usr/local/lib/
#8. sudo cp /home/boost_1_58_0/sdk/lib/libboost_thread.so.1.61.0 /usr/local/lib/
#9.把/home/boost_1_58_0/sdk/lib/目录加入/etc/ld.so.conf文件里面，然后执行：sudo ldconfig 命令即可。
   执行export LD_LIBRARY_PATH=/usr/local/lib:/home/boost_1_58_0/sdk/lib:$LD_LIBRARY_PATH。（二选一）
10. sudo ldconfig
11. sudo su - root切换到root
12. 进入工程comm/lib目录下，执行make_libs.sh
 
12. 进入工程comm/lib目录下，执行protobuf,,./configure&&make&&sudo make install
13. sudo apt-get install libcrypto++-dev
************配置IDE****************
1.将eclipse拷贝到Linux运行环境中
2.为eclipse增加执行权限 chmod +x eclipse
3.java -version 若系统没有安装jdk,
  sudo apt-get install default-jre
  sudo apt-get install default-jdk

4.sudo apt-get install libmysqlclient-dev
4.导入工程文件，enjoy yourself

yum install cryptopp cryptopp-devel

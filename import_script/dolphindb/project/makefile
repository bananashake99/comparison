#指定用到的头文件和库文件路径指定(根据需要自行添加)
INC=-I../include
LIB=-lDolphinDBAPI /usr/lib/x86_64-linux-gnu/libssl.so.1.0.0 /usr/lib/x86_64-linux-gnu/libcrypto.so.1.0.0 -lpthread -luuid -L../bin -Wl,-rpath ../bin/ 

#编译器
CXX=g++ -std=c++11 #c++

#compile and lib parameter
#编译参数
DEFINES=-D_GLIBCXX_USE_CXX11_ABI=0 -DLINUX -DLOGGING_LEVEL_2 
CFLAGS=-g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAG=$(CFLAGS) -DHAVE_CONFIG_H

#object file 
OBJECT=./test.cpp 
# OBJECT=$(wildcard *.cpp) 

#source file
ALL= ./dolphindbTest
# ALL=$(patsubst %cpp, %o, $(OBJECT))  

$(ALL):$(OBJECT)
	$(CXX) $(INC) $(CXXFLAG) $^ -o $@ $(LIB)
%*.o:%*.cpp
	$(CXX) $(INC) $(CXXFLAG) -c $^ -o $@ $(LIB)

.PRONY:
	clean
clean:
	@echo "Removing linked and compiled files......"
	rm -rf $(ALL) $(OBJECT)
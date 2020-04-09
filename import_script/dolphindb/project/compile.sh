colNum=3
if [[ $# -eq 0 ]]; then
    echo "Using default colNum: $colNum"
    g++ test.cpp -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -DLINUX -DLOGGING_LEVEL_2 -O2 -I../include -lDolphinDBAPI /usr/lib/x86_64-linux-gnu/libssl.so.1.0.0 /usr/lib/x86_64-linux-gnu/libcrypto.so.1.0.0 -lpthread -luuid -L../bin -Wl,-rpath ../bin/ -o dolphindbTest
else
    colNum=$1
    echo "colNum: $colNum"
    g++ test128.cpp -std=c++11 -D_GLIBCXX_USE_CXX11_ABI=0 -DLINUX -DLOGGING_LEVEL_2 -O2 -I../include -lDolphinDBAPI /usr/lib/x86_64-linux-gnu/libssl.so.1.0.0 /usr/lib/x86_64-linux-gnu/libcrypto.so.1.0.0 -lpthread -luuid -L../bin -Wl,-rpath ../bin/ -o dolphindbTest128
fi
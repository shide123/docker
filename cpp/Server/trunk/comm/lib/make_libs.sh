CURDIR=`dirname $0`
cd $CURDIR
CURDIR=`pwd`

failed=""
clean=0
if [[ $# = 1 && $1 = "clean" ]];then
    clean=1
fi

libs=("com" "db" "hiredis" "libconhash" "SocketLite/src" "libnode")
libs_=("libcom.a" "libdb.a" "libhiredis.a" "libconhash.a" "libsocketlite.a" "libnode.a")
for(( i = 0; i < ${#libs[@]}; i++ )) ; do 
    lib=${libs[i]}
    lib_=${libs_[i]}
    cd $CURDIR/$lib
    if [[ $clean = 1 ]];then
        make clean > /dev/null
    fi
    echo "making [$lib]..." | awk '{printf"%s %-20s ",$1,$2}'
    
    if make -j4 1> /dev/null 2> make.log;then
        rm -f make.log
        echo "successful"
    else
        failed="$failed""$lib,"
        echo "failed"
    fi
done

if [[ -n $failed ]];then
    echo "[${failed:0:$(( ${#failed} - 1 ))}] failed to build, see make.log in relevant directory!" >&2
    exit 1
fi

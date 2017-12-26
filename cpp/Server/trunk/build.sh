suffix="_ql"
ALLPROG=("worker" "gateway" "webgate" "logonsvr" "pushmsg" "tppushsvr" "chatsvr" "roomsvr" "consumesvr" "syncsvr" "usermgrsvr")

CURDIR=`dirname $0`
cd $CURDIR
CURDIR=`pwd`
failed=""

usage()
{
    echo "Usage: ${0##*/} {all/list} [clean]"
    echo "  [all]    build all projects in list"
    echo "  [list]   ${ALLPROG[@]}"
    echo "  [clean]  project will be cleaned before build."
}

#parse input params
for i in $@; do
    if [[ $i = "all" ]];then
        for p in ${ALLPROG[@]};do
            if echo ${progs[@]} | grep -v $p 1>/dev/null;then
                progs[${#progs[@]}]=$p
            fi
        done
    elif [[ $i = "clean" ]];then
        clean='clean'
    elif [[ $i = "-h" || $i = "help" ]];then
        usage
        exit
    elif echo ${ALLPROG[@]} | grep -w $i 1>/dev/null;then
        #be sure that worker would be builed firstly
        if echo ${progs[@]} | grep -v worker 1>/dev/null;then
            progs[${#progs[@]}]=worker
        fi

        if echo ${progs[@]} | grep -v $i 1>/dev/null;then
            progs[${#progs[@]}]=$i
        fi
    else
        echo "error: unknown parameter[$i], please read the usage.."
        echo
        usage
        exit
    fi
done

if [[ ${#progs[@]} = 0 ]];then
    usage
    exit
fi


buildOne()
{
    prog=$1
    cd $CURDIR/$prog
    if [[ $prog = "worker" ]];then
        cmpWorker libworker.a 1>/dev/null
    else
        cmpWorker $prog$suffix 1>/dev/null
    fi
    if [[ $clean = 'clean' ]];then
        make clean 1> /dev/null
    fi
    echo "making [$prog]..." | awk '{printf"%s %-20s ",$1,$2}'
    if make -j10 1> /dev/null 2> make.log;then
        rm -f make.log
        echo "success"
    else
        failed="$failed$prog,"
        echo "failed"
    fi
}

cmpWorker()
{
    worker="../worker/libworker.a"
    new=`find $worker -newer $1 2>/dev/null`
    if [[ $new = $worker ]];then
        sudo rm -f $1
    fi
}

svn up 

if ! bash $CURDIR/comm/lib/make_libs.sh 1> /dev/null;then
    exit 1
fi

for(( i = 0; i < ${#progs[@]}; i++ )) ; do 
    buildOne ${progs[i]}
done

if [[ -n $failed ]];then
    echo
    echo "project[${failed:0:$(( ${#failed} - 1 ))}] failed to build, see make.log in relevant directory!" >&2
    exit 1
fi

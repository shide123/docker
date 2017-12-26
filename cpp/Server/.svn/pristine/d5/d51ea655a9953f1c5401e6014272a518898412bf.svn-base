cd `dirname $0`
user=`whoami`
localip=`ifconfig eth0 | grep "inet addr" | awk '{ print $2}' | awk -F: '{print $2}'`
qlip="123.103.74.6"
qlbkip="123.103.74.10"
suffix="_ql"  
#ALLPROG=("gateway" "logonsvr" "pushmsg" "tppushsvr" "syncsvr" "chatsvr" "roomsvr" "consumesvr" "usermgrsvr")
ALLPROG=("gateway" "webgate" "logonsvr" "pushmsg" "tppushsvr" "chatsvr" "roomsvr" "consumesvr" "usermgrsvr")
packet="ql.zip"
packetdir="deploy_history"
backupdir="backup"
now=`date +%Y%m%d%H%M%S`
packetname="${packet}-${now}"
backupname="services${suffix}-${now}.zip"
deploy_dir="/usr/local/services_ql"
cmdlist=("start" "stop" "restart")

HOSTS=("local" "ql" "qlbk")

function usage()
{
	echo "usage: deploy.sh [-h hostname(gz,bj)] [-i program(roomsvr,chatsvr)] [-e program(roomsvr,syncsvr)]"
	echo "  -h  hostname split by comma, like [-h gz,bj]"
	echo "  -i  include programs split by comma, like [-i syncsvr,roomsvr]"
	echo "  -e  exclude programs split by comma, like [-e syncsvr,roomsvr]"
	echo "  --nopacket  do not zip programs into $packet, but zip file $packet must exist."
	echo "  --cmd=[restart,stop]   restart services of remote server only."
	echo "  --transferonly   transfer programs to remote server only."
	exit
}

function ask_for_continue()
{
	if [ $# -lt 1 ]; then
		return 1
	fi

	if [[ "1" = ${noask} ]]; then
		return 0
	fi

	while [[ 1 = 1 ]];do
		read -p "${1} y/n? " opt
		opt=${opt:0:1}
		if [[ $opt = "y" || $opt = "Y" ]];then
		    return 0
		elif [[ $opt = "n" || $opt = "N" ]];then
			return 1
		fi
	done
}

function is_host()
{
	if [ $# -lt 1 ]; then
		return 1
	fi
	if echo ${HOSTS[@]} | grep -w $1 1>/dev/null; then
		return 0
	else
		echo "[ERROR] $1 is not a host."
		exit
	fi
}

function is_program()
{
	if [ $# -lt 1 ]; then
		return 1
	fi
	if echo ${ALLPROG[@]} | grep -w $1 1>/dev/null; then
		return 0
	else
		echo "[ERROR] $1 is not a program."
		exit
	fi
}

function parse_param()
{
	num=$#
	for ((i=0;i<$num;i++)); do
		if [[ "-h" = $1 ]];then
			shift
			((i++))
			if [[ $i -lt $num ]];then
				input=`echo ${1//,/ }`
				for host in $input; do
					is_host $host
					hosts[${#hosts[@]}]=$host
				done
			fi
		elif [[ "-i" = $1 ]];then
			shift
			((i++))
			if [[ $i -lt $num ]];then
				incs=`echo ${1//,/ }`
				for inc in $incs; do
					is_program "$inc"
					INCPROG[${#INCPROG[@]}]=$inc
				done
			fi
		elif [[ "-e" = $1 ]];then
			shift
			((i++))
			if [[ $i -lt $num ]];then
				excs=`echo ${1//,/ }`
				for exc in $excs; do
					is_program "$exc"
					EXCPROG[${#EXCPROG[@]}]=$exc
				done
			fi
		elif [[ "--nopacket" = $1 ]]; then
			nopacket=1
		elif [[ "--cmd" = ${1%=*} ]]; then
			if echo ${cmdlist[@]} | grep -w ${1/--cmd=/};then
				cmd=${1/--cmd=/}
			else
				echo "cmd is unknown: ${1/--cmd=/}"
				exit
			fi
		elif [[ "--transferonly" = $1 ]]; then
			transferonly=1
		elif [[ "--noask" = $1 ]]; then
			noask=1
		else
			usage
		fi
		shift
	done

	if [ ${#hosts[@]} -eq 0 ]; then
		hosts=("local")
	fi

	for host in ${hosts[@]};do
		ip=`eval echo '$'"$host""ip"`
		if [ ! -z "$ip" ];then
			ips[${#ips}]=$ip
		fi
	done

	if [ 0 -eq ${#ips[@]} ]; then
		echo "ip list is empty"
		exit
	fi

	echo "[hosts]: "${ips[@]}
	
	gen_deploy_programs
}

function is_localhost()
{
	if [[ $localip = $1 ]]; then
		return 0
	fi

	return 1
}

#param $1:ip
function transfer()
{
	if [ $# -lt 1 ]; then
		echo "error invoke[transfer $@], example[transfer ip]"
		return 1
	fi
	if is_localhost $1; then
		sudo cp ${packet} ${deploy_dir}/${packetdir}/${packetname} -f
	else
		if scp -P 1221 -l 40000 ${packet} ${user}"@"${1}:~/${packetname}; then
			ssh_cmd $1 "sudo mv ~/${packetname} ${deploy_dir}/${packetdir}/${packetname} -f"
		fi
	fi
	return $?
}

#input $1:ip $2:command
function ssh_cmd()
{
	if [ $# -lt 2 ];then
		echo "error invoke, usage: ssh_cmd ip command"
		return 1
	fi

	if is_localhost $1; then
		$2 >/dev/null
	else
		ssh ${user}@${1} -p 1221 "$2" >/dev/null
	fi

	return $?
}

function packet_programs()
{
	index=0
	for i in $@; do
	    ALLPATH[${index}]=$i/$i$suffix
	    ((index++))
	done
	zip -r ${packet} ${ALLPATH[@]}

	return $?
}

function gen_deploy_programs()
{
	if [ 0 -ne ${#INCPROG[@]} ];then
		unset ALLPROG
		ALLPROG=${INCPROG[@]}
	fi
#	echo "INCPROG: ${ALLPROG[@]}"
#	echo "EXCPROG: ${EXCPROG[@]}"
	for exc in ${EXCPROG[@]}; do
		num=${#ALLPROG[@]}
		pos=0
		for ((i=0;i<$num;i++)); do
			if [[ $exc = ${ALLPROG[$i]} ]];then
				unset ALLPROG[$i]
			else
				if [ $pos -ne $i ]; then
					ALLPROG[$pos]=${ALLPROG[$i]}
					unset ALLPROG[$i]
				fi
				((pos++))
			fi
		done
	done

	echo "[PROGAMS]: "${ALLPROG[@]/ /,}
	echo
}

function stop_one()
{
	host=$1
	serv=$2$suffix

	if ! ssh_cmd $host "sudo pkill -0 $serv"; then
		return 0
	fi

	ssh_cmd $host "sudo ${deploy_dir}/$2/$serv stop"
	if ssh_cmd $host "sudo pkill -0 $serv"; then
		if ! ask_for_continue "[$host][$2] stop failed, continue?"; then
			exit
		fi
	else
		echo "[$host][$2] stop success"
	fi

	return 0
}

function restart_one()
{
	host=$1
	serv=$2$suffix

	stop_one $1 $2

	ssh_cmd $host "sudo ${deploy_dir}/$2/$serv restart -d" 
	if ssh_cmd $host "sudo pkill -0 $serv"; then
		echo "[$host][$2] start success"
	else
		if ! ask_for_continue "[$host][$2] start failed, continue?"; then
			exit
		fi
	fi
}

function restart_serv()
{
	if [ $# -lt 2 ]; then
		return 1
	fi

	host=$1
	serv=$2$suffix
	if ssh_cmd $host "sudo pkill -0 $serv"; then
		restart_one $1 $2
	elif ask_for_continue "[$host][$2] has not start previously, start it?"; then
		restart_one $1 $2
	else
		echo "[$host][$2] has not run previously, ignore."

	fi

	return 0
}

parse_param $@

if [ ! -z $cmd ]; then
	if ! ask_for_continue "${cmd} [${ALLPROG[@]/ /,}]?"; then
		exit
	fi

	for host in ${ips[@]}; do
		for serv in ${ALLPROG[@]}; do
			if [[ "stop" = $cmd ]]; then
				stop_one $host $serv
			elif [[ "restart" = $cmd ]]; then
				restart_serv $host $serv
			else
				echo "unknown cmd: ${cmd}."
				exit
			fi
		done
	done
	exit
fi

#packet programs
if [[ ! "1" = $nopacket ]]; then
	echo "[Packet]------------------------------------------------------------"
	sudo rm -f "${packet}"
	if ! packet_programs ${ALLPROG[@]}; then
		exit
	fi
	echo "[Packet]------------------------------------------------------------"
fi

if [[ "1" = ${transferonly} ]]; then
	for host in ${ips[@]}; do
		if transfer $host; then
			if ask_for_continue "[$host] unpacket programs?"; then
				for serv in ${ALLPROG[@]}; do
					ssh_cmd $host "cd ${deploy_dir} && sudo zip ${backupdir}/${backupname} $serv/$serv$suffix"
					if ssh_cmd $host "sudo unzip -o ${deploy_dir}/${packetdir}/${packetname} -d ${deploy_dir} $serv/$serv$suffix" >/dev/null; then
						echo "[$host][$serv] unpacket success"
					else
						echo "[$host][$serv] unpacket failed"
						exit
					fi
				done
			fi
		else
			echo "[$host] transfer failed"
			exit
		fi
	done
	exit
fi

echo
echo "[Deploy]------------------------------------------------------------"
#transfer to remote server
for host in ${ips[@]}; do
	if transfer $host; then
		echo "[$host] transfer success"

		#unpacket and restart all services
		if ask_for_continue "[$host] unpacket and restart?"; then
			for serv in ${ALLPROG[@]}; do
				ssh_cmd $host "sudo zip ${deploy_dir}/${backupdir}/${backupname} ${deploy_dir}/$serv/$serv$suffix"
				if ssh_cmd $host "sudo unzip -o ${deploy_dir}/${packetdir}/${packetname} -d ${deploy_dir} $serv/$serv$suffix" >/dev/null; then
					restart_serv $host $serv
				else
					echo "[$host] unpacket failed"
					exit
				fi
			done
		else
			echo "exit deploy.sh"
			exit
		fi
	else
		echo "[$host] transfer failed"
		exit
	fi
done
echo "[Deploy]------------------------------------------------------------"

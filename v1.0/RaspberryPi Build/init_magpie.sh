	#!/bin/bash
#figlet MAGPIE -f "slant"
pkill screen
pkill python3
pkill tshark
pkill parse
pkill synchro
pkill rx_power
sh wifimon_init.sh
sleep 1
rm ./ready.txt 2> /dev/null
toilet -f mono12 'MAGPIE' 
echo Monitoring AGainst cyber-PhysIcal thrEats \>.
cat << "EOF"

                          .--.
                        ."  o \__
                     _.-"    ,(  `
                 _.-"      ,;;|
            _.-=" _,"    ,,;;;'
        .-"`_.-"``-..,,;;;;:'
        `"'`          `\`\
                       /^\\\
EOF

rpp=1
window=3
collect=5
#activity=1
#while IFS=, read -r rpp window activity collect
#do
#done < ./config.csv
echo "[INPUT] Enter MAGPIE runtime mode (1 - Monitor, 2 - Attack Testing)"
        read mode
echo "[INPUT] Enable Activity Presence Recognition? (1 - Yes, 0 - No)"
        read activity
	echo
        echo "-----------------------------------------------------------------------------------"
        echo "| [CONFIGURATION] Current loaded parameters in config.csv:"
        echo "-----------------------------------------------------------------------------------"
        echo "| Monitoring sensitivity (1 - Balanced, 2 - Low, 3 - High): $rpp"
        echo "| Monitoring window interval: $window seconds"
        echo "| Activity presence recognition (1 - Enabled, 2 - Disabled): $activity"
        echo "| Datastream collection period (enabled in learning phase only): $collect minutes"
        echo "-----------------------------------------------------------------------------------"
        echo
if [ $mode -eq 1 ]; then
	if [ $activity -eq 1 ]; then
		file1=/home/pi/magpie/datastore/0.train.rf.csv
        	file2=/home/pi/magpie/datastore/1.train.rf.csv
		if [ -e "$file1" ] && [ -e "$file2" ]; then
			activity_mode=1
			activity_presence=1
		else
                	echo "\e[33m[WARNING] Activity presence recognition cannot run without both 'activity' & 'no activity' MDS data)\e[0m"
			echo "\e[33m[WARNING] Activity presence recognition: Disabled\e[0m"
			echo "[INPUT] Enter activity presence state for monitoring (1 - Activity presence, 0 - No activity presence)"
                	read activity_presence
                	activity_mode=0
		fi
	elif [ $activity -eq 0 ]; then
		echo "\e[33m[WARNING] Activity presence recognition is disabled\e[0m"
		echo "[INPUT] Enter activity presence state for monitoring (1 - Activity presence, 0 - No activity presence)"
       		read activity_presence
		#file=./datastore/$activity_presence.train.rf.csv
                #while [ ! -e "$file" ]; do
		#	echo -e "\e[91mLight red[ERROR] No MDS datastore samples exist for activity presence state ($activity_presence)"
		#	echo "[INPUT] Enter activity presence state for monitoring (1 - Activity presence, 0 - No activity presence)" 
		#	file=./datastore/$activity_presence.train.rf.csv
		activity_mode=0
	fi
        filename="$activity_presence.train"
elif [ $mode -eq 2 ]; then
	echo "[INPUT] Enter activity presence state for testing (1 - Activity presence, 0 - No activity presence)"
        read activity_presence
	echo "[INPUT] Enter attack ID number"
        read attack
	echo "[INPUT] Enter attack testing start time (hh:mm:ss)"
        read start_time
	echo "[INPUT] Enter attack testing stop time (hh:mm:ss)"
        read stop_time
        date=`date '+%Y-%m-%d'`
        start="$date $start_time"
        stop="$date $stop_time"
        start_epoch=$(date -d "$start" +"%s")
        stop_epoch=$(date -d "$stop" +"%s")
        echo "\e[32mGreen[CONFIG SET] Attack testing start (EPOCH timestamp): $start_epoch\e[0m"
        echo "\e[32mGreen[CONFIG SET] Attack testing stop (EPOCH timestamp): $stop_epoch\e[0m"
        echo "$start_epoch,$stop_epoch" > /home/pi/magpie/datastore/$activity_presence.$attack.csv
        filename="$activity_presence.$attack"
	activity_mode=0
	python stop.py -ep $stop_epoch &
elif [ $mode -eq 3 ]; then
	activity_mode=0
	filename="$activity_presence.train"
fi
#begin main script initiation process
#echo "Test: activity check = $activity_mode"
python3  magpie_main.py -r $rpp -w $window -a $activity_mode -p $activity_presence -c $collect -f $filename -m $mode &
file=./ready.txt
while [ ! -e "$file" ]; do
	:
done
echo " \e[33mYellow[LOADING] Initiating datastream feed collection and MDS generation \e[0m"

# initiate MDS parser server
./cocoon_parser/synchro &

# initiate rf MDS feed (master feed for synchronisation)
/home/pi/rx_tools/rx_power -f 2.401G:2.495G:2M -i 1 -P 2> /dev/null | \
./cocoon_parser/parse -l -t -p -d $window | \
python3 /home/pi/magpie/push_feed.py -p 10001 -f "RF" &

sleep 0.1

# initate audio MDS feed
python sound.py  2>/dev/null | \
./cocoon_parser/parse -l -t -a -d $window | \
python3 /home/pi/magpie/push_feed.py -p 10002 -f "Audio" &


# initate ZigBee MDS feed
tshark -l -t ud -i usb0 -E separator="," 2>/dev/null | \
./cocoon_parser/parse -l -t -z -d $window | \
python3 /home/pi/magpie/push_feed.py -p 10003 -f "ZigBee" &

# copy existing protocol translation map for ZigBee node identities
if [  -f "/home/pi/cocoon_parser/protocols.csv" ]; then
	mv /home/pi/magpie/cocoon_parser/protocols.csv /home/pi/magpie/cocoon_parser/protocol_map.csv
fi

# initiate WiFi MDS feed
tshark -l -t ud -i wlan1 -y IEEE802_11_RADIO -Y "wlan.fc.type==0 and (wlan.bssid==8C:0D:76:64:69:31 or wlan_mgt.ssid==Cocoon1)" \
-T fields -e frame.time_epoch -e wlan.sa -e wlan.da -e frame.len -e wlan.fc.type -e wlan.fc.type_subtype -e radiotap.dbm_antsignal \
-E separator="," 2>/dev/null | \
./cocoon_parser/parse -l -t -w -d | \
python3 /home/pi/magpie/push_feed.py -p 10004 -f "WiFi" & 

# initiate IP MDS feed
tshark -l -i eth0 -f "ip and (tcp)" -Y "not (ip.addr==192.168.1.100)" -n \
-T fields -e frame.time_epoch -e ip.src -e ip.dst -e ip.len -e ip.ttl -e ip.proto -e _ws.col.srcport -e _ws.col.dstport \
-E separator="," -o 'gui.column.format:"No.","%m","Time","%t","Source","%s","Destination","%d","Length","%L","Info","%i","srcport","%S","dstport","%D"' 2>/dev/null | \
./cocoon_parser/parse -l -t -s -d $window | \
python3 /home/pi/magpie/push_feed.py -p 10005 -f "IP" & 

# set collection pipe collection begin collecting MDS feeds
echo -n "1" > ./collection_pipe

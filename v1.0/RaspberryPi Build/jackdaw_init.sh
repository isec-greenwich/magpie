#!/bin/bash
screen -dmS "jackdaw" /dev/ttyACM0
screen -S "jackdaw" -X stuff 's'
screen -S "jackdaw" -X stuff 'c'
screen -S "jackdaw" -X stuff '20'
screen -S "jackdaw" -X stuff '6'
screen -S "jackdaw" -X stuff 'r'
screen -S "jackdaw" -X eval "stuff \015"
sleep 1
ifconfig usb0 up


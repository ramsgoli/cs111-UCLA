#!/bin/bash

./lab4b --period=2 --scale=C --log="LOGFILE" <<-EOF
SCALE=F
PERIOD=1
START
STOP
OFF
EOF

ret=$?
if [ $ret -ne 0 ]
then
	echo "executable fails to process input!"
fi
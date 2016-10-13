#!/bin/sh

DEV_SAMPLE="/dev/pto_sample"
ZAPTEL_MODULE="pto4M"

F_PTO4M=`lsmod | grep ${ZAPTEL_MODULE} | sed 2,/^$/d | awk -F " " '{print $1}'`

echo -e " * Checking ${ZAPTEL_MODULE} module: \c"

if [ ! -z $F_PTO4M ]; then

	if [ $ZAPTEL_MODULE == $F_PTO4M ]; then
		echo "OK"
	else
		echo "KO... Exit"
		exit 2
	fi
else
	echo "KO... Exit"
	exit 3
fi

echo -e " * Checking sample device: \c"
if [ -c ${DEV_SAMPLE} ] ; then
	echo "${DEV_SAMPLE}"
else
	echo "${DEV_SAMPLE} doesn't exists"
	exit 4
fi

echo " * Lancement de l'applicatif"
./comm_ucc_pdt $@

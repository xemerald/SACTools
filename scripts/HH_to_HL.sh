#!/bin/bash
#
#
if [ $# -ne 1 ]
then
	echo "Usage: HH_to_HL.sh <Directory Path>"
	exit 0
fi

echo "Listing all the P-Alert archived SAC files..."
FILELIST=`cd ${1}; ls | grep "^[DHLPSTW][0-9]\{2,3\}[A-F]\{0,1\}\.HH[ZNE]\.TW\.--"`
for file in ${FILELIST}
do
	STATION=`echo ${file} | cut -d. -f1`
	NEW_CHAN=`echo ${file} | cut -d. -f2 | sed 's/HH/HL/'`
	NETWORK=`echo ${file} | cut -d. -f3`
	LOCATION=`echo ${file} | cut -d. -f4`
	NEW_FILE=${STATION}.${NEW_CHAN}.${NETWORK}.${LOCATION}
	scnl_mod_sac -c ${NEW_CHAN} ${1}/${file} ${1}/${NEW_FILE}
	rm -f ${1}/${file}
done
#
echo "Listing all the new archived SAC files..."
cd ${1}; echo "# SAC files list" > saclist; ls *.*.*.* >> saclist; cd -
exit

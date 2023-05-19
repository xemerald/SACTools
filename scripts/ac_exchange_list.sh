#!/bin/bash
#
#
WORKING_DIR=/root/working
TMP_DIR=${WORKING_DIR}/tmp
SRC_DIR=${WORKING_DIR}/data_src
RES_DIR=${WORKING_DIR}/archive_res
#
if [ ! -p /dev/stdin ]
then
	echo "Please input the list from STDIN!"
	exit 0
fi
#
while read line
do
	line_a=(${line})
	arcfile=${line_a[0]}
	arcfile=${arcfile//[[:space:]]/}
	filepath=${SRC_DIR}/EVENTS_NTU/${arcfile:0:6}
	if [ ${arcfile} ] && [ -e "${filepath}/${arcfile}" ]; then
		echo "Found the archived SAC file: ${arcfile}!"
		echo "Start to process the SAC file: ${arcfile}..."
		#
		echo "Moving the archived SAC file to local..."
		cp -R ${filepath}/${arcfile} ${TMP_DIR}
		echo "Decompressing the archived SAC files..."
		lbzip2 -dc ${TMP_DIR}/${arcfile} | tar -C ${TMP_DIR} -x
		rm -f ${TMP_DIR}/${arcfile}
		#
		echo "Exchanging the Z & E components of those stations on the list..."
		arcfolder=${TMP_DIR}/${arcfile:0:19}
		stations=${line_a[@]:1}
		for station in ${stations}
		do
			scnl_mod_sac -c HLE ${arcfolder}/${station}.HLZ.TW.-- ${arcfolder}/${station}.HLE.TW.--.temp
			scnl_mod_sac -c HLZ ${arcfolder}/${station}.HLE.TW.-- ${arcfolder}/${station}.HLZ.TW.--.temp
			#
			if [ -e "${arcfolder}/${station}.HLZ.TW.--.temp" ]; then
				mv ${arcfolder}/${station}.HLZ.TW.--.temp ${arcfolder}/${station}.HLZ.TW.--
			else
				rm -f ${arcfolder}/${station}.HLZ.TW.--
			fi
			if [ -e "${arcfolder}/${station}.HLE.TW.--.temp" ]; then
				mv ${arcfolder}/${station}.HLE.TW.--.temp ${arcfolder}/${station}.HLE.TW.--
			else
				rm -f ${arcfolder}/${station}.HLE.TW.--
			fi
		done
		#
		echo "Compressing all the archived SAC files..."
		cd ${TMP_DIR} 1> /dev/null; tar -cf ${arcfile} --use-compress-program=lbzip2 ${arcfile:0:19}; cd - 1> /dev/null
		mv ${TMP_DIR}/${arcfile} ${RES_DIR}
		#
		echo "Deleting the temporary folder..."
		rm -rf ${arcfolder}
		echo "Finish process the archived SAC file: ${arcfile}!"
	else
		echo "Can't find the archived SAC file: '${arcfile}'!"
	fi
done
echo "Finish process all the archived SAC files on the list!"
exit

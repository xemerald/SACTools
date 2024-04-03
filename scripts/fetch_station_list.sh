#!/bin/bash
#
#
if [ $# -ne 1 ]
then
	echo "Usage: fetch_station_list.sh <DATABASE_HOST> <USER_NAME> <DATABASE_NAME> <TABLE_NAME> <OUTPUT_FILE>"
	exit 0
fi
#
QUERY="SELECT station, location, network, latitude, longitude, elevation, 'HLZ', 0.05981400, 'HLN', 0.05981400, 'HLE', 0.05981400 FROM ${4};"
#
echo "Fetching the newest stations list from database..."
echo ${QUERY} | mysql -u ${2} -p ${3} -h ${1} | awk '{print $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12}' > ${5}
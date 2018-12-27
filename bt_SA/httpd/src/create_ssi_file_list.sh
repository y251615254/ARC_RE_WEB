#!/bin/bash

function Usage()
{
	echo "Please specify the www dir and .h file name!!"
	echo "Usage: $0 <www_dir> <.h file name>"
}

if [ "$1" == "" ];then
	Usage;
	exit 1;
fi

if [ ! -d "$1" ];then
	Usage;
	exit 1;
fi

if [ "$2" == "" ];then
	Usage;
	exit 1;
fi

CUR_PWD=`pwd`
ARRAY_NAME="ssi_files_list"
OUT_PUT_H_FILE_BASE_NAME=$2
TMP_FILE="$CUR_PWD/${OUT_PUT_H_FILE_BASE_NAME}.tmp"
OUT_PUT_H_FILE="$CUR_PWD/$OUT_PUT_H_FILE_BASE_NAME"

cd $1
# grep the files which contain "<% xxxxx %>"
grep -rnl "<%.*%>" * > $TMP_FILE

# Count how many files contain "<% xxxxx %>"
#FILES_LIST_NUM=`wc -l $TMP_FILE | awk '{print $1}'`


# Format the body of the .h file
# 1. convert abc.htm --> "abc.htm",
sed -i 's/\(.*\)/\t"\1"\,/g' $TMP_FILE 

# Construct the .h file
# 1. construct the header of the .h file.
echo "char * const ${ARRAY_NAME}[] =" > $OUT_PUT_H_FILE
echo "{" >> $OUT_PUT_H_FILE
# 2. construct the body of the .h file.
cat $TMP_FILE >> $OUT_PUT_H_FILE
# 3. construct the foot of the .h file.
echo -e "\tNULL" >> $OUT_PUT_H_FILE
echo "};" >> $OUT_PUT_H_FILE

rm $TMP_FILE

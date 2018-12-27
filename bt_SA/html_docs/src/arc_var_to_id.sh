#!/bin/sh
# This tool will find all the js files, search for the same parameter names (character string) 
# as in cfgID_list file and replace it with cfgID (numeric format)

jspath=$1
listpath=$2
files=(`find $jspath -iname "*.js"`)
parameters=(`sed 's|'":"'||g' $listpath |sed 's|'"(0"'|'" "'|g'| awk '{print $1}'`)
cfgID=(`sed 's|'":"'||g' $listpath |sed 's|'"(0"'|'" "'|g'| awk '{print $2}'`)


for ((index=0; index<${#parameters[@]}; index++)); do
for ((index2=0; index2<${#files[@]}; index2++)); do
#  echo "vars[$index] => ${values[$index]}"
  sed -i '/ABS_/s/'"\<${parameters[$index]}\>"'/'"${cfgID[$index]}"'/g'  ${files[$index2]}
done
done

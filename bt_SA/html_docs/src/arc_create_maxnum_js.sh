#!/bin/sh
# This tool will create a Javascript file maxnum_define.js that include all maxnum for absctarct layer parameter with array defined in package/arcadyan-utility/lib-arc-cfg/src/proj-zz/arc_def_maxnum.h

header_file=$1
js_file=$2

`grep ARC_DEF_ $header_file |sed 's|'"\r"'|'" "'|g' | awk '{print "var " $2 "=" $3";" }' > $js_file`

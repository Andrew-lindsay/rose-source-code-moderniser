#!/usr/bin/env bash

build_path="/home/sheep/Minf_project/rose-source-code-moderniser/build"
tool="modernise-range-for"
DIR_SPEC="/home/sheep/Minf_project/rose-source-code-moderniser/benchmarks/SPEC_Benchmarks_srcs"
FLAGS="-edg:I${DIR_SPEC}/523.xalancbmk_r/src -edg:I${DIR_SPEC}/510.parest_r/src/include"
transform=$build_path/$tool
echo $transform $FLAGS

dir_name=$(basename $PWD)
out_file=${dir_name}.out
res_file=${dir_name}.res
echo "Removing $out_file"
echo "Removing $res_file"
rm "$out_file" > /dev/null
rm "$res_file" > /dev/null

for f in $@; do
	echo "$transform $f >> $out_file"
	$transform $FLAGS $f > temp.out
	cat temp.out >> $out_file  
	totals=$(cat temp.out | grep "Total Number of For loops" | perl -p -e 's/\n/\t\t\t/')
	echo -e "$f:\t\t\t$totals" >> $res_file
done

rm temp.out > /dev/null

# Find top 19 files that have the most lines
# wc -l $(find -name "*.cpp") | sort -nr | head -n 20 | awk '{print $2}' | tr '\n' ' '

############################################################################
# This script will rebuild the icons.txt file from the files
# found in the icons directory and set the visible scales to the defaults
############################################################################
#
# Notes:
# Run it from the data directory
#
rm -f icons.new
for dir in classic square.big square.small ; do 
    find icons/$dir/ -type f -name '*.png' | while read file ; do 
	# take away icons
	file=${file#*/}

	# take waway $dir
	file=${file#*/}

	# take waway .png
	file=${file%.png}

	# take waway .png
	file=${file//\//.}

	echo ${file}
    done
done | sort -u | \
while read file ; do
    #echo
    #echo "Testing $file"
    file_search="^${file/\+/\\+} "
    if grep -q -E -e "$file_search" icons.txt ; then 
	#echo "------- take old for '$file'"
	#grep -E -e "$file_search" icons.txt
	#echo "------- "
	grep -E -e "$file_search" icons.txt >>icons.new
    else 
	echo "------- create new entry for '$file'"
	printf "%-40s 1\t10000\n" "${file}" >> icons.new
    fi
done
diff -u -b icons.txt icons.new 
echo "New icons list can be found in icons.new"

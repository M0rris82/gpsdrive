############################################################################
# This script will rebuild the icons.txt file from the files
# found in the icons directory and set the visable scales to the defaults
############################################################################
#
# Notes:
# Run it from the data directory
#
cd icons
find . -name *.png |\
sed 's/.\///' |\
sed 's/.png//' |\
sed 's/\//./' |\
sed 's/$/\t1\t10000/' \
> ../icons.txt

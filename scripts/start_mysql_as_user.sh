#!/bin/bash

home=
mkdir ~/.gpsdrive
mkdir ~/.gpsdrive/mysql
mkdir ~/.gpsdrive/mysql/etc
mkdir ~/.gpsdrive/mysql/data
mkdir ~/.gpsdrive/mysql/data/mysql

cp ../data/mysql/my.cnf ~/.gpsdrive/mysql/etc


echo "setup mysql"
basedir=/home/tweety/.gpsdrive/mysql
ldata=/home/tweety/.gpsdrive/mysql/data
create_option=real
mdata=$ldata/mysql
mysql_create_system_tables $create_option $mdata `hostname` \
  | eval mysqld $defaults $mysqld_opt --bootstrap \
    --skip-grant-tables --basedir=$basedir --datadir=$ldata --skip-innodb \
    --skip-bdb --skip-ndbcluster --max_allowed_packet=8M --net_buffer_length=16K

ls -l  $ldata 
#mysql_create_system_tables   --defaults-file=/home/tweety/.gpsdrive/mysql/etc/my.cnf

echo "reset local mysql password"
mysqladmin --defaults-file=/home/tweety/.gpsdrive/mysql/etc/my.cnf -u root password "toor" 

echo "Starting mysqld"
echo ""
mysqld   --defaults-file=/home/tweety/.gpsdrive/mysql/etc/my.cnf

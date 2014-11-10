#!/bin/sh

echo "install mongodb c driver start......"
yum install  gcc automake autoconf libtool
git clone https://github.com/mongodb/mongo-c-driver.git
cd mongo-c-driver
./autogen.sh
make
make install
cd ..
echo "PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/:\$PKG_CONFIG_PATH" >> /etc/profile
echo "LD_LIBRARY_PATH=/usr/local/lib:\$LD_LIBRARY_PATH" >> /etc/profile
source /etc/proflie
echo "install mongodb c driver end......"

echo "delete old postfix"
rpm -e postfix --nodeps
echo "reinstall postfix"
wget ftp://ftp.porcupine.org/mirrors/project-history/postfix/official/postfix-2.6.6.tar.gz
tar xvzf postfix-2.6.6.tar.gz
rm -rf  postfix-2.6.6/src/global/log_adhoc.c && cp log_adhoc.c postfix-2.6.6/src/global/
make makefiles CCARGS="$(pkg-config --cflags  libmongoc-1.0)" AUXLIBS="$(pkg-config  --libs libmongoc-1.0)"

yum -y install db4
yum -y install db4-devel

make
make install


echo "install postfix service"
chkconfig --del  postfix
cp postfix /etc/init.d
chmod +x /etc/init.d/postfix
chkconfig --add postfix
chkconfig --level 2345 postfix on



echo "config /etc/postfix/main.cf"

echo -n "Enter your email domain:"
read mydomain
postconf -e "mydomain=$mydomain"
postconf -e "virtual_mailbox_domains=$mydomain"
echo -n "Enter  virtual mailbox base(/var/vmail):"
read virtual_mailbox_base

if test -z "$virtual_mailbox_base"; then
	virtual_mailbox_base="/var/vmail"
fi

postconf -e "virtual_mailbox_base=$virtual_mailbox_base"
echo "no-reply@$mydomain   $mydomain/no-reply" >> /etc/postfix/virtual
postmap hash:/etc/postfix/virtual
postconf -e "virtual_mailbox_maps = hash:/etc/postfix/virtual"

echo -n "Enter virtual mail  owner(vmail):"
read vmail_owner

if test -z "$vmail_owner"; then
	vmail_owner="vmail"
fi

groupadd $vmail_owner
useradd $vmail_owner -g $vmail_owner -b/var -s/sbin/nologin

vmail_owner_uid=`id -u $vmail_owner`
vmail_owner_gid=`id -g $vmail_owner`

postconf -e "virtual_uid_maps = static:$vmail_owner_uid"
postconf -e "virtual_gid_maps = static:$vmail_owner_uid"

echo "api_record_enabled = 1" >> /etc/postfix/main.cf

echo -n "Enter mongodb host:"
read mongodb_host
echo -n "Enter mongodb port(27017):"
read mongodb_port

if [ -n "$mongodb_host" ]; then
	echo "mongodb_host = $mongodb_host" >> /etc/postfix/main.cf
fi

if [ -n "$mongodb_port" ]; then
	echo "mongodb_port = $mongodb_port" >> /etc/postfix/main.cf
fi

service postfix reload

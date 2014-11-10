#!/bin/bash
build() {
	#check boost 
	is_install=`rpm -qa boost`
	if test -z "$is_install"; then
		yum -y install boost-devel
	fi

	#if you want install boost source
	#rpm -e boost
	#wget http://downloads.sourceforge.net/project/boost/boost/1.57.0/boost_1_57_0.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fboost%2Ffiles%2Fboost%2F1.57.0%2F&ts=1415263508&use_mirror=jaist
	#tar xvzf boost_1_57_0.tar.gz
	#cd boost_1_57_0
	#./bootstrap.sh  --prefix=/usr/local/boost
	#./b2 intall


	#check mysql-devel
	is_install=`rpm -qa mysql-devel`
	if test -z "$is_install"; then
		yum -y install mysql-devel
	fi
	
	cd base
	make
	cd ../server
	make
	cd ../client
	make
}

clean() {
	cd base
	make clean
	cd ../server
	make clean
	cd ../client
	make clean
}

print_help() {
	echo "Usage: "
	echo "  $0 clean --- clean all build"
	echo "  $0 version version_str --- build a version"
}

exec=$1
exec=${exec:="build"}

case $exec in
	clean)
		echo "clean all build..."
		clean
		;;
	build)
		build
		;;
esac

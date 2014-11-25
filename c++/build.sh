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

	#check libjson.a
	#see http://wiki.hourui.de/c_cpp/json_cpp
	if [ ! -f "./base/json/libjson.a" ]; then
		pwd=$(pwd)
		cd ../jsoncpp
		tar xvzf jsoncpp-src-0.5.0.tar.gz
		tar xvzf scons-2.2.0.tar.gz
		export SCONS=$(pwd)/scons-2.2.0
		export SCONS_LIB_DIR=$SCONS/engine
		cd jsoncpp-src-0.5.0
		python $SCONS/script/scons platform=linux-gcc

		cd libs
		gcc_ver=$(ls)
		cd $gcc_ver
		static_lib_name=$(find ./ -name "lib*.a")

		cp $static_lib_name $pwd/base/json/libjson.a
		cd $pwd
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

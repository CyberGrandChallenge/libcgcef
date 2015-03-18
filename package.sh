#!/bin/bash

if [ x"$1" == x"" ] ; then
	echo "Usage: $0 version"
	echo "i.e. $0 1.0"
	exit 0
fi
ver="$1"

pwd=`pwd`
pkgname=`basename $pwd`
pkgvername="${pkgname}-${ver}"
echo "Packaging ${pkgvername}"

if [ -d "../${pkgvername}" ] ; then
	echo "Removing old ../${pkgvername}"
	rm -rf "../${pkgvername}" || exit 1
fi
mkdir "../${pkgvername}"
cp -R . "../${pkgvername}/"
rm -rf "../${pkgvername}/.svn/"
(cd .. && tar -czf "${pkgname}_${ver}.orig.tar.gz" "${pkgvername}")
cd "../${pkgvername}"
debuild -us -uc
if [ $? != 0 ] ; then
	exit 1
fi
cd "${pwd}"
rm -rf "../${pkgvername}"

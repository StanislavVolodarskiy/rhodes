#!/usr/bin/env bash
set -x

cp ./rhobuild.yml.example ./rhobuild.yml

set -e

echo Downloading Qt
wget -q https://s3.amazonaws.com/files.tau-technologies.com/buildenv/Qt5.9.5.tar.gz -O $HOME/Qt5.9.5.tar.gz
tar -xzf $HOME/Qt5.9.5.tar.gz -C $HOME/
echo Qt installed

echo "Installing Rhoconnect client"	
git clone -b master https://github.com/rhomobile/rhoconnect-client.git ../rhoconnect-client

git clone -b OpenSSL_1_1_0-stable https://github.com/tauplatform/openssl.git ../openssl
cd ../openssl
./tau_build_macos_lib.sh

echo "Building rhosim"
cd $TRAVIS_BUILD_DIR
#rm $TRAVIS_BUILD_DIR/platform/osx/bin/RhoSimulator/RhoSimulator.app.zip
rm -rf $TRAVIS_BUILD_DIR/platform/osx/bin/RhoSimulator/*
rake build:osx:rhosimulator
cd $TRAVIS_BUILD_DIR/platform/osx/bin/RhoSimulator/
zip -r -y RhoSimulator.app.zip RhoSimulator.app
rm -rf $TRAVIS_BUILD_DIR/platform/osx/bin/RhoSimulator/RhoSimulator.app
cd $TRAVIS_BUILD_DIR
# > build.log

OUT=$?

if [ $OUT -eq 0 ];then
   echo "RhoSimulator built successfully"
else
   echo "Error building RhoSimulator"
   cat build.log
fi

exit $OUT
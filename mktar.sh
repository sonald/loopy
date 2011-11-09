#!/bin/sh
#set -x

LOOPY_REPO=`pwd`
VERSION=0.5.5

CLONE_DIR=loopy-$VERSION
CLONE_REPO=${LOOPY_REPO}/$CLONE_DIR
TAR_FILE=loopy-${VERSION}.tar.bz2 

rm -rf $CLONE_REPO

git clone $LOOPY_REPO $CLONE_REPO
tar jcf $TAR_FILE $CLONE_DIR

if [ -d $HOME/rpmbuild/SPECS ]; then
    echo "copying spec file..."
    cp $CLONE_REPO/loopy.spec $HOME/rpmbuild/SPECS/
fi

if [ -d $HOME/rpmbuild/SOURCES ]; then
    echo "copying source file..."
    cp $TAR_FILE $HOME/rpmbuild/SOURCES/
fi


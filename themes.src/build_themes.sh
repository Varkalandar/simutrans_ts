#!/bin/bash

#
# This file is part of the Simutrans project under the Artistic License.
# (see LICENSE.txt)
#

# assumes makeobj is in the trunk directory

cd aero
rm -rf *.pak
../makeobj pak aerotheme.pak skins_aero.dat
mv *.pak ../../simutrans/themes
cp *.tab ../../simutrans/themes

cd ../flat
rm -rf *.pak
../makeobj pak flat.pak flat-skin.dat
mv *.pak ../../simutrans/themes
cp *.tab ../../simutrans/themes

cd ../standard
rm -rf *.pak
../makeobj pak classic.pak standard.dat
mv *.pak ../../simutrans/themes
cp *.tab ../../simutrans/themes

cd ../highcontrast
rm -rf *.pak
../makeobj pak highcontrast.pak theme.dat
mv *.pak ../../simutrans/themes
cp *.tab ../../simutrans/themes

cd ../highcontrast-large
rm -rf *.pak
../makeobj pak highcontrast-large.pak theme.dat
mv *.pak ../../simutrans/themes
cp *.tab ../../simutrans/themes

cd ../modern
rm -rf *.pak
../makeobj pak modern.pak standard.dat
../makeobj pak modern-large.pak standard-large.dat
mv *.pak ../../simutrans/themes
cp *.tab ../../simutrans/themes

cd ../silver
rm -rf *.pak
../makeobj pak silver.pak standard.dat
../makeobj pak silver-large.pak standard-large.dat
mv *.pak ../../simutrans/themes
cp *.tab ../../simutrans/themes

cd ../purple_night
rm -rf *.pak
../makeobj pak purple_night.pak standard.dat
../makeobj pak purple_night-large.pak standard-large.dat
mv *.pak ../../simutrans/themes
cp *.tab ../../simutrans/themes

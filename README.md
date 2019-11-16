# RasPedal

1) To install the Raspbian OS, follow the instruction in: https://www.raspberrypi.org/documentation/installation/installing-images/README.md

2) To install all other dependencies, run the following commands:

sudo apt install libsox-dev libeigen3-dev libfftw3-dev libasound2-dev libjack-jackd2-dev sox libgtk2.0-dev

sudo apt-get install git g++ autoconf libtool

git clone https://github.com/flatmax/gtkiostream.git

cd gtkiostream

./tools/autotools.sh 

./configure --disable-octave

make -j2 (This command can take about one hour to run)

sudo make install

3) Reboot the unit.

4) Download and install the .deb.tar.gaz file in: http://forum.audioinjector.net/viewtopic.php?f=5&t=3

5) Run:

audioInjector-setup.sh

6) Enjoy! :)


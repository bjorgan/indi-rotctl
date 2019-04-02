INDI drivers for controlling az,el rotors through rotctld.  Pretends to be a
telescope.

Using calculated Az, El, tries to keep rotctld rotor pointing towards the RA
and DEC provided by the client program, given the lat, lon coordinates given by
the client program and the current time.

Installation
------------

Need to have access to the CMake modules of INDI for less hassle, and need to
install using prefix /usr and not /usr/local since INDI seems to be unable to
find the driver if it gets installed in /usr/local.

```
mkdir build
cd build
git clone https://github.com/indilib/indi
cmake -D INDI_SOURCE_DIRECTORY=$PWD/indi -D CMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```

Usage instructions for KStars
-----------------------------

Select "Tools" -> "Devices" -> "Device manager". Locate "Rotors", select
"Rotctl interface" and press "Run service".

In "Connection" tab, enter rotctld hostname and port in "Address" and "Port"
fields (closest to the "Set" button), and press "Set". Press "Connect" in "Main
Control" tab. (Press "Save" under "Configuration" in Options tab to save
connection information so that it is not needed to re-enter it the next time.)

Right click in KStars map, choose "Rotctl interface" and press "Track" in order
to start tracking.

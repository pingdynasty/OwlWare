BIN=Build/OwlWare.bin
FS=./Tools/FirmwareSender
FS=../FirmwareSender/Builds/Linux/build/FirmwareSender
$FS -flash `crc32 $BIN` -in $BIN -out OWL-MIDI

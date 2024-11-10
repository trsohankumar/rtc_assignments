idf.py -p /dev/tty.usbserial-0001 erase-flash
idf.py build
idf.py -p /dev/tty.usbserial-0001 flash
idf.py -p /dev/tty.usbserial-0001 monitor > ./output/output.txt
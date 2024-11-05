# Chatterbox App

Project Structure:
```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
├── README.md
├── sdkconfig
├── sdkconfig.ci
└── sdkconfig.old
```

It is your task to fill in the missing TODOs in *main/main.c*.
You should not have to touch any other files.

You can build the project as usual:
```
$ get_idf
$ idf.py build
```

Assuming your ESP32 is visible under /dev/ttyUSB0, you can flash and monitor the chatterbox app with
```
$ idf.py -p /dev/ttyUSB0 flash monitor
```

Finally, to stop the LEDs from blinking, run
```
$ idf.py -p /dev/ttyUSB0 erase-flash
```
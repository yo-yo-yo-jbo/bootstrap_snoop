# Bootstrap snoop

Bootstrap snoop is a tiny project that can show the Mach Messages of newly created processes `fork` + `execve` sent to the `bootstrap port`.  
It might be useful for macOS research purposes.  
For more information about the motivation, check out [this blogpost](https://github.com/yo-yo-yo-jbo/macos_mach_ports/).

## Compilation
Simply run `make` and have fun.

## Usage
```shell
./bootstrap_snoop <binary> [args...]
```

## Example
Here is the output for running `sudo` with no arguments:

```shell
jbo@McJbo ~ % ./bootstrap_snoop /usr/bin/sudo
Checking command-line arguments ................................................................................................................ [  OK  ]
Setting up a fresh new port .................................................................................................................... [  OK  ]
Switching bootstrap port ....................................................................................................................... [  OK  ]
Forking ........................................................................................................................................ [  OK  ]
Switching bootstrap port back on parent process ................................................................................................ [  OK  ]
Waiting for messages ........................................................................................................................... [  OK  ]

00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
===============================================
01 00 00 00 03 1d 00 00 00 00 00 00 00 00 11 00
43 50 58 40 05 00 00 00 00 f0 00 00 c0 00 00 00
07 00 00 00 68 61 6e 64 6c 65 00 00 00 40 00 00
00 00 00 00 00 00 00 00 69 6e 73 74 61 6e 63 65
00 00 00 00 00 a0 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 66 6c 61 67 73 00 00 00
00 40 00 00 08 00 00 00 00 00 00 00 6e 61 6d 65
00 00 00 00 00 90 00 00 25 00 00 00 63 6f 6d 2e
61 70 70 6c 65 2e 73 79 73 74 65 6d 2e 6e 6f 74
69 66 69 63 61 74 69 6f 6e 5f 63 65 6e 74 65 72
00 00 00 00 74 79 70 65 00 00 00 00 00 40 00 00
07 00 00 00 00 00 00 00 74 61 72 67 65 74 70 69
64 00 00 00 00 30 00 00 00 00 00 00 00 00 00 00
64 6f 6d 61 69 6e 2d 70 6f 72 74 00 00 d0 00 00
00 00 00 00 08 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
sudo: you do not exist in the passwd database

jbo@McJbo ~ % 
```

Note the data sent to the `bootstrap port` is this case is `XPC` data.

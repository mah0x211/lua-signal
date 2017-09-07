# lua-signal

signal module.

## Installation

```sh
luarocks install --from=http://mah0x211.github.io/rocks/ signal
```


## functions

### block( signo ) / unblock( signo )
block/unblock the specified signal.

**Parameters**

- signo: valid signal number.

**Returns**

1. boolean: true on success, or false and errstr on failure.
2. errstr: dependent on a system.


### blockAll() / unblockAll()
block/unblock the signals that specified by sigfillset on internally.

**Returns**

1. boolean: true on success, or false and errstr on failure.
2. errstr: dependent on a system.


### isblock( signo )
return true if the specified signal is blocked.

**Parameters**

- signo: valid signal number.

**Returns**

1. boolean: true on block, or false on not block or failure.
2. errstr: dependent on a system.


### raise( signo )
send the specified signal to the current process.

**Parameters**

- signo: valid signal number.

**Returns**

1. boolean: true on success, or false and errstr on failure.
2. errstr: dependent on a system.


### kill( signo [, pid] )

send the specified signal to the specified process.

**Parameters**

- signo: valid signal number.
- pid: process id. (`default: id of the calling process`)

**Returns**

1. boolean: true on success, or false and errstr on failure.
2. errstr: dependent on a system.


### killpg( signo [, pgrp] )

send signal to the specified process group

**Parameters**

- signo: valid signal number.
- pgrp: process group id (`default: id of the calling process`)

**Returns**

1. boolean: true on success, or false and errstr on failure.
2. errstr: dependent on a system.



## constants

please check a `signo.txt`.
the signal numbers that listed in the file will be added automatically.

# lua-signal

signal module.

## Installation

```sh
luarocks install --from=http://mah0x211.github.io/rocks/ signal
```


## Functions

### ok, err = block( ... ) 

block the specified signal(s).

**Parameters**

- `...`: valid signal numbers.

**Returns**

1. `ok`: true on success.
2. `err`: error message.


### ok, err = unblock( ... )

unblock the specified signal.

**Parameters**

- `...`: valid signal numbers.

**Returns**

1. `ok`: true on success.
2. `err`: error message.


### ok, err = blockAll()

block the signals that specified by sigfillset on internally.

**Returns**

1. `ok`: true on success.
2. `err`: error message.


### ok, err = unblockAll()

unblock the signals that specified by sigfillset on internally.

**Returns**

1. `ok`: true on success.
2. `err`: error message.


### blocked, err, ... = isblock( ... )

return true if the specified signal(s) is blocked.

**Parameters**

- `...`: valid signal numbers.

**Returns**

1. `blocked`: true on blocked.
2. `err`: error message.
3. `...`: unblocked signals.


### ok, err = raise( signo )

send the specified signal to the current process.

**Parameters**

- `signo`: valid signal number.

**Returns**

1. `ok`: true on success.
2. `err`: error message.


### ok, err = kill( signo [, pid] )

send the specified signal to the specified process.

**Parameters**

- `signo`: valid signal number.
- `pid`: process id. (`default: id of the calling process`)

**Returns**

1. `ok`: true on success.
2. `err`: error message.


### ok, err = killpg( signo [, pgrp] )

send signal to the specified process group

**Parameters**

- `signo`: valid signal number.
- `pgrp`: process group id (`default: id of the calling process`)

**Returns**

1. `ok`: true on success.
2. `err`: error message.


### sec = alarm( sec )

deliver the signal `SIGALRM` after the specified number of seconds.

**Parameters**

- `sec`: number of seconds.

**Returns**

1. `sec`: remaining time of timer from the previous function call.


### ok, err = ignore( signo )

ignore the specified signal.

**Parameters**

- `signo`: valid signal number.

**Returns**

1. `ok`: true on success.
2. `err`: error message.


### ok, err = default( signo )

set the default action to the specified signal.

**Parameters**

- `signo`: valid signal number.

**Returns**

1. `ok`: true on success.
2. `err`: error message.


## constants

please check a `signo.txt`.
the signal numbers that listed in the file will be added automatically.

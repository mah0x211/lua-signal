# lua-signal

signal module.


## Installation

```sh
luarocks install signal
```


## Usage

```lua
local signal = require('signal')
signal.raise(signal.SIGTERM)
```


## Error Handling

the functions are return the error object created by https://github.com/mah0x211/lua-errno module.


## Constants

please check a `signo.txt`.
the signal numbers that listed in the file will be added automatically.


## ok, err = block( ... ) 

block the specified signal(s).

**Parameters**

- `...:integer`: valid signal numbers.

**Returns**

- `ok:boolean`: `true` on success.
- `err:error`: error object.


## ok, err = unblock( ... )

unblock the specified signal.

**Parameters**

- `...:integer`: valid signal numbers.

**Returns**

- `ok:boolean`: `true` on success.
- `err:error`: error object.


## ok, err = blockall()

block the signals that specified by sigfillset on internally.

**Returns**

- `ok:boolean`: `true` on success.
- `err:error`: error object.


## ok, err = unblockall()

unblock the signals that specified by sigfillset on internally.

**Returns**

- `ok:boolean`: `true` on success.
- `err:error`: error object.


## ok, err, ... = isblock( ... )

return `true` if the specified signal(s) is blocked.

**Parameters**

- `...:integer`: valid signal numbers.

**Returns**

- `ok:boolean`: `true` on success.
- `err:error`: error object.
- `...:integer`: unblocked signals.


## ok, err = raise( signo )

send the specified signal to the current process.

**Parameters**

- `signo:integer`: valid signal number.

**Returns**

- `ok:boolean`: `true` on success.
- `err:error`: error object.


## ok, err = kill( signo [, pid] )

send the specified signal to the specified process.

**Parameters**

- `signo:integer`: valid signal number.
- `pid:integer`: process id. (`default: id of the calling process`)

**Returns**

- `ok:boolean`: `true` on success.
- `err:error`: error object.


## ok, err = killpg( signo [, pgrp] )

send signal to the specified process group

**Parameters**

- `signo:integer`: valid signal number.
- `pgrp:integer`: process group id (`default: id of the calling process`)

**Returns**

- `ok:boolean`: `true` on success.
- `err:error`: error object.


## sec = alarm( sec )

deliver the signal `SIGALRM` after the specified number of seconds.

**Parameters**

- `sec:integer`: number of seconds.

**Returns**

- `sec:integer`: remaining time of timer from the previous function call.


## ok, err = ignore( signo )

ignore the specified signal.

**Parameters**

- `signo:integer`: valid signal number.

**Returns**

- `ok:boolean`: `true` on success.
- `err:error`: error object.


## ok, err = default( signo )

set the default action to the specified signal.

**Parameters**

- `signo:integer`: valid signal number.

**Returns**

- `ok:boolean`: `true` on success.
- `err:error`: error object.


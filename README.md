# lua-signal

[![test](https://github.com/mah0x211/lua-signal/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-signal/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/mah0x211/lua-signal/branch/master/graph/badge.svg)](https://codecov.io/gh/mah0x211/lua-signal)

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

## NOTE

On some platforms, `SIGCHLD` cannot be captured by the sigwait function. To solve this problem, set the NOOP (no operation) handler if the `SIGCHLD` signal handler is set to `SIG_DFL` or `SIG_IGN` when the module is loaded.


## Error Handling

the functions are return the error object created by https://github.com/mah0x211/lua-errno module.


## Constants

please check a `signo.txt`.
the signal numbers that listed in the file will be added automatically.


## ok, err = block( ... ) 

block the specified signal(s).

**Parameters**

- `...:string|integer`: valid signal names or numbers.

**Returns**

- `ok:boolean`: `true` on success.
- `err:any`: error object.


## ok, err = unblock( ... )

unblock the specified signal.

**Parameters**

- `...:string|integer`: valid signal names or numbers.

**Returns**

- `ok:boolean`: `true` on success.
- `err:any`: error object.


## ok, err = blockall()

block the signals that specified by sigfillset on internally.

**Returns**

- `ok:boolean`: `true` on success.
- `err:any`: error object.


## ok, err = unblockall()

unblock the signals that specified by sigfillset on internally.

**Returns**

- `ok:boolean`: `true` on success.
- `err:any`: error object.


## ok, err, ... = isblock( ... )

return `true` if the specified signal(s) is blocked.

**Parameters**

- `...:string|integer`: valid signal names or numbers.

**Returns**

- `ok:boolean`: `true` on success.
- `err:any`: error object.
- `...:integer`: unblocked signals.


## ok, err = raise( sig )

send the specified signal to the current process.

**Parameters**

- `sig:string|integer`: valid signal name or number.

**Returns**

- `ok:boolean`: `true` on success.
- `err:any`: error object.


## ok, err = kill( sig [, pid] )

send the specified signal to the specified process.

**Parameters**

- `sig:string|integer`: valid signal name or number.
- `pid:integer`: process id. (`default: id of the calling process`)

**Returns**

- `ok:boolean`: `true` on success.
- `err:any`: error object.


## ok, err = killpg( sig [, pgrp] )

send signal to the specified process group

**Parameters**

- `sig:string|integer`: valid signal name or number.
- `pgrp:integer`: process group id (`default: id of the calling process`)

**Returns**

- `ok:boolean`: `true` on success.
- `err:any`: error object.


## sec = alarm( sec )

deliver the signal `SIGALRM` after the specified number of seconds.

**Parameters**

- `sec:integer`: number of seconds.

**Returns**

- `sec:integer`: remaining time of timer from the previous function call.


## ok, err = ignore( sig )

ignore the specified signal.

**Parameters**

- `sig:string|integer`: valid signal name or number.

**Returns**

- `ok:boolean`: `true` on success.
- `err:any`: error object.


## ok, err = default( sig )

set the default action to the specified signal.

**Parameters**

- `sig:string|integer`: valid signal name or number.

**Returns**

- `ok:boolean`: `true` on success.
- `err:any`: error object.


## ok, err, timeout = wait( sec, ... )

waits for interrupt by the specified signals or pending signals to be delivered.

**NOTE**

this function returns immediately and with no return value if no signals are specified and no signals are pending.

**Parameters**

- `sec:number`: seconds. (default `0` means wait forever)
- `...:string|integer`: signal names or numbers. if not specified, wait for the pending signals.

**Returns**

- `signo:integer`: received signal number or `nil`.
- `err:any`: error object.
- `timeout:boolean`: `true` on timed out.
- `signame:string`: received signal name or `nil`.


## ... = tosigname( ... )

convert the signal numbers to the signal names.

**Parameters**

- `...:string|integer`: signal names or numbers.

**Returns**

- `...:string`: signal names.


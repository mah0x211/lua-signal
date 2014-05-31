# lua-signal

signal module.

## Installation

```sh
luarocks install --from=http://mah0x211.github.io/rocks/ signal
```

or 

```sh
git clone https://github.com/mah0x211/lua-signal.git
cd lua-signal
luarocks make
```


## functions

### block( signo ) / unblock( signo )
block/unblock the specified signal.

**Parameters**

- signo: valid signal number.

**Returns**

1. boolean: true on success, or false and errno on failure.
2. errno: EINVAL or EFAULT.


### blockAll() / unblockAll()
block/unblock the signals that specified by sigfillset on internally.

**Returns**

1. boolean: true on success, or false and errno on failure.
2. errno: EINVAL | EFAULT.


### isblock( signo )
return true if the specified signal is blocked.

**Parameters**

- signo: valid signal number.

**Returns**

1. boolean: true on block, or false on not block or failure.
2. errno: EINVAL.


### raise( signo )
send the specified signal to the current process.

**Parameters**

- signo: valid signal number.

**Returns**

1. boolean: true on success, or false and errno on failure.
2. errno: EINVAL, or dependent on a system.


### kill( pid, signo )
send the specified signal to the specified process.

**Parameters**

- pid: process id.
- signo: valid signal number.

**Returns**

1. boolean: true on success, or false and errno on failure.
2. errno: EINVAL, EPERM or ESRCH.


## constants
you can use these constants for signo parameter.

1. signal.HUP
2. signal.INT
3. signal.QUIT
4. signal.ILL
5. signal.TRAP
6. signal.ABRT
7. signal.EMT
8. signal.FPE
9. signal.KILL
10. signal.BUS
11. signal.SEGV
12. signal.SYS
13. signal.PIPE
14. signal.ALRM
15. signal.TERM
16. signal.URG
17. signal.STOP
18. signal.TSTP
19. signal.CONT
20. signal.CHLD
21. signal.TTIN
22. signal.TTOU
23. signal.IO
24. signal.XCPU
25. signal.XFSZ
26. signal.VTALRM
27. signal.PROF
28. signal.WINCH
29. signal.INFO
30. signal.USR1
31. signal.USR2




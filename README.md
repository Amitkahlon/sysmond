# sysmond

## Build
```
make
```

## Run
```
./bin/sysmond [wait_time]
```
`wait_time` is the refresh interval in seconds (default: 5).
Example: `./bin/sysmond 2`

Or build and run in one step:
```
make run            # default interval
make run WAIT=2     # refresh every 2 seconds
```

Press `q` (or Ctrl+C) to quit.


## Configuration
Defaults live in `src/config.h`:
- `DEFAULT_WAIT_TIME`: the refresh interval when none is given
- `ALERT_THRESHOLD`: the load average that triggers the alert

## Clean
```
make clean
```

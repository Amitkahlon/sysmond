# sysmond

## Build
```
make
```

## Run
```
./bin/sysmond [sample_interval_sec]
```
`sample_interval_sec` is the sampling interval in seconds (default: 5).
Example: `./bin/sysmond 2`

Or build and run in one step:
```
make run               # default sampling interval
make run INTERVAL=2    # sample every 2 seconds
```

Press `q` (or Ctrl+C) to quit.


## Configuration
Defaults live in `src/config.h`:
- `DEFAULT_SAMPLE_INTERVAL_SEC`: the sampling interval when none is given
- `ALERT_THRESHOLD`: the load average that triggers the alert

## Clean
```
make clean
```

## Preparation
1. Place [pendant-continuous-*.g](macros/) files in `/sys/`
2. Add `M98 P"pendant-continuous-init.g"` to `/sys/config.g`
3. Run `M98 P"pendant-continuous-loop.g"` from a loop in `/sys/daemon.g`, for example like:
   1. `/sys/daemon.g`
      ```
      while global.run_daemon
          G4 P100
          M98 P"pendant-continuous-loop.g"
      ```
   2. add `global run_daemon=true` to `/sys/config.g`
   3. if needed (eg. to edit daemon.g), stop daemon loop by `set global.run_daemon=false`

## Usage
### Numpad
* Switch to continuous mode via the ./DEL key (back to step mode via 0/INS)
* The top key row now is used to set the relative feed rate instead of the step size (off/25%/50%/100%)
* As long as arrow-keys (X/Y) or +/- keys are pressed, the associated axis should move.
* Only one axis can be moved at a time

### WHB04B-6 Wireless CNC Pendant
* Switch between step and continuous mode with the corresponding buttons
* In continuous mode, the selected axis is moved with the selected relative feed rate as long es the wheel is rotated

### PS3 Controller
* Continuous movements are controlled via the left stick (for X/Y) and right stick (for Z).
* Relative feed rate is set like the step size (off/25%/50%/75%/100%)
* Only one axis can be moved at a time

## Working principle
* Movement commands are done on the Duet itself, inside a loop in the daemon.g
* Control of the movements happens via global variables (GCode meta commands), variables are set by macros
* A continuous movement happens in small steps. After each step a counter is decreased. If counter is zero, no movement happens.
* In intervals, if a continuous movement is commanded, the counter is reset to a higher value by a called macro.
* If continuous movement is commanded to be stopped, the counter is set to zero by a called macro.
* The counter is meant to work as a watchdog/keepalive, so that movement automatically stops if no command is received anymore


# AWQAT - Prayer times CLI tool

A tool suitable for terminal users to look at prayer times without moving away from their terminal. Uses Aladhan API for the prayer times.

## Installation

Thanks to [nob.h](https://github.com/tsoding/nob.h/), you only need C compiler to install nob, and subsequently the application.

For the first time, you need to bootstrap the `nob` build tool by compiling it.

```bash
cc -o nob nob.c
```

Then launch `nob`.
```bash
./nob
```
It will do the build for you and recompile nob if needed.

## Usage

Get prayer times based on your location.
```bash
./awqat
```

Check usage help for more options.
```bash
./awqat -h
```

Run GUI with
```bash
./awqat_gui
```

# sipcmd — the command line SIP/H.323/RTP softphone

## Introduction

Command line soft phone that makes phone calls, accepts calls, enters DTMF digits, plays back WAV files and records them. A useful testing tool for VoIP systems. Runs on Linux.

## News 

Upgraded to latest versions of ptlib and opal avaliable on apt repos on Ubuntu 12.04. (3.10.2 and 2.10.2, respectively).

## Install

### Dependencies

##### Ubuntu

```sh
apt-get install opal-dev ptlib-dev 
```

##### Ubuntu 18.04 Bionic

```sh
apt-get install libopal-dev sip-dev libpt-dev
```

##### Debian

```sh
apt-get install libopal-dev libpt-dev 
```

### Download

Get source tarball from GitHub.

### Compile

```sh
make 
```

To disable debug messages, comment out DEBUG flag from Makefile

### Environment

If you compile the dependencies from source, make sure that libpt and libopal are in your `LD_LIBRARY_PATH`. The default installation location is `/usr/local/lib`.

## Usage

**testphone options:**

```
-u  --user          username (required)
-c  --password      password for registration
-a  --alias         username alias
-l  --localaddress  local address to listen on
-o  --opallog       enable extra opal library logging to file
-p  --listenport    the port to listen on
-P  --protocol      sip/h323/rtp (required)
-r  --remoteparty   the party to call to
-x  --execute       program to follow
-d  --audio-prefix  recorded audio filename prefix
-f  --file          the name of played sound file
-g  --gatekeeper    gatekeeper to use
-w  --gateway       gateway to use
-m  -mediaformat    one or more codecs to use, separated by semicolon; wildcards are supported (e.g. -m "G.711*;G.722*")
```

`-l` or `-p` without `-x` assumes answer mode. Additional `-r` forces caller id checking. `-r` without `-l`, `-p` or `-x`assumes call mode. 
To register to a gateaway, specify `-c`, `-g` and `-w` 
**Example:**

```
./sipcmd -P sip -u [username] -c [password] -w [server] -x "c;w200;d12345"
```

**WAV file requirements:**

- mono
- 8 kHz sampling rate
- 16 bits sample size

**The EBNF definition of the program syntax:**

```
prog	:=  cmd ';'  |
cmd	:=  call | answer | hangup
	  | dtmf | voice | record | wait
	  | setlabel | loop
call	:=  'c' remoteparty [ timeout ]
answer	:=  'a' [ expectedremoteparty ]
hangup	:=  'h'
dtmf	:=  'd' digits
voice	:=  'v' audiofile
record	:=  'r' [ append ] [ silence ] [ iter ] millis audiofile
append	:=  'a'
silence	:=  's'
closed	:=  'c'
iter	:=  'i'
activity:=  'a'
wait	:=  'w' [ activity | silence ] [ closed ] millis
timeout	:=  'w' millis
setlabel:=  'l' label
loop	:=  'j' [ how-many-times ] [ 'l' label ]
```

**Example:**

```
"l4;c333;ws3000;d123;w200;lthrice;ws1000;vaudio;rsi4000f.out;j3lthrice;h;j4" 
```

Parses to the following:

1. do this four times:
   1. call to 333
   2. wait until silent (max 3000 ms)
   3. send DTMF digits 123
   4. wait 200 ms 
   5. do this three times:
      1. wait until silent (max 1000 ms)
      2. send sound file 'audio'
      3. record until silent (max 4000 ms) to files 'f-[0-3]-[0-2].out'
   6. hangup
   7. wait 2000 ms

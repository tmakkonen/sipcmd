testphone
=========


### Introduction

Command line soft phone that makes phone calls, accepts calls,
enters DTMF digits, plays back WAV files and records them.

Not quite yet, though. WIP

### Run

testphone options:
-u <name> --user <name>         username (required)
-a <name> --alias <name>        username alias
-l <addr> --localaddress <addr> local address to listen on
-p <port> --listenport <port>   the port to listen on
-r <nmbr> --remoteparty <nmbr>  the party to call to
-x <prog> --execute <prog>      program to follow
-d <prfx> --audio-prefix <prfx> recorded audio filename prefix
-f <file> --file <file>         the name of played sound file
-g <addr> --gatekeeper <addr>   gatekeeper to use
-w <addr> --gateway <addr>      gateway to use

-l or -p without -x assumes answer mode. Additional -r forces
caller id checking. -r without -l, -p or -x assumes call mode.

WAV file requirements:
* format PCM
* mono
* 8 kHz sampling rate
* 16 bits sample size


The EBNF definition of the program syntax:
<prog>	:=  cmd ';' <prog> |
cmd	:=  call | answer | hangup
	  | dtmf | voice | record | wait
	  | setlabel | loop
call	:=  'c' remoteparty
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
setlabel:=  'l' label
loop	:=  'j' [ how-many-times ] [ 'l' label ]

Example:
"c333;ws3000;d123;w200;lthrice;ws1000;vaudio;rsi4000f.out;j3lthrice;h;j4"
parses to
1. do this four times:
1.1. call to 333
1.2. wait until silent (max 3000 ms)
1.3. send digits 123
1.4. wait 200 ms
1.5. do this three times:
1.5.1. wait until silent (max 1000 ms)
1.5.2. send voice file 'audio'
1.5.3. record until silent (max 4000 ms) to files 'f-'<0-3>'-'<0-2>'.out'
1.6. hangup
1.7. wait 2000 ms


### END OF FILE ###

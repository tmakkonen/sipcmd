<html>
<head>
<meta name="title" content="sipcmd - the command line sip/h323 softphone">
<title>sipcmd - the command line SIP and H.323 softphone</title>

<!-- i has ccs! -->
<style>
   pre       { border-style: solid;
               border-width: 1px 1px 1px 1px;
               border-color: #CCCCCC;
               background-color: #FFFFF0;
               padding: 6px 0px 5px 10px;
             }
</style>

<head>
<body>

<h2>sipcmd - the command line SIP/H.323/RTP softphone</h2>

<h3>Introduction</h3>
<p>
Command line soft phone that makes phone calls, accepts calls, enters DTMF digits, plays back WAV files and records them. A useful testing tool for VoIP systems. Runs on Linux.
</p>

<h3> NEWS </h3>
Upgraded to latest versions of ptlib and opal avaliable on apt repos on Ubuntu 12.04. (3.10.2 and 2.10.2, respectively).

<h3> HOWTO </h3>
<h4>### Dependencies</h4>
<p>
Apt-get install opal-dev and ptlib-dev packages.

<h4>### Download</h4>
<p>
Get source tarball from GitHub.
</p>

<h4>### Compile</h4>
<p>
<code> make </code>
</p>

<h4>### Environment</h4>
<p>
If you compile the dependencies from source, make sure that libpt and libopal are in your <code>LD_LIBRARY_PATH</code>. The default installation location is <code>/usr/local/lib</code>.
</p>

<h4>### Run</h4>
<b>testphone options:</b>
<pre>
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
</pre>
<p>
<code>-l</code> or <code>-p</code> without <code>-x</code> assumes answer mode. Additional <code>-r</code> forces caller id checking. <code>-r</code> without <code>-l</code>, <code>-p</code> or <code>-x</code> assumes call mode.
</p>

<br>
<b>WAV file requirements:</b>
<ul>
<li>mono
<li> 8 kHz sampling rate
<li> 16 bits sample size
</ul>

<b>The EBNF definition of the program syntax:</b>
<pre>
prog	:=  cmd ';' <prog> |
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
</pre>
<b>Example:</b><br><br>
<code>
"l4;c333;ws3000;d123;w200;lthrice;ws1000;vaudio;rsi4000f.out;j3lthrice;h;j4"
</code>
<br><br>
Parses to the following:
<ol>
  <li> do this four times:
    <ol> 
      <li> call to 333
      <li> wait until silent (max 3000 ms)
      <li> send dtmf digits 123
      <li> wait 200 ms 
      <li> do this three times:
      <ol>
        <li> wait until silent (max 1000 ms)
        <li> send sound file 'audio'
        <li> record until silent (max 4000 ms) to files 'f-[0-3]-[0-2].out'
      </ol>
      <li> hangup
      <li> wait 2000 ms
  </ol>
</ol>
<hr>
</body>
</html>


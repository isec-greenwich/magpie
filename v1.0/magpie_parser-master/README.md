# MAGPIE Parser

Utility used for parsing tshark output. <br>

Dependencies:<br><br>

> g++ <br><br>

Compilation:<br><br>
Temporary:<br>
> cd ./src
> g++ ./analysis/processor.c ./analysis/generic.c ./analysis/math_func.c ./analysis/windowing.c $(ls | grep .c) -o p -g


> 1. cmake .<br>
> 2. make <br><br>

Usage:<br><br>

> ./parser -**i** [file] -**o** [file] -**[wzs]**		(.csv file parsing) <br>
> ./parser -**l** -**o** [file] -**[wzsap]** -d 1		(live mode parsing, needs to be piped)<br><br>


Help:<br><br>

> **-l** - Live mode<br>
> **-io** - Input/Output<br>
> **-t** - Direct output<br>
> **-d** - Window mode with interrupt<br>
> **-wzsap** - WiFi/ZigBee/IP Short Header/Audio/Spectrum<br>
> **[file]** - Any existing/non-existing filename <br>


For WiFi use [Input]	| ./parser -l -w -t (will generate output)<br>
For Audio use [Input]	| ./parser -l -a -t <br>
<br>

For windowing use [Input] | ./parser -l [wzsap] -t -d [seconds] e.g. tshark ... | ./parser -l -s -t -d 5 (process IP headers live with direct output and 5 second window)<br>

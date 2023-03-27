# c86
hacks on c68 by Dave Walker / Keith Walker

Using elements from:

http://web.archive.org/web/20010206053218/http://www.itimpi.freeserve.co.uk/index1.htm

http://web.archive.org/web/20060831132539/http://homepage.ntlworld.com/itimpi/index1.htm

https://github.com/plusk01/8086-toolchain

https://github.com/lkundrak/dev86

Right now it set to EPOC target, in essense 8086 machine. I reconfigured c86 back to as86 as assembler because new nasm choke on output with undefined calls.
It goes as far as as compiling .o file correctly recognizable by objdump86 from dev86.

No idea about libraries, I used headers from https://github.com/alexfru/SmallerC

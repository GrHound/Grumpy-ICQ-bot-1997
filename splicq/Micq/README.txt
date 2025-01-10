README for Grumpy-v5 - revived partially as standalone (without ICQ)
                       December 2024

    Grumpy dates from about 1997 and was based on Splotch.c
    It was modified to have topic control on the basis of a leaky
    integrator model with 'affect' variables. Furthermore: attempts
    to keep the conversation variable but still getting back to the
    'current' (decaying) topic.
    A calculator was built in and several web sources at the time
    were used via a system() call, lynx and some parsing etc.


Building on Linux, assuming gcc

1) Go to the source tree
   cd ./grumpy-v5
   cd ./splicq
2) ./make-grumpy
3) cd ..
4) Start a standalone, non-ICQ session :
   ./splicq/grumpy

Sessions are logged and the HUMAN entries in the logs are used 
to spit out future answers

Grumpy ran on a server at Nijmegen University in the NICI institute.
One ICQ user came back more than 800 times.

Lambert Schomaker, December 2024

# Grumpy-ICQ-bot-1997

In view of the recent popularity I thought I should add this for posteriority. ICQ was a chat service in the late 1990s. Grumpy was implemented in 1997 on the basis of the plain bot 'splotch.c' by Duane Fields. Modifications include: an activation based (leaky-integrator) topicality mechanism to obtain some consistence in the dialog per ICQ user, over sessions. Invocation of terminal-based games (not functioning now). Integration with the ICQ api and mechanism with a UIN per user. Several mechanisms to address extraneous sources such as wordnet, altavista, later google, wikipedia etc. One ICQ use came back 800 times to talk to Grumpy. It is clear that the content and log files are partly incomprehensible and in any case highly likely to contain non-politically correct fragments coming from the user community 1997-2000. In short: The owner of the repository take no responsibility for inappropriate content. Using decency criteria the user logs were cleaned but norms are in a continuous flux. Also, more innocuously, there will be a lot of technical, nerdy jargon and abbreviations that will be utterly incomprehensible to current computer users.

________________________________________________________________________________________________________________
README for Grumpy-v5 - revived partially as standalone (without ICQ)
                       December 2024

    Grumpy dates from about 1997 and was based on Splotch.c
    It was modified to have topic control on the basis of a leaky
    integrator model with 'affect' variables. Furthermore: attempts
    to keep the conversation variable but still getting back to the
    'current' (decaying) topic.
    A calculator was built in and several web sources at the time
    were used via a system() call, lynx and some parsing etc.,
    as well as a number of terminal-based games such as hangman, block
    and phalanx. Those games are not contained in this repository.

    It should be possible to build a standalone version on Linux with gcc.
    The ICQ link is obviously defunct. No guarantees, the repository is
    mainly a time stamp and for enthusiasts who would like to find our
    how nineties' chatbots (i.e., Eliza deratives) worked.

Building on Linux, assuming gcc

1) Go to the source tree
   cd ./grumpy-v5
   cd ./splicq
2) ./make-grumpy
3) cd ..
4) Start a standalone, non-ICQ session :
   ./splicq/grumpy

Sessions are logged and the HUMAN entries in the logs are used 
to spit out future answers. This was obviously dangerous and required
daily inspection to prevent malign content to be regurgitated by Grumpy.

Grumpy ran on a server at Nijmegen University in the NICI institute.
One ICQ user came back more than 800 times.

Lambert Schomaker, December 2024

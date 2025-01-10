# Grumpy-ICQ-bot-1997

In view of the recent popularity I thought I should add this for posteriority. ICQ was a chat service in the late 1990s. 
Grumpy was implemented in 1997 on the basis of the plain bot 'splotch.c' by Duane Fields. 
Modifications include: an activation based (leaky-integrator) topicality mechanism to obtain some consistence in the 
dialog per ICQ user, over sessions. Invocation of terminal-based games (not functioning now). Integration with the ICQ 
api and mechanism with a UIN per user. Several mechanisms to address extraneous sources such as wordnet, altavista, later 
google, wikipedia etc. One ICQ user came back 800 times to talk to Grumpy. It is clear that the content and log files are 
partly incomprehensible and in any case highly likely to contain non-politically correct fragments coming from the user 
community 1997-2000. In short: The owner of the repository take no responsibility for inappropriate content. Using decency 
criteria the user logs were cleaned but norms are in a continuous flux. Also, more innocuously, there will be a lot of technical, 
nerdy jargon and abbreviations that will be utterly incomprehensible to current computer users.

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

_____________________________________________________________________________________________________________________

Data directories of interest

words/                The words that the bot knows. An associated dot file .Act-{word}.w contains the topicality activation (see example below)
  about-my-status.w
            Content (answers):
                  fine
                  allright
                  ok
                  so-so
                  pretty good
                  fine thanks, and you?  
                  just fine, thanks
                  I am a bit under the weather
                  I am bored, actually
                  perfect
                  couldn't be better
                  I'm happy
                  I'm ok  
                  I'm fine
  .Act-about-my-status.w
                  15
               0.99
               0.99
               0.81
               0.89
               0.99
               0.99
               0.15
               0.99
               0.94
              -1.43     The answer "perfect" apparently was recently uttered, so its likelihood is pulled down
               0.97
               0.99
               0.54
               0.99
               0.97


  adjust
  adj.w
  affirmative.w
  aggies.w
  aggressive.w
  ai.w
  alive.w
  amiga.w
  angry.w
  animal.w
  artificial.w
  aussie.w
  bored.w
  bot.w
  bye.w
  canada.w
   .
   .
  yes.w
  you_forgot.w

phrases/         Precooked, longer answers
  capitalism.p    
             .phrase
             Adam Smith and J. S. Mill were well aware of the actual and
             possible defects of capitalism. But neither one of them exhibited the 
             quasi-religious nostalgic utopianism of contemporary libertarianism, 
             nor did they attempt to conflate capitalism with anarchy.

  death-penalty-contra.p
  death-penalty-pro.p
  guns-contra.p
  socialism.p


episodes/   Even longer narratives
   all-dict-words.e
   eliza.e
   fairy-tales.e
   family-puzzles.e
   injustice.e
   martial-arts.e
   smurfs.e
      .episode 
         When anyone thinks of communism they think of three names; Stalin, Lenin, 
         and Marx. The fact that there are three head communism figures leaves us with 
         a problem; which one was Papa Smurf Modeled after?  Many believe that it was 
         Karl Marx, the man who developed the idea of socialism. You must admit they 
         look shockingly alike. 

   topics.e


Note: not only words, but also phrases and episodes were subject to the activation likelihood mechanism controlling the random selection.

![Example image](potfield-example.png)

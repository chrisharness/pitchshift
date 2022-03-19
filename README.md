# pitchshift~

## notes about this project

### The help patch is unfortuntely the main working component for this assignment. I dealt with many pd bugs, specifically revolving around ADC/signal inlets into pitchshift~.

Inside pitchshift~.c you will find my attempt at creating the "blackbox" implementation of what you will find on the LHS of the help patch. I successfully created the abstraction for pitchshift~, which is distinguishable from Miller's G09 pitch shift help patch in that this abstraction dealt with "real-time" pitch shifting vs. static pitch shifting (where Miller uses the rotating tape head method in his abstraction). I scowered the internet and found as many resources as I could to help me, but I think I need a bit more guidance before this project can be completed. I think I am misconstruing some ideas in the logic flow of the external but I have left some commented code as my thoughts to how to go about implementing my ideas. 


### The LHS tree in the help patch does function correctly and implements real-time pitch shifting although is not an external.

Above pitchshift_perform inside pitchshift~.c, I have laid out my high level thinking. 


using CP;

int numActors = ...;
range Actors = 1..numActors;
int actorPay[Actors] = ...;
int numScenes = ...;
range Scenes = 1..numScenes;
int sceneDuration[Scenes] = ...;

int actorInScene[Actors][Scenes]  = ...;

dvar int scene[Scenes] in Scenes;
dvar int slot[Scenes] in Scenes;

dvar int revscene[i in Scenes] = scene[numScenes + 1 - i];


// First and last slots where each actor plays
dexpr int firstSlot[a in Actors] = min(s in Scenes:actorInScene[a][s] == 1) slot[s];
dexpr int lastSlot[a in Actors] = max(s in Scenes:actorInScene[a][s] == 1) slot[s];

// Expression for the waiting time for each actor
dexpr int actorWait[a in Actors] = sum(s in Scenes: actorInScene[a][s] == 0)  
   (sceneDuration[s] * (firstSlot[a] <= slot[s] && slot[s] <= lastSlot[a]));

// Expression representing the global cost
dexpr int idleCost = sum(a in Actors) actorPay[a] * actorWait[a];

minimize idleCost;
subject to {
   // use the slot-based secondary model
   inverse(scene, slot);
   lex(scene, revscene);
}


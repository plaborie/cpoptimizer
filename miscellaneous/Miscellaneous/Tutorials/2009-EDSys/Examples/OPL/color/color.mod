
/* ------------------------------------------------------------

Problem Description
-------------------

The problem involves choosing colors for the countries on a map in 
such a way that at most four colors (blue, white, yellow, green) are 
used and no neighboring countries are the same color. In this exercise, 
you will find a solution for a map coloring problem with six countries: 
Belgium, Denmark, France, Germany, Luxembourg, and the Netherlands. 

------------------------------------------------------------ */
using CP;

range r = 0..3;

string Names[r] = ["blue", "white", "yellow", "green"]; 

dvar int Belgium in r;
dvar int Denmark in r;
dvar int France in r;
dvar int Germany  in r;
dvar int Luxembourg in r;
dvar int Netherlands in r;

subject to {  
   Belgium != France; 
   Belgium != Germany; 
   Belgium != Netherlands;
   Belgium != Luxembourg;
   Denmark != Germany; 
   France != Germany; 
   France != Luxembourg; 
   Germany != Luxembourg;
   Germany != Netherlands;    
}

execute {
   writeln("Belgium:     ", Names[Belgium]);
   writeln("Denmark:     ", Names[Denmark]);
   writeln("France:      ", Names[France]);
   writeln("Germany:     ", Names[Germany]);
   writeln("Luxembourg:  ", Names[Luxembourg]);
   writeln("Netherlands: ", Names[Netherlands]);
}

/*
Belgium:     green
Denmark:     blue
France:      yellow
Germany:     white
Luxembourg:  blue
Netherlands: blue
*/

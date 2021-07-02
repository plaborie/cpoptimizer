var int x in 5..20;
var int y in 0..10;
var int tab[0..1] in 0..20;
solve
{
   tab[0] = x;
   tab[1] = y;
   x<y;
   y<8;
};
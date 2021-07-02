using CP;

{ string } Task     = ...;
{ string } Resource = ...;

tuple Distance {
  string pre;
  string post;
  int    dist;
};

tuple Precedence {
  string pre;
  string post;
};

int duration[Task] = ...;
{Distance} max_nf  = ...;
{Distance} min_sf  = ...;
{Distance} max_ef  = ...;
{Distance} min_nf  = ...;
{Distance} min_af  = ...;
{string} res[Resource] = ...;
{Precedence} precedences = ...;

dvar interval a[t in Task] size duration[t];

execute {
  cp.param.FailLimit = 5000;
}

minimize max(t in Task) endOf(a[t]);
subject to {  
  forall(t in precedences ) 
    endBeforeStart(a[t.pre], a[t.post]); 
  
  forall( t in max_nf )
    startBeforeEnd(a[t.post], a[t.pre], -t.dist);
  
  forall(t in max_ef )
    endBeforeEnd( a[t.post],  a[t.pre], - t.dist); 
  
  forall( t in min_af )
    startBeforeStart(a[t.pre], a[t.post], t.dist);   
  
  forall( t in min_sf )
    endBeforeStart(a[t.post], a[t.pre], -t.dist);
  
  forall( t in min_nf )
    endBeforeStart(a[t.pre], a[t.post], t.dist);
    
  forall( r in Resource)
    noOverlap(all( t in res[r]) a[t]);

};

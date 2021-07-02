//-------------------------------------------------------------------------
// dtc_cfnp_turbo.c : body of the dtc_cfnp_turbo class
//
// Giulio Pasqualone 1997;
// modified by D. Frigioni and C. Zaroliagis 1998
//-------------------------------------------------------------------------

#include <LEP/dynamic_graphs/dtc_cfnp_turbo.h>


//***************************************************************************
void dtc_cfnp_turbo::closure(node v,node x)
{
node i;
forall_adj_nodes(i,x)
   { (TC(v,i))++;
      if (TC(v,i)==1)
          closure(v,i);
   }

}


void dtc_cfnp_turbo::transitive_closure()
{
node v;
    forall_nodes(v,*MGP)
      { TC(v,v)=1;
        closure(v,v);
      } 
}


//***************************************************************************


void dtc_cfnp_turbo::initialize_new_g(msg_graph& G)
{    
    MGP=&G;
    TC.init(*MGP,0);
    transitive_closure();
    updated=true;
}

//***************************************************************************


void dtc_cfnp_turbo::initialize_same_g(msg_graph& G)
{    
    TC.set(0);
    transitive_closure();
    updated=true;
}

//***************************************************************************

void dtc_cfnp_turbo::propagate(node x, node z)
{
    node v;
    if (x!=z)
      {
        TC(x,z)++;
        forall_adj_nodes(v,z)
          {
            if (TC(x,v)==0)
              propagate(x,v);
          }
      }
}

//***************************************************************************

void dtc_cfnp_turbo::tc_edge_insertion_turbo(node v,node w)
{
if (TC(v,w)==0)  
 {
  node k;
  list<node> L;

  forall_nodes(k,*MGP)
    {
      if (TC(k,v)>0 && TC(k,w)==0)
        propagate(k,w);
    }
 }
updated=false; //data structures are ALTERATED
}    

//***************************************************************************


void dtc_cfnp_turbo::tc_edge_deletion_turbo(node v,node w)
{
if (!updated) 
 { 
  update(MGP);
 }
else
 {
  node k,y;
  list<node> L;

  forall_nodes(k,*MGP)
    {
      if(TC(k,v)>0)
        {
	  L.append(w);
	  while(!L.empty())
            {
	      node h=L.pop();
	      if(h!=k)
		{
		  (TC(k,h))--;
		  if(TC(k,h)==0) 
		    {
		      forall_adj_nodes(y,h)
			{
			  L.append(y);
			}
		    }
		}
	    }
	}
    }
 }
}


//***************************************************************************

void dtc_cfnp_turbo::tc_node_insertion(node x)
{
TC.new_node(x,0);
TC(x,x)=1; 
}

//***************************************************************************

void dtc_cfnp_turbo::tc_node_deletion(node x)
{
TC.del_node(x);
}

//***************************************************************************

bool dtc_cfnp_turbo::path(node x,node y)
{
    int  numero;    
    numero=TC(x,y);
    if(numero>0)
        return(true);
    else
        return(false);    
}

//***************************************************************************

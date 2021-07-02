//-------------------------------------------------------------------------
// dtc_cfnp.c : body of the dtc_cfnp class
//
// Giulio Pasqualone 1997
//-------------------------------------------------------------------------

#include <LEP/dynamic_graphs/dtc_cfnp.h>


//***************************************************************************
void dtc_cfnp::closure(node v,node x)
{
node i;
forall_adj_nodes(i,x)
   { (TC(v,i))++;
      if (TC(v,i)==1)
          closure(v,i);
   }

}


void dtc_cfnp::transitive_closure()
{
node v;
    forall_nodes(v,*MGP)
      { TC(v,v)=1;
        closure(v,v);
      } 
}


//***************************************************************************


void dtc_cfnp::initialize_new_g(msg_graph& G)
{    
    MGP=&G;
    TC.init(*MGP,0);
    transitive_closure();         
}

//***************************************************************************


void dtc_cfnp::initialize_same_g(msg_graph& G)
{    
    TC.set(0);
    transitive_closure();         
}

//***************************************************************************

void dtc_cfnp::tc_edge_insertion(node v,node w)
{
    node k;
    forall_nodes(k,*MGP)
    {
        if(TC(k,v)>0)
        {
            list<node> L;
        //  L.append(v);
            L.append(w);
            while(!L.empty())
            {
        //      node l=L.pop();
                node h=L.pop();
                (TC(k,h))++;
                if(TC(k,h)==1)
                {
                    node y;
                    forall_adj_nodes(y,h)
                    {
        //              L.append(h);
                        L.append(y);
                    }
                }
            }
        }
    }
}    


//***************************************************************************


void dtc_cfnp::tc_edge_deletion(node v,node w)
{
    node k;
    forall_nodes(k,*MGP)
    {
        if(TC(k,v)>0)
        {
            list<node> L;
     //     L.append(v);
            L.append(w);
            while(!L.empty())
            {
     //         node l=L.pop();
                node h=L.pop();
                (TC(k,h))--;
                if(TC(k,h)==0) 
                {
                    node y;
                    forall_adj_nodes(y,h)
                    {
     //                 L.append(h);
                        L.append(y);
                    }
                }
            }
        }
    }
}

//***************************************************************************

void dtc_cfnp::tc_node_insertion(node x)
{
TC.new_node(x,0);
TC(x,x)=1; 
}

//***************************************************************************

void dtc_cfnp::tc_node_deletion(node x)
{
TC.del_node(x);
}

//***************************************************************************

bool dtc_cfnp::path(node x,node y)
{
    int  numero;    
    numero=TC(x,y);
    if(numero>0)
        return(true);
    else
        return(false);    
}

//***************************************************************************

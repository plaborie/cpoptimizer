// --------------------------------------------------------------------
//
//  File:        dtc_italiano_gen.c
//  Date:        10/97
//  Last update: 03/99
//  Description: A generalized version of Italiano's dynamic transitive
//               closure algorithm; its decremental part works for any
//               directed graph.
//
//  (C) 1997, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#include <LEP/dynamic_graphs/dtc_italiano_gen.h>
#include <LEDA/graph_alg.h>
     
/******************** SCC CONSTRUCTOR ********************/

dtc_italiano_gen::scc::scc(dtc_italiano_gen* t, node v) : This(t)
  {
    Parent.init(*(This->the_graph),NULL);
    This->SCC[v]=this;
    This->GVIndex[v]=G.new_node(v);
    Visit.init(G);
    Time.init(*(This->the_graph),0);
    RemItem=NULL;
  }

dtc_italiano_gen::scc::scc(dtc_italiano_gen* t, list<node> &L) : This(t)
  {
    node v;
    Parent.init(*(This->the_graph),NULL);
    forall(v,L)
      {
        This->SCC[v]=this;
        This->GVIndex[v]=G.new_node(v);
      }
    Visit.init(G);
    Time.init(*(This->the_graph),0);
    RemItem=NULL;
  }

/******************** UPDATE ********************/

void dtc_italiano_gen::update(msg_graph *G)
  {
    SCC.init(*G,NULL);
    Desc.init(*G,false);
    GVIndex.init(*G,NULL);
    GEIndex.init(*G,NULL);
    InIndex.init(*G,NULL);
    OutIndex.init(*G,NULL);
    EIndex.init(*G);
    Reset=Time=0;
    node v;
    edge e;

    forall_nodes(v,*G) { Desc(v,v)=true; new scc(this,v); }
    list<edge> L=G->all_edges();
    forall(e,L) G->hide_edge(e);
    forall(e,L) { G->restore_edge(e); insert_edge(e); }
  }

/******************** QUERIES ********************/

bool dtc_italiano_gen::path(node v, node w, list<node> &Path)
  {
    Path.clear();
    if (! Desc(v,w)) return false;
    edge e;
    while (SCC[v] != SCC[w])
      {
        e=SCC[w]->Parent[v];
        SCC[w]->set_visit(1);
        SCC[w]->path(GVIndex[target(e)],GVIndex[w],Path);
        w=source(e);
      }
    SCC[w]->set_visit(1);
    SCC[w]->path(GVIndex[v],GVIndex[w],Path);
    return true;
  }
  
bool dtc_italiano_gen::scc::path(node v, node w, list<node> &Path)
  {
    if (v == w)
      {
        Path.push(G[v]);
        return true;
      }
    edge e;
    Visit[v]=0;
    forall_out_edges(e,v)
        if (Visit[target(e)])
            if (path(target(e),w,Path))
              {
                Path.push(G[v]);
                return true;
              }
    return false;
  }

bool dtc_italiano_gen::path(node v, node w, list<edge> &Path)
  {
    Path.clear();
    if (! Desc(v,w)) return false;
    edge e;
    while (SCC[v] != SCC[w])
      {
        e = SCC[w]->Parent[v];
        SCC[w]->set_visit(1);
        SCC[w]->path(GVIndex[target(e)], GVIndex[w], Path);
        Path.push(e);
        w=source(e);
      }
    SCC[w]->set_visit(1);
    SCC[w]->path(GVIndex[v], GVIndex[w], Path);
    return true;
  }
  
bool dtc_italiano_gen::scc::path(node v, node w, list<edge> &Path)
  {
    if (v == w) return true;
    edge e;
    Visit[v]=0;
    forall_out_edges(e,v)
        if (Visit[target(e)])
            if (path(target(e),w,Path))
              {
                Path.push(G[e]);
                return true;
              }
    return false;
  }

bool dtc_italiano_gen::path(node v, node w, list<node> &Path, list<edge> &Cut)
  {
    Cut.clear();
    if (path(v,w,Path)) return true;
    edge e;
    forall_edges(e,*the_graph)
        if (Desc(v,source(e)) != Desc(v,target(e))) Cut.append(e);
    return false;
  }

bool dtc_italiano_gen::path(node v, node w, list<edge> &Path, list<edge> &Cut)
  {
    Cut.clear();
    if (path(v,w,Path)) return true;
    edge e;
    forall_edges(e,*the_graph)
        if (Desc(v,source(e)) != Desc(v,target(e))) Cut.append(e);
    return false;
  }

void dtc_italiano_gen::desc(node v, list<node> &L)
  {
    node w;
    L.clear();
    forall_nodes(w,*the_graph)
        if (Desc(v,w)) L.push(w);
  }

void dtc_italiano_gen::scc_graph(node v, list<node> &LNodes, list<edge> &LEdges)
  {
    node w;
    edge e;
    LNodes.clear();
    LEdges.clear();
    forall_nodes(w,SCC[v]->G) LNodes.push(SCC[v]->G[w]);
    forall_edges(e,SCC[v]->G) LEdges.push(SCC[v]->G[e]);
  }

/******************** INSERT EDGE ********************/

void dtc_italiano_gen::insert_edge(edge e, double weight)
  {
    node v,s=source(e),t=target(e);
    if (SCC[s] == SCC[t])  /* internal edge */
      {
        GEIndex[e]=SCC[s]->G.new_edge(GVIndex[s],GVIndex[t],e);
        SCC[s]->G.hide_edge(GEIndex[e]);
        return;
      }
    
    Reset=1;              /* external edge */
    OutIndex[e]=SCC[s]->Out.append(e);
    InIndex[e]=SCC[t]->In.append(e);
    if (! Desc(s,t))      /* no previous path */
      {
        forall_nodes(v,*the_graph)
            if (Desc(v,s) && (! Desc(v,t))) meld(v,e);
        if (Desc(t,s)) merge(s);  /* new scc created */
      }
  }

void dtc_italiano_gen::meld(node v, edge e)
  {
    node w, t=target(e);
    SCC[t]->Parent[v]=e;
    forall_nodes(w,SCC[t]->G) Desc(v,SCC[t]->G[w])=true;
    forall(e,SCC[t]->Out)
        if (! Desc(v,target(e))) meld(v,e);
  }

void dtc_italiano_gen::merge(node x)
  {
    node v,s,t;
    edge e,f;
    list<node> V;
    list<scc*> OldList;
    
    /* compute vertices and sccs to be merged */
    forall_nodes(v,*the_graph) SCC[v]->Flag=1;
    forall_nodes(v,*the_graph)
        if (Desc(x,v) && Desc(v,x))
          {
            V.append(v);
            if (SCC[v]->Flag) OldList.append(SCC[v]);
            SCC[v]->Flag=0;
          }
    
    /* insert edges into G, In, Out of new scc */
    scc *Old, *New=new scc(this,V);
    forall_edges(e,*the_graph)
      {
        if (SCC[s=source(e)] == New)
          {
            if (SCC[t=target(e)] == New)
                GEIndex[e]=New->G.new_edge(GVIndex[s],GVIndex[t],e);
            else OutIndex[e]=New->Out.append(e);
          }
        else if (SCC[target(e)] == New)
                InIndex[e]=New->In.append(e);
      }
    New->compute_sparse();

    /* extract parent references from old sccs */
    forall_nodes(v,*the_graph)
        if (Desc(v,x) && (SCC[v] != New))
            forall(Old,OldList)
                if (SCC[source(e=Old->Parent[v])] != New)
                  {
                    New->Parent[v]=e;
                    break;
                  }
    forall(Old,OldList) delete Old;
  }

/******************** REMOVE EDGE ********************/

void dtc_italiano_gen::remove_edge(edge e)
  {
    if (Reset) reset();

    node v, s=source(e), t=target(e);
    scc *S=SCC[s], *T=SCC[t];
    
    /* internal edge */
    if (S == T)
      {
        bool hidden=S->G.is_hidden(GEIndex[e]);
        S->G.del_edge(GEIndex[e]);
        if (hidden) return;
        S->G.restore_all_edges();
        if (S->compute_sparse()) return;
        list<scc*> NewList;
        S->compute_break(NewList);
        split(S,NewList,e);
      }

    /* external edge */
    /* Important: all except e are valid hooks !!! */
    SCC[t]->RemItem=InIndex[e];

    forall_nodes(v,*the_graph) SCC[t]->hook(v,e);
    SCC[t]->In.del(InIndex[e]);
    SCC[s]->Out.del(OutIndex[e]);

    SCC[t]->RemItem=NULL;
  }

void dtc_italiano_gen::scc::hook(node v, edge e)
  {
    if (Parent[v] != e) return;
    node w=target(e);
    list_item it;
    
    if (Time[v] != This->Time)
      {
        it=In.first();
        Time[v]=This->Time;
      }
    else it=In.succ(This->InIndex[e]);
    
    if (search_hook(v,it)) return;
    
    forall_nodes(w,G) This->Desc(v,G[w])=false;
    forall(e,Out) This->SCC[target(e)]->hook(v,e);
  }

bool dtc_italiano_gen::scc::search_hook(node v, list_item it)
  {
    for (it; it != NULL; it=In.succ(it))
        if ((it != RemItem) && This->Desc(v,source(In[it])))
          {
            Parent[v]=In[it];
            return true;
          }
    Parent[v]=NULL;
    return false;
  }

void dtc_italiano_gen::split(scc* Old, list<scc*>& NewList, edge rem)
  {
    node v, w;
    edge e, f;
    scc* New;

    /* insert edges into G, In, Out of new sccs */
    forall(e,Old->In) InIndex[e]=SCC[target(e)]->In.append(e);
    forall(e,Old->Out) OutIndex[e]=SCC[source(e)]->Out.append(e);
    forall_edges(e,Old->G)
      {
        f=Old->G[e];
        scc *S=SCC[source(f)], *T=SCC[target(f)];
        if (S == T)
            GEIndex[f]=S->G.new_edge(GVIndex[source(f)],GVIndex[target(f)],f);
        else
          {
            OutIndex[f]=S->Out.append(f);
            InIndex[f]=T->In.append(f);
          }
      }
    OutIndex[rem]=SCC[source(rem)]->Out.append(rem);
    InIndex[rem]=SCC[target(rem)]->In.append(rem);

    /* transfer parent references to new sccs */
    forall_nodes(v,*the_graph)
        if ((e=Old->Parent[v]) != NULL) EIndex[e].append(v);
    forall(New,NewList) New->Item=New->In.first();
    forall(e,Old->In)
      {
        forall(v,EIndex[e])
            forall(New,NewList) New->Parent[v]=New->In[New->Item];
        EIndex[e].clear();
        New=SCC[target(e)];
        New->Item=New->In.succ(New->Item);
      }

    forall(New,NewList)
      {
        forall_nodes(v,*the_graph)
          {
            if ((e=New->Parent[v]) != NULL)
              {
                if (Old->Time[v] == Time) New->search_hook(v,InIndex[e]);
                else New->search_hook(v,New->In.first());
              }
            else	/* if old scc is root of descendant tree of v */
                if (Desc(v,New->G[New->G.first_node()]) && (New != SCC[v]))
                    New->search_hook(v,New->In.first());
            New->Time[v]=Time;
          }
        New->compute_sparse();
      }
    
    delete Old;
  }

/******************** Reset ********************/

void dtc_italiano_gen::reset()
  {
    node v, w;
    Time++;
    if (Time > the_graph->number_of_nodes())
      {
        Time=1;
        forall_nodes(v,*the_graph)
            forall_nodes(w,*the_graph) SCC[v]->Time[w]=0;
      }
    
    Reset=0;
  }

/******************** COMPUTE BREAK ********************/
void dtc_italiano_gen::scc::compute_break(list<scc*> &NewList)
  {
    node_array<int> num(G,0);
    int c=STRONG_COMPONENTS(G,num);
    
    node v;
    list<node> V;
    for (int i=0; i<c; i++)
      {
        V.clear();
        forall_nodes(v,G)
            if (num[v]==i) V.append(G[v]);
        NewList.append(new scc(This,V));
      }
  }

/******************** COMPUTE SPARSE GRAPH ********************/

bool dtc_italiano_gen::scc::compute_sparse()
  {
    node v, f=G.first_node();
    edge e;
    list<node> V;
    edge_array<int> Hide(G,1);
    int c=0;
        
    set_visit(0);
    Visit[f]=1;
    V.append(f);
    while (! V.empty())
      {
        v=V.pop();
        forall_out_edges(e,v)
            if (Visit[target(e)] == 0)
              {
                Visit[target(e)]=1;
                V.append(target(e));
                Hide[e]=0; c++;
              }
      }
    if (c < G.number_of_nodes()-1) return false;
    
    Visit[f]=2;
    V.append(f);
    while (! V.empty())
      {
        v=V.pop();
        forall_in_edges(e,v)
            if (Visit[source(e)] == 1)
              {
                Visit[source(e)]=2;
                V.append(source(e));
                Hide[e]=0; c++;
              }
      }
    if (c < 2*G.number_of_nodes()-2) return false;
    
    forall_edges(e,G) if (Hide[e]) G.hide_edge(e);
    return true;
  }

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
// input: n m then m lines of 5 lits. output assignment string
int n,m,K=5; int *cl; int *occ,*occs; int *val,*ntrue; int *unsat,*pos,nu=0;
unsigned long long rs=88172645463325252ULL; static inline unsigned long long xr(){rs^=rs<<13;rs^=rs>>7;rs^=rs<<17;return rs;}
double probs[64];
int main(int argc,char**argv){double cb=argc>1?atof(argv[1]):5.4; long maxf=argc>2?atol(argv[2]):2000000000L; rs^=(unsigned long long)time(0)^((unsigned long long)getpid()<<20)^(argc>5?strtoull(argv[5],0,10)*0x9E3779B97F4A7C15ULL:0);
 if(scanf("%d %d",&n,&m)!=2)return 1; cl=malloc(sizeof(int)*m*K);
 int *cnt=calloc(2*n+2,sizeof(int));
 for(int i=0;i<m*K;i++){scanf("%d",&cl[i]);cnt[cl[i]+n]++;}
 occs=malloc(sizeof(int)*(2*n+2)); int t=0; for(int i=0;i<2*n+1;i++){occs[i]=t;t+=cnt[i];} occs[2*n+1]=t;
 occ=malloc(sizeof(int)*t); int *f=calloc(2*n+2,sizeof(int));
 for(int c=0;c<m;c++)for(int j=0;j<K;j++){int l=cl[c*K+j]+n;occ[occs[l]+f[l]++]=c;}
 val=malloc(sizeof(int)*(n+1)); 
 // majority init
 for(int v=1;v<=n;v++) val[v]= cnt[v+n]>cnt[-v+n]?1:(cnt[v+n]<cnt[-v+n]?0:(xr()&1));
 if(argc>3&&atoi(argv[3])==0) for(int v=1;v<=n;v++) val[v]=xr()&1;
 ntrue=calloc(m,sizeof(int)); unsat=malloc(sizeof(int)*m); pos=malloc(sizeof(int)*m);
 for(int i=0;i<64;i++)probs[i]=pow(cb,-i);
 for(int c=0;c<m;c++){for(int j=0;j<K;j++){int l=cl[c*K+j]; if((l>0)==val[abs(l)])ntrue[c]++;} if(!ntrue[c]){pos[c]=nu;unsat[nu++]=c;}}
 fprintf(stderr,"init unsat %d\n",nu);
 long fl=0; double pr[5]; int best=1<<30;
 while(nu>0 && fl<maxf){
  int c=unsat[xr()%nu]; double s=0;
  for(int j=0;j<K;j++){int v=abs(cl[c*K+j]); int lt = val[v]? v+n : -v+n; // currently true literal of v
    int b=0; for(int q=occs[lt];q<occs[lt+1];q++) if(ntrue[occ[q]]==1) b++;
    pr[j]=probs[b<63?b:63]; s+=pr[j];}
  double r=(double)(xr()>>11)/9007199254740992.0*s; int j=0; while(j<K-1 && r>pr[j]){r-=pr[j];j++;}
  int v=abs(cl[c*K+j]); int lt = val[v]? v+n : -v+n; int lf= val[v]? -v+n : v+n;
  val[v]^=1; fl++; if(nu<best){best=nu; if(nu<80 && argc>4){FILE*o=fopen(argv[4],"w"); for(int w=1;w<=n;w++)fputc(val[w]?'1':'0',o); fprintf(o,"\n%d\n",nu); fclose(o);}} if((fl&((1L<<24)-1))==0){fprintf(stderr,"fl %ld best %d cur %d\n",fl,best,nu);fflush(stderr);}
  for(int q=occs[lt];q<occs[lt+1];q++){int d=occ[q]; if(--ntrue[d]==0){pos[d]=nu;unsat[nu++]=d;}}
  for(int q=occs[lf];q<occs[lf+1];q++){int d=occ[q]; if(ntrue[d]++==0){int p=pos[d];unsat[p]=unsat[--nu];pos[unsat[p]]=p;}}
 }
 fprintf(stderr,"flips %ld unsat %d\n",fl,nu);
 if(nu==0){for(int v=1;v<=n;v++)putchar(val[v]?'1':'0');putchar('\n');return 0;} return 2;
}

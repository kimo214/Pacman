#include <bits/stdc++.h>
using namespace std;
#define sc(x)            scanf("%d",&x)
#define scc(x)           scanf("%c",&x)
#define scl(x)           scanf("%lld",&x)
#define sz(v)	     	(v.size())
#define mem(v, d)		memset(v, d, sizeof(v))
#define oo				2000000100
#define OO				4000000000000000100
#define PI 3.14159265
typedef unsigned int uint;
typedef long long ll;
//-----------------------------------------------------

int main()
{
#ifndef ONLINE_JUDGE
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
#endif
    int n,m,x,y,num;
    sc(n); sc(m);
    printf("Width = %d,Hight = %d;\n",n,m);
    sc(num);
    for(int i=0;i<num;++i)
    {
        sc(x); sc(y);
        printf("Barrier[%d][%d] = true;\n",x,y);
        if(y == 0 && x+n < n*m) printf("Barrier[%d][2] = true;\n",x+n);
        else if(y == 1 && x%5 + 1 < n) printf("Barrier[%d][3] = true;\n",x+1);
        else if(y == 2 && x-n >= 0) printf("Barrier[%d][0] = true;\n",x-n);
        else if(y == 3 && x%5 - 1 >=0) printf("Barrier[%d][1] = true;\n",x-1);
    }

    return 0;
}

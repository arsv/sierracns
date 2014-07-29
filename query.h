#define SHOW_R (1<<0)
#define SHOW_E (1<<1)

#define nn 0
#define Rn SHOW_R
#define nE SHOW_E
#define RE (SHOW_R | SHOW_E)

int querycns(int mode, int oid, int op, int len, char* payload);
int getcns(int oid);
int setcns(int oid, int len, char* payload);

int watchcns(int m, int* term, int* which);
int queryall(int n, int* oids, int op);

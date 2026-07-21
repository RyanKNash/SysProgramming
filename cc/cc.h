enum {
	Descsz = 256-3*sizeof(int)-16-16-8,
};

typedef struct CComp CComp;

struct CComp {
	int id;
	char name[16];
	int year;
	char maker[16];
	char cpu[8];
	int memory;
	char desc[Descsz];
};

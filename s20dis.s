#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

typedef struct S20word S20word;
typedef struct Sym Sym;
typedef enum Wtype Wtype;

enum {
	NMEM = 32768,
	BPERW = 3
};

enum Wtype {
	Unknown,
	Instr,
	Data
};

struct S20word {
	unsigned long contents;
	Wtype wtype;
	char visited;
	unsigned int opcode;
	union {
		struct {
			unsigned int reg;
			unsigned int addr;
		} a;
		struct {
			unsigned int rA;
			unsigned int rB;
			unsigned int rC;
			unsigned int subop;
		} r;
	} u;
};

struct Sym {
	char *name;
	unsigned int addr;
	Sym *next;
};

int loadfile(char *);
void decode(int);
void disassm(int);
Sym *newsym(unsigned int);
Sym *lookaddr(unsigned int);
Sym *lookname(char *);

S20word mem[NMEM];
Sym *symtab;

int
main(int argc, char *argv[])
{
	int nwords;
	int i;

	if(argc != 2) {
		fprintf(stderr, "Usage: s20dis file\n");
		exit(1);
	}
	nwords = loadfile(argv[1]);

	decode(nwords);
	disassm(nwords);

	exit(0);
}

int
loadfile(char *fn)
{
	int fd, n, nword, i;
	unsigned char buf[NMEM * BPERW];
	unsigned char *p;
	unsigned long word;

	fd = open(fn, O_RDONLY);
	if(fd < 0) {
		perror("loadfile");
		exit(2);
	}
	n = read(fd, buf, NMEM * BPERW);
	close(fd);
	nword = n / BPERW;
	p = buf;
	for(i = 0; i < nword; i++) {
		word = *p++ << 16;
		word |= *p++ << 8;
		word |= *p++;
		mem[i].contents = word;
	}
	return nword;
}

void
decode(int n)
{
	int i, addr;
	Sym *p;

	for(i = 0; i < NMEM; i++) {
		mem[i].wtype = Unknown;
		mem[i].visited = 0;
	}
	for(i = 0; i < n; i++) {
		mem[i].opcode = (mem[i].contents & 0xf00000) >> 20;
		if(mem[i].opcode == 0) {
			mem[i].u.r.rA = (mem[i].contents & 0x0f8000) >> 15;
			mem[i].u.r.rB = (mem[i].contents & 0x007c00) >> 10;
			mem[i].u.r.rC = (mem[i].contents & 0x0003e0) >> 5;
			mem[i].u.r.subop = mem[i].contents & 0x1f;
		}
		else {
			mem[i].u.a.reg = (mem[i].contents & 0x0f8000) >> 15;
			addr = mem[i].contents & 0x7fff;
			mem[i].u.a.addr = addr;
			p = lookaddr(addr);
			if(p == NULL) {
				p = newsym(addr);
			}
			if(mem[i].opcode >= 1 && mem[i].opcode <= 2) {
				mem[addr].wtype = Data;
			}
			else if(mem[i].opcode >= 3 && mem[i].opcode <= 8) {
				mem[addr].wtype = Instr;
			}
		}
	}
}

void
dosubop(S20word *p)
{
	switch(p->u.r.subop) {
	case 0x00:
		printf("nop\n");
		break;
	case 0x01:
		printf("ldi\tr%d, r%d, r%d\n", p->u.r.rA, p->u.r.rB, p->u.r.rC);
		break;
	case 0x02:
		printf("sti\tr%d, r%d, r%d\n", p->u.r.rC, p->u.r.rA, p->u.r.rB);
		break;
	case 0x03:
		printf("add\tr%d, r%d, r%d\n", p->u.r.rA, p->u.r.rB, p->u.r.rC);
		break;
	case 0x04:
		printf("sub\tr%d, r%d, r%d\n", p->u.r.rA, p->u.r.rB, p->u.r.rC);
		break;
	case 0x05:
		printf("and\tr%d, r%d, r%d\n", p->u.r.rA, p->u.r.rB, p->u.r.rC);
		break;
	case 0x06:
		printf("or\tr%d, r%d, r%d\n", p->u.r.rA, p->u.r.rB, p->u.r.rC);
		break;
	case 0x07:
		printf("xor\tr%d, r%d, r%d\n", p->u.r.rA, p->u.r.rB, p->u.r.rC);
		break;
	case 0x08:
		printf("shl\tr%d, %d, r%d\n", p->u.r.rA, p->u.r.rB, p->u.r.rC);
		break;
	case 0x09:
		printf("sal\tr%d, %d, r%d\n", p->u.r.rA, p->u.r.rB, p->u.r.rC);
		break;
	case 0x0a:
		printf("shr\tr%d, %d, r%d\n", p->u.r.rA, p->u.r.rB, p->u.r.rC);
		break;
	case 0x0b:
		printf("sar\tr%d, %d, r%d\n", p->u.r.rA, p->u.r.rB, p->u.r.rC);
		break;
	case 0x10:
		printf("rts\n");
		break;
	case 0x1f:
		printf("halt\n");
		break;
	default:
		printf("Undefined\n");
		break;
	}
}

void
disassm(int n)
{
	int i;
	Sym *p;

	for(i = 0; i < n; i++) {
		printf("%04x: %06lx    ", i, mem[i].contents);
		p = lookaddr(i);
		if(p != NULL) {
			printf("%s", p->name);
		}
		printf("\t");
		if(mem[i].wtype == Data) {
			printf("data\t%06lx\n", mem[i].contents);
		}
		else {
			p = lookaddr(mem[i].u.a.addr);
			switch(mem[i].opcode) {
			case 0x0:
				dosubop(&mem[i]);
				break;
			case 0x1:
				if(p == NULL) {
					printf("ld\t%04x, r%d\n", mem[i].u.a.addr, mem[i].u.a.reg);
				}
				else {
					printf("ld\t%s, r%d\n", p->name, mem[i].u.a.reg);
				}
				break;
			case 0x2:
				if(p == NULL) {
					printf("st\tr%d, %04x\n", mem[i].u.a.reg, mem[i].u.a.addr);
				}
				else {
					printf("st\tr%d, %s\n", mem[i].u.a.reg, p->name);
				}
				break;
			case 0x3:
				if(p == NULL) {
					printf("br\t%04x\n", mem[i].u.a.addr);
				}
				else {
					printf("br\t%s\n", p->name);
				}
				break;
			case 0x4:
				if(p == NULL) {
					printf("bsr\t%04x\n", mem[i].u.a.addr);
				}
				else {
					printf("bsr\t%s\n", p->name);
				}
				break;
			case 0x5:
				if(p == NULL) {
					printf("brz\tr%d, %04x\n", mem[i].u.a.reg, mem[i].u.a.addr);
				}
				else {
					printf("brz\tr%d, %s\n", mem[i].u.a.reg, p->name);
				}
				break;
			case 0x6:
				if(p == NULL) {
					printf("bnz\tr%d, %04x\n", mem[i].u.a.reg, mem[i].u.a.addr);
				}
				else {
					printf("bnz\tr%d, %s\n", mem[i].u.a.reg, p->name);
				}
				break;
			case 0x7:
				if(p == NULL) {
					printf("brn\tr%d, %04x\n", mem[i].u.a.reg, mem[i].u.a.addr);
				}
				else {
					printf("brn\tr%d, %s\n", mem[i].u.a.reg, p->name);
				}
				break;
			case 0x8:
				if(p == NULL) {
					printf("bnn\tr%d, %04x\n", mem[i].u.a.reg, mem[i].u.a.addr);
				}
				else {
					printf("bnn\tr%d, %s\n", mem[i].u.a.reg, p->name);
				}
				break;
			default:
				printf("Undefined\n");
				break;
			}
		}
	}
}

Sym *
newsym(unsigned int addr)
{
	static int lnum = 1;
	Sym *n;

	n = malloc(sizeof(Sym));
	if(n == NULL) {
		perror("malloc");
		exit(3);
	}
	n->addr = addr;
	n->name = malloc(7);
	sprintf(n->name, "L%05d", lnum);
	lnum++;
	n->next = symtab;
	symtab = n;
	return n;
}

Sym *
lookaddr(unsigned int addr)
{
	Sym *p;

	for(p = symtab; p != NULL && p->addr != addr; p = p->next) ;
	return p;
}

Sym *
lookname(char *n)
{
	Sym *p;

	for(p = symtab; p != NULL && strcmp(n, p->name) != 0; p = p->next) ;
	return p;
}
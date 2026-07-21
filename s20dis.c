#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

enum {
	NMEM = 32768,
	BPERW = 3
};

int loadfile(char *);
void disassm(int);

unsigned long mem[NMEM];

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
		mem[i] = word;
	}
	return nword;
}

void
dosubop(unsigned long inst)
{
	int rA, rB, rC, subop;

	rA = (inst & 0x0f8000) >> 15;
	rB = (inst & 0x007c00) >> 10;
	rC = (inst & 0x0003e0) >> 5;
	subop = inst & 0x1f;
	switch(subop) {
	case 0x00:
		printf("nop\n");
		break;
	case 0x01:
		printf("ldi\tr%d, r%d, r%d\n", rA, rB, rC);
		break;
	case 0x02:
		printf("sti\tr%d, r%d, r%d\n", rC, rA, rB);
		break;
	case 0x03:
		printf("add\tr%d, r%d, r%d\n", rA, rB, rC);
		break;
	case 0x04:
		printf("sub\tr%d, r%d, r%d\n", rA, rB, rC);
		break;
	case 0x05:
		printf("and\tr%d, r%d, r%d\n", rA, rB, rC);
		break;
	case 0x06:
		printf("or\tr%d, r%d, r%d\n", rA, rB, rC);
		break;
	case 0x07:
		printf("xor\tr%d, r%d, r%d\n", rA, rB, rC);
		break;
	case 0x08:
		printf("shl\tr%d, %d, r%d\n", rA, rB, rC);
		break;
	case 0x09:
		printf("sal\tr%d, %d, r%d\n", rA, rB, rC);
		break;
	case 0x0a:
		printf("shr\tr%d, %d, r%d\n", rA, rB, rC);
		break;
	case 0x0b:
		printf("sar\tr%d, %d, r%d\n", rA, rB, rC);
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
	int opcode, reg, addr;

	for(i = 0; i < n; i++) {
		printf("%04x: %06lx    ", i, mem[i]);
		opcode = (mem[i] & 0xf00000) >> 20;
		reg = (mem[i] & 0x0f8000) >> 15;
		addr = mem[i] & 0x007fff;
		switch(opcode) {
		case 0x0:
			dosubop(mem[i]);
			break;
		case 0x1:
			printf("ld\t%04x, r%d\n", addr, reg);
			break;
		case 0x2:
			printf("st\tr%d, %04x\n", reg, addr);
			break;
		case 0x3:
			printf("br\t%04x\n", addr);
			break;
		case 0x4:
			printf("bsr\t%04x\n", addr);
			break;
		case 0x5:
			printf("brz\tr%d, %04x\n", reg, addr);
			break;
		case 0x6:
			printf("bnz\tr%d, %04x\n", reg, addr);
			break;
		case 0x7:
			printf("brn\tr%d, %04x\n", reg, addr);
			break;
		case 0x8:
			printf("bnn\tr%d, %04x\n", reg, addr);
			break;
		default:
			printf("Undefined\n");
			break;
		}
	}
}
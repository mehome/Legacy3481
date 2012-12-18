#include <stdio.h>

class mainobject {
public:
	int captain;
	mainobject();
	~mainobject();
	};

class part1 : public mainobject {
public:
	part1();
	~part1();
	void dothis();
	};

class part2 : public part1 {
public:
	part2();
	~part2();
	};


mainobject::mainobject() {
	printf("init main\n");
	}
mainobject::~mainobject() {
	printf("destruct main\n");
	}

part1::part1() {
	printf("init part1\n");
	captain=5;
	}
void part1::dothis() {
	printf("DoThis\n");
	}
part1::~part1() {
	printf("destruct part1\n");
	}

part2::part2() {
	printf("init part2 %d\n",captain);
	}
part2::~part2() {
	printf("destruct part2\n");
	}

static part2 myobject;

void main() {
	printf("hello\n");
	myobject.dothis();
	}

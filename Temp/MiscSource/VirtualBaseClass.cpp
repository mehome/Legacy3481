#include <stdio.h>

class glue {
public:
	glue();
	~glue();
	int captain;
	};

class part1 : virtual public glue {
public:
	part1();
	~part1();
	void dothis();
	};

class part2 : virtual public glue {
public:
	part2();
	~part2();
	};

class mainobject : public part1,part2 {
public:
	mainobject();
	~mainobject();
	};

glue::glue() {
	printf("init glue\n");
	}
glue::~glue() {
	printf("destruct glue\n");
	}

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

static mainobject myobject;

void main() {
	printf("hello\n");
	myobject.dothis();
	}

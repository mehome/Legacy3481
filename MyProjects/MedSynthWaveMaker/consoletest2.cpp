#include <stdlib.h>         /* For _MAX_PATH definition */
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

class Child_interface {
	public:
		virtual void test()=0;
	};

class Parent_interface {
	public:
		virtual Child_interface *GetChildInterface()=0;
	};


class Child : public Child_interface {
	public:

		Child(Parent_interface *parent) : m_Parent(parent) {
			assert (parent);
			printf("Child\n");
			}

		virtual void test() {
			printf("%x\n",m_Parent);
			}
	private:
		Parent_interface * const m_Parent;
	};

class ChildInherited : public Child {
	public:
		ChildInherited(Parent_interface *parent) : Child(parent) {
			printf("ChildInherited\n");
			}
	};

class Parent : public Parent_interface {
	public:
		Parent() {m_child=NULL;}
		~Parent() {if (m_child) free(m_child);}
		virtual Child_interface *GetChildInterface() {
			if (!m_child) m_child=new ChildInherited(this);
			return m_child;
			}
	private:
	Child_interface *m_child;
	};


void main() {
	Parent test;
	Child_interface *childtest=test.GetChildInterface();
	if (childtest) {
		childtest->test();
		}
	}


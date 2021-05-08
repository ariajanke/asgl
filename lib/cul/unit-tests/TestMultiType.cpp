/****************************************************************************

    File: TestMultiType.cpp
    Author: Aria Janke
    License: GPLv3

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#include <common/MultiType.hpp>
#include <common/TestSuite.hpp>

#include <iostream>
#include <variant>

#include <cassert>

class Base {
public:
	virtual ~Base();
};

class A final : public Base {
public:
	A() { ++s_count; }
	A(const A  &) { ++s_count; }
	A(      A &&) { ++s_count; }
	A & operator = (const A  &) { return *this; }
	A & operator = (      A &&) { return *this; }
	~A() override { --s_count; }
	static int get_count() { return s_count; }
	static void reset_count() { s_count = 0; }
private:
	static int s_count;
};

class SideBase {
public:
	~SideBase();
};

class B final : public Base {
public:
};

class C final : public Base, public SideBase {
public:
};

/* static */ int A::s_count = 0;

Base::~Base() {}

SideBase::~SideBase() {}

static_assert(TypeList<A, B, C, int>::HasType<int>::k_value, "");
static_assert(TypeList<A, B, C, int>::HasType<  A>::k_value, "");
static_assert(TypeList<A, B, C, int>::HasType<  B>::k_value, "");
static_assert(TypeList<A, B, C, int>::HasType<  C>::k_value, "");

using ABCTypeList = TypeList<A, B, C>;

class D {};
class E {};
using ABCDETypeList = ABCTypeList::CombineWith<D, E>;

int main() {
    {
    using MyVariant = TypeList<A, B, C>::DefineWithListTypes<std::variant>::Type;
    MyVariant var { A() };
    assert(std::holds_alternative<A>(var));
    }
	// assert fails: the tests/utils for the tests are badly designed
	// test on the ts fails: the MultiType class has the issue
	using TestMt = MultiType<int, double, A, B>;
	
	ts::TestSuite suite;
	suite.start_series("MultiType");
	
	// there's some assumption here that type_id will work...
	// default constructor
	suite.test([]() {
		TestMt a;
		return ts::test(a.type_id() == TestMt::k_no_type);
	});
	// value constructor
	suite.test([]() {
		A a;
		TestMt b(a);
		return ts::test(A::get_count() == 2);
	});
	A::reset_count();
	
	// copy constructor
	suite.test([]() {
		A a;
		TestMt b(a);
		TestMt c(b);
		return ts::test(A::get_count() == 3);
	});
	A::reset_count();
	
	// destructor
	suite.test([]() {
		A c;
		{
		A a;
		TestMt b(a);
		}
		return ts::test(A::get_count() == 1);
	});
	A::reset_count();
	
	// as_pointer (three cases, one const)
	suite.test([]() {
		TestMt a;
		return ts::test(!a.as_pointer<int>());
	});
	suite.test([]() {
		int a = 10;
		TestMt b(a);
		return ts::test(b.as_pointer<int>());
	});
	suite.test([]() {
		B a;
		TestMt b(a);
		const TestMt & c = b;
		return ts::test(!c.as_pointer<int>());
	});
	// as (two cases, one throws)
	suite.test([]() {
		int t = 10;
		TestMt b(t);
		return ts::test(b.as<int>() == 10);
	});
	suite.test([]() {
		bool did_throw = false;
		try {
			TestMt a;
			a.as<int>() = 10;
		} catch (...) {
			did_throw = true;
		}
		return ts::test(did_throw);
	});
	// reset
	suite.test([]() {
		B a;
		TestMt b(a);
		b.reset<int>(10);
		return ts::test(b.as<int>() == 10);
	});
	// unset
	suite.test([]() {
		A a;
		TestMt b(a);
		b.unset();
		return ts::test(A::get_count() == 1);
	});
	A::reset_count();
	
	// is_type
	suite.test([]() {
		A a;
		TestMt b(a);
		return ts::test(b.is_type<A>());
	});
	suite.test([]() {
		TestMt b;
		return ts::test(!b.is_type<A>());
	});
	// is_valid (two cases)
	suite.test([]() {
		A a;
		TestMt b(a);
		return ts::test(b.is_valid());
	});
	suite.test([]() {
		TestMt b;
		return ts::test(!b.is_valid());
	});
	// C++ won't let you static_cast from a class to an int (without 
	// operators at least)
	// set_by_type_id_and_upcast (two cases: A, and B)
	suite.test([]() {
		TestMt a;
		auto gv = a.set_by_type_id_and_upcast<Base>
			(TestMt::GetTypeId<A>::k_value);
		return ts::test(   gv.object_pointer && gv.upcasted_pointer
		                && A::get_count() == 1);
	});
	A::reset_count();
	
	suite.test([]() {
		// an upcast fails (returns nullptr) if the leaf type is not 
		// derived from Base, a bit different from static_cast
		// this allows that MultiType to exist where not all types have
		// have to inherit from a common base
		TestMt a;
		auto gv = a.set_by_type_id_and_upcast<Base>
			(TestMt::GetTypeId<int>::k_value);
		return ts::test(gv.object_pointer && !gv.upcasted_pointer);
	});
	// get_by_type_id_and_upcast
	// match ok
	suite.test([]() {
		B a;
		TestMt b(a);
		auto gv = b.get_by_type_id_and_upcast<Base>(TestMt::GetTypeId<B>::k_value);
		return ts::test(gv.object_pointer && gv.upcasted_pointer);
	});
	// I'm not sure what the proper thing to do here is... so I'm leaving the
	// previous behavior in place
#	if 0
	// type mismatch
	suite.do_test([]() {
		TestMt b(int(1));
		auto gv = b.get_by_type_id_and_upcast<Base>(TestMt::GetTypeId<B>::k_value);
		return ts::test(!gv.object_pointer && !gv.upcasted_pointer);
	});
#	endif
	// cast to base fails
	suite.test([]() {
		B a;
		TestMt b(a);
		auto gv = b.get_by_type_id_and_upcast<int>(TestMt::GetTypeId<B>::k_value);
		return ts::test(gv.object_pointer && !gv.upcasted_pointer);
	});
	// match ok, constant
	suite.test([]() {
		B a;
		TestMt b(a);
		const auto & c = b;
		
		auto gv = c.get_by_type_id_and_upcast<Base>(TestMt::GetTypeId<B>::k_value);
		return ts::test(gv.object_pointer && gv.upcasted_pointer);
	});
	// not sure how useful dynamic_cast_ and static_cast_ are for 
	// MultiType
	// I can think of a single instance...?
#	if 0
	// dynamic_cast_ (cross-cast with pointee, with null, to unrelated (null))
	suite.do_test([]() {
		TestMt b;
		b.reset<C>();
		return ts::test(false);
	});
	suite.do_test([]() {
		return ts::test(false);
	});
	suite.do_test([]() {
		return ts::test(false);
	});
#	endif
	// static_cast_ ???
	// test copying uninitialized MultiType (on that stores nothing) on to one with
	// an object on it
	// expected result: the second multitype should become "unset"
	suite.test([]() {
		TestMt a;
		TestMt c(a);
		return ts::test(!c.is_valid());
	});

    return 0;
}

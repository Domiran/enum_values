#include "enum_values.h"
#include <assert.h>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>

// these values are used by those values, top kek!
enum class myvalues
{
	zero = 0, // nope
	one = 1,
	two = 2,

	// this is not the world we want to save!
	three = 3,
	also_three = three, //yay?
	four = 4, //  ahh!
	five = 5
};

enum class yourvalues : int
{
	zero, // nope
	one,
	two,
	also_two = two, // does it work?
	three, // ah!!
	four,
	five,
};

enum class theirvalues : int //ns@ourvalues
{
	zero, // nope
	one,
	two,
	also_two = two, // does it work?
	three, // ah!!
	four,
	five,
};

void that(myvalues v)
{
	v = myvalues::also_three;
}

int main()
{
	enum_value<myvalues> val1;
	enum_value<myvalues> val2;

	val1 = myvalues::two;
	val1.data() = 10;
	val1.clear();
	auto ok = (val1.data() == 5);
	ok = (val1 == val2);
	ok = (val1 == myvalues::three);
	val1 = val2;
	val2.data() = 5;
	*val1 = myvalues::three;

	auto c = val1.data();
	std::cout << c << std::endl;

	for (auto v : enum_static<myvalues>::iterable_by_name())
	{
		std::cout << v.second << std::endl;
	}

	val1.from_string("zero");
	std::cout << val1.to_string() << std::endl;

	auto unknowns = val1.from_flag_string("one!+!two!+!four", "!+!");
	assert(unknowns.empty());

	unknowns = val1.from_flag_string("one+two+four", "!+!");
	assert(unknowns == "one+two+four");

	unknowns = val1.from_flag_string("eight+sixteen+two+four+thirty-two", "+");
	assert(unknowns == "eight+sixteen+thirty-two");

	std::cout << val1.to_flag_string("+") << std::endl;

	that(*val1);

	return 0;
}

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

namespace espace
{
	enum class theirvalues : int //ns@espace
	{
		zero, // nope
		one,
		two,
		also_two = two, // does it work?
		three, // ah!!
		four,
		five,
	};
}

void foo(myvalues v)
{
	v = myvalues::also_three; // this line brought to you by compiler warnings
}

int main()
{
	// get description
	std::cout << enum_static<myvalues>::description() << std::endl;
	std::cout << enum_static<espace::theirvalues>::description() << std::endl;

	// using the enum_value class to hold values (useful for flags
	enum_value<myvalues> val1;
	enum_value<myvalues> val2;

	// reset to 0
	val1.clear();

	// assign to enum
	val1 = myvalues::two;

	// assignment by data()
	val1.data() = 10;
	val2.data() = 5;

	// assignment by enum directly
	*val1 = myvalues::three;

	auto ok = (val1.data() == 5);

	// comparison of two enum_values
	ok = (val1 == val2);

	// comparison by enum
	ok = (val1 == myvalues::three);

	// enum_value asignment
	val1 = val2;

	auto c = val1.data();
	std::cout << c << std::endl;

	// iterate over full list of values
	for (auto v : enum_static<myvalues>::iterable_by_names())
	{
		std::cout << v.second << std::endl;
	}

	// assign by string
	val1.from_string("zero");
	std::cout << val1.to_string() << std::endl;

	// assign by string flag
	auto unknowns = val1.from_flag_string("one!+!two!+!four", "!+!");
	assert(unknowns.empty());

	// detect unknown flag text
	unknowns = val1.from_flag_string("one+two+four", "!+!");
	assert(unknowns == "one+two+four");

	unknowns = val1.from_flag_string("eight+sixteen+two+four+thirty-two", "+");
	assert(unknowns == "eight+sixteen+thirty-two");

	// to flag string
	std::cout << val1.to_flag_string("+") << std::endl;

	// pass enum direct
	foo(*val1);

	return 0;
}

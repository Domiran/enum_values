# enum_values
Yet another post-build step and class to bring reflection to C++ enumerations!

*Fair warning: this project has not undergone much testing (yet?). I have also not used it in a project (also yet?).*

# Description

This project consists of two parts: a pre-build step called `enum_reader` and two classes: `enum_value` and `enum_static`.

Given the following enumeration, the reader will parse out the enumeration and create a file name in a location of your choosing with the values and their names.

```cpp
namespace MyNamespace
{
  // an enumeration description!
  enum class theirvalues : int //ns@MyNamespace
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
```

It does this by creating a compilable cpp file intended to be included directly in your application that are linked to the `enum_value` and `enum_static` classes. Forward declarations of all enums it captures are included. Which one you use ~~really depends if you prefer a little C# in your life~~ is a matter of preference.

Unlike some other enum reflection solutions, this does not require much/any editing of your existing code and has no size or syntax limitations. (However, its effectiveness is entirely dependent on the parser. The initial release does not have a very robust C++ syntax parser.)

# Example

The above `enum` will result in a file with the following contents:

```cpp
namespace MyNamespace
{
	enum class theirvalues : int;
}
const std::string enum_static<espace::theirvalues>::enum_desc = "";
const std::unordered_map<int, std::string> enum_static<MyNamespace::theirvalues>::value_to_name = {
	{ 0, "zero" },
	{ 1, "one" },
	{ 2, "two" },
	{ 2, "also_two" },
	{ 3, "three" },
	{ 4, "four" },
	{ 5, "five" },
};
const std::unordered_map<std::string, int> enum_static<MyNamespace::theirvalues>::name_to_value = {
	{ "zero", 0 },
	{ "one", 1 },
	{ "two", 2 },
	{ "also_two", 2 },
	{ "three", 3 },
	{ "four", 4 },
	{ "five", 5 },
};
```

# Usage

There are two methods of using *enum_values*: the static `enum_static` class or the enum wrapper `enum_value`.

`enum_static` provides static versions of common enum operations:
* `has_flag`
* `flag_set`
* `flag_remove`
* `flag_toggle`
* `from_string`
* `to_string`
* `to_flag_string`
* `from_flag_string`
* `iterable_by_values`
* `iterable_by_name`

`enum_value` contains the above functions as members, as well as:
* Enum value and underlying type constructor
* `enum_value` assignment and equality
* Postfix and prefix increment and decrement
* Overloaded `operator*`, returning enum value (non-const overload allows value to be directly changed)
* `data()`, returning the value of the enum's underlying type (non-const overload allows value to be directly changed)

```cpp
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

// get description
std::cout << enum_static<myvalues>::description() << std::endl;
std::cout << enum_static<espace::theirvalues>::description() << std::endl;

// using the enum_value class to hold values (useful for flags)
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
```


# Limitations

The parser currently has the following limitations:
* Any and all classes and namespaces required for a forward declaration are ignored. (Provide this information manually with the `//@ns` comment in the format shown.)
* An alias will return the first value seen in the enum definition.
* Only single-term expressions will be evaluated. `two_and_three = two | three` will currently set the value to `-1`.

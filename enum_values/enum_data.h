template<>
class enum_data<tester_ns::myvalues>
{
public:
	static constexpr auto get_data()
	{
		constexpr std::array<data_pair<tester_ns::myvalues>, 7> d = { {
			{ tester_ns::myvalues::zero, std::string_view("zero") },
			{ tester_ns::myvalues::one, std::string_view("one") },
			{ tester_ns::myvalues::two, std::string_view("two") },
			{ tester_ns::myvalues::three, std::string_view("three") },
			{ tester_ns::myvalues::also_three, std::string_view("Also Three") },
			{ tester_ns::myvalues::four, std::string_view("four") },
			{ tester_ns::myvalues::five, std::string_view("five") },
		} };
		return d;
	}

	static constexpr auto get_desc()
	{
		return std::string("these values are used by those values, top kek!");
	}
};

template<>
class enum_data<tester_ns::yourvalues>
{
public:
	static constexpr auto get_data()
	{
		constexpr std::array<data_pair<tester_ns::yourvalues>, 7> d = { {
			{ tester_ns::yourvalues::zero, std::string_view("zero") },
			{ tester_ns::yourvalues::one, std::string_view("one") },
			{ tester_ns::yourvalues::two, std::string_view("two") },
			{ tester_ns::yourvalues::also_two, std::string_view("also_two") },
			{ tester_ns::yourvalues::three, std::string_view("three") },
			{ tester_ns::yourvalues::four, std::string_view("four") },
			{ tester_ns::yourvalues::five, std::string_view("five") },
		} };
		return d;
	}

	static constexpr auto get_desc()
	{
		return std::string("this is a longer comment with lots of description !");
	}
};

template<>
class enum_data<tester_ns::espace::myclass::theirvalues>
{
public:
	static constexpr auto get_data()
	{
		constexpr std::array<data_pair<tester_ns::espace::myclass::theirvalues>, 7> d = { {
			{ tester_ns::espace::myclass::theirvalues::zero, std::string_view("zero") },
			{ tester_ns::espace::myclass::theirvalues::one, std::string_view("one") },
			{ tester_ns::espace::myclass::theirvalues::two, std::string_view("two") },
			{ tester_ns::espace::myclass::theirvalues::also_two, std::string_view("also_two") },
			{ tester_ns::espace::myclass::theirvalues::three, std::string_view("three") },
			{ tester_ns::espace::myclass::theirvalues::four, std::string_view("four") },
			{ tester_ns::espace::myclass::theirvalues::five, std::string_view("five") },
		} };
		return d;
	}

	static constexpr auto get_desc()
	{
		return std::string("");
	}
};


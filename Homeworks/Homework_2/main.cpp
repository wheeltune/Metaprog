#include <iostream>

namespace NTypeList {

//////////////////////////////////////////////////////////

template<typename ... Args>
struct TTypeList {};

template<typename Head, typename ... Tail>
struct TTypeList<Head, Tail...> {
    typedef Head head;
    typedef TTypeList<Tail...> tail;
};

//////////////////////////////////////////////////////////

template<typename TypeList>
struct Length
    : std::integral_constant<
        unsigned int, 
        Length<typename TypeList::tail>::value + 1
    >
{};

template<>
struct Length<TTypeList<>>
    : std::integral_constant<unsigned int, 0> {
};

//////////////////////////////////////////////////////////

template<typename Head, typename TypeList>
struct Insert {};

template<typename Head, typename ... Args>
struct Insert<Head, TTypeList<Args...>> {
    typedef TTypeList<Head, Args...> result;
};

//////////////////////////////////////////////////////////

template<int Position, typename TypeList>
struct Split {
    typedef typename Insert<
        typename TypeList::head, 
        typename Split<
            Position - 1, 
            typename TypeList::tail
        >::left
    >::result left;

    typedef typename Split<
        Position - 1, 
        typename TypeList::tail
    >::right right;
};

template<typename TypeList>
struct Split<0, TypeList> {
    typedef TTypeList<> left;
    typedef TypeList right;
};

template<int Position>
struct Split<Position, TTypeList<>> {
    typedef TTypeList<> left;
    typedef TTypeList<> right;
};

} // NTypeList

namespace NFibonachi {

//////////////////////////////////////////////////////////

template<int Number>
struct Fibonachi {
    static const int value = Fibonachi<Number - 1>::value + Fibonachi<Number - 2>::value;
};

template<>
struct Fibonachi<1> {
    static const int value = 1;
};

template<>
struct Fibonachi<2> {
    static const int value = 1;
};

} // NFibonachi

namespace {

using namespace NTypeList;
using namespace NFibonachi;

//////////////////////////////////////////////////////////

template<typename TypeList>
struct LinearHierarchy
    : public TypeList::head
    , public LinearHierarchy<typename TypeList::tail>
{
    typedef typename TypeList::head head;
    typedef LinearHierarchy<typename TypeList::tail> next;
};

template<>
struct LinearHierarchy<TTypeList<>> {};

//////////////////////////////////////////////////////////

template<int Number, typename TypeList>
struct FibonachiHierarchyInternal
    : public LinearHierarchy<
        typename Split<
            Fibonachi<Number>::value + 1, 
            TypeList
        >::left
    >
    , public FibonachiHierarchyInternal<
        Number + 1, 
        typename Split<
            Fibonachi<Number>::value + 1, 
            TypeList
        >::right
    > 
{
    typedef LinearHierarchy<
        typename Split<
            Fibonachi<Number>::value + 1, 
            TypeList
        >::left
    > first;

    typedef FibonachiHierarchyInternal<
        Number + 1, 
        typename Split<
            Fibonachi<Number>::value + 1, 
            TypeList
        >::right
    > next;
};

template<int Number>
struct FibonachiHierarchyInternal<Number, TTypeList<>> {};

template<typename TypeList>
struct FibonachiHierarchy
    : public FibonachiHierarchyInternal<1, TypeList> 
{};

} // namespace

template<int Id>
struct Test {
    static const int id = Id;
};


int main() {
    typedef FibonachiHierarchy<TTypeList<Test<0>, Test<1>, Test<2>, Test<3>, Test<4>, Test<5>, Test<6>, Test<7>>> test;

    std::cout << test::first::head::id << std::endl;
    std::cout << test::first::next::head::id << std::endl;

    std::cout << std::endl;

    std::cout << test::next::first::head::id << std::endl;
    std::cout << test::next::first::next::head::id << std::endl;

    std::cout << std::endl;

    std::cout << test::next::next::first::head::id << std::endl;
    std::cout << test::next::next::first::next::head::id << std::endl;
    std::cout << test::next::next::first::next::next::head::id << std::endl;

    std::cout << std::endl;

    std::cout << test::next::next::next::first::head::id << std::endl;
}

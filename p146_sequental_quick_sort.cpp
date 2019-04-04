#include <list>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <random>

using namespace std;

template <typename T>
list<T> sequental_quick_sort(list<T> input) {
    if (input.empty()) { return input; }
    list<T> result;

    // move the first input's element into the result
    result.splice(begin(result), input, begin(input));

    const T & pivot = *result.begin();
    auto divide_point = partition(begin(input), end(input),
            [&](const T & t){ return t < pivot; });

    list<T> lower_part;
    lower_part.splice(end(lower_part), input, begin(input), divide_point);

    auto new_lower{  sequental_quick_sort(move(lower_part)) };
    auto new_higher{ sequental_quick_sort(move(input)) };

    result.splice(  end(result), new_higher);
    result.splice(begin(result), new_lower );

    return result;
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(1, 1000);

    list<int> lst;
    for (auto i = 0; i < 1000; i++) {
        lst.push_back(distr(gen));
    }

    copy(begin(lst), end(lst), ostream_iterator<int>(cout, " "));
    cout << endl;
    lst = sequental_quick_sort(move(lst));
    copy(begin(lst), end(lst), ostream_iterator<int>(cout, " "));
    cout << endl;
}

#include "test_runner.h"
#include "iterator_range.h"

#include <numeric>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

template<typename Iterator>
class Paginator {
public:
	Paginator(Iterator begin, Iterator end, size_t page_size) {
		for (size_t left = distance(begin, end); left > 0;) {
			size_t current_page_size = min(page_size, left);
			Iterator current_page_end = next(begin, current_page_size);
			pages.push_back( { begin, current_page_end });

			left -= current_page_size;
			begin = current_page_end;
		}
	}
	auto begin() const {
		return pages.begin();
	}
	auto end() const {
		return pages.end();
	}
	size_t size() const {
		return pages.size();
	}
private:
	vector<IteratorRange<Iterator>> pages;
};

template<typename C>
auto Paginate(C &c, size_t page_size) {
	return Paginator(c.begin(), c.end(), page_size);
}

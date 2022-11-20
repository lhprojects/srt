#ifndef SRT_ITER_H
#define SRT_ITER_H
#include<iterator>

namespace srt {
	template<class T>
	struct Iter {
		virtual void next() = 0;
		virtual bool end() = 0;
		virtual T get() = 0;
	};

	template<class IT>
	struct Range : Iter<
		typename std::iterator_traits<IT>::value_type > {

		template<class R>
		Range(R& r) : fB(r.begin()), fE(r.end()) {}

		template<class IT>
		Range(IT b, IT e) : fB(b), fE(e)
		{
		}

		bool end() override;
		void next() override;
		typename std::iterator_traits<IT>::value_type
			get() override;
	private:
		IT fB;
		IT fE;
	};


	template<class IT>
	bool Range<IT>::end()
	{
		return fB == fE;
	}

	template<class IT>
	void Range<IT>::next()
	{
		++fB;
	}

	template<class IT>
	typename std::iterator_traits<IT>::value_type
		Range<IT>::get()
	{
		return *fB;
	}

	// return an Iter(Range) from c++ style range
	template<class R>
	auto range(R&& r)
		->Range<decltype(std::forward<R>(r).begin())>
	{
		using IT = decltype(std::forward<R>(r).begin());
		return Range<IT>{ r };
	}
}
#endif

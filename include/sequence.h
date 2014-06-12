#ifndef SEQUENCE_H
#define SEQUENCE_H

namespace detail {
//Stores an integer sequence
template<int...>
struct Sequence{};
/*
 * Generate an integer sequence from [0, N-1], accessed through
 * GenSequence::seq
 */
template<int N, int... S>
struct GenSequence : GenSequence<N - 1, N - 1, S...>{};
template<int... S>
struct GenSequence<0, S...>{
	typedef Sequence<S...> seq;
};
}

#endif


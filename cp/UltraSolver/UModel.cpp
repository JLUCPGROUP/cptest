#pragma once
#include "UModel.h"

namespace cp {

	UVar::UVar(HVar* v) :
		id(v->id),
		capacity(v->vals.size()),
		limit(capacity & MOD_MASK),
		num_bit(ceil(float(capacity) / BITSIZE)) {

	}


	void UVar::clear_marks() {
		mark_ = 0;
	}

	void UVar::restrict()	{
		size_ = mark_;
	}

	void UVar::mark(const int a) {
		if (map_[a] < size_&&map_[a] >= mark_) {
			swap(map_[a], mark_);
			++mark_;
		}
	}

	inline void UVar::swap(const int i, const int j) {
		SWAP(dom_[i], dom_[j]);
		map_[dom_[i]] = i;
		map_[dom_[j]] = j;
	}

	void UVar::remove_value(const int a) {
		if (map_[a] < size_) {
			swap(map_[a], size_ - 1);
		}
	}

	void UVar::reduece_to(const int a) {
		if (map_[a] > size_) {
			size_ = 0;
		}
		else {
			swap(map_[a], 0);
			size_ = 1;
		}
	}

	bool UVar::have(const int a) {
		return map_[a] < size_;
	}



}
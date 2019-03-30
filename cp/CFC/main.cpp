// testbrs.cpp : Defines the entry point for the console application.
//

#include <iostream> 
#include <cstdlib>
#include <ctime>
#include <random>
#include "../includes/Timer.h"
//#include <intrin.h>
#include <future>
using namespace std;
typedef unsigned long long u64;
int f1(u64 num);
int f2(u64 num);
int f3(u64 num);
int f4(u64 num);
int f5(u64 num);
int f6(u64 num);

void correctness_test();
void performance_test();
void prepare_test_data(u64 data[], int size);

int main() {
	correctness_test();
	performance_test();
	return 0;
}

int f1(u64 num) {
	int count = 0;
	while (num) {
		if (num & 1) ++count;
		num >>= 1;
	}
	return count;
}

int f2(u64 num) {
	const u64 M1 = 0x5555555555555555;
	const u64 M2 = 0x3333333333333333;
	const u64 M4 = 0x0F0F0F0F0F0F0F0F;
	const u64 M8 = 0x00FF00FF00FF00FF;
	const u64 M16 = 0x0000ffff0000ffff;
	const u64 M32 = 0xFFFFFFFF;

	num = (num & M1) + ((num >> 1) & M1);
	num = (num & M2) + ((num >> 2) & M2);
	num = (num & M4) + ((num >> 4) & M4);
	num = (num & M8) + ((num >> 8) & M8);
	num = (num & M16) + ((num >> 16) & M16);
	num = (num & M32) + ((num >> 32) & M32);

	return num;
}

int f3(u64 num) {
	int count = 0;
	while (num) {
		num &= (num - 1);
		++count;
	}
	return count;
}

//int f4(u64 num) {
//	int count = 0;
//	while (num) {
//		int n;
//		__asm {
//			bsr eax, num
//			mov n, eax
//		}
//		++count;
//		num ^= (1 << n);
//	}
//	return count;
//}

//int f4(u64 num) {
//	static const signed char TABLE[256] = {
//		0, 1, 1, 2, 1, 2, 2, 3,	1, 2, 2, 3, 2, 3, 3, 4,
//		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
//		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
//		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
//		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
//		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
//		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
//		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
//		1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
//		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
//		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
//		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
//		2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
//		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
//		3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
//		4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
//	};
//
//	int count = 0;
//	//unsigned char* p = reinterpret_cast<unsigned char*>(&num);
//	//while(p != reinterpret_cast<unsigned char*>(&num + 1)) {
//	//	count += TABLE[*p++];		
//	//}
//	count = TABLE[num & 0xff] + TABLE[(num >> 8) & 0xff] + TABLE[(num >> 16) & 0xff] + TABLE[(num >> 24) & 0xff]
//		+ TABLE[(num >> 32) & 0xff] + TABLE[(num >> 40) & 0xff] + TABLE[(num >> 48) & 0xff] + TABLE[(num >> 56) & 0xff];
//	return count;
//}

int f4(u64 num) {
	const char *const _Bitsperbyte =
		"\0\1\1\2\1\2\2\3\1\2\2\3\2\3\3\4"
		"\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
		"\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
		"\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		"\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
		"\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		"\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		"\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
		"\1\2\2\3\2\3\3\4\2\3\3\4\3\4\4\5"
		"\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		"\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		"\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
		"\2\3\3\4\3\4\4\5\3\4\4\5\4\5\5\6"
		"\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
		"\3\4\4\5\4\5\5\6\4\5\5\6\5\6\6\7"
		"\4\5\5\6\5\6\6\7\5\6\6\7\6\7\7\x8";
	const unsigned char *_Ptr = &reinterpret_cast<const unsigned char&>(num);
	const unsigned char *const _End = _Ptr + sizeof(num);
	size_t _Val = 0;
	for (; _Ptr != _End; ++_Ptr)
		_Val += _Bitsperbyte[*_Ptr];
	return (_Val);
}

struct _byte {
	unsigned a : 1;
	unsigned b : 1;
	unsigned c : 1;
	unsigned d : 1;
	unsigned e : 1;
	unsigned f : 1;
	unsigned g : 1;
	unsigned h : 1;
};

inline int get_bit_count(unsigned char b) {
	struct _byte *by = (struct _byte*)&b;
	return (by->a + by->b + by->c + by->d + by->e + by->f + by->g + by->h);
}

int f5(u64 num) {
	int count = 0;
	count = get_bit_count(num & 0xff) + get_bit_count((num >> 8) & 0xff) +
		get_bit_count((num >> 16) & 0xff) + get_bit_count((num >> 24) & 0xff) +
		get_bit_count((num >> 32) & 0xff) + get_bit_count((num >> 40) & 0xff) +
		get_bit_count((num >> 48) & 0xff) + get_bit_count((num >> 56) & 0xff);
	return count;
}

//int f6(u64 num) {
//	u64 uCount;
//	uCount = num - ((num >> 1) & 03333333333333333333333) - ((num >> 2) & 01111111111111111111111);
//	return ((uCount + (uCount >> 3)) & 1070707070707070707070) % 63;
//}

int f6(u64 num) {
	return __popcnt64(num);
}


int f7(u64 i) {
	i = i - ((i >> 1) & 0x5555555555555555L);
	i = (i & 0x3333333333333333L) + ((i >> 2) & 0x3333333333333333L);
	i = (i + (i >> 4)) & 0x0f0f0f0f0f0f0f0fL;
	i = i + (i >> 8);
	i = i + (i >> 16);
	i = i + (i >> 32);
	return i & 0x7f;
}

int f8(u64 num) {
	return _mm_popcnt_u64(num);
}

void correctness_test() {
	u64 test_data[] = { 0, 1, 2, 3, 0x01234567, 0x0809a0b0c0d00e0f, 0xf0f0f0f0f0f0f0f0 };
	u64 corect_result[] = { 0, 1, 1, 2, 12, 20, 32 };

	int(*fn[])(u64) = { f1, f2, f3, f4, f5, f6, f7 ,f8 };
	for (int i = 0; i < sizeof(fn) / sizeof(*fn); ++i) {
		for (int j = 0; j < sizeof(test_data) / sizeof(*test_data); ++j) {
			if (fn[i](test_data[j]) != corect_result[j]) {
				cout << "f" << (i + 1) << " failed!" << endl;
				exit(-1);
			}
		}
	}
	cout << "All methods passed correctness test." << endl;
}

void performance_test() {
	const int TEST_DATA_SIZE = 100000000;
	u64* test_data = new u64[TEST_DATA_SIZE];
	cp::Timer t;
	prepare_test_data(test_data, TEST_DATA_SIZE);
	cout << t.elapsed() << endl;
	int(*fn[])(u64) = { f1, f2, f3,f4, f5, f6, f7,f8 };

	for (int i = 0; i < sizeof(fn) / sizeof(*fn); ++i) {
		clock_t start = clock();
		for (int j = 0; j < TEST_DATA_SIZE; ++j) {
			fn[i](test_data[j]);
		}
		int ticks = clock() - start;
		double seconds = ticks * 1.0 / CLOCKS_PER_SEC;
		cout << t.elapsed() << endl;
		cout << "f" << (i + 1) << " consumed " << seconds << " seconds." << endl;
	}
	cout << t.elapsed() << endl;
	delete[] test_data;
}

void prepare_test_data(u64* data, int len) {

	std::random_device rd;
	const uniform_int_distribution<u64> dist(0, ULLONG_MAX);
	for (int i = 0; i < len; ++i) {
		data[i] = dist(rd);
	}
}
//// testbrs.cpp : Defines the entry point for the console application.
////
//
//#include <iostream> 
//#include <cstdlib>
//#include <ctime>
//using namespace std;
//
//int f1(unsigned int num);
//int f2(unsigned int num);
//int f3(unsigned int num);
//int f4(unsigned int num);
//int f5(unsigned int num);
//int f6(unsigned int num);
//
//void correctness_test();
//void performance_test();
//void prepare_test_data(unsigned int data[], int size);
//
//int main() {
//	correctness_test();
//	performance_test();
//	return 0;
//}
//
//int f1(unsigned int num) {
//	int count = 0;
//	while (num) {
//		if (num & 1) ++count;
//		num >>= 1;
//	}
//	return count;
//}
//
//int f2(unsigned int num) {
//	const unsigned int M1 = 0x55555555;
//	const unsigned int M2 = 0x33333333;
//	const unsigned int M4 = 0x0f0f0f0f;
//	const unsigned int M8 = 0x00ff00ff;
//	const unsigned int M16 = 0x0000ffff;
//
//	num = (num & M1) + ((num >> 1) & M1);
//	num = (num & M2) + ((num >> 2) & M2);
//	num = (num & M4) + ((num >> 4) & M4);
//	num = (num & M8) + ((num >> 8) & M8);
//	num = (num & M16) + ((num >> 16) & M16);
//	return num;
//}
//
//int f3(unsigned int num) {
//	int count = 0;
//	while (num) {
//		num &= (num - 1);
//		++count;
//	}
//	return count;
//}
//
////int f4(unsigned int num) {
////	int count = 0;
////	while (num) {
////		int n;
////		__asm {
////			bsr eax, num
////			mov n, eax
////		}
////		++count;
////		num ^= (1 << n);
////	}
////	return count;
////}
//
//int f5(unsigned int num) {
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
//	count = TABLE[num & 0xff] + TABLE[(num >> 8) & 0xff] + TABLE[(num >> 16) & 0xff] + TABLE[(num >> 24) & 0xff];
//	return count;
//}
//
//struct _byte {
//	unsigned a : 1;
//	unsigned b : 1;
//	unsigned c : 1;
//	unsigned d : 1;
//	unsigned e : 1;
//	unsigned f : 1;
//	unsigned g : 1;
//	unsigned h : 1;
//};
//
//inline int get_bit_count(unsigned char b) {
//	struct _byte *by = (struct _byte*)&b;
//	return (by->a + by->b + by->c + by->d + by->e + by->f + by->g + by->h);
//}
//
//int f6(unsigned int num) {
//	int count = 0;
//	count = get_bit_count(num & 0xff) + get_bit_count((num >> 8) & 0xff) + get_bit_count((num >> 16) & 0xff) + get_bit_count((num >> 24) & 0xff);
//	return count;
//}
//
//int f7(unsigned int num) {
//	unsigned int uCount;
//	uCount = num - ((num >> 1) & 033333333333) - ((num >> 2) & 011111111111);
//	return ((uCount + (uCount >> 3)) & 030707070707) % 63;
//}
//
//
//void correctness_test() {
//	unsigned int test_data[] = { 0, 1, 2, 3, 0x01234567, 0x89abcdef, 0xffffffff };
//	unsigned int corect_result[] = { 0, 1, 1, 2, 12, 20, 32 };
//
//	int(*fn[])(unsigned int) = { f1, f2, f3, f5, f6, f7 };
//	for (int i = 0; i < sizeof(fn) / sizeof(*fn); ++i) {
//		for (int j = 0; j < sizeof(test_data) / sizeof(*test_data); ++j) {
//			if (fn[i](test_data[j]) != corect_result[j]) {
//				cout << "f" << (i + 1) << " failed!" << endl;
//				exit(-1);
//			}
//		}
//	}
//	cout << "All methods passed correctness test." << endl;
//}
//
//void performance_test() {
//	const int TEST_DATA_SIZE = 100000000;
//	unsigned int* test_data = new unsigned int[TEST_DATA_SIZE];
//	prepare_test_data(test_data, TEST_DATA_SIZE);
//
//	int(*fn[])(unsigned int) = { f1, f2, f3, f5, f6, f7 };
//	for (int i = 0; i < sizeof(fn) / sizeof(*fn); ++i) {
//		clock_t start = clock();
//		for (int j = 0; j < TEST_DATA_SIZE; ++j) {
//			fn[i](test_data[j]);
//		}
//		int ticks = clock() - start;
//		double seconds = ticks * 1.0 / CLOCKS_PER_SEC;
//
//		cout << "f" << (i + 1) << " consumed " << seconds << " seconds." << endl;
//	}
//	delete[] test_data;
//}
//
//void prepare_test_data(unsigned int* data, int len) {
//	srand(0);
//	for (int i = 0; i < len; ++i) {
//		data[i] = static_cast<unsigned int>(rand() * 1.0 / RAND_MAX * 0xffffffff);
//	}
//}
#include <iostream>
#include <limits>
#include <random>
using namespace std;
typedef unsigned long long u64;
int f2(u64 num) {
	const u64 M1 = 0x5555555555555555;
	const u64 M2 = 0x3333333333333333;
	const u64 M4 = 0x0F0F0F0F0F0F0F0F;
	const u64 M8 = 0x00FF00FF00FF00FF;
	const u64 M16 = 0x0000ffff0000ffff;
	const u64 M32 = 0x00000000FFFFFFFF;

	num = (num & M1) + ((num >> 1) & M1);
	num = (num & M2) + ((num >> 2) & M2);
	num = (num & M4) + ((num >> 4) & M4);
	num = (num & M8) + ((num >> 8) & M8);
	num = (num & M16) + ((num >> 16) & M16);
	num = (num & M32) + ((num >> 32) & M32);

	return num;
}
//int f5(u64 num) {
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

int f5(u64 num) {
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
int f7(int num) {
	int uCount;
	uCount = num - ((num >> 1) & 033333333333) - ((num >> 2) & 011111111111);
	return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

int main() {
	std::random_device rd;
	const std::uniform_int_distribution<u64> dist(0, ULLONG_MAX);
	auto c = dist(rd);
	std::cout << sizeof(c) << "," << c << std::endl;

	u64 a = ULLONG_MAX;
	cout << f2(c) << ", " << f5(c) << endl;
	return 0;
}
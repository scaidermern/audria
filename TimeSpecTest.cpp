#include "TimeSpec.h"

#include <iostream>
#include <cassert>

int main() {
	TimeSpec ts1;
	TimeSpec ts2;
    
    // test fractional seconds initialization
    ts1 = TimeSpec(3);
    assert(ts1.sec() == 3 && ts1.nsec() == 0);
    ts1 = TimeSpec(3.14159265);
    assert(ts1.sec() == 3 && ts1.nsec() == 141592650); // I guess this test may fail on some architectures ;/
    
    // test timestruct initialization
    ts2 = TimeSpec(ts1.ts);
    assert(ts1 == ts2);
	
    // test comparison operators ==, < and >
    ts1 = ts2 = TimeSpec(1, 100000000);
    assert(ts1 == ts2);
    
    ts2 = TimeSpec(1, 100000001);
    assert(ts1 < ts2);
    ts2 = TimeSpec(2, 100000000);
    assert(ts1 < ts2);
    
    ts1 = TimeSpec(2, 100000001);
    assert(ts1 > ts2);
    ts1 = TimeSpec(3, 100000000);
    assert(ts1 > ts2);
    
    // test arithmetic operators - and + with carry
    ts1 = TimeSpec(2, 400000000);
    ts2 = TimeSpec(1, 700000000);
    
    ts1 = ts1 - ts2;
    assert(ts1.sec() == 0 && ts1.nsec() == 700000000);
    ts1 = ts1 + ts2;
    assert(ts1.sec() == 2 && ts1.nsec() == 400000000);
    ts1 = ts1 + ts2;
    assert(ts1.sec() == 4 && ts1.nsec() == 100000000);
    
    // test arithmetic operators -= and += with carry
    ts1 = TimeSpec(2, 400000000);
    ts2 = TimeSpec(1, 700000000);
    
    ts1 -= ts2;
    assert(ts1.sec() == 0 && ts1.nsec() == 700000000);
    ts1 += ts2;
    assert(ts1.sec() == 2 && ts1.nsec() == 400000000);
    ts1 += ts2;
    assert(ts1.sec() == 4 && ts1.nsec() == 100000000);
    
    std::cout << "all tests passed" << std::endl;
    return 0;   
}

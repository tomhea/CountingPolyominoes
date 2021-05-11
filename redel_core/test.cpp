#include "redelServer.h"
#include "graphCreator.h"
#include <cassert>
#include <iostream>

using namespace std;


void testPoly(int steps, u64 expected_result) {
    u64 result;
    createPolyominoGraphFile("graph", 21);

    if (canIFinishIt("graph", steps, &result)) {
        if (result != expected_result) {
            cout << "Poly(" << steps << "):  " << result << " != " << expected_result << endl;
            exit(1);
        }
        assert(result == expected_result);
    } else {
        printf("Can't finish it (steps = %d, expected_result = %" PRIu64 ").\n", steps, expected_result);
        assert(false);
    }
}


void testAllPoly() {
    testPoly(4, 19);
    testPoly(10, 36446);
    testPoly(13, 1903890);
    // testPoly(17, 400795844);
}


int main() {

    testAllPoly();

    printf("All tests passed! 100%%\n");
    return 0;
}

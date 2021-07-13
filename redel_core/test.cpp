#include "redelServer.h"
#include "redelClient.h"
#include "graphCreator.h"
#include <cassert>
#include <iostream>

using namespace std;

void reverse2(char str[], int length)
{
    int start = 0;
    int end = length -1;
    while (start < end)
    {
        swap(*(str+start), *(str+end));
        start++;
        end--;
    }
}
// Implementation of itoa()
char* itoa2(int num, char* str, int base) {
    int i = 0;
    bool isNegative = false;

    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }

    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    reverse2(str, i);

    return str;
}

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


void test2(const char* graph,int steps, u64 expected_result) {
    u64 result;
    if (canIFinishIt(graph, steps, &result)) {
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

void test3(const char* graph,int steps,int approx, u64 expected_result) {
    u64 result = jobsCreator(graph,steps,approx,"jobs/");
    if (result != expected_result) {
        cout << "Poly(" << steps << "):  " << result << " != " << expected_result << endl;
        exit(1);
    }
    assert(result == expected_result);
}

void test4(const char* graph,int steps,int approx, u64 expected_result) {
    char path[20] = "jobs/_";
    // u64 jobs = jobsCreator(graph,steps,approx,"jobs/");
    u64 jobs = 1;
    int result = 0;
    for (int i = 0; i < jobs; i++){
        itoa2(i, path+strlen("jobs/_"), 10);
        int result_job = executeJob(graph,path);
        result += result_job;
    }
    assert(result == expected_result);
}


void testAllPoly() {
    testPoly(4, 19);
    testPoly(10, 36446);
    testPoly(13, 1903890);
    // testPoly(17, 400795844);
}


int main() {

    // testAllPoly();
    // test2("test10",3, 5);
    // test2("test10",1, 1);

    // test2("small18",1, 1);
    // test2("small18",2, 7);
    // test2("small18",3, 7);

    // test2("lattice_5",3, 9);
    // test3("lattice_10",1,100,1);
    test4("lattice_18",1,1,1);
    printf("All tests passed! 100%%\n");
    return 0;
}

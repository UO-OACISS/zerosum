#define UNUSED(expr) do { (void)(expr); } while (0)

// C++ program to illustrate the use of condition variable
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <unistd.h>

using namespace std;

// condition variable and mutex lock
condition_variable cv;
mutex m;

// shared resource
int val = 0;

void add(int num)
{
    printf("add locking...\n");
    lock_guard<mutex> lock(m);
    val += num;
    cout << "After addition: " << val << endl;
    cv.notify_one();
}

void sub(int num)
{
    printf("sub locking...\n");
    unique_lock<mutex> ulock(m);
    cv.wait(ulock,
            [] { return (val != 0) ? true : false; });
    if (val >= num) {
        val -= num;
        cout << "After subtraction: " << val << endl;
    }
    else {
        cout << "Cannot Subtract now!" << endl;
    }
    cout << "Total number Now: " << val << endl;
}

// driver code
int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    sleep(1);
    thread t2(sub, 600);
    thread t1(add, 900);
    t1.join();
    t2.join();
    return 0;
}
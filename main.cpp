#include "ReqHandler.h"
using namespace std;

thread_local bool thread_interrupt_flag = false;
mutex coutLocker;

void taskFunc(int id, int delay)
{
    for (int i = 0; i < delay; i++) {
        if (IntThread::checkInterrupted())
        {
            unique_lock<mutex> l(coutLocker);
            cout << "task " << id << " was interrupted" << endl;
            return;
        }
        this_thread::sleep_for(chrono::seconds(1));
    }
    unique_lock<mutex> l(coutLocker);
    cout << "task " << id << " made by thread_id " << this_thread::get_id() << endl;
}

int main()
{
    srand(0);
    RequestHandler rh;
    std::vector<res_type> results;
    for (int i = 0; i < 1000000; i++)
    {
        results.push_back(rh.pushRequest(taskFunc, i, rand()%10));
    }
    for (auto& r: results)
    {
        r.wait();
    }
    /*
    ThreadPool pool;
    pool.start();
    res_type res = pool.push_task(taskFunc, 0, 1 + rand() % 6);
    while (res.wait_for(chrono::seconds(0)) != std::future_status::ready)
    {
        {
            lock_guard<mutex> l(coutLocker);
            cout << "future is not ready" << endl;
        }
        this_thread::sleep_for(chrono::seconds(1));
    }
    cout << "future is ready" << std::endl;*/
    return 0;
}
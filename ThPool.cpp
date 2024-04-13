#include "ThPool.h"

IntThread::IntThread(ThreadPool* pool, int qindex)
{
    m_pFlag = nullptr;
    m_thread = std::thread(&IntThread::startFunc, this, pool, qindex);
}

IntThread::~IntThread()
{
    m_thread.join();
}

bool IntThread::checkInterrupted()
{
    return thread_interrupt_flag;
}

void IntThread::startFunc(ThreadPool* pool, int qindex)
{
    {
        std::lock_guard<std::mutex> l(m_defender);
        m_pFlag = &thread_interrupt_flag;
    }
    pool->threadFunc(qindex);
    {
        std::lock_guard<std::mutex> l(m_defender);
        m_pFlag = nullptr;
    }
}

void IntThread::interrupt()
{
    std::lock_guard<std::mutex> l(m_defender);
    if (m_pFlag)
    {
        *m_pFlag = true;
    }
}

ThreadPool::ThreadPool(): m_thread_count(std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 4), m_thread_queues(m_thread_count) {}

void ThreadPool::start()
{
    for (int i = 0; i < m_thread_count; i++)
    {
        m_threads.push_back(new IntThread(this, i));
    }
}

void ThreadPool::threadFunc(int qindex)
{
    while (true)
    {
        if (IntThread::checkInterrupted())
        {
            std::cout << "thread was interrupted" << std::endl;
            return;
        }
        task_type task_to_do;
        bool res;
        int i = 0;
        for (; i < m_thread_count; i++)
        {
            if (res = m_thread_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
                break;
        }
        if (!res)
        {
            m_thread_queues[qindex].pop(task_to_do);
        }
        else if (!task_to_do.valid())
        {
            m_thread_queues[(qindex + i) % m_thread_count].push(task_to_do);
        }
        if (IntThread::checkInterrupted())
        {
            std::cout << "thread was interrupted!" << std::endl;
        }
        
        if (!task_to_do.valid())
        {
            return;
        }
        task_to_do();
    }
}

void ThreadPool::interrupt()
{
    for (auto& t : m_threads)
    {
        t->interrupt();
    }
}


void ThreadPool::stop()
{
    for (int i = 0; i < m_thread_count; i++)
    {
        task_type empty_task;
        m_thread_queues[i].push(empty_task);
    }
    for (auto& t : m_threads)
    {
        t->~IntThread();
    }
}

res_type ThreadPool::push_task(FuncType f, int id, int arg)
{
    int queue_to_push = m_index++ % m_thread_count;
    task_type task([=]{f(id, arg);});
    auto res = task.get_future();
    m_thread_queues[queue_to_push].push(task);
    return res;
}
/*
void ThreadPool::push_task(FuncType f, int id, int arg)
{
    int queue_to_push = m_index++ % m_thread_count;
    task_type task = [=] {f(id, arg); };
    m_thread_queues[queue_to_push].push(task);
}
*/
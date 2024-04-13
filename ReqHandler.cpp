#include "ReqHandler.h"

RequestHandler::RequestHandler()
{
    m_tpool.start();
}
RequestHandler::~RequestHandler()
{
    m_tpool.stop();
}
res_type RequestHandler::pushRequest(FuncType f, int id, int arg)
{
    return m_tpool.push_task(f, id, arg);
}

void RequestHandler::interruptPool()
{
    m_tpool.interrupt();
}
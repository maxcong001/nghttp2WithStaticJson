#include "http_router.hpp"
#include "tree.h"
using namespace cinatra;
using namespace radixtree;
struct log_t
{
    bool before(request &req, response &res)
    {
        std::cout << "before log" << std::endl;
        return true;
    }

    bool after(request &req, response &res)
    {
        std::cout << "after log" << std::endl;
       
        return true;
    }
};
int main()
{

    http_router http_router_;
    request req;
    response res;

    http_router_.register_handler<GET, POST>(
        "/test/:test/:testagain/aaa", [](request &req, response &res) {
            req.print();
        },
        log_t{});

    http_router_.route("POST", "/test/abc/bcd/aaa", req, res);
}
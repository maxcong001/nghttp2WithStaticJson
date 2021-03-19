#include "http_router.hpp"
#include "tree.h"
using namespace cinatra;
using namespace radixtree;
int main()
{

    http_router http_router_;
    request req;
    response res;

    http_router_.register_handler<GET, POST>("/test/:test/aaa", [](request &req, response &res) {
        req.print();
    });

    http_router_.route("POST", "/test/abc/aaa",req,res);
    
}
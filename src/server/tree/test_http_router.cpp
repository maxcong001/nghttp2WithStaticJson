#include "http_router.hpp"

int main()
{

    http_router http_router_;
    request req;
    response res;

    http_router_.register_handler<GET, POST>("/test/:test/aaa"sv, [](request &req, response &res) {
        req.print();
    });

    http_router_.route("POST"sv, "/test/abc/aaa",req,res);
    
}
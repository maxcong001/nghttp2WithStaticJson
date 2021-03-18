#include <iostream>
#include <time.h>

#include "tree.h"

using std::cout;
using std::endl;
using std::tie;

using radixtree::HandleFunc;
using radixtree::RadixTree;
using radixtree::Request;

HandleFunc handle = [](Request *req) {
    cout << "Handled! The params are:" << endl;

    for (auto i : req->params)
    {
        cout << i.first << ": "
             << i.second << endl;
    }
};

RadixTree tree;
auto req = new Request("/user/Lime/male/25/test/", "GET");

void testWork()
{
    bool pathExisted;
    HandleFunc handler;

    cout << tree.insert("/user", handle, {"GET"}) << endl;
    cout << tree.insert("/user/:name", handle, {"GET"}) << endl;
    cout << tree.insert("/user/:name", handle, {"GET"}) << endl;
    cout << tree.insert("/user/:name/:sex/:age/test", handle, {"GET"}) << endl;
    cout << tree.insert("/user/lime", handle, {"GET"}) << endl;
    cout << tree.insert("/src/*filename", handle, {"GET"}) << endl;
    cout << tree.insert("/src/image.png", handle, {"GET"}) << endl;

    tie(pathExisted, handler, req->params) = tree.get(req->path, req->method);

    cout << pathExisted << endl;
    if (handler)
        handler(req);
}

void testPerformance()
{
    auto start = clock();
    for (auto i = 0; i < 100; i++)
        tree.get("/user/Lime/male/25", "GET");
    auto end = clock();

    cout << "Total cost " << (end - start) * 1.0 / CLOCKS_PER_SEC
         << "seconds" << endl;
}

int main()
{
    testWork();
    testPerformance();
    delete req;
}
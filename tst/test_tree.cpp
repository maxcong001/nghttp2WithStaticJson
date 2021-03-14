#include <iostream>
#include <time.h>

#include "tree.h"

using std::cout;
using std::endl;
using std::tie;

using radixtree::HandleFunc;
using radixtree::RadixTree;
using radixtree::Request;
using namespace radixtree;

HandleFunc handle = [](Request *req) {
    cout << "Handled! The params are:" << endl;
    for (auto i = 0; i < req->params.size; ++i)
        cout << req->params.params[i].first << ": "
             << req->params.params[i].second << endl;
};

RadixTree tree;
auto req = new Request(std::string("/user/Lime/male/25/tests/"), httpMethod::GET);

void testWork()
{
    bool pathExisted;
    HandleFunc handler;

    cout << "1"<<tree.insert("/user", handle, {httpMethod::GET}) << endl;
    cout << "2"<<tree.insert("/user/:name", handle, {httpMethod::GET}) << endl;
    cout << "3"<<tree.insert("/user/:name", handle, {httpMethod::GET}) << endl;
    cout << "4"<<tree.insert("/user/:name/:sex/:age/tests", handle, {httpMethod::GET}) << endl;
    cout << "5"<<tree.insert("/user/lime", handle, {httpMethod::GET}) << endl;
    cout << "6"<<tree.insert("/src/*filename", handle, {httpMethod::GET}) << endl;
    cout << "7"<<tree.insert("/src/image.png", handle, {httpMethod::GET}) << endl;

    tie(pathExisted, handler, req->params) = tree.get(req->path, req->method);

    cout << pathExisted << endl;
    if (handler)
        handler(req);
}

void testPerformance()
{
    auto start = clock();
    for (auto i = 0; i < 1000000; i++)
        tree.get("/user/Lime/male/25", httpMethod::GET);
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
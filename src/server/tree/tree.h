#ifndef TREE_H
#define TREE_H

#include <string>
#include <vector>
#include <tuple>

namespace radixtree
{

    using std::pair;
    using std::string;
    using std::tuple;
    using std::vector;

    const char kAsterisk = '*';
    const char kColon = ':';
    const char kSlash = '/';

    typedef pair<string, string> Parameter;

    struct Params
    {
        Parameter *params;
        int size;
    };

    enum class httpMethod{
        GET = 0,
        POST,
        PUT,
        DELETE,
        PATCH,
        HEAD,
        OPTIONS,
        TRACE,
        CONNECT,
        HTTPMETHODMAX

    };

    struct Request
    {
        string path;
        httpMethod method;
        Params params;

        Request( string &&path,  httpMethod method);
    };

    typedef void (*HandleFunc)(Request *req);
    typedef tuple<bool, HandleFunc, Params> ParseResult;

    struct Handler
    {
        httpMethod method;
        HandleFunc handler;
    };

    struct RadixTreeNode
    {
        string path;
        vector<Handler> handlers;
        string indices;
        vector<RadixTreeNode *> children;
        int maxParams;

        RadixTreeNode() = default;
        explicit RadixTreeNode(const string &path);
        ~RadixTreeNode();

        HandleFunc getHandler(httpMethod method);
        int addHandler(HandleFunc handler, const vector<httpMethod> &methods);

        RadixTreeNode *insertChild(
            char index,
            RadixTreeNode *child);
        RadixTreeNode *getChild(char index);
        int getIndexPosition(char target);
    };

    int find(const string &str, char target, int start);

    class RadixTree
    {
    public:
        RadixTree();
        ~RadixTree();
        int insert(
            const string &path,
            HandleFunc handler,
            const vector<httpMethod> &methods);
        ParseResult get(const string &path, httpMethod method);

    private:
        RadixTreeNode *root;
    };

} // namespace radixtree

#endif
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

    struct Request
    {
        string path;
        string method;
        Params params;

        Request( string &&path, const string &method);
    };

    typedef void (*HandleFunc)(Request *req);
    typedef tuple<bool, HandleFunc, Params> ParseResult;

    struct Handler
    {
        string method;
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

        HandleFunc getHandler(const string &method);
        int addHandler(HandleFunc handler, const vector<string> &methods);

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
            const vector<string> &methods);
        ParseResult get(const string &path, const string &method);

    private:
        RadixTreeNode *root;
    };

} // namespace radixtree

#endif
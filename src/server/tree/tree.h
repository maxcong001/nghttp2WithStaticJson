#ifndef TREE_H
#define TREE_H

#include <string>
#include <vector>
#include <tuple>
#include <functional>
#include <iostream>
#include <string_view>
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

    class request
    {
    public:
        void setPara(std::vector<Parameter> &&arr)
        {
            para = arr;
        }
        void print()
        {
            std::cout << "Handled! The params are:" << std::endl;

            for (auto i : para)
            {
                std::cout << i.first << ": "
                     << i.second << std::endl;
            }
        }
        std::vector<Parameter> para;
    };
    class response
    {
    };

    typedef std::function<void(request &, response &)> HandleFunc;
    typedef tuple<bool, HandleFunc, std::vector<Parameter>> ParseResult;

    struct Handler
    {
        std::string_view method;
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
        explicit RadixTreeNode(const string &path)
        {
            this->path = path;
        }
        ~RadixTreeNode()
        {
            for (auto &c : this->children)
                delete c;
        }

        HandleFunc getHandler(std::string_view method)
        {
            for (auto &h : this->handlers)
            {
                if (h.method == method)
                {
                    return h.handler;
                }
            }
            return nullptr;
        }
        int addHandler(HandleFunc handler, const std::vector<std::string_view> &methods)
        {
            for (auto &m : methods)
            {
                auto oldHandler = this->getHandler(m);
                // already got the handler, just return
                if (oldHandler)
                    return -1;

                this->handlers.push_back(Handler{m, handler});
            }
            return 0;
        }

        RadixTreeNode *insertChild(char index, RadixTreeNode *child)
        {

            auto i = this->getIndexPosition(index);
            this->indices.insert(this->indices.begin() + i, index);
            this->children.insert(this->children.begin() + i, child);
            return child;
        }
        RadixTreeNode *getChild(char index)
        {
            auto i = this->getIndexPosition(index);
            return this->indices[i] != index ? nullptr : this->children[i];
        }
        int getIndexPosition(char target)
        {
            int low = 0, high = this->indices.size(), mid;

            while (low < high)
            {
                mid = low + ((high - low) >> 1);
                if (this->indices[mid] < target)
                    low = mid + 1;
                else
                    high = mid;
            }
            return low;
        }
    };

    class RadixTree
    {
    public:
        RadixTree()
        {
            this->root = new RadixTreeNode();
        }
        ~RadixTree()
        {
            delete this->root;
        }
        int insert(const std::string& path, HandleFunc handler, const std::vector<std::string_view> &methods)
        {
            auto root = this->root;
            int i = 0, n = path.size(), paramCount = 0, code = 0;

            while (i < n)
            {
                if (!root->indices.empty() && (root->indices[0] == kAsterisk ||
                                               path[i] == kAsterisk ||
                                               (path[i] != kColon && root->indices[0] == kColon) ||
                                               (path[i] == kColon && root->indices[0] != kColon) ||
                                               (path[i] == kColon && root->indices[0] == kColon && path.substr(i + 1, find(path, kSlash, i) - i - 1) != root->children[0]->path)))
                {
                    code = -1;
                    break;
                }

                auto child = root->getChild(path[i]);
                if (!child)
                {
                    auto p = find(path, kColon, i);

                    if (p == n)
                    {
                        p = find(path, kAsterisk, i);

                        root = root->insertChild(
                            path[i], new RadixTreeNode(path.substr(i, p - i)));

                        if (p < n)
                        {
                            root = root->insertChild(kAsterisk, new RadixTreeNode(path.substr(p + 1)));
                            ++paramCount;
                        }

                        code = root->addHandler(handler, methods);
                        break;
                    }

                    root = root->insertChild(path[i], new RadixTreeNode(path.substr(i, p - i)));

                    i = find(path, kSlash, p);
                    root = root->insertChild(kColon, new RadixTreeNode(path.substr(p + 1, i - p - 1)));
                    ++paramCount;

                    if (i == n)
                    {
                        code = root->addHandler(handler, methods);
                        break;
                    }
                }
                else
                {
                    root = child;

                    if (path[i] == kColon)
                    {
                        ++paramCount;
                        i += root->path.size() + 1;

                        if (i == n)
                        {
                            code = root->addHandler(handler, methods);
                            break;
                        }
                    }
                    else
                    {
                        std::size_t j = 0, m = root->path.size();

                        for (; i < n && j < m && path[i] == root->path[j]; ++i, ++j)
                        {
                        }

                        if (j < m)
                        {
                            auto child = new RadixTreeNode(root->path.substr(j));
                            child->handlers = root->handlers;
                            child->indices = root->indices;
                            child->children = root->children;

                            root->path = root->path.substr(0, j);
                            root->handlers = {};
                            root->indices = child->path[0];
                            root->children = {child};
                        }

                        if (i == n)
                        {
                            code = root->addHandler(handler, methods);
                            break;
                        }
                    }
                }
            }

            if (paramCount > this->root->maxParams)
                this->root->maxParams = paramCount;

            return code;
        }
        ParseResult get(const string &path, const string &method)
        {
            std::vector<Parameter> params;
            params.reserve(root->maxParams);

            auto root = this->root;
            int i = 0, n = path.size(), p;

            while (i < n)
            {
                if (root->indices.empty())
                    return ParseResult();

                if (root->indices[0] == kColon)
                {
                    root = root->children[0];

                    p = find(path, kSlash, i);
                    params.push_back(Parameter{
                        root->path, path.substr(i, p - i)});
                    i = p;
                }
                else if (root->indices[0] == kAsterisk)
                {
                    root = root->children[0];
                    params.push_back(Parameter{root->path, path.substr(i)});
                    break;
                }
                else
                {
                    root = root->getChild(path[i]);
                    if (!root || path.substr(i, root->path.size()) != root->path)
                        return ParseResult();
                    i += root->path.size();
                }
            }

            return ParseResult{true, root->getHandler(method), params};
        }

    private:
        int find( std::string_view str, char target, int start)
        {
            auto i = str.find(target, start);
            return i == -1 ? str.size() : i;
        }
        RadixTreeNode *root;
    };

} // namespace radixtree

#endif
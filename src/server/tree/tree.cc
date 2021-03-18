#include "tree.h"

namespace radixtree
{

    int find(const string &str, char target, int start)
    {
        auto i = str.find(target, start);
        return i == -1 ? str.size() : i;
    }

    Request::Request( string &&path, const string &method)
    {
        if (path.back() == kSlash)
        {
            path.pop_back();
        }
        this->path = std::move(path);
        this->method = method;
    }

    RadixTreeNode::RadixTreeNode(const string &path)
    {
        this->path = path;
    }

    RadixTreeNode::~RadixTreeNode()
    {
        for (auto &c : this->children)
            delete c;
    }

    HandleFunc RadixTreeNode::getHandler(const string &method)
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

    int RadixTreeNode::addHandler(
        HandleFunc handler,
        const vector<string> &methods)
    {

        for (auto &m : methods)
        {
            auto oldHandler = this->getHandler(m);

            if (oldHandler && oldHandler != handler)
                return -1;

            this->handlers.push_back(Handler{m, handler});
        }
        return 0;
    }

    RadixTreeNode *RadixTreeNode::insertChild(
        char index,
        RadixTreeNode *child)
    {

        auto i = this->getIndexPosition(index);
        this->indices.insert(this->indices.begin() + i, index);
        this->children.insert(this->children.begin() + i, child);
        return child;
    }

    RadixTreeNode *RadixTreeNode::getChild(char index)
    {
        auto i = this->getIndexPosition(index);
        return this->indices[i] != index ? nullptr : this->children[i];
    }

    int RadixTreeNode::getIndexPosition(char target)
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

    RadixTree::RadixTree()
    {
        this->root = new RadixTreeNode();
    }

    RadixTree::~RadixTree()
    {
        delete this->root;
    }

    int RadixTree::insert(
        const string &path,
        HandleFunc handler,
        const vector<string> &methods)
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
                        root = root->insertChild(
                            kAsterisk, new RadixTreeNode(path.substr(p + 1)));
                        ++paramCount;
                    }

                    code = root->addHandler(handler, methods);
                    break;
                }

                root = root->insertChild(
                    path[i], new RadixTreeNode(path.substr(i, p - i)));

                i = find(path, kSlash, p);
                root = root->insertChild(
                    kColon, new RadixTreeNode(path.substr(p + 1, i - p - 1)));
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

    ParseResult RadixTree::get(const string &path, const string &method)
    {
        Params params = Params{new Parameter[root->maxParams]};

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
                params.params[params.size++] = Parameter{
                    root->path, path.substr(i, p - i)};
                i = p;
            }
            else if (root->indices[0] == kAsterisk)
            {
                root = root->children[0];
                params.params[params.size++] = Parameter{root->path, path.substr(i)};
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

} // namespace radixtree
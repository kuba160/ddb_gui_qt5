#ifndef TREENODE_H
#define TREENODE_H

#include <QString>
#include <QStringList>
#include <QRegularExpression>

template <class T>
class TreeNode {
public:
    // create leaf node
    TreeNode(TreeNode *parent, QString title, T value);
    // create parent node
    TreeNode(TreeNode *parent = nullptr, QString title = {});

    ~TreeNode();

    // group text
    QString title;
    //
    bool leafNode;
    // value (default constructed for group node)
    T value;

    // parent node (for optimalization)
    TreeNode *parent;

    // insert leaf node (where title is separated by '/')
    void insertChild(QString title, T value);
    void insertChildIter(QStringList title, T value);

    bool operator== (const TreeNode& cmp)  {
        return title == cmp.title;
    }

    // children operations
    int getChildrenCount() const { return children.count(); }
    TreeNode * getChild(int idx) const { return children.at(idx);}
    TreeNode * getParent() const { return parent;}
    int getNodeIndex() const { return parent->getChildIdx(this);}

    int getChildIdx(const TreeNode *child) const {return child->children.indexOf(child);}


    // modifying operations
    void pushChildrenFirst(QStringList path);
    void pushChildrenLast(QStringList path);

protected:
    QList<TreeNode *> children;
};

template <class T>
TreeNode<T>::TreeNode(TreeNode *parent, QString title, T value) {
    this->title = title;
    this->parent = parent;
    if (value != T{}) {
        this->value = value;
        this->leafNode = true;
    }
    else {
        this->leafNode = false;
    }
}

template <class T>
TreeNode<T>::TreeNode(TreeNode *parent, QString title) {
    this->title = title;
    this->value= T{};
    this->parent = parent;
    this->leafNode = false;
}

template <class T>
TreeNode<T>::~TreeNode() {
    for (TreeNode<T>* child : children) {
        delete child;
    }
}

template <class T>
void TreeNode<T>::insertChild(QString title, T value) {
    static QRegularExpression re("\\/(?<!\\\\\\/)"); // regex go brrrrr
    insertChildIter(QString(title).split(re), value);
}

template <class T>
void TreeNode<T>::insertChildIter(QStringList list, T value) {
    if (list.length() == 1) {
        children.append(new TreeNode(this, list[0], value));
    }
    else {
        for (TreeNode *i : children) {
            if (i->title == list[0]) {
                list.removeFirst();
                i->insertChildIter(list, value);
                return;
            }
        }
        // no child node found, create a new one
        TreeNode<T> *child = new TreeNode<T>(this, list[0], T{});
        children.append(child);
        list.removeFirst();
        child->insertChildIter(list, value);
    }
}

template <class T>
void TreeNode<T>::pushChildrenLast(QStringList list) {
    if (list.length() == 1) {
        for (int i = 0; i < children.length(); i++) {
            TreeNode<T> *child = children[i];
            if (child->title == list[0]) {
                children.swapItemsAt(i, children.length()-1);
//                child = children.takeAt(i);
//                children.append(child);
            }
        }
    }
    else {
        for (TreeNode *i : children) {
            if (i->title == list[0]) {
                list.removeFirst();
                i->pushChildrenLast(list);
                return;
            }
        }
    }
}

template <class T>
void TreeNode<T>::pushChildrenFirst(QStringList list) {
    if (list.length() == 1) {
        for (int i = 0; i < children.length(); i++) {
            TreeNode<T> child = children[i];
            if (child.title == list[0]) {
                children.prepend(children.takeAt(i));
            }
        }
    }
    else {
        for (TreeNode *i : children) {
            if (i->title == list[0]) {
                list.removeFirst();
                i->pushChildrenFirst(list);
                return;
            }
        }
    }
}

#endif // TREENODE_H

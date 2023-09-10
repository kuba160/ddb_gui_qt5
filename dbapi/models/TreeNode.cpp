#include "TreeNode.h"
#include <QRegularExpression>

//template <class T>
//TreeNode<T>::TreeNode(TreeNode *parent, QString title, T value) {
//    this->title = title;
//    this->parent = parent;
//    if (value != T{}) {
//        this->value = value;
//        this->leafNode = true;
//    }
//    else {
//        this->leafNode = false;
//    }
//}

//template <class T>
//TreeNode<T>::TreeNode(TreeNode *parent, QString title) {
//    this->title = title;
//    this->value= T{};
//    this->parent = parent;
//    this->leafNode = false;
//}

//template <class T>
//TreeNode<T>::~TreeNode() {
//    for (TreeNode<T>* child : children) {
//        delete child;
//    }
//}

//template <class T>
//void TreeNode<T>::insertChild(QString title, T value) {
//    static QRegularExpression re("\\/(?<!\\\\\\/)"); // regex go brrrrr
//    insertChildIter(QString(title).split(re), value);
//}

//template <class T>
//void TreeNode<T>::insertChildIter(QStringList list, T value) {
//    if (list.length() == 1) {
//        children.append(new TreeNode(this, list[0], value));
//    }
//    else {
//        for (TreeNode *i : children) {
//            if (i->title == list[0]) {
//                list.removeFirst();
//                i->insertChildIter(list, value);
//                return;
//            }
//        }
//        // no child node found, create a new one
//        TreeNode<T> *child = new TreeNode<T>(this, list[0], T{});
//        children.append(child);
//        list.removeFirst();
//        child->insertChildIter(list, value);
//    }
//}

//template <class T>
//void TreeNode<T>::pushChildrenLast(QStringList list) {
//    if (list.length() == 1) {
//        for (int i = 0; i < children.length(); i++) {
//            TreeNode<T> child = children[i];
//            if (child.title == list[0]) {
//                children.takeAt(i);
//                children.append(i);
//            }
//        }
//    }
//    else {
//        for (TreeNode *i : children) {
//            if (i->title == list[0]) {
//                list.removeFirst();
//                i->pushChildrenLast(list);
//                return;
//            }
//        }
//    }
//}

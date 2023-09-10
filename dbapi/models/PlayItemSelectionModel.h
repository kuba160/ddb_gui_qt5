#ifndef PLAYITEMSELECTIONMODEL_H
#define PLAYITEMSELECTIONMODEL_H

#include <QItemSelectionModel>
#include <QObject>

class PlayItemSelectionModel : public QItemSelectionModel
{
public:
    explicit PlayItemSelectionModel(QObject *parent = nullptr);
};

#endif // PLAYITEMSELECTIONMODEL_H

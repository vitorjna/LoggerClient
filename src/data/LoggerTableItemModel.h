#pragma once

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QStandardItemModel>

class LoggerTableItemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit LoggerTableItemModel(QObject *parent = nullptr);
    ~LoggerTableItemModel() override;

//reimplemented methods
public:
    QVariant data(const QModelIndex &index, int role) const override;

};



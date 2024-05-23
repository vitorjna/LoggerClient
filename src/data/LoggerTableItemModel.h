#pragma once

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QStandardItemModel>

class LoggerTableProxyModel;

//TODO the model <-> proxy structure is wrong. Most stuff should be in the model, not the proxy. And the model should not call the proxy for data access

class LoggerTableItemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit LoggerTableItemModel(LoggerTableProxyModel *myProxyModel, QObject *parent = nullptr);
    ~LoggerTableItemModel() override;

    static QColor getColorForLevel(const QString &szLevel);

    void updateKeywords(const QStringList &szaKeywords);

private:
    LoggerTableProxyModel *myProxyModel;
    QStringList szaKeywords;

//reimplemented methods
public:
    QVariant data(const QModelIndex &index, int role) const override;

};



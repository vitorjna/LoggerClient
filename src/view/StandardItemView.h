#pragma once

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QKeyEvent>
#include <QStandardItem>
#include <QTableView>
#include <QWidget>

class StandardItemModel;

class StandardItemView : public QTableView
{
    Q_OBJECT

public:
    explicit StandardItemView(QWidget *parent = nullptr);
    ~StandardItemView() override;

    QString toString(int nTop = 0, int nBottom = -1, int nLeft = 0, int nRight = -1);

private:
    StandardItemModel *myModel;

//reimplemented
public:
    void setModel(StandardItemModel *model);

protected:
    void keyPressEvent(QKeyEvent *myKeyEvent) override;

private:
    inline void setModel(QAbstractItemModel *model) override { Q_UNUSED(model); } //hidden

};



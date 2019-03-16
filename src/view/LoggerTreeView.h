#pragma once

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QKeyEvent>
#include <QTableView>
#include <QWidget>

class LoggerTreeView : public QTableView
{
    Q_OBJECT

public:
    explicit LoggerTreeView(QWidget *parent = nullptr);
    ~LoggerTreeView() override;

    QString toString(int nTop = 0, int nBottom = -1, int nLeft = 0, int nRight = -1);

private:
    int getColumnMaxWidth(const int nCol, int nRowTop = 0, int nRowBottom = -1);

//reimplemented
protected:
    void keyPressEvent(QKeyEvent *myKeyEvent) override;

};



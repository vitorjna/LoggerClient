#pragma once

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QShortcut>
#include <QTreeView>
#include <QWidget>


class LoggerTreeView : public QTreeView
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



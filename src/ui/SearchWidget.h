#pragma once

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QStyle>

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget() override;

    bool isEmpty();

    void triggerSearchTextChanged();

    void clear();

private:
    void setupUi();
    void setupSignalsAndSlots();

    QLabel          *labelSearchText{};
    QLineEdit       *lineEditSearchText{};

    QPushButton     *pushButtonPreviousResult{};
    QPushButton     *pushButtonNextResult{};

signals:
    void searchTextChanged(QString);

public slots:
    void previousResult();
    void nextResult();

//reimplemented
protected:
    void focusInEvent(QFocusEvent *event) override;

};

#pragma once

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QRegularExpression>
#include <QShortcut>
#include <QStyle>

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = nullptr);
    ~SearchWidget() override;

    bool isEmpty() const;

    void triggerSearchTextChanged();

    void clear();

private:
    static const QRegularExpression myRegexCheck;

    void setupUi();
    void setupSignalsAndSlots();
    void setupShortcuts();

    QLabel          *labelSearchText{};
    QLineEdit       *lineEditSearchText{};

    QPushButton     *pushButtonPreviousResult{};
    QPushButton     *pushButtonNextResult{};

Q_SIGNALS:
    void searchTextChanged(const QString &szSearchText, QRegularExpression::PatternOptions eOptions);

public Q_SLOTS:
    void previousResult();
    void nextResult();

    void focusAndSelect();

//reimplemented
protected:
    void focusInEvent(QFocusEvent *event) override;

};

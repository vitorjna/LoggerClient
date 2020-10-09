#pragma once

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QShortcut>

class KeywordHighlightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KeywordHighlightWidget(QWidget *parent = nullptr);
    ~KeywordHighlightWidget() override;

    bool isEmpty() const;

    void clear();

    QStringList getKeywords() const;

private:
    void setupUi();
    void setupSignalsAndSlots();
    void setupShortcuts();
    void loadSettings();

    QLabel      *labelKeywordsText{};
    QLineEdit   *lineEditKeywords{};

signals:
    void keywordsChangedSignal(QStringList);

public slots:
    void focusAndSelect();

private slots:
    void keywordsChanged(const QString &szKeywords);

//reimplemented
protected:
    void focusInEvent(QFocusEvent *event) override;

};

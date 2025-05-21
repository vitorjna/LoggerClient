#include "SearchWidget.h"

const QRegularExpression SearchWidget::myRegexCheck = QRegularExpression(R"(\[|\(|\\|\{)", QRegularExpression::CaseInsensitiveOption);

SearchWidget::SearchWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupSignalsAndSlots();

    setupShortcuts();
}

SearchWidget::~SearchWidget()
{
}

bool SearchWidget::isEmpty() const
{
    return lineEditSearchText->text().isEmpty();
}

void SearchWidget::triggerSearchTextChanged()
{
    QString szText = lineEditSearchText->text();

    if (szText.contains(myRegexCheck)
        && QRegularExpression(szText).isValid()) {

        // qDebug() << "Searching with Regex";

        Q_EMIT searchTextChanged(szText, QRegularExpression::NoPatternOption);

    } else {
        Q_EMIT searchTextChanged(szText, QRegularExpression::CaseInsensitiveOption);
    }
}

void SearchWidget::clear()
{
    lineEditSearchText->clear();
}

void SearchWidget::setupUi()
{
    QHBoxLayout *mySearchTextLayout = new QHBoxLayout(this);
    mySearchTextLayout->setContentsMargins(0, 0, 0, 0);
    {
        labelSearchText = new QLabel(this);
        labelSearchText->setText(tr("Search:"));
        labelSearchText->setToolTip(tr("Plain text: case-insensitive\nRegex: case-sensitive"));

        lineEditSearchText = new QLineEdit(this);
        lineEditSearchText->setPlaceholderText(tr("Search: Plain text: case-insensitive. Regex: case-sensitive"));
        lineEditSearchText->setToolTip(tr("Ctrl+F"));
        lineEditSearchText->setClearButtonEnabled(true);
        labelSearchText->setBuddy(lineEditSearchText);

        mySearchTextLayout->addWidget(labelSearchText);
        mySearchTextLayout->addWidget(lineEditSearchText, 99);
    }
    {
        pushButtonPreviousResult = new QPushButton(this);
        pushButtonPreviousResult->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowBack));
        pushButtonPreviousResult->setToolTip(tr("Previous result"));

        pushButtonNextResult = new QPushButton(this);
        pushButtonNextResult->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowForward));
        pushButtonNextResult->setToolTip(tr("Next result"));

        //TODO for when the search will act as actual Search, not Filter
//        mySearchTextLayout->addWidget(pushButtonPreviousResult);
//        mySearchTextLayout->addWidget(pushButtonNextResult);
        pushButtonPreviousResult->hide();
        pushButtonNextResult->hide();
    }
}

void SearchWidget::setupSignalsAndSlots()
{
    connect(lineEditSearchText,         &QLineEdit::textChanged,
            this,                       &SearchWidget::triggerSearchTextChanged);

    connect(lineEditSearchText,         &QLineEdit::returnPressed,
            pushButtonNextResult,       &QPushButton::click);

    connect(pushButtonNextResult,       &QPushButton::clicked,
            this,                       &SearchWidget::nextResult);

    connect(pushButtonPreviousResult,   &QPushButton::clicked,
            this,                       &SearchWidget::previousResult);
}

void SearchWidget::setupShortcuts()
{
    QShortcut *shortcutSearchText       = new QShortcut(QKeySequence(QStringLiteral("Ctrl+F")), this);
    connect(shortcutSearchText,         &QShortcut::activated,
            this,                       &SearchWidget::focusAndSelect);

    QShortcut *shortcutSearchNext       = new QShortcut(QKeySequence(QStringLiteral("F3")), this);
    connect(shortcutSearchNext,         &QShortcut::activated,
            this,                       &SearchWidget::nextResult);

    QShortcut *shortcutSearchPrevious   = new QShortcut(QKeySequence(QStringLiteral("Shift+F3")), this);
    connect(shortcutSearchPrevious,     &QShortcut::activated,
            this,                       &SearchWidget::previousResult);
}

void SearchWidget::previousResult()
{
    qDebug() << "previous result";
}

void SearchWidget::nextResult()
{
    qDebug() << "next result";
}

void SearchWidget::focusAndSelect()
{
    setFocus();

    lineEditSearchText->selectAll();
}

void SearchWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    lineEditSearchText->setFocus();
}


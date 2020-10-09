#include "KeywordHighlightWidget.h"
#include "application/AppSettings.h"
#include "application/GlobalConstants.h"

KeywordHighlightWidget::KeywordHighlightWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupSignalsAndSlots();

    setupShortcuts();

    loadSettings();
}

KeywordHighlightWidget::~KeywordHighlightWidget()
{
}

bool KeywordHighlightWidget::isEmpty() const
{
    return lineEditKeywords->text().isEmpty();
}

void KeywordHighlightWidget::clear()
{
    lineEditKeywords->clear();
}

QStringList KeywordHighlightWidget::getKeywords() const
{
    QStringList szaKeywords = lineEditKeywords->text().split(GlobalConstants::SEPARATOR_SETTINGS_LIST_WORDS);
    QStringList szaKeywordsClean;

    for (const QString &szKeyword : szaKeywords) {
        if (szKeyword.trimmed().size() != 0) {
            szaKeywordsClean.push_back(szKeyword.trimmed());
        }
    }

    return szaKeywordsClean;
}

void KeywordHighlightWidget::setupUi()
{
    QHBoxLayout *mySearchTextLayout = new QHBoxLayout(this);
    mySearchTextLayout->setContentsMargins(0, 0, 0, 0);
    {
        labelKeywordsText = new QLabel(this);
        labelKeywordsText->setText(tr("Keywords:"));
        labelKeywordsText->setToolTip(tr("Keywords to highlight. Comma separated, case-sensitive"));

        lineEditKeywords = new QLineEdit(this);
        lineEditKeywords->setPlaceholderText(tr("Keywords to highlight. Comma separated, case-sensitive"));
        lineEditKeywords->setToolTip(tr("Ctrl+K to select"));
        lineEditKeywords->setClearButtonEnabled(true);
        labelKeywordsText->setBuddy(lineEditKeywords);

        mySearchTextLayout->addWidget(labelKeywordsText);
        mySearchTextLayout->addWidget(lineEditKeywords, 99);
    }
}

void KeywordHighlightWidget::setupSignalsAndSlots()
{
    connect(lineEditKeywords,   &QLineEdit::textChanged,
            this,               &KeywordHighlightWidget::keywordsChanged);
}

void KeywordHighlightWidget::setupShortcuts()
{
    QShortcut *shortcutHighlightText    = new QShortcut(QKeySequence(QStringLiteral("Ctrl+K")), this);
    connect(shortcutHighlightText,      &QShortcut::activated,
            this,                       &KeywordHighlightWidget::focusAndSelect);
}

void KeywordHighlightWidget::loadSettings()
{
    QStringList szaKeywords = AppSettings::getValue(AppSettings::KEY_KEYWORDS_HIGHLIGHT, AppSettings::getDefaultValue(AppSettings::KEY_KEYWORDS_HIGHLIGHT)).toStringList();

    lineEditKeywords->setText(szaKeywords.join(GlobalConstants::SEPARATOR_SETTINGS_LIST_WORDS));
}

void KeywordHighlightWidget::focusAndSelect()
{
    setFocus();

    lineEditKeywords->selectAll();
}

void KeywordHighlightWidget::keywordsChanged(const QString &szKeywords)
{
    Q_UNUSED(szKeywords)

    QStringList szaKeywordsClean = getKeywords();

    AppSettings::setValue(AppSettings::KEY_KEYWORDS_HIGHLIGHT, szaKeywordsClean);

    emit keywordsChangedSignal(szaKeywordsClean);
}

void KeywordHighlightWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    lineEditKeywords->setFocus();
}


#ifndef DLANGHOVERHANDLER_H
#define DLANGHOVERHANDLER_H

#include <QSharedPointer>

#include <texteditor/basehoverhandler.h>

namespace DCodeModel {
class IModel;
typedef QSharedPointer<IModel> IModelSharedPtr;
}

namespace DlangEditor {

class DlangHoverHandler : public TextEditor::BaseHoverHandler
{
    Q_OBJECT
public:
    explicit DlangHoverHandler(QObject *parent = 0);
    virtual ~DlangHoverHandler() Q_DECL_OVERRIDE;

    // pure TextEditor::BaseHoverHandler
    virtual void identifyMatch(TextEditor::TextEditorWidget *editor, int pos) Q_DECL_OVERRIDE;

    // overriden
    virtual void decorateToolTip() Q_DECL_OVERRIDE;

signals:

public slots:

private:
    int lastPos;
    QString lastSymbol;
    QString lastTooltip;
    DCodeModel::IModelSharedPtr m_codeModel;
};

} // namespace DlangEditor

#endif // DLANGHOVERHANDLER_H

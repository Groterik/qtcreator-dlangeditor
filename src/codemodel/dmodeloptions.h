#ifndef DMODELOPTIONS_H
#define DMODELOPTIONS_H

#include <QWidget>

namespace DCodeModel {

class IModelOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    IModelOptionsWidget(QWidget *parent = 0) : QWidget(parent) {}
    virtual ~IModelOptionsWidget() {}
    virtual void apply() = 0;

signals:
    void updatedAndNeedRestart();
};

} // namespace DCodeModel

#endif //DMODELOPTIONS_H

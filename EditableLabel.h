#ifndef EDITABLELABEL_H
#define EDITABLELABEL_H

#include <QWidget>
#include <QLineEdit>

namespace Ui {
class EditableLabel;
}

class EditableLabel : public QLineEdit
{
    Q_OBJECT

public:
    explicit EditableLabel(QWidget *parent = nullptr);
    ~EditableLabel();

signals:
  void Focussed(bool);

protected:
  virtual void focusInEvent(QFocusEvent*);
  virtual void focusOutEvent(QFocusEvent*);
  virtual void mousePressEvent(QMouseEvent*);

private:
    Ui::EditableLabel *ui;
};

#endif // EDITABLELABEL_H

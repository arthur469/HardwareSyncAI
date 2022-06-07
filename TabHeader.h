#ifndef TABHEADER_H
#define TABHEADER_H

#include <QWidget>

namespace Ui {
class TabHeader;
}

class TabHeader : public QWidget
{
    Q_OBJECT

public:
    explicit TabHeader(QWidget *parent = nullptr);
    ~TabHeader();
    void Rename(QString);

signals:
    void CloseRequest();
    void RenameRequest(QString);

private slots:
    void on_close_clicked();
    void on_editor_editingFinished();

private:
    Ui::TabHeader *ui;
};

#endif // TABHEADER_H

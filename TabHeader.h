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

    void ToogleRunningIndicator(bool);

signals:
    void CloseRequest();
    void RenameRequest(QString);
    void StartStopRequest();

private slots:
    void on_close_clicked();
    void on_editor_editingFinished();
    void on_start_stop_clicked();

private:
    Ui::TabHeader *ui;
};

#endif // TABHEADER_H

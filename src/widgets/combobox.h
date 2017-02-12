#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QWidget>

namespace Ui {
    class ComboBox;
}

class ComboBox : public QWidget
{
    Q_OBJECT

public:
    explicit ComboBox(QWidget* parent = 0);
    ~ComboBox();

    void setLabel(QString label);

private:
    Ui::ComboBox* ui;
};

#endif // COMBOBOX_H

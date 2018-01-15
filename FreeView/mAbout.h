#ifndef MABOUT_H
#define MABOUT_H

#include <QDialog>

namespace Ui {
class mAbout;
}

class mAbout : public QDialog
{
    Q_OBJECT

public:
    explicit mAbout(QWidget *parent = 0);
    ~mAbout();

private:
    Ui::mAbout *ui;
};

#endif // MABOUT_H

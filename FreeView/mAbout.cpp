#include "mAbout.h"
#include "ui_mAbout.h"

mAbout::mAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mAbout)
{
    ui->setupUi(this);
}

mAbout::~mAbout()
{
    delete ui;
}

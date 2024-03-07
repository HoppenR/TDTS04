#include "GuiTextArea.h"

#include <QFont>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QString>

using namespace std;

GuiTextArea::GuiTextArea(string const& title)
    : myGUI{ new QMainWindow }, textedit{ new QPlainTextEdit{ myGUI } } {
    QFont font("Monospace");
    font.setStyleHint(QFont::Monospace);
    textedit->setFont(font);
    textedit->setReadOnly(true);
    myGUI->setCentralWidget(textedit);
    myGUI->setFixedSize(300, 600);
    myGUI->setWindowTitle(QString::fromStdString(title));
    myGUI->show();
}

void GuiTextArea::print(string const& s) {
    textedit->setPlainText(textedit->toPlainText() + QString::fromStdString(s));
}

void GuiTextArea::println(string const& s) {
    print(s + '\n');
}

void GuiTextArea::println() {
    println("");
}

#include "GuiTextArea.h"

#include <QFont>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QString>

using namespace std;

/*
 * Set window info and text style to make the window system more user friendly
 */
GuiTextArea::GuiTextArea(string const& title)
    : myGUI{ new QMainWindow }, textedit{ new QPlainTextEdit{ myGUI } } {
    QFont font("Monospace");
    font.setStyleHint(QFont::Monospace);
    textedit->setFont(font);
    textedit->setReadOnly(true);
    myGUI->setCentralWidget(textedit);
    myGUI->setMinimumSize(600, 600);
    myGUI->setWindowTitle(QString::fromStdString(title));
    myGUI->show();
}

/*
 * Update the text for the window.
 * This is expensive, should prefer to build up strings before calling this!
 */
void GuiTextArea::print(string const& s) {
    textedit->setPlainText(textedit->toPlainText() + QString::fromStdString(s));
}

void GuiTextArea::println(string const& s) {
    print(s + '\n');
}

void GuiTextArea::println() {
    println("");
}

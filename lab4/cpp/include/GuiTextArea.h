#pragma once

#include <QMainWindow>
#include <QPlainTextEdit>
#include <string>

class GuiTextArea {
public:
    GuiTextArea(const GuiTextArea&) = delete;
    // RouterNode requires its GuiTextArea instance to have a move constructor
    GuiTextArea(GuiTextArea&&) = default;
    GuiTextArea& operator=(const GuiTextArea&) = delete;
    GuiTextArea& operator=(GuiTextArea&&) = delete;

    GuiTextArea(std::string const&);
    void print(std::string const&);
    void println(std::string const&);
    void println();

private:
    QMainWindow* myGUI;
    QPlainTextEdit* textedit;
};

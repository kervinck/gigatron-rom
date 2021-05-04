#ifndef DIALOG_H
#define DIALOG_H

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include "graphics.h"


#define MAX_DIALOG_COLS  16
#define MAX_DIALOG_ROWS  16
#define DEF_DIALOG_COLS  6
#define DEF_DIALOG_ROWS  5


namespace Dialog
{
    class Item
    {
    public:
        enum JustifyX {LeftX=-3, RightX=-2, CenterX=-1};
        enum JustifyY {CurrentY=-2, NextY=-1};
        enum Colour {Bg=-3, Fg=-2, Bd=-1};

    private:
        int _index;
        int _jstX = CenterX;
        int _jstY = NextY;
        int _posX = 0;
        int _posY = 0;
        Colour _colour = Bg;
        std::string _text;
        bool _isInteractable = false;

    public:
        Item();
        Item(bool isInteractable, const std::string& text);
        Item(int index, bool isInteractable, const std::string& text);
        Item(bool isInteractable, const std::string& text, int jstX);
        Item(bool isInteractable, const std::string& text, int jstX, Colour colour);
        Item(bool isInteractable, const std::string& text, int jstX, int jstY);
        Item(bool isInteractable, const std::string& text, int jstX, int jstY, Colour colour);

    public:
        int getIndex(void);
        int getJstX(void);
        int getJstY(void);
        int getPosX(void);
        int getPosY(void);
        Colour getColour(void);
        const std::string& getText(void);
        bool getIsInteractable(void);

        void setIndex(int index);
        void setPosX(int posX);
        void setPosY(int posY);
        void setColour(Colour colour);
        void setText(const std::string& text);
    };


    class Dialog
    {
    public:
        enum Size {Regular, DoubleWidth};

    private:
        bool _isActive = false;

        int _maxCols = MAX_SCREEN_COLS;
        int _maxRows = MAX_SCREEN_ROWS; 
        int _numCols = DEF_DIALOG_COLS;
        int _numRows = DEF_DIALOG_ROWS;
        int _dialogX = 0;
        int _dialogY = 0;
        int _offsetX = 2;
        int _offsetY = 5;
        int _gigaX   = 0;
        int _gigaY   = 0;

        std::string _text;
        std::vector<Item> _items;

        Size _size = Regular;

    public:
        int getDialogX(void);
        int getDialogY(void);
        bool getIsActive(void);
        Size getSize(void);
        bool getDialogItem(int index, Item& item);
        bool setDialogItem(int index, Item& item);
        bool setDialogItemText(int index, const std::string& text);

        std::vector<Item>& getItems(void);

        void setIsActive(bool isActive);
        void setMaxColsRows(int maxCols, int maxRows);
        void setSize(Size size);

        bool create(const std::string& name, const std::vector<Item>& items, int numCols, int numRows, Size size=DoubleWidth, int offsetX=2, int offsetY=5,
                                                                                                       int maxCols=MAX_SCREEN_COLS, int maxRows=MAX_SCREEN_ROWS);
        void gigaToDialog(int gigaX, int gigaY);
        void mouseToDialogSpace(int mouseX, int mouseY, int& dspaceX, int& dspaceY);
        int getDialogItemIndex(int mouseX, int mouseY);

        void render(int mouseX, int mouseY);
    };


    void initialise(void);

    bool getDialog(const std::string& name, Dialog& dialog);
    bool getDialogItem(const std::string& name, int index, Item& item);
    int getDialogItemIndex(const std::string& name, int mouseX, int mouseY);

    bool setDialogItem(const std::string& name, int index, Item& item);
    bool setDialogItemText(const std::string& name, int index, const std::string& text);

    bool createDialog(const std::string& name, const std::string& text, const std::vector<Item>& items, int numCols, int numRows, Dialog::Size size=Dialog::DoubleWidth,
                                                                                                        int offsetX=2, int offsetY=5, int maxCols=MAX_SCREEN_COLS, int maxRows=MAX_SCREEN_ROWS);
    bool positionDialog(const std::string& name, int gigaX, int gigaY);
    bool renderDialog(const std::string& name, int mouseX, int mouseY);
}

#endif
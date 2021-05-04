#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

#include "graphics.h"


#define MAX_MENU_COLS  16
#define MAX_MENU_ROWS  16
#define DEF_MENU_COLS  6
#define DEF_MENU_ROWS  5


namespace Menu
{
    class Item
    {
        int _index;
        std::string _name;
        bool _isActive = false;

    public:
        Item(int index, const std::string& name)
        {
            _index = index;
            _name = name;
        }

    public:
        int getIndex(void);
        const std::string& getName(void);

        void setIndex(int index);
        void setName(const std::string& name);
    };

    class Menu
    {
        bool _isActive = false;

        int _maxCols = MAX_SCREEN_COLS;
        int _maxRows = MAX_SCREEN_ROWS; 
        int _numCols = DEF_MENU_COLS;
        int _numRows = DEF_MENU_ROWS;
        int _numItems = DEF_MENU_ROWS - 1;
        int _menuX = 0;
        int _menuY = 0;
        int _itemX = 0;
        int _itemY = 0;
        int _itemIndex = -1;

        std::string _name;
        std::vector<Item> _items;

    public:
        int getMenuX(void);
        int getMenuY(void);
        int getNumItems(void);
        int getItemIndex(void);
        bool getIsActive(void);

        std::vector<Item>& getItems(void);

        void setIsActive(bool isActive);
        void setItemIndex(int itemIndex);
        void setMaxColsRows(int maxCols, int maxRows);

        bool create(const std::string& name, const std::vector<std::string>& items, int numCols, int numRows, int maxCols, int maxRows);

        void mouseToMenu(int mouseX, int mouseY);
        int mouseToItem(int mouseX, int mouseY);

        void render(void);
    };


    void initialise(void);

    bool getMenu(const std::string& name, Menu& menu);
    bool createMenu(const std::string& name, const std::vector<std::string>& items, int numCols, int numRows, int maxCols=MAX_SCREEN_COLS, int maxRows=MAX_SCREEN_ROWS-1);
    bool captureMenu(const std::string& name, int mouseX, int mouseY);
    bool captureItem(const std::string& name, int mouseX, int mouseY);
    bool getMenuItemIndex(const std::string& name, int& itemIndex);
    bool renderMenu(const std::string& name);
}

#endif
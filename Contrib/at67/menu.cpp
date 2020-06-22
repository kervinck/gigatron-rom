#include "menu.h"


namespace Menu
{
    std::map<std::string, Menu> _menuMap;


    int Item::getIndex(void) {return _index;}
    const std::string& Item::getName(void) {return _name;}

    void Item::setIndex(int index) {_index = index;}
    void Item::setName(const std::string& name) {_name = name;}

    int  Menu::getMenuX(void) {return _menuX;}
    int  Menu::getMenuY(void) {return _menuY;}
    int  Menu::getNumItems(void) {return _numItems;}
    int  Menu::getItemIndex(void) {return _itemIndex;}
    bool Menu::getIsActive(void) {return _isActive;}

    std::vector<Item>& Menu::getItems(void) {return _items;}

    void Menu::setIsActive(bool isActive) {_isActive = isActive;}
    void Menu::setItemIndex(int itemIndex) {_itemIndex = itemIndex;}
    void Menu::setMaxColsRows(int maxCols, int maxRows) {_maxCols = maxCols; _maxRows = maxRows;}


    bool Menu::create(const std::string& name, const std::vector<std::string>& items, int numCols, int numRows, int maxCols, int maxRows)
    {
        _maxCols = maxCols;
        _maxRows = maxRows;
        _numCols = numCols;
        _numRows = numRows;
        _numItems = int(items.size());
        _name = name;

        for(int i=0; i<int(items.size()); i++)
        {
            Item item = {i, items[i]};
            _items.push_back(item);
        }

        return true;
    }

    void convertToMenuSpace(int mouseX, int mouseY, int& mspaceX, int& mspaceY)
    {
        // Normalise mouse coords and convert to menu space
        float mx = float(mouseX) / float(Graphics::getWidth());
        float my = float(mouseY) / float(Graphics::getHeight());
        mspaceX = int(mx * float(SCREEN_WIDTH)) / FONT_WIDTH;
        mspaceY = int(my * float(SCREEN_HEIGHT)) / (FONT_HEIGHT+2);
    }

    void Menu::mouseToMenu(int mouseX, int mouseY)
    {
        convertToMenuSpace(mouseX, mouseY, _menuX, _menuY);

        if(_menuX > (_maxCols-_numCols)) _menuX = (_maxCols-_numCols);
        if(_menuY > (_maxRows-_numRows)) _menuY = (_maxRows-_numRows);
    }

    int Menu::mouseToItem(int mouseX, int mouseY)
    {
        convertToMenuSpace(mouseX, mouseY, _itemX, _itemY);

        int menuIdx = std::min(std::max(0, (_itemY - _menuY)), _numItems) - 1;
        bool onMenuX = ((_itemX - _menuX) >= 0)  &&  ((_itemX - _menuX) < _numCols);
        bool onMenuY = ((_itemY - _menuY) >= 0)  &&  ((_itemY - _menuY) < _numRows);

        _itemIndex = (onMenuX && onMenuY) ? menuIdx : -1;
        return _itemIndex;
    }

    void Menu::render(void)
    {
        // Heading
        Graphics::drawMenuItem(_items[0].getName(), _menuX * FONT_WIDTH, _menuY * (FONT_HEIGHT+2), 0xFFFFFFFF, true, 80, 0x55555555);

        // Items
        for(int i=1; i<_numItems; i++)
        {
            Graphics::drawMenuItem(_items[i].getName(), _menuX * FONT_WIDTH, (_menuY + i) * (FONT_HEIGHT+2), 0x55555555, (i != _itemIndex + 1), 80, 0xFFFFFFFF);
        }
    }


    void initialise(void)
    {
    }


    bool getMenu(const std::string& name, Menu& menu)
    {
        if(_menuMap.find(name) == _menuMap.end()) return false;

        menu = _menuMap[name];

        return true;
    }

    bool createMenu(const std::string& name, const std::vector<std::string>& items, int numCols, int numRows, int maxCols, int maxRows)
    {
        if(_menuMap.find(name) != _menuMap.end()) return false;

        Menu menu;
        menu.create(name, items, numCols, numRows, maxCols, maxRows);
        _menuMap[name] = menu;

        return true;
    }

    bool captureMenu(const std::string& name, int mouseX, int mouseY)
    {
        if(_menuMap.find(name) == _menuMap.end()) return false;

        _menuMap[name].mouseToMenu(mouseX, mouseY);

        return true;
    }

    bool captureItem(const std::string& name, int mouseX, int mouseY)
    {
        if(_menuMap.find(name) == _menuMap.end()) return false;

        _menuMap[name].mouseToItem(mouseX, mouseY);

        return true;
    }

    bool getMenuItemIndex(const std::string& name, int& itemIndex)
    {
        if(_menuMap.find(name) == _menuMap.end())
        {
            itemIndex = -1;
            return false;
        }

        itemIndex = _menuMap[name].getItemIndex();
        _menuMap[name].setItemIndex(-1);
        _menuMap[name].setIsActive(false);

        return true;
    }

    bool renderMenu(const std::string& name)
    {
        if(_menuMap.find(name) == _menuMap.end()) return false;

        _menuMap[name].render();
        _menuMap[name].setIsActive(true);

        return true;
    }
}
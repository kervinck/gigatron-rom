#include "dialog.h"
#include "graphics.h"


namespace Dialog
{
    int _uniqueItemId = 0;

    std::map<std::string, Dialog> _dialogMap;


    //*****************************************************************************************************************
    // Item object
    //*****************************************************************************************************************
    Item::Item()
    {
        _index = _uniqueItemId++;
    }

    Item::Item(bool isInteractable, const std::string& text)
    {
        _index = _uniqueItemId++;
        _isInteractable = isInteractable;
        _text = text;
    }

    Item::Item(int index, bool isInteractable, const std::string& text)
    {
        _index = index;
        _isInteractable = isInteractable;
        _text = text;
    }

    Item::Item(bool isInteractable, const std::string& text, int jstX) : Item(isInteractable, text)
    {
        _jstX = jstX;
    }

    Item::Item(bool isInteractable, const std::string& text, int jstX, Colour colour) : Item(isInteractable, text, jstX)
    {
        _colour = colour;
    }

    Item::Item(bool isInteractable, const std::string& text, int jstX, int jstY) : Item(isInteractable, text, jstX)
    {
        _jstY = jstY;
    }

    Item::Item(bool isInteractable, const std::string& text, int jstX, int jstY, Colour colour) : Item(isInteractable, text, jstX, jstY)
    {
        _colour = colour;
    }


    int Item::getIndex(void) {return _index;}
    int Item::getJstX(void) {return _jstX;}
    int Item::getJstY(void) {return _jstY;}
    int Item::getPosX(void) {return _posX;}
    int Item::getPosY(void) {return _posY;}
    Item::Colour Item::getColour(void) {return _colour;}
    const std::string& Item::getText(void) {return _text;}
    bool Item::getIsInteractable(void) {return _isInteractable;}

    void Item::setIndex(int index) {_index = index;}
    void Item::setPosX(int posX) {_posX = posX;}
    void Item::setPosY(int posY) {_posY = posY;}
    void Item::setColour(Item::Colour colour) {_colour = colour;}
    void Item::setText(const std::string& text) {_text = text;}


    //*****************************************************************************************************************
    // Dialog object
    //*****************************************************************************************************************
    int  Dialog::getDialogX(void) {return _dialogX;}
    int  Dialog::getDialogY(void) {return _dialogY;}
    bool Dialog::getIsActive(void) {return _isActive;}
    Dialog::Size Dialog::getSize(void) {return _size;}

    std::vector<Item>& Dialog::getItems(void) {return _items;}

    void Dialog::setIsActive(bool isActive) {_isActive = isActive;}
    void Dialog::setMaxColsRows(int maxCols, int maxRows) {_maxCols = maxCols; _maxRows = maxRows;}
    void Dialog::setSize(Dialog::Size size) {_size = size;}

    bool Dialog::getDialogItem(int index, Item& item)
    {
        if(index >=0  &&  index < int(_items.size()))
        {
            item = _items[index]; 
            return true;
        }

        return false;
    }

    int Dialog::getDialogItemIndex(int mouseX, int mouseY)
    {
        for(int i=0; i<int(_items.size()); i++)
        {
            if(_items[i].getIsInteractable())
            {
                int dx, dy;
                mouseToDialogSpace(mouseX, mouseY, dx, dy);

                int x = _items[i].getPosX();
                int y = _items[i].getPosY();
                int ox = int(_items[i].getText().size()) * FONT_WIDTH;
                if(dx >= x  &&  dx < x + ox  &&  dy >= y  &&  dy < y + FONT_HEIGHT)
                {
                    return i;
                }
            }
        }

        return -1;
    }

    bool Dialog::setDialogItem(int index, Item& item)
    {
        if(index >=0  &&  index < int(_items.size()))
        {
            _items[index] = item;
            return true;
        }

        return false;
    }

    bool Dialog::setDialogItemText(int index, const std::string& text)
    {
        if(index >=0  &&  index < int(_items.size()))
        {
            _items[index].setText(text);
            return true;
        }

        return false;
    }

    bool Dialog::create(const std::string& text, const std::vector<Item>& items, int numCols, int numRows, Size size, int offsetX, int offsetY, int maxCols, int maxRows)
    {
        _text     = text;
        _items    = items;
        _numCols  = numCols;
        _numRows  = numRows;
        _size     = size;
        _offsetX  = offsetX;
        _offsetY  = offsetY;
        _maxCols  = maxCols;
        _maxRows  = maxRows;

        return true;
    }

    void Dialog::gigaToDialog(int gigaX, int gigaY)
    {
        // Convert to dialog space
        _dialogX = int(float(gigaX) * 4.0f / FONT_WIDTH);
        _dialogY = int(float(gigaY) * 4.0f / FONT_HEIGHT);

        _gigaX = gigaX;
        _gigaY = gigaY;

        if(_dialogX > (_maxCols-_numCols)) _dialogX = (_maxCols-_numCols);
        if(_dialogY > (_maxRows-_numRows)) _dialogY = (_maxRows-_numRows);
    }

    void Dialog::mouseToDialogSpace(int mouseX, int mouseY, int& dspaceX, int& dspaceY)
    {
        // Normalise mouse coords and convert to dialog space
        float mx = float(mouseX) / float(Graphics::getWidth());
        float my = float(mouseY) / float(Graphics::getHeight());
        dspaceX = int(mx * float(SCREEN_WIDTH));
        dspaceY = int(my * float(SCREEN_HEIGHT));
    }

    void Dialog::render(int mouseX, int mouseY)
    {
        // Width and height in giga coords
        int w = 0;
        int h = _offsetY*2;
        for(int i=0; i<int(_items.size()); i++)
        {
            if(_items[i].getJstY() == Item::NextY) h += 2;

            int size = int(_items[i].getText().size());
            if(size > w) w = size;
        }
        if(_items.size() > 1  &&  (w == 0  ||  int(_text.size()) > w))
        {
            w = int(_text.size());
        }
        w += _offsetX*2;

        // Size
        int scale = (_size == DoubleWidth) ? 2 : 1;

        // Border and interior
        Graphics::drawDialog(int16_t(_gigaX), int16_t(_gigaY), int16_t(w*2*scale), int16_t(h), 0x55555555, 0x88888888);

        int tx = _gigaX*3 + 1;
        int ty = (_gigaY + _offsetY + 1)*4;

        // Title
        int ox = (w*FONT_WIDTH - int(_text.size())*FONT_WIDTH/2);

        // Items
        for(int i=0; i<int(_items.size()); i++)
        {
            // Justification
            switch(_items[i].getJstX())
            {
                case Item::LeftX:   ox = (w*FONT_WIDTH*scale/4   - int(_items[i].getText().size())*FONT_WIDTH/2); break;
                case Item::CenterX: ox = (w*FONT_WIDTH*scale/2   - int(_items[i].getText().size())*FONT_WIDTH/2); break;
                case Item::RightX:  ox = (w*FONT_WIDTH*scale*3/4 - int(_items[i].getText().size())*FONT_WIDTH/2); break;

                default: break;
            }

            // Different background colour for interactable items
            uint32_t bgColour = 0x55555555;
            switch(_items[i].getColour())
            {
                case Item::Bg: bgColour = 0x55555555; break;
                case Item::Fg: bgColour = 0xFFFFFFFF; break;
                case Item::Bd: bgColour = 0x88888888; break;

                default: break;
            }

            // Highlight interactable item for mouse over
            uint32_t fgColour = 0xFFFFFFFF;
            if(_items[i].getIsInteractable())
            {
                int index = getDialogItemIndex(mouseX, mouseY);
                if(index == i) std::swap(fgColour, bgColour);
            }
            else
            {
                fgColour = 0xAAAAAAAA;
            }

            // Multiple items per line
            if(_items[i].getJstY() == Item::CurrentY)
            {
                ty -= (FONT_HEIGHT);
            }

            // Save item position in screen coords
            _items[i].setPosX((tx + ox));
            _items[i].setPosY(ty);

            Graphics::drawDialogItem(_items[i].getText(), tx + ox, ty, fgColour, bgColour);

            // Next line
            ty += (FONT_HEIGHT);
        }
    }


    //*****************************************************************************************************************
    // Dialog
    //*****************************************************************************************************************
    void initialise(void)
    {
    }

    bool getDialog(const std::string& name, Dialog& dialog)
    {
        if(_dialogMap.find(name) == _dialogMap.end()) return false;

        dialog = _dialogMap[name];

        return true;
    }

    bool getDialogItem(const std::string& name, int index, Item& item)
    {
        if(_dialogMap.find(name) == _dialogMap.end()) return false;

        return _dialogMap[name].getDialogItem(index, item);
    }

    int getDialogItemIndex(const std::string& name, int mouseX, int mouseY)
    {
        if(_dialogMap.find(name) == _dialogMap.end()) return false;

        return _dialogMap[name].getDialogItemIndex(mouseX, mouseY);
    }

    bool setDialogItem(const std::string& name, int index, Item& item)
    {
        if(_dialogMap.find(name) == _dialogMap.end()) return false;

        return _dialogMap[name].setDialogItem(index, item);
    }

    bool setDialogItemText(const std::string& name, int index, const std::string& text)
    {
        if(_dialogMap.find(name) == _dialogMap.end()) return false;

        return _dialogMap[name].setDialogItemText(index, text);
    }

    bool createDialog(const std::string& name, const std::string& text, const std::vector<Item>& items, int numCols, int numRows, Dialog::Size size, int offsetX, int offsetY, int maxCols, int maxRows)
    {
        if(_dialogMap.find(name) != _dialogMap.end()) return false;

        Dialog dialog;
        dialog.create(text, items, numCols, numRows, size, offsetX, offsetY, maxCols, maxRows);
        _dialogMap[name] = dialog;

        return true;
    }

    bool positionDialog(const std::string& name, int gigaX, int gigaY)
    {
        if(_dialogMap.find(name) == _dialogMap.end()) return false;

        _dialogMap[name].gigaToDialog(gigaX, gigaY);

        return true;
    }

    bool renderDialog(const std::string& name, int mouseX, int mouseY)
    {
        if(_dialogMap.find(name) == _dialogMap.end()) return false;

        _dialogMap[name].render(mouseX, mouseY);
        _dialogMap[name].setIsActive(true);

        return true;
    }
}
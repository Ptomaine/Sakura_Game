#pragma once

/***************************************************************************
 *   Copyright (C) 2008 by Arlen Albert Keshabyan                          *
 *   <arlen.albert@gmail.com>                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "juce/juce_amalgamated.h"
#include "version.h"

#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <map>
#include <set>
#include <list>
#include <functional>
#include <ctime>
#include <cmath>
//#include <tr1/unordered_map>

#define sakura_map std::map
//#define sakura_map std::tr1::unordered_map

// This class helps programmers to get prepared to the new upcoming C++ standard.
// Although, this class does not fully conform to the new nullptr requirements but it is very close to those ones.
// Just remove this class declaration right after you get the compiler that supports the new C++ standard.
const class
{
public:
	template<class T> operator T*() const // convertible to any type of null non-member
    {
    	return 0; // pointer...
	}

	template<class C, class T> operator T C::*() const // or any type of null member pointer...
    {
    	return 0;
	}

private:
	void operator&() const; // whose address can't be taken
}
nullptr = {}; // and whose name is nullptr


namespace Direction
{
	enum direction
	{
		Left = 0,
		Top,
		Right,
		Bottom
	};
}

typedef std::deque<bool> DirectionSetType;

class CellDirections
{
private:

	DirectionSetType _directions;

public:

	CellDirections() : _directions(4, false)
	{
	}

	CellDirections(const CellDirections &copy) : _directions(4, false)
	{
		std::copy(copy._directions.begin(), copy._directions.end(), _directions.begin());
	}

	CellDirections(const DirectionSetType &directionsCopy) : _directions(4, false)
	{
		std::copy(directionsCopy.begin(), directionsCopy.end(), _directions.begin());
	}

	CellDirections &operator = (const CellDirections &copy)
	{
		std::copy(copy._directions.begin(), copy._directions.end(), _directions.begin());

		return *this;
	}

	DirectionSetType getDirections() const
	{
		return _directions;
	}

	void setDirection(int direction, const bool value)
	{
		_directions[direction] = value;
	}

	bool getDirection(int direction) const
	{
		return _directions[direction];
	}

	void rotate(const bool left = true)
	{
		std::rotate(_directions.begin(), ((left) ? _directions.begin() + 1 : _directions.end() - 1), _directions.end());
	}

	void reset()
	{
		std::fill(_directions.begin(), _directions.end(), false);
	}

	bool empty() const
	{
		return count(true) == 0;
	}

	bool notEmpty() const
	{
		return !empty();
	}

	int count(bool value = true) const
	{
		return std::count(_directions.begin(), _directions.end(), value);
	}

	std::string to_string()
	{
		std::stringstream s;

		std::copy(_directions.begin(), _directions.end(), std::ostream_iterator<int>(s, ""));

		return s.str();
	}

	bool operator == (const CellDirections &directions) const
	{
		return std::equal(_directions.begin(), _directions.end(), directions._directions.begin());
	}

	std::pair<int, int> getConnectionType() const
	{
		int type(int(std::count(_directions.begin(), _directions.end(), true)));
		int sub_type(0); // straight

		if(type == 2)
		{
			DirectionSetType connected1(2, true);
			DirectionSetType connected2(2, false);

			if(std::search(_directions.begin(), _directions.end(), connected1.begin(), connected1.end()) != _directions.end() ||
				std::search(_directions.begin(), _directions.end(), connected2.begin(), connected1.end()) != _directions.end())
				sub_type = 1; // gay
		}

		return std::pair<int, int>(type, sub_type);
	}

	std::string dump(bool add_end_of_line = false) const
	{
		std::stringstream s;
		std::copy(_directions.begin(), _directions.end(), std::ostream_iterator<int>(s, ""));

		if(add_end_of_line)
			s << std::endl;

		return s.str();
	}

	std::string dumpLevel(const int level)
	{
		std::stringstream s;

		if(empty())
		{
			s << "`" << "`" << "`";
		}
		else
		{
			switch(level)
			{
			case 0:
				s << " " << (_directions[1] ? "*" : " ") << " ";
				break;

			case 1:
				s << (_directions[0] ? "*" : " ") << ((_directions[0] || _directions[1] || _directions[2] || _directions[3]) ? "*" : " ") << (_directions[2] ? "*" : " ");
				break;

			case 2:
				s << " " << (_directions[3] ? "*" : " ") << " ";
				break;
			}
		}

		return s.str();
	}
};

class Cell
{
protected:

	CellDirections _originalState;
	CellDirections _currentState;

public:

	Cell()
	{
	}

	DirectionSetType getDirections(bool original = false) const
	{
		if(original)
			return _originalState.getDirections();

		return _currentState.getDirections();
	}

	void putInPlace()
	{
		_currentState = _originalState;
	}

	bool isInPlace() const
	{
		return (_currentState == _originalState);
	}

	void rotate(bool left = true)
	{
		_currentState.rotate(left);
	}

	void reset()
	{
		_originalState.reset();
		_currentState.reset();
	}

	void setDirection(int direction, const bool value)
	{
		_originalState.setDirection(direction, value);
		_currentState.setDirection(direction, value);
	}

	void setDirection(int direction, const bool value, const bool original)
	{
		if(original)
			_originalState.setDirection(direction, value);
		else
			_currentState.setDirection(direction, value);
	}

	bool getDirection(int direction, bool original = false) const
	{
		if(original)
			return _originalState.getDirection(direction);

		return _currentState.getDirection(direction);
	}

	std::pair<int, int> getConnectionType(bool original = false) const
	{
		if(original)
			return _originalState.getConnectionType();

		return _currentState.getConnectionType();
	}

	bool empty() const
	{
		return _originalState.empty();
	}

	bool notEmpty() const
	{
		return _originalState.notEmpty();
	}

	int count(bool original = false, bool value = true) const
	{
		if(original)
			return _originalState.count(value);

		return _currentState.count(value);
	}

	std::string to_string(bool original = false)
	{
		if(original)
			return _originalState.to_string();

		return _currentState.to_string();
	}

	std::string dump(bool add_end_of_line = false, bool original = false) const
	{
		if(original)
			return _originalState.dump(add_end_of_line);

		return _currentState.dump(add_end_of_line);
	}

	std::string dumpLevel(const int level, bool original = false)
	{
		if(original)
			return _originalState.dumpLevel(level);

		return _currentState.dumpLevel(level);
	}
};

class SakuraMatrix;

class CellComponent : public Cell, public Component, public Timer
{
private:

	typedef sakura_map<std::string, sakura_map<bool, Image*> > CellFiguresType;

	int _live;
	int _requested_live;
	bool _force_redraw;
	bool _drawOriginal;
	bool _drawFocus;

	SakuraMatrix *_pParentComponent;

public:

	enum CellState
	{
		Dead = 0,
		Alive = 1,
		Solved = 2
	};

	CellComponent(SakuraMatrix *pParentComponent) :
		_live(false),
		_requested_live(false),
		_force_redraw(false),
		_drawOriginal(false),
		_drawFocus(false),
		_pParentComponent(pParentComponent)
	{
	}

	~CellComponent()
	{
	}

	void paint(Graphics &g);
	void resized();

	void setLive(int live)
	{
		_requested_live = live;
	}

	int getLive() const
	{
		return _live;
	}

	int getRequestedLive() const
	{
		return _requested_live;
	}

	void forceRedraw()
	{
		_force_redraw = true;
	}

	void repaintLiveIfNeeded()
	{
		if(_requested_live != _live || _force_redraw)
		{
			_live = _requested_live;
			_force_redraw = false;
			repaint();
		}
	}

	void rotateRandomly()
	{
		bool left = (std::rand() % 2) == 0;
		int count = std::rand() % 4;

		if(!(count & 1) && !left)
			count = (count + 1) % 4;

		for(int idx = 0; idx < count; idx++)
			rotate(left);

        _force_redraw = true;
	}

	void setDrawOriginal(bool original)
	{
		if(_drawOriginal != original)
		{
			_drawOriginal = original;

			repaint();
		}
	}

	void setDrawFocus(bool draw)
	{
		if(_drawFocus != draw)
		{
			_drawFocus = draw;

			repaint();
		}
	}

	bool isFocusDrawn()
	{
		return _drawFocus;
	}

	void reset()
	{
		_drawFocus = false;

		Cell::reset();
	}

	virtual void timerCallback();

	virtual void mouseDown(const MouseEvent &);
	virtual void mouseWheelMove(const MouseEvent &, float, float);

	/*void connectCells(const CellComponent *pLeftNeighbour, const CellComponent *pTopNeighbour, const CellComponent *pRightNeighbour, const CellComponent *pBottomNeighbour) const
	{
		_neighbours[int(Direction::Left)] = pLeftNeighbour;
		_neighbours[int(Direction::Top)] = pTopNeighbour;
		_neighbours[int(Direction::Right)] = pRightNeighbour;
		_neighbours[int(Direction::Bottom)] = pBottomNeighbour;
	}

	const CellComponent &connectCell(int direction, const CellComponent *pNeighbour) const
	{
		_neighbours[direction] = pNeighbour;

		return *this;
	}

	void interconnectCells(int direction, const CellComponent *pNeighbour) const
	{
		pNeighbour->connectCell(direction, this);
		_neighbours[(direction + 2) % 4] = pNeighbour;
	}*/
};

class StaticGradientComponent : public Component
{
private:
	GradientBrush *_pBrush;
	Colour _brushColour;
	bool _isRadial;

public:

	StaticGradientComponent(String name = String::empty) : Component(name),
		_pBrush(0),
		_brushColour(Colours::black),
		_isRadial(false)
	{
	}

	~StaticGradientComponent()
	{
		delete _pBrush;
	}

	void setColour(const Colour color)
	{
		if(_brushColour != color)
		{
			_brushColour = color;
			resized();

			repaint();
		}
	}

	Colour getColour() const
	{
		return _brushColour;
	}

	void setRadial(bool radial)
	{
		if(_isRadial != radial)
		{
			_isRadial = radial;
			resized();

			repaint();
		}
	}

	bool isRadial() const
	{
		return _isRadial;
	}

	void resized()
	{
		delete _pBrush;

		_pBrush = new GradientBrush(_brushColour, getWidth() / 2, getHeight() / 2, _brushColour.withAlpha(.0f), .0f, 0.f, _isRadial);
	}

	void paint(Graphics &g)
	{
		g.setBrush(_pBrush);
		g.fillAll();
	}
};

class CommandButton : public Button
{
private:
	Image *_pImage;

public:

	CommandButton(String sName) : Button(sName), _pImage(nullptr)
	{
	}

	void paintButton(Graphics &g, bool isMouseOverButton, bool isButtonDown)
	{
		g.drawImageAt(_pImage, 0, 0);
	}

	void setImage(Image *pImage)
	{
	}

	virtual void resized()
	{
		drawButton();
	}

	void drawButton()
	{
		int width = getWidth();
		int height = getHeight();

		if(_pImage && _pImage->getWidth() == width && _pImage->getHeight() == height)
			return;

		delete _pImage;

		_pImage = new Image(Image::ARGB, width, height, false);

		Graphics g(*_pImage);

		g.drawRect(0, 0, width, height, 1);
	}
};

class MainWindow;
class SakuraMatrix;

class StatusBarComponent : public Component, public ButtonListener
{
private:

	Font _font;
	Colour _foreColour;
	Colour _backColour;
	String _text;

	Button *_pGenerateButton;
	Button *_pShuffleButton;
	Button *_pPeekModeButton;
	Button *_pSettingsButton;

	SakuraMatrix *_pMatrix;

	Image *_pBackImage;

public:

	StatusBarComponent(SakuraMatrix *matrix) : Component(T("StatusBar")),
		_foreColour(Colours::yellowgreen),
		_backColour(Colours::orange),
		_text(String::empty),
		_pMatrix(matrix),
		_pBackImage(nullptr)
	{
		_font.setBold(true);

		addAndMakeVisible(_pGenerateButton = createButton(Colours::white));
		_pGenerateButton->addButtonListener(this);

		addAndMakeVisible(_pShuffleButton = createButton(Colours::darkgreen));
		_pShuffleButton->addButtonListener(this);

		addAndMakeVisible(_pPeekModeButton = createButton(Colours::aquamarine));
		_pPeekModeButton->addMouseListener(this, true);

		addAndMakeVisible(_pSettingsButton = createButton(Colours::darkviolet));
		_pSettingsButton->addButtonListener(this);
	}

	~StatusBarComponent()
	{
		deleteAllChildren();

		delete _pBackImage;
	}

	void mouseDown(const MouseEvent &e);
	void mouseUp(const MouseEvent &e);

	void setText(String text)
	{
		_text = text;

		repaint();
	}

	void resized()
	{
		delete _pBackImage;
		_pBackImage = 0;

		_font.setHeight(getHeight() * .7f);

		int width = getWidth();

		_pSettingsButton->setBounds(width - 24, 4, 24, 24);
		_pPeekModeButton->setBounds(width - 48, 4, 24, 24);
		_pShuffleButton->setBounds(width - 72, 4, 24, 24);
		_pGenerateButton->setBounds(width - 96, 4, 24, 24);
	}

	void paint(Graphics &g)
	{
		int width = getWidth();
		int height = getHeight();

		if(!_pBackImage)
		{
			_pBackImage = new Image(Image::ARGB, width, height, true);
			Graphics gg(*_pBackImage);

			/*LookAndFeel::drawGlassLozenge(gg, 0, 0,
								width, height, _backColour, 1.0f, 10.0f,
								true, true, true, true);*/
			GradientBrush br(Colour(0x00ffea95), 0, 0, Colour(0xffffea95).darker(), 0, height, false);
			gg.setBrush(&br);
			gg.fillAll();
		}

		g.drawImageAt(_pBackImage, 0, 0);

		g.setFont(_font);
		g.setColour(_foreColour);
		g.drawFittedText (_text,
						  5,
						  5,
						  width - 40,
						  height - 10,
						  Justification::centredLeft, 1);
	}

	void setColours(Colour fore, Colour back)
	{
		_foreColour = fore;
		_backColour = back;
	}

	Colour getForeColour() const
	{
		return _foreColour;
	}

	Colour getBackColour() const
	{
		return _backColour;
	}

	void buttonClicked(Button *button);

	Button *createButton(const Colour color)
	{
		Path p;
		p.addEllipse (-10.0f, -10.0f, 120.0f, 120.0f);

		DrawablePath ellipse;
		ellipse.setPath (p);
		ellipse.setSolidFill (Colours::white.withAlpha (0.6f));

		p.clear();
		p.addEllipse (0.0f, 0.0f, 100.0f, 100.0f);
		p.addEllipse (25.f, 25.f, 50.f, 50.f);

		p.setUsingNonZeroWinding (false);

		DrawablePath dp;
		dp.setPath (p);
		dp.setSolidFill (color.withAlpha (0.35f));

		DrawableComposite normalImage;
		normalImage.insertDrawable (ellipse);
		normalImage.insertDrawable (dp);

		dp.setSolidFill (color.withAlpha (0.8f));

		DrawableComposite overImage;
		overImage.insertDrawable (ellipse);
		overImage.insertDrawable (dp);

		DrawableButton* db = new DrawableButton (T("tabs"), DrawableButton::ImageFitted);
		db->setImages (&normalImage, &overImage, 0);
		return db;
	}
};

template<class T>
class IntValueTextPropertyComponent : public TextPropertyComponent, public SettableTooltipClient
{
private:

	T *_component;
	int &_value;

public:

	IntValueTextPropertyComponent(const String name, int &value, T *component = nullptr) : TextPropertyComponent(name, 0, false),
		_component(component), _value(value)
	{
	}

	void setText(const String &newText)
	{
		_value = newText.getIntValue();

		if(_component)
			_component->propertyChangeNotify(this);
	}

	const String getText () const
	{
		return String(_value);
	}
};

template<class T>
class BooleanValuePropertyComponent : public BooleanPropertyComponent, public SettableTooltipClient
{
private:

	T *_component;
	bool &_value;

public:

	BooleanValuePropertyComponent(const String name, const String onText, const String offText, bool &value, T *component = nullptr) : BooleanPropertyComponent(name, onText, offText),
		_component(component), _value(value)
	{
		refresh();
	}

	void setState(const bool newState)
	{
		_value = newState;

		refresh();

		if(_component)
			_component->propertyChangeNotify(this);
	}

	bool getState() const
	{
		return _value;
	}
};

template<class T>
class ProxyButtonPropertyComponent : public ButtonPropertyComponent, public SettableTooltipClient
{
private:

	T *_component;
	String _buttonText;

public:

	ProxyButtonPropertyComponent(const String name, const String buttonText, bool trigger, T *component = nullptr) : ButtonPropertyComponent(name, trigger),
		_component(component), _buttonText(buttonText)
	{
	}

	void buttonClicked()
	{
		if(_component)
			_component->propertyChangeNotify(this);
	}

	const String getButtonText() const
	{
		return _buttonText;
	}
};

template<class T>
class DoubleValueSliderPropertyComponent : public SliderPropertyComponent, public SettableTooltipClient
{
private:

	T *_component;
	double &_value;

public:

	DoubleValueSliderPropertyComponent(const String name, double min, double max, double interval, double skew, double &value, T *component = nullptr) : SliderPropertyComponent(name, min, max, interval, skew),
		_component(component), _value(value)
	{
	}

	void setValue(const double newValue)
	{
		_value = newValue;

		if(_component)
			_component->propertyChangeNotify(this);
	}

	const double getValue() const
	{
		return _value;
	}
};

class KeyMappingsPropertyComponent : public PropertyComponent, public SettableTooltipClient
{
private:
	KeyPressMappingSet *_pKeySet;
	KeyMappingEditorComponent *_pKeyEditor;

public:

	KeyMappingsPropertyComponent(const String name, KeyPressMappingSet *keySet) :
		PropertyComponent(name, 290),
		_pKeySet(nullptr),
		_pKeyEditor(nullptr)
	{
		setKeyMappingSet(keySet);
	}

	~KeyMappingsPropertyComponent()
	{
		deleteAllChildren();
	}

	void setKeyMappingSet(KeyPressMappingSet *keySet)
	{
		if(!keySet)
			return;

		deleteAllChildren();

		_pKeySet = keySet;
		addAndMakeVisible(_pKeyEditor = new KeyMappingEditorComponent(_pKeySet, true));
	}

	void refresh()
	{
	}

	void paint(Graphics &g)
	{
		getLookAndFeel().drawPropertyComponentBackground (g, getWidth(), getHeight(), *this);
	}

	void resized()
	{
		_pKeyEditor->setBounds(0, 0, getWidth(), getHeight());
	}
};

class SakuraMatrix : public Component, public MultiTimer, public ApplicationCommandTarget
{
private:

	typedef std::vector<CellComponent*> MatrixRowType;
	typedef std::vector<MatrixRowType> MatrixType;
	typedef MatrixRowType::iterator MatrixRowTypeIterator;
	typedef MatrixType::iterator MatrixTypeIterator;
	typedef std::set<std::pair<int, int> > CellsCoordsType;

	enum TimerCommands
	{
		generateCommandId = 1000,
		shuffleCommandId,
		peekCommandId,
		settingsCommandId,
		keyboardMoveLeftCommandId,
		keyboardMoveUpCommandId,
		keyboardMoveRightCommandId,
		keyboardMoveDownCommandId,
		rotateLeftCommandId,
		rotateRightCommandId
	};

	MatrixType _matrix;

	int _x_root_cell;
	int _y_root_cell;
	bool _infiniteMode;
	bool _infiniteMode_current;
	bool _solved;
	bool _peekMode;
	bool _autoShuffle;
	int _autoShuffle_Milliseconds;
	int _settingsPanelWidth;
	bool _settingsVisible;
	bool _swapMouseButtonsDirections;
	bool _swapMouseWheelDirections;
	bool _keyboardSupport;
	int _x_focus;
	int _y_focus;

	static std::vector<std::pair<int, int> > __delta;
	CellsCoordsType _forbiddenCells;

	Image *_pBackImage;

	MainWindow *_pParentComponent;
	StatusBarComponent *_pStatusBar;
	int _iStatusBarHeight;

	int _numberOfCellsX;
	int _numberOfCellsY;
	int _cellSize;
	double _relaxMatrix;

	Drawable *_decor_side_right;
	Drawable *_decor_side_left;
	Drawable *_decor_side_bottom;
	Drawable *_decor_conner_left;
	Drawable *_decor_conner_right;
	Drawable *_decor_conner_left_top;
	Drawable *_decor_conner_right_top;
	Drawable *_decor_butterfly_left;
	Drawable *_decor_title;

	int _tempWidthInCells;
	int _tempHeightInCells;
	int _tempCellSize;

	PropertyPanel *_pSettingsPanel;
	IntValueTextPropertyComponent<SakuraMatrix> *_pWidthInCellsProperty;
	IntValueTextPropertyComponent<SakuraMatrix> *_pHeightInCellsProperty;
	IntValueTextPropertyComponent<SakuraMatrix> *_pCellSizeProperty;
	ProxyButtonPropertyComponent<SakuraMatrix> *_pRebuildMatrixButtonProperty;

	BooleanValuePropertyComponent<SakuraMatrix> *_pInfiniteModeProperty;
	DoubleValueSliderPropertyComponent<SakuraMatrix> *_pRelaxMarixProperty;
	BooleanValuePropertyComponent<SakuraMatrix> *_pAutoShuffleModeProperty;
	IntValueTextPropertyComponent<SakuraMatrix> *_pAutoShuffleMillisecondsProperty;
	BooleanValuePropertyComponent<SakuraMatrix> *_pSwapMouseButtonsDirectionsProperty;
	BooleanValuePropertyComponent<SakuraMatrix> *_pSwapMouseWheelDirectionsProperty;
	BooleanValuePropertyComponent<SakuraMatrix> *_pKeyboardSupportProperty;

	KeyMappingsPropertyComponent *_pKeyMappingsProperty;

	void clear()
	{
		for(MatrixTypeIterator it = _matrix.begin(), end = _matrix.end(); it != end; ++it)
			for(MatrixRowTypeIterator it2 = (*it).begin(), end2 = (*it).end(); it2 != end2; ++it2)
			{
				removeChildComponent(*it2);

				delete (*it2);
			}

		_matrix.clear();
	}

	void reset()
	{
		for(MatrixTypeIterator it = _matrix.begin(), end = _matrix.end(); it != end; ++it)
			for(MatrixRowTypeIterator it2 = (*it).begin(), end2 = (*it).end(); it2 != end2; ++it2)
				(*it2)->reset();
	}

	void timerCallback (const int timerId)
	{
		switch(timerId)
		{
			case shuffleCommandId:
			{
				if(_autoShuffle && _solved)
					shuffleMatrix();

				stopTimer(timerId);

				break;
			}
		}
	}

	template<class value>
	void wrapAround(value &a, value &b, value a_limit, value b_limit)
	{
		if(a < 0)
			a = a_limit;
		else
			if(a > a_limit)
				a = 0;
			else
				if(b < 0)
					b = b_limit;
				else
					b = 0;
	}

	void setupLive(int x, int y, int fromDirection)
	{
		CellComponent *pCell = _matrix[y][x];

		if(pCell->getRequestedLive() || pCell->empty())
			return;

		DirectionSetType this_directions = pCell->getDirections(false);

		int r_x = 0;
		int r_y = 0;
		int next_from_direction = -1;

		for(int direction = 0; direction < 4; direction++)
		{
			if(!this_directions[direction] || direction == fromDirection)
				continue;

			r_x = x + __delta[direction].first;
			r_y = y + __delta[direction].second;

			if(r_x < 0 || r_y < 0 || r_x >= _numberOfCellsX || r_y >= _numberOfCellsY)
			{
				if(_infiniteMode_current)
					wrapAround(r_x, r_y, _numberOfCellsX - 1, _numberOfCellsY - 1);
				else
					continue;
			}

			CellComponent *pNextCell = _matrix[r_y][r_x];

			next_from_direction = (direction + 2) % 4;

			if(!pNextCell->getDirection(next_from_direction))
				continue;

			pCell->setLive(true);

			setupLive(r_x, r_y, next_from_direction);

			pNextCell->setLive(true);
		}
	}

public:

	typedef sakura_map<std::string, sakura_map<int, Drawable*> > FiguresType;
	typedef sakura_map<std::string, sakura_map<int, Image*> > FigureImagesType;

	static FigureImagesType __figure_images;
	static FiguresType __figures;

	void paint(Graphics &g);
	void resized();
	void generateBackground();

	SakuraMatrix(MainWindow *pParentComponent);

	~SakuraMatrix();

	CellComponent *getCellAt(int x, int y)
	{
		return _matrix[y][x];
	}

	void buildMatrix(int x_cells, int y_cells, int cellSize);

	void setAllCellsLive(int live)
	{
		for(MatrixTypeIterator it = _matrix.begin(), end = _matrix.end(); it != end; ++it)
			for(MatrixRowTypeIterator it2 = (*it).begin(), end2 = (*it).end(); it2 != end2; ++it2)
				(*it2)->setLive(live);
	}

	void killAllCells()
	{
		setAllCellsLive(false);
	}

	void enlivenAllCells()
	{
		setAllCellsLive(true);
	}

	void setAllCellsSolved()
	{
		setAllCellsLive(2);
	}

	void setPeekMode(bool mode)
	{
		if(_peekMode != mode)
		{
			_peekMode = _solved = mode;

			for(MatrixTypeIterator it = _matrix.begin(), end = _matrix.end(); it != end; ++it)
				for(MatrixRowTypeIterator it2 = (*it).begin(), end2 = (*it).end(); it2 != end2; ++it2)
					(*it2)->setDrawOriginal(_peekMode);
		}
	}

	void togglePeekMode()
	{
		setPeekMode(!_peekMode);
	}

	bool isPeekMode() const
	{
		return _peekMode;
	}

	void generateBranch()
	{
		_infiniteMode_current = _infiniteMode;

		reset();

		_forbiddenCells.clear();
		for(int idx = 0, count = int(((_numberOfCellsX * _numberOfCellsY) * _relaxMatrix) + .5) ; idx < count; idx++)
			_forbiddenCells.insert(std::pair<int, int>(std::rand() % _numberOfCellsX, std::rand() % _numberOfCellsY));

		_forbiddenCells.erase(std::pair<int, int>(_x_root_cell, _y_root_cell));

		int f_x = 0;
		int f_y = 0;
		for(std::set<std::pair<int, int> >::iterator it = _forbiddenCells.begin(), end = _forbiddenCells.end(); it != end; ++it)
		{
			f_x = std::abs(_x_root_cell - (*it).first);
			f_y = std::abs(_y_root_cell - (*it).second);

			if(f_x <= _numberOfCellsX / 2 && f_y <= _numberOfCellsY / 2)
			{
				std::set<std::pair<int, int> >::iterator it2 = it;
				it2++;

				_forbiddenCells.erase(it);

				it = it2;

				if(it == end)
					break;
			}
		}

		generateCell(std::rand() % _numberOfCellsX, std::rand() % _numberOfCellsY);

		positionRoot();

		enlivenAllCells();

		repaintLiveCellsIfNeeded();

		_solved = true;

		if(isTimerRunning(shuffleCommandId))
			stopTimer(shuffleCommandId);

		if(_autoShuffle)
		{
			if(_autoShuffle_Milliseconds > 100)
				startTimer(shuffleCommandId, _autoShuffle_Milliseconds);
			else
				shuffleMatrix();
		}
	}

	bool positionRoot();

	void generateCell(int x, int y)
	{
		CellComponent *pCell = _matrix[y][x];

		int r_direction = 0;
		int shift_start = std::rand() % 4;
		int r_x = 0;
		int r_y = 0;
		int max_connections = 2;

		for(int direction = 0; direction < 4 && max_connections; direction++)
		{
			r_direction = (direction + shift_start) % 4;

			if(pCell->getDirection(r_direction))
				continue;

			r_x = x + __delta[r_direction].first;
			r_y = y + __delta[r_direction].second;

			if(r_x < 0 || r_y < 0 || r_x >= _numberOfCellsX || r_y >= _numberOfCellsY)
			{
				if(_infiniteMode_current)
					wrapAround(r_x, r_y, _numberOfCellsX - 1, _numberOfCellsY - 1);
				else
					continue;
			}

			CellComponent *pNextCell = _matrix[r_y][r_x];

			if(pNextCell->notEmpty() || _forbiddenCells.find(std::pair<int, int>(r_x, r_y)) != _forbiddenCells.end())
				continue;

			pCell->setDirection(r_direction, true);
			pNextCell->setDirection((r_direction + 2) % 4, true);

			max_connections--;

			generateCell(r_x, r_y);
		}
	}

	void drawAlivePath()
	{
		killAllCells();

		setupLive(_x_root_cell, _y_root_cell, -1);

		repaintLiveCellsIfNeeded();
	}

	void repaintLiveCellsIfNeeded()
	{
		for(MatrixTypeIterator it = _matrix.begin(), end = _matrix.end(); it != end; ++it)
			for(MatrixRowTypeIterator it2 = (*it).begin(), end2 = (*it).end(); it2 != end2; ++it2)
				(*it2)->repaintLiveIfNeeded();
	}

	void shuffleMatrix()
	{
		if(isTimerRunning(shuffleCommandId))
			stopTimer(shuffleCommandId);

		if(_peekMode)
			setPeekMode(false);

		for(MatrixTypeIterator it = _matrix.begin(), end = _matrix.end(); it != end; ++it)
			for(MatrixRowTypeIterator it2 = (*it).begin(), end2 = (*it).end(); it2 != end2; ++it2)
				(*it2)->rotateRandomly();

		while(isAllCellsInPlace())
			for(MatrixTypeIterator it = _matrix.begin(), end = _matrix.end(); it != end; ++it)
				for(MatrixRowTypeIterator it2 = (*it).begin(), end2 = (*it).end(); it2 != end2; ++it2)
					(*it2)->rotateRandomly();

		drawAlivePath();


		if(_keyboardSupport)
		{
			_matrix[_y_focus][_x_focus]->setDrawFocus(false);
			_matrix[0][0]->setDrawFocus(true);
		}

		_x_focus = _y_focus = 0;

		_solved = false;
	}

	void toggleSettings();

	void setInfiniteMode(const bool infinite = true)
	{
		_infiniteMode = infinite;
	}

	const bool getMode() const
	{
		return _infiniteMode;
	}

	bool isAllCellsInPlace()
	{
		for(MatrixTypeIterator it = _matrix.begin(), end = _matrix.end(); it != end; ++it)
			for(MatrixRowTypeIterator it2 = (*it).begin(), end2 = (*it).end(); it2 != end2; ++it2)
				if(!(*it2)->isInPlace())
					return false;

		return (_solved = true);
	}

	bool isAlreadySolved() const
	{
		return _solved;
	}

	void propertyChangeNotify(PropertyComponent *property)
	{
		if(property == _pRebuildMatrixButtonProperty)
		{
			if(_tempWidthInCells < 3)
			{
				_tempWidthInCells = 3;
				_pWidthInCellsProperty->refresh();
			}

			if(_tempHeightInCells < 3)
			{
				_tempHeightInCells = 3;
				_pHeightInCellsProperty->refresh();
			}

			if(_tempCellSize < 16)
			{
				_tempCellSize = 16;
				_pCellSizeProperty->refresh();
			}

			buildMatrix(_tempWidthInCells, _tempHeightInCells, _tempCellSize);
		}

		if(property == _pKeyboardSupportProperty)
			if(!_solved)
				_matrix[_y_focus][_x_focus]->setDrawFocus(_keyboardSupport);

		if(property == _pAutoShuffleModeProperty)
		{
			_pAutoShuffleMillisecondsProperty->setEnabled(_autoShuffle);
		}
	}

	void rotateCell(CellComponent *component, bool left, bool putInPlace = false)
	{
		if(putInPlace)
			component->putInPlace();
		else
			component->rotate(left);

		component->repaint();

		drawAlivePath();

		if(isAllCellsInPlace())
		{
			_matrix[_y_focus][_x_focus]->setDrawFocus(false);

			setAllCellsSolved();
			repaintLiveCellsIfNeeded();
		}
	}

	bool isMouseButtonsDirectionsSwapped()
	{
		return _swapMouseButtonsDirections;
	}

	bool isMouseWheelDirectionsSwapped()
	{
		return _swapMouseWheelDirections;
	}

	void generateDecor();
	void generateFigures();

	ApplicationCommandTarget *getNextCommandTarget()
	{
		return findFirstTargetParentComponent();
	}

	void getAllCommands (Array< CommandID > &commands)
	{
		commands.add(CommandID(int(generateCommandId)));
		commands.add(CommandID(int(shuffleCommandId)));
		commands.add(CommandID(int(peekCommandId)));
		commands.add(CommandID(int(settingsCommandId)));
		commands.add(CommandID(int(keyboardMoveLeftCommandId)));
		commands.add(CommandID(int(keyboardMoveUpCommandId)));
		commands.add(CommandID(int(keyboardMoveRightCommandId)));
		commands.add(CommandID(int(keyboardMoveDownCommandId)));
		commands.add(CommandID(int(rotateLeftCommandId)));
		commands.add(CommandID(int(rotateRightCommandId)));
	}

	void getCommandInfo(const CommandID commandID, ApplicationCommandInfo &result)
	{
		String generalGroup(T("General"));
		String keyboardGroup(T("Keyboard support"));

		switch(commandID)
		{
			case generateCommandId:
				result.setInfo(T("generate"), T("generates a new branch to solve"), generalGroup, 0);
				result.addDefaultKeypress(KeyPress::insertKey, 0);
				break;

			case shuffleCommandId:
				result.setInfo(T("shuffle"), T("shuffles a branch"), generalGroup, 0);
				result.addDefaultKeypress(KeyPress::backspaceKey, 0);
				break;

			case peekCommandId:
				result.setInfo(T("peek"), T("shows the currently shuffled branch in a solved state"), generalGroup, ApplicationCommandInfo::wantsKeyUpDownCallbacks);
				result.addDefaultKeypress(KeyPress::homeKey, 0);
				break;

			case settingsCommandId:
				result.setInfo(T("settings"), T("shows or hides the settings pane"), generalGroup, 0);
				result.addDefaultKeypress(KeyPress::F12Key, 0);
				break;

			case keyboardMoveLeftCommandId:
				result.setInfo(T("move left"), T("steps the current focus left"), keyboardGroup, 0);
				result.addDefaultKeypress(KeyPress::leftKey, 0);
				break;

			case keyboardMoveUpCommandId:
				result.setInfo(T("move up"), T("steps the current focus up"), keyboardGroup, 0);
				result.addDefaultKeypress(KeyPress::upKey, 0);
				break;

			case keyboardMoveRightCommandId:
				result.setInfo(T("move right"), T("steps the current focus right"), keyboardGroup, 0);
				result.addDefaultKeypress(KeyPress::rightKey, 0);
				break;

			case keyboardMoveDownCommandId:
				result.setInfo(T("move down"), T("steps the current focus down"), keyboardGroup, 0);
				result.addDefaultKeypress(KeyPress::downKey, 0);
				break;

			case rotateLeftCommandId:
				result.setInfo(T("rotate left"), T("rotates a cell to the left"), keyboardGroup, 0);
				result.addDefaultKeypress(KeyPress::pageUpKey, 0);
				break;

			case rotateRightCommandId:
				result.setInfo(T("rotate right"), T("rotates a cell to the right"), keyboardGroup, 0);
				result.addDefaultKeypress(KeyPress::pageDownKey, 0);
				break;
		}
	}

	bool perform(const InvocationInfo &info);

	void dumpMatrix(bool original = false)
	{
		for(int i = 0; i < _numberOfCellsY; i++)
		{
			for(int idx = 0; idx < 3; idx++)
			{
				for(int ii = 0; ii < _numberOfCellsX; ii++)
					std::cout << _matrix[i][ii]->dumpLevel(idx, original);

				std::cout << std::endl;
			}

			std::cout << std::endl;
		}
	}
};

class MainWindow : public DocumentWindow
{
private:

	SakuraMatrix *_pContentWindow;
	TooltipWindow _tooltipWindow;

public:

	static ApplicationCommandManager *__pCommandManager;

	MainWindow(const String &name, const Colour &backgroundColour, const int requiredButtons, const bool addToDesktop = true);

	~MainWindow()
	{
		setContentComponent(0, true);
	}

	void closeButtonPressed()
	{
		JUCEApplication::getInstance()->systemRequestedQuit();
	}
};

class Sakura : public JUCEApplication
{
private:

	DocumentWindow *_pMainWindow;

public:

    Sakura() : _pMainWindow(0)
    {
    	std::srand(std::time(0));
    }

    void initialise (const String&)
    {
    	String title(T("Sakura"));
    	title += String(T(" (")) + AutoVersion::FULLVERSION_STRING + String(T(" ")) + AutoVersion::STATUS + String(T(") "));

    	_pMainWindow = new MainWindow(title, Colours::cornsilk.withMultipliedAlpha(.99f), DocumentWindow::minimiseButton | DocumentWindow::closeButton, true);
    	_pMainWindow->setVisible(true);
    }

    void shutdown()
    {
    	delete _pMainWindow;
    }

    const String getApplicationName()
    {
    	return T("Sakura");
    }

    bool moreThanOneInstanceAllowed()
    {
    	return false;
    }

    void anotherInstanceStarted (const String&)
    {
    }

	void systemRequestedQuit()
	{
		JUCEApplication::systemRequestedQuit();
	}
};

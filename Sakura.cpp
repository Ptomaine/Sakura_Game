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

#include "Sakura.h"
#include "figures.h"
#include "graphics.h"

std::vector<std::pair<int, int> > SakuraMatrix::__delta;
SakuraMatrix::FiguresType SakuraMatrix::__figures;
SakuraMatrix::FigureImagesType SakuraMatrix::__figure_images;
ApplicationCommandManager *MainWindow::__pCommandManager = 0;

MainWindow::MainWindow(const String &name, const Colour &backgroundColour, const int requiredButtons, const bool addToDesktop) :
	DocumentWindow(name, backgroundColour, requiredButtons, addToDesktop),
	_pContentWindow(nullptr),
	_tooltipWindow(this)
{
	setDropShadowEnabled(false);
	//setTitleBarHeight(24);
	setTitleBarTextCentred(false);

	Image *l_pAppIcon = ImageCache::getFromMemory(SAKURA_ICON_PNG, SAKURA_ICON_PNG_SIZE);
	setIcon(l_pAppIcon);

	setContentComponent(_pContentWindow = new SakuraMatrix(this), true, true);
}

SakuraMatrix::SakuraMatrix(MainWindow *pParentComponent) :
	_x_root_cell(0),
	_y_root_cell(0),
	_infiniteMode(false),
	_infiniteMode_current(false),
	_solved(false),
	_peekMode(false),
	_autoShuffle(true),
	_autoShuffle_Milliseconds(5000),
	_settingsPanelWidth(250),
	_settingsVisible(false),
	_swapMouseButtonsDirections(false),
	_swapMouseWheelDirections(false),
	_keyboardSupport(false),
	_x_focus(0),
	_y_focus(0),
	_pBackImage(nullptr),
	_pParentComponent(pParentComponent),
	_pStatusBar(nullptr),
	_iStatusBarHeight(48),
	_numberOfCellsX(7),
	_numberOfCellsY(6),
	_cellSize(48),
	_relaxMatrix(.1f),
	_decor_side_right(nullptr),
	_decor_side_left(nullptr),
	_decor_side_bottom(nullptr),
	_decor_conner_left(nullptr),
	_decor_conner_right(nullptr),
	_decor_conner_left_top(nullptr),
	_decor_conner_right_top(nullptr),
	_decor_butterfly_left(nullptr),
	_decor_title(nullptr),
	_tempWidthInCells(_numberOfCellsX),
	_tempHeightInCells(_numberOfCellsY),
	_tempCellSize(_cellSize),
	_pSettingsPanel(nullptr),
	_pWidthInCellsProperty(nullptr),
	_pHeightInCellsProperty(nullptr),
	_pCellSizeProperty(nullptr),
	_pInfiniteModeProperty(nullptr),
	_pAutoShuffleModeProperty(nullptr),
	_pAutoShuffleMillisecondsProperty(nullptr),
	_pKeyboardSupportProperty(nullptr),
	_pKeyMappingsProperty(nullptr)
{
	std::srand(std::time(nullptr));

	if(!__delta.size())
	{
		__delta.resize(4);
		__delta[int(Direction::Left)] = std::pair<int, int>(-1, 0);
		__delta[int(Direction::Top)] = std::pair<int, int>(0, -1);
		__delta[int(Direction::Right)] = std::pair<int, int>(1, 0);
		__delta[int(Direction::Bottom)] = std::pair<int, int>(0, 1);
	}

	if(!MainWindow::__pCommandManager)
	{
		MainWindow::__pCommandManager = new ApplicationCommandManager();
		MainWindow::__pCommandManager->registerAllCommandsForTarget(this);
		MainWindow::__pCommandManager->getKeyMappings()->resetToDefaultMappings();

		_pParentComponent->addKeyListener(MainWindow::__pCommandManager->getKeyMappings());
	}

	generateDecor();
	generateFigures();

	addAndMakeVisible(_pStatusBar = new StatusBarComponent(this));

	Array<PropertyComponent*> sizeProperties;
	sizeProperties.add(_pWidthInCellsProperty = new IntValueTextPropertyComponent<SakuraMatrix>(T("width in cells"), _tempWidthInCells));
	sizeProperties.add(_pHeightInCellsProperty = new IntValueTextPropertyComponent<SakuraMatrix>(T("height in cells"), _tempHeightInCells));
	sizeProperties.add(_pCellSizeProperty = new IntValueTextPropertyComponent<SakuraMatrix>(T("cell size in pixels"), _tempCellSize));
	sizeProperties.add(_pRebuildMatrixButtonProperty = new ProxyButtonPropertyComponent<SakuraMatrix>(T("apply changes"), T("Resize"), false, this));

	Array<PropertyComponent*> additinalProperties;
	additinalProperties.add(_pInfiniteModeProperty = new BooleanValuePropertyComponent<SakuraMatrix>(T("complexity"), T("infinite mode (torus)"), T("regular mode"), _infiniteMode));
	_pInfiniteModeProperty->setTooltip(T("<regular mode>: confines a branch within the matrix's edges\n\n<ninfinite mode>: the matrix's edges are no more restrictive\n\n(this setting takes effect on the next time you generate a branch)"));
	additinalProperties.add(_pRelaxMarixProperty = new DoubleValueSliderPropertyComponent<SakuraMatrix>(T("relax matrix"), .0f, .7f, .1f, 1.f, _relaxMatrix));
	_pRelaxMarixProperty->setTooltip(T("adds empty cells to the matrix so making a branch more relaxed\n\nthe greater the value the more a branch is relaxed\n\n(be careful with the high values because a branch can appear too relaxed at times :)"));
	additinalProperties.add(_pAutoShuffleModeProperty = new BooleanValuePropertyComponent<SakuraMatrix>(T("shuffle"), T("automatically"), T("manually"), _autoShuffle, this));
	_pAutoShuffleModeProperty->setTooltip(T("<automatically>: shuffles a new generated branch automatically in some time\n\n<manually>: you have to shuffle the matrix manually by pressing an appropriate button"));
	additinalProperties.add(_pAutoShuffleMillisecondsProperty = new IntValueTextPropertyComponent<SakuraMatrix>(T("milliseconds before auto-shuffle"), _autoShuffle_Milliseconds));
	_pAutoShuffleMillisecondsProperty->setEnabled(_autoShuffle);
	_pAutoShuffleMillisecondsProperty->setTooltip(T("it shuffles a branch automatically in a specified time right after a branch is generated"));
	additinalProperties.add(_pSwapMouseButtonsDirectionsProperty = new BooleanValuePropertyComponent<SakuraMatrix>(T("swap mouse buttons"), T("yes"), T("no"), _swapMouseButtonsDirections));
	additinalProperties.add(_pSwapMouseWheelDirectionsProperty = new BooleanValuePropertyComponent<SakuraMatrix>(T("swap mouse wheel directions"), T("yes"), T("no"), _swapMouseWheelDirections));
	additinalProperties.add(_pKeyboardSupportProperty = new BooleanValuePropertyComponent<SakuraMatrix>(T("keyboard"), T("enabled"), T("disabled"), _keyboardSupport, this));
	_pKeyboardSupportProperty->setTooltip(T("enables keyboard support thus letting you move and rotate cells by using keyboard buttons"));

	Array<PropertyComponent*> keymappingsProperties;
	keymappingsProperties.add(_pKeyMappingsProperty = new KeyMappingsPropertyComponent(T("keyboard"), MainWindow::__pCommandManager->getKeyMappings()));

	addAndMakeVisible(_pSettingsPanel = new PropertyPanel());
	_pSettingsPanel->addSection(T("Size properties"), sizeProperties, true);
	_pSettingsPanel->addSection(T("Additinal properties"), additinalProperties, true);
	_pSettingsPanel->addSection(T("Key-mapping properties"), keymappingsProperties, true);

	buildMatrix(_numberOfCellsX, _numberOfCellsY, _cellSize);

	_pParentComponent->centreAroundComponent(0, getWidth(), getHeight());
}

SakuraMatrix::~SakuraMatrix()
{
	deleteAllChildren();

	for(FiguresType::iterator it = __figures.begin(), end = __figures.end(); it != end; it++)
		for(sakura_map<int, Drawable*>::iterator it2 = (*it).second.begin(), end2 = (*it).second.end(); it2 != end2; it2++)
			delete (*it2).second;

	__figures.clear();

	for(FigureImagesType::iterator it = __figure_images.begin(), end = __figure_images.end(); it != end; it++)
		for(sakura_map<int, Image*>::iterator it2 = (*it).second.begin(), end2 = (*it).second.end(); it2 != end2; it2++)
			delete (*it2).second;

	__figure_images.clear();

	delete _decor_side_right;
	delete _decor_side_left;
	delete _decor_side_bottom;
	delete _decor_conner_left;
	delete _decor_conner_right;
	delete _decor_conner_left_top;
	delete _decor_conner_right_top;
	delete _decor_butterfly_left;
	delete _decor_title;

	delete MainWindow::__pCommandManager;
	MainWindow::__pCommandManager = nullptr;
}

void SakuraMatrix::generateDecor()
{
	XmlDocument doc_side_right(String((const char*)SIDE_RIGHT_SVG));
	XmlDocument doc_side_left(String((const char*)SIDE_LEFT_SVG));
	XmlDocument doc_side_bottom(String((const char*)SIDE_BOTTOM_SVG));
	XmlDocument doc_conner_left(String((const char*)BORDER_CONNER_LEFT_SVG));
	XmlDocument doc_conner_right(String((const char*)BORDER_CONNER_RIGHT_SVG));
	XmlDocument doc_conner_left_top(String((const char*)BORDER_CONNER_LEFT_TOP_SVG));
	XmlDocument doc_conner_right_top(String((const char*)BORDER_CONNER_RIGHT_TOP_SVG));
	XmlDocument doc_butterfly_left(String((const char*)BUTTERFLY_LEFT_SVG));
	XmlDocument doc_title(String((const char*)SAKURA_TITLE_SVG));

	_decor_side_right = Drawable::createFromSVG(*doc_side_right.getDocumentElement());
	_decor_side_left = Drawable::createFromSVG(*doc_side_left.getDocumentElement());
	_decor_side_bottom = Drawable::createFromSVG(*doc_side_bottom.getDocumentElement());
	_decor_conner_left = Drawable::createFromSVG(*doc_conner_left.getDocumentElement());
	_decor_conner_right = Drawable::createFromSVG(*doc_conner_right.getDocumentElement());
	_decor_conner_left_top = Drawable::createFromSVG(*doc_conner_left_top.getDocumentElement());
	_decor_conner_right_top = Drawable::createFromSVG(*doc_conner_right_top.getDocumentElement());
	_decor_butterfly_left = Drawable::createFromSVG(*doc_butterfly_left.getDocumentElement());
	_decor_title = Drawable::createFromSVG(*doc_title.getDocumentElement());
}

void SakuraMatrix::generateFigures()
{
	for(FiguresType::iterator it = __figures.begin(), end = __figures.end(); it != end; ++it)
		for(sakura_map<int, Drawable*>::iterator it2 = (*it).second.begin(), end2 = (*it).second.end(); it2 != end2; ++it2)
			delete (*it2).second;

	__figures.clear();

	XmlDocument doc_0011(String((const char*)_0011_SVG));
	XmlDocument doc_0011_LIVE(String((const char*)_0011_LIVE_SVG));
	XmlDocument doc_0011_SOLVED(String((const char*)_0011_SOLVED_SVG));
	XmlDocument doc_0101(String((const char*)_0101_SVG));
	XmlDocument doc_0101_LIVE(String((const char*)_0101_LIVE_SVG));
	XmlDocument doc_0101_SOLVED(String((const char*)_0101_SOLVED_SVG));
	XmlDocument doc_0110(String((const char*)_0110_SVG));
	XmlDocument doc_0110_LIVE(String((const char*)_0110_LIVE_SVG));
	XmlDocument doc_0110_SOLVED(String((const char*)_0110_SOLVED_SVG));
	XmlDocument doc_0111(String((const char*)_0111_SVG));
	XmlDocument doc_0111_LIVE(String((const char*)_0111_LIVE_SVG));
	XmlDocument doc_0111_SOLVED(String((const char*)_0111_SOLVED_SVG));
	XmlDocument doc_1001(String((const char*)_1001_SVG));
	XmlDocument doc_1001_LIVE(String((const char*)_1001_LIVE_SVG));
	XmlDocument doc_1001_SOLVED(String((const char*)_1001_SOLVED_SVG));
	XmlDocument doc_1010(String((const char*)_1010_SVG));
	XmlDocument doc_1010_LIVE(String((const char*)_1010_LIVE_SVG));
	XmlDocument doc_1010_SOLVED(String((const char*)_1010_SOLVED_SVG));
	XmlDocument doc_1011(String((const char*)_1011_SVG));
	XmlDocument doc_1011_LIVE(String((const char*)_1011_LIVE_SVG));
	XmlDocument doc_1011_SOLVED(String((const char*)_1011_SOLVED_SVG));
	XmlDocument doc_1100(String((const char*)_1100_SVG));
	XmlDocument doc_1100_LIVE(String((const char*)_1100_LIVE_SVG));
	XmlDocument doc_1100_SOLVED(String((const char*)_1100_SOLVED_SVG));
	XmlDocument doc_1101(String((const char*)_1101_SVG));
	XmlDocument doc_1101_LIVE(String((const char*)_1101_LIVE_SVG));
	XmlDocument doc_1101_SOLVED(String((const char*)_1101_SOLVED_SVG));
	XmlDocument doc_1110(String((const char*)_1110_SVG));
	XmlDocument doc_1110_LIVE(String((const char*)_1110_LIVE_SVG));
	XmlDocument doc_1110_SOLVED(String((const char*)_1110_SOLVED_SVG));
	XmlDocument doc_1000(String((const char*)_1000_SVG));
	XmlDocument doc_1000_LIVE(String((const char*)_1000_LIVE_SVG));
	XmlDocument doc_1000_SOLVED(String((const char*)_1000_SOLVED_SVG));
	XmlDocument doc_0100(String((const char*)_0100_SVG));
	XmlDocument doc_0100_LIVE(String((const char*)_0100_LIVE_SVG));
	XmlDocument doc_0100_SOLVED(String((const char*)_0100_SOLVED_SVG));
	XmlDocument doc_0010(String((const char*)_0010_SVG));
	XmlDocument doc_0010_LIVE(String((const char*)_0010_LIVE_SVG));
	XmlDocument doc_0010_SOLVED(String((const char*)_0010_SOLVED_SVG));
	XmlDocument doc_0001(String((const char*)_0001_SVG));
	XmlDocument doc_0001_LIVE(String((const char*)_0001_LIVE_SVG));
	XmlDocument doc_0001_SOLVED(String((const char*)_0001_SOLVED_SVG));


	XmlElement *pRootDead = nullptr, *pRootLive = nullptr, *pRootSolved = nullptr;

	pRootDead = doc_1000.getDocumentElement();
	pRootLive = doc_1000_LIVE.getDocumentElement();
	pRootSolved = doc_1000_SOLVED.getDocumentElement();
	__figures["1000"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["1000"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["1000"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_0100.getDocumentElement();
	pRootLive = doc_0100_LIVE.getDocumentElement();
	pRootSolved = doc_0100_SOLVED.getDocumentElement();
	__figures["0100"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["0100"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["0100"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_0010.getDocumentElement();
	pRootLive = doc_0010_LIVE.getDocumentElement();
	pRootSolved = doc_0010_SOLVED.getDocumentElement();
	__figures["0010"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["0010"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["0010"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_0001.getDocumentElement();
	pRootLive = doc_0001_LIVE.getDocumentElement();
	pRootSolved = doc_0001_SOLVED.getDocumentElement();
	__figures["0001"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["0001"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["0001"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_1100.getDocumentElement();
	pRootLive = doc_1100_LIVE.getDocumentElement();
	pRootSolved = doc_1100_SOLVED.getDocumentElement();
	__figures["1100"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["1100"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["1100"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_0110.getDocumentElement();
	pRootLive = doc_0110_LIVE.getDocumentElement();
	pRootSolved = doc_0110_SOLVED.getDocumentElement();
	__figures["0110"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["0110"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["0110"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_0011.getDocumentElement();
	pRootLive = doc_0011_LIVE.getDocumentElement();
	pRootSolved = doc_0011_SOLVED.getDocumentElement();
	__figures["0011"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["0011"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["0011"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_1001.getDocumentElement();
	pRootLive = doc_1001_LIVE.getDocumentElement();
	pRootSolved = doc_1001_SOLVED.getDocumentElement();
	__figures["1001"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["1001"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["1001"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_1010.getDocumentElement();
	pRootLive = doc_1010_LIVE.getDocumentElement();
	pRootSolved = doc_1010_SOLVED.getDocumentElement();
	__figures["1010"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["1010"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["1010"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_0101.getDocumentElement();
	pRootLive = doc_0101_LIVE.getDocumentElement();
	pRootSolved = doc_0101_SOLVED.getDocumentElement();
	__figures["0101"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["0101"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["0101"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_1110.getDocumentElement();
	pRootLive = doc_1110_LIVE.getDocumentElement();
	pRootSolved = doc_1110_SOLVED.getDocumentElement();
	__figures["1110"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["1110"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["1110"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_0111.getDocumentElement();
	pRootLive = doc_0111_LIVE.getDocumentElement();
	pRootSolved = doc_0111_SOLVED.getDocumentElement();
	__figures["0111"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["0111"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["0111"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_1011.getDocumentElement();
	pRootLive = doc_1011_LIVE.getDocumentElement();
	pRootSolved = doc_1011_SOLVED.getDocumentElement();
	__figures["1011"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["1011"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["1011"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);

	pRootDead = doc_1101.getDocumentElement();
	pRootLive = doc_1101_LIVE.getDocumentElement();
	pRootSolved = doc_1101_SOLVED.getDocumentElement();
	__figures["1101"][CellComponent::Dead] = Drawable::createFromSVG(*pRootDead);
	__figures["1101"][CellComponent::Alive] = Drawable::createFromSVG(*pRootLive);
	__figures["1101"][CellComponent::Solved] = Drawable::createFromSVG(*pRootSolved);
}

void SakuraMatrix::paint(Graphics &g)
{
	if(!_pBackImage)
		generateBackground();

	g.drawImageAt(_pBackImage, 0, 0, false);
}

void SakuraMatrix::resized()
{
	delete _pBackImage;
	_pBackImage = 0;
}

void SakuraMatrix::generateBackground()
{
	int width = _cellSize * _numberOfCellsX + (_cellSize * 2);
	int bkWidth = getWidth();
	int height = getHeight();

	delete _pBackImage;
	_pBackImage = new Image(Image::RGB, bkWidth, height, false);

	Graphics g(*_pBackImage);
	GradientBrush mbrush(Colours::skyblue, 0, 0, Colour(0xffffea95), 0, height, false);
	g.setBrush(&mbrush);
	g.fillAll();

	GradientBrush dividerBrush(Colours::skyblue, 0, height, Colour(0xffffea95), .0f, 0.f, false);
	g.setBrush(&dividerBrush);
	g.fillRect(width, 0, 1, height);

	Image im(Image::ARGB, bkWidth, height, true);
	Graphics gg(im);

	_decor_side_right->drawWithin(gg, width - _cellSize, _cellSize, _cellSize, height - _cellSize - _iStatusBarHeight, RectanglePlacement::onlyReduceInSize);
	_decor_side_left->drawWithin(gg, 0, 0, _cellSize, height - _cellSize - _iStatusBarHeight, RectanglePlacement::onlyReduceInSize);
	_decor_side_bottom->drawWithin(gg, 0, height - _cellSize - _iStatusBarHeight, width - _cellSize, _cellSize, RectanglePlacement::onlyReduceInSize);

	_decor_conner_left->drawWithin(gg, 0, height - (_cellSize * 2) - _iStatusBarHeight, _cellSize * 2, _cellSize * 2, RectanglePlacement::onlyReduceInSize);
	_decor_conner_right->drawWithin(gg, width - (_cellSize * 2), height - (_cellSize * 2) - _iStatusBarHeight, _cellSize * 2, _cellSize * 2, RectanglePlacement::onlyReduceInSize);
	_decor_conner_left_top->drawWithin(gg, 0, 0, _cellSize * 2, _cellSize * 2, RectanglePlacement::onlyReduceInSize);
	_decor_conner_right_top->drawWithin(gg, width - (_cellSize * 2), 0, _cellSize * 2, _cellSize * 2, RectanglePlacement::onlyReduceInSize);

	GradientBrush brush(Colours::cornsilk, _cellSize, _cellSize, Colour(0xffffea95), _cellSize, _numberOfCellsY * _cellSize, false);
	gg.setBrush(&brush);
	gg.fillRoundedRectangle(_cellSize, _cellSize, _numberOfCellsX * _cellSize, _numberOfCellsY * _cellSize, 10);
	gg.setColour(Colours::yellowgreen.brighter());
	gg.drawRoundedRectangle(_cellSize, _cellSize, _numberOfCellsX * _cellSize, _numberOfCellsY * _cellSize, 10, 1);

	gg.setOpacity(.3f);
	for(int x_ = 1; x_ < _numberOfCellsX; x_++)
		gg.drawVerticalLine(_cellSize + (x_ * _cellSize), _cellSize, height - _cellSize - _iStatusBarHeight);

	for(int y_ = 1; y_ < _numberOfCellsY; y_++)
		gg.drawHorizontalLine(_cellSize + (y_ * _cellSize), _cellSize, width - _cellSize);

	gg.setOpacity(1.f);
	_decor_butterfly_left->drawWithin(gg, width - _cellSize - (_cellSize / 4), (_cellSize * 2) - (_cellSize / 4), _cellSize / 2, _cellSize / 2, RectanglePlacement::onlyReduceInSize);

	gg.setOpacity(.7f);
	_decor_title->drawWithin(gg, 10, 10, width - 20, _cellSize - 20, RectanglePlacement::onlyIncreaseInSize);

	DropShadowEffect eff;
	eff.applyEffect(im, g);
}

void SakuraMatrix::buildMatrix(int x_cells, int y_cells, int cellSize)
{
	if(x_cells < 3 || y_cells < 3 ||
		(_numberOfCellsX == x_cells && _numberOfCellsY == y_cells && _cellSize == cellSize && _matrix.size()))
		return;

	if(_cellSize != cellSize || !SakuraMatrix::__figure_images.size())
	{
		for(SakuraMatrix::FigureImagesType::iterator it = __figure_images.begin(), end = __figure_images.end(); it != end; it++)
			for(sakura_map<int, Image*>::iterator it2 = (*it).second.begin(), end2 = (*it).second.end(); it2 != end2; it2++)
				delete (*it2).second;

		__figure_images.clear();

		Image *pTempImage = 0;

		for(SakuraMatrix::FiguresType::iterator it = SakuraMatrix::__figures.begin(), end = SakuraMatrix::__figures.end(); it != end; ++it)
		{
			for(sakura_map<int, Drawable*>::iterator it2 = (*it).second.begin(), end2 = (*it).second.end(); it2 != end2; ++it2)
			{
				pTempImage = new Image(Image::ARGB, cellSize, cellSize, true);
				Graphics g(*pTempImage);

				(*it2).second->drawWithin(g, 0, 0, pTempImage->getWidth(), pTempImage->getHeight(), RectanglePlacement::stretchToFit);
				SakuraMatrix::__figure_images[(*it).first][(*it2).first] = pTempImage;
			}
		}
	}

	_numberOfCellsX = _tempWidthInCells = x_cells;
	_numberOfCellsY = _tempHeightInCells = y_cells;
	_cellSize = _tempCellSize = cellSize;

	int iWindowWidth = _cellSize * _numberOfCellsX + (_cellSize * 2);
	int iWindowHeight = _cellSize * _numberOfCellsY + _iStatusBarHeight + (_cellSize * 2);

	centreWithSize(iWindowWidth + (_settingsVisible ? _settingsPanelWidth : 0), iWindowHeight);

	clear();

	int index = 0;
	int x = 0;
	int y = 0;
	_matrix.resize(_numberOfCellsY);
	for(MatrixTypeIterator it = _matrix.begin(), end = _matrix.end(); it != end; ++it)
	{
		(*it).resize(_numberOfCellsX);
		for(MatrixRowTypeIterator it2 = (*it).begin(), end2 = (*it).end(); it2 != end2; ++it2)
		{
			(*it2) = new CellComponent(this);
			addAndMakeVisible((*it2));

			x = index % _numberOfCellsX; y = index / _numberOfCellsX;
			(*it2)->setBounds(_cellSize + (x * _cellSize), _cellSize + (y * _cellSize), _cellSize, _cellSize);
			index++;
		}
	}

	generateBranch();

	_pStatusBar->setBounds(0, iWindowHeight - _iStatusBarHeight, iWindowWidth, _iStatusBarHeight);
	_pSettingsPanel->setBounds(iWindowWidth, 0, _settingsPanelWidth, iWindowHeight);
}

void CellComponent::paint(Graphics &g)
{
	g.fillAll(Colours::transparentWhite);
	g.drawImageAt(SakuraMatrix::__figure_images[to_string(_drawOriginal)][_drawOriginal ? _drawOriginal : _live], 0, 0, false);

	if(_drawFocus)
	{
		g.setColour(Colours::crimson);
		g.drawRect(0, 0, getWidth(), getHeight());
	}
}

void CellComponent::resized()
{
}


void CellComponent::timerCallback()
{
}

void CellComponent::mouseDown(const MouseEvent &e)
{
	if(_pParentComponent->isAlreadySolved())
		return;

	_pParentComponent->rotateCell(this, _pParentComponent->isMouseButtonsDirectionsSwapped() ? !e.mods.isLeftButtonDown() : e.mods.isLeftButtonDown(), e.mods.isMiddleButtonDown());
}

void CellComponent::mouseWheelMove(const MouseEvent &, float x, float y)
{
	if(_pParentComponent->isAlreadySolved())
		return;

	_pParentComponent->rotateCell(this, _pParentComponent->isMouseWheelDirectionsSwapped() ? !(x > 0 || y > 0) : (x > 0 || y > 0));
}

void StatusBarComponent::buttonClicked(Button *button)
{
	if(button == _pGenerateButton)
		_pMatrix->generateBranch();
	else
		if(button == _pShuffleButton)
			_pMatrix->shuffleMatrix();
		else
			if(button == _pSettingsButton)
				_pMatrix->toggleSettings();
}

void SakuraMatrix::toggleSettings()
{
	if(_settingsVisible)
	{
		_pSettingsPanel->setVisible(_settingsVisible = false);

		setSize(getWidth() - _settingsPanelWidth, getHeight());
	}
	else
	{
		_pSettingsPanel->setVisible(_settingsVisible = true);

		setSize(getWidth() + _settingsPanelWidth, getHeight());
	}
}

bool SakuraMatrix::positionRoot()
{
	_x_root_cell = _numberOfCellsX / 2;
	_y_root_cell = _numberOfCellsY / 2;

	if(_matrix[_y_root_cell][_x_root_cell]->count() < 2)
	{
		for(int y_clip_top = _y_root_cell - ((_numberOfCellsY & 1) ? 0 : 1), y_clip_bottom = _y_root_cell,
				x_clip_left = _x_root_cell - ((_numberOfCellsX & 1) ? 0 : 1), x_clip_right = _x_root_cell;
				y_clip_top >= 0 && x_clip_left >= 0;
				y_clip_top--, y_clip_bottom++, x_clip_left--, x_clip_right++)
		{
			for(int y_ = y_clip_top; y_ <= y_clip_bottom; y_++)
			{
				for(int x_ = x_clip_left; x_ <= x_clip_right; x_++)
				{
					if(_matrix[y_][x_]->count() >= 2)
					{
						_x_root_cell = x_;
						_y_root_cell = y_;

						return true;
					}
				}
			}
		}
	}

	return false;
}

bool SakuraMatrix::perform(const InvocationInfo &info)
{
	bool commandProcessed = false;
	bool moveFocus = false;
	int direction = -1;

	switch(info.commandID)
	{
		case generateCommandId:
			generateBranch();
			repaint();

			commandProcessed = true;
			break;

		case shuffleCommandId:
			shuffleMatrix();

			commandProcessed = true;
			break;

		case peekCommandId:
			if(!info.isKeyDown)
			{
				setPeekMode(false);

				commandProcessed = true;
			}
			else
			{
				if(!isAlreadySolved())
				{
					setPeekMode(true);

					commandProcessed = true;
				}
			}
			break;

		case settingsCommandId:
			toggleSettings();
			break;

		case keyboardMoveLeftCommandId:
			if(_keyboardSupport && !_solved)
			{
				moveFocus = true;

				direction = int(Direction::Left);
			}
			break;

		case keyboardMoveUpCommandId:
			if(_keyboardSupport && !_solved)
			{
				moveFocus = true;

				direction = int(Direction::Top);
			}
			break;

		case keyboardMoveRightCommandId:
			if(_keyboardSupport && !_solved)
			{
				moveFocus = true;

				direction = int(Direction::Right);
			}
			break;

		case keyboardMoveDownCommandId:
			if(_keyboardSupport && !_solved)
			{
				moveFocus = true;

				direction = int(Direction::Bottom);
			}
			break;

		case rotateLeftCommandId:
			if(_keyboardSupport && !_solved)
			{
				rotateCell(_matrix[_y_focus][_x_focus], true);
			}
			break;

		case rotateRightCommandId:
			if(_keyboardSupport && !_solved)
			{
				rotateCell(_matrix[_y_focus][_x_focus], false);
			}
			break;
	}

	if(moveFocus && direction >= 0)
	{
		_matrix[_y_focus][_x_focus]->setDrawFocus(false);

		_x_focus += __delta[direction].first;
		_y_focus += __delta[direction].second;

		if(_x_focus < 0)
			_x_focus = _numberOfCellsX - 1;
		else
			if(_x_focus >= _numberOfCellsX)
				_x_focus = 0;
			else
				if(_y_focus < 0)
					_y_focus = _numberOfCellsY - 1;
				else
					if(_y_focus >= _numberOfCellsY)
						_y_focus = 0;

		_matrix[_y_focus][_x_focus]->setDrawFocus(true);

		commandProcessed = true;
	}

	return commandProcessed;
}

void StatusBarComponent::mouseDown(const MouseEvent &e)
{
	if(e.eventComponent == _pPeekModeButton &&
		!_pMatrix->isPeekMode() &&
		!_pMatrix->isAlreadySolved())
		_pMatrix->setPeekMode(true);
}

void StatusBarComponent::mouseUp(const MouseEvent &e)
{
	if(e.eventComponent == _pPeekModeButton)
		_pMatrix->setPeekMode(false);
}

START_JUCE_APPLICATION(Sakura)

#include "Arkanoid.h"
#include "LevelDesigner.h"
#include "loadbmp.h"
#include <fstream>
#include <iomanip>
#include <string>

/*
    longest string: 54 characters
    "****************************************************",
    maximum 8 lines
*/
const char *blockNrSelectorText[] = 
{
    "Use the + and - buttons to change the button nr you",
    "want to edit. The game can keep track of a maximum",
    "nr of " MAX_BLOCK_TYPES_STR " different blocks. I suggest you design",
    "them in your image editing tool and save them as 24",
    "bit " GFX_FILENAME_EXTENSION " files: "
    SOURCE_FILES_PATH BLOCK_SKIN_FILENAME_BASE "XX" GFX_FILENAME_EXTENSION,
    "where XX is the next skin library number (there can",
    "be no gap between file nrs). Use ++ & -- for 25 step",
    "intervals. The blocks can have any size you want.",
    nullptr
};

const char *levelSelectorText[] = 
{
    "Go to the next or the previous level by clicking the",
    "+ and - buttons. The level you're currently working",
    "on will be saved automatically.",
    nullptr
};

const char *backgroundSelectorText[] = 
{
    "Click the + and - buttons to select a different",
    "background file nr to be used as background image to",
    "this level. Background files should have at least a",
    "resolution of 1024x768, be named \"backgroundXX.bmp\"",
    "where XX is lower than 100 and always 2 digits wide.",
    "The files should be place in the following folder:",
    SOURCE_FILES_PATH ".",
    nullptr
};

const char *backgroundNrText[] =
{
    "Click this area to cycle between the different",
    "directions the backgrounds can scroll in. Or remain",
    "static if the background is not a repeating one.",
    nullptr
};

const char *skinPreviewClientText[] = 
{
    "Drag the image around with the mouse to view a",
    "different area of the block skin library. Click the",
    "right mouse button to bring the area selector cursor",
    "to the location of the mouse pointer. The cursor 'll",
    "automatically wrap around the selected block. You",
    "can't use full black (0x00) in the block as this",
    "color is used as a key color to separate the blocks",
    "in the block skin library image.",
    nullptr
};

const char *skinLibrarySelectorText[] = 
{
    "Click the + and - buttons to select a different",
    "button skin library file nr to be loaded into the",
    "editor. Skin library files should have at least a",
    "resolution of 1024x768, be named as follows:",
    BLOCK_SKIN_FILENAME_BASE "XX" GFX_FILENAME_EXTENSION
    " where XX is lower than 100",
    "and always 2 digits wide. The files should be",
    "placed in " SOURCE_FILES_PATH ". The blocks",
    "should be rectangular and surrounded by black (0x0).",
    nullptr
};

const char *colorRGBText[] = 
{
    "Change the color of the block by modifying the",
    "R(ed), G(reen) and B(lue) primary colors. This",
    "color is only used if you choose not to use a skin",
    "for the currently selected button from the library.",
    "Click the + and - buttons with the left mouse button",
    "for a slow change or use the right mouse button for",
    "a faster change rate.",
    nullptr
};

const char *hitsToDestructText[] = 
{
    "Click this button to set the number of hits a block",
    "can take before it is destroyed. The infinity sign",
    "means the block is persistant and does not need to",
    "be destroyed to finish the level.",
    nullptr
};

const char *blockSkinOrColorText[] = 
{
    "Click this button to choose between a block with a",
    "textured surface or a plain block without one. You",
    "can use this function in case you want to make a ",
    "set of levels that take up less (disk) space. It is",
    "only here to make sure the game will still run if",
    "some files are missing.",
    nullptr
};

const char *levelTitleText[] =
{
    "To change the title of the level, you 'll have to",
    "edit the text file containing all the titles:" ,
    SOURCE_FILES_PATH LEVEL_TITLE_FILENAME,
    "I suggest you use an editor with line numbering such",
    "as Notepad++ to edit the level titles.",
    nullptr
};

const char *gameWindowText[] =
{
    "Click the left mouse button to place the currently",
    "active block on the playfield. If you press the",
    "spacebar while clicking the left mouse button it'll",
    "delete the block on the cursor's position. Click the",
    "right mouse button to cycle through the power ups.",
    "This will only work if you're pointing at a block of",
    "course. Newly placed blocks will align to the grid",
    "automatically.",
    nullptr
};

const char *toLevelEditorSwitchText[] =
{
    "Click this button to switch between the level",
    "designer mode and the block designer mode. You can",
    "keep editing the level in both modes, but only",
    "modify the blocks' design in the block designer",
    "mode. The level designer mode will make choosing the",
    "right block easier.",
    nullptr
};


LevelData::LevelData()
{
    blocks = nullptr;
    levels = nullptr;

    //remove(SOURCE_FILES_PATH LEVELDATA_FILENAME ); // TEMP DEBUG!

    int result = loadLevelData();
    if( result != 0 )
    {
        generateLevelData();
        result = loadLevelData();
        if( result != 0 ) exit ( - 1 );
    }
}
LevelData::~LevelData()
{
    saveLevelData();
    if( (blocks != nullptr) && (nrOfBlocks > 0) ) delete [] blocks;
    if( (levels != nullptr) && (nrOfLevels > 0) ) delete [] levels;
}
int LevelData::loadLevelData()
{
    LevelDataHeader levelDataHeader;    
    std::ifstream   levelDataFile;

    levelDataFile.open(SOURCE_FILES_PATH LEVELDATA_FILENAME,
                                std::ios_base::in | std::ios_base::binary );
    if ( ! levelDataFile ) return - 1;
    levelDataFile.read( (char *)(&levelDataHeader),sizeof(LevelDataHeader) );
    if ( ( levelDataHeader.asciiz != 0 )                            ||
         ( levelDataHeader.blockDataOffset != BLOCK_DATA_OFFSET )   ||
         ( levelDataHeader.eofChar != END_OF_FILE_CHAR )            ||
         ( levelDataHeader.nrOfBlocks < 0 )                         ||
         ( levelDataHeader.nrOfBlocks > MAX_BLOCK_TYPES )           ||
         ( levelDataHeader.nrOfLevels < 0 )                         ||
         ( levelDataHeader.nrOfLevels > MAX_NR_OF_LEVELS )          ||
         ( levelDataHeader.maxStartLevel >= MAX_NR_OF_LEVELS )
        ) return -1;
    nrOfBlocks = levelDataHeader.nrOfBlocks;
    nrOfLevels = levelDataHeader.nrOfLevels;
    maxStartLevel = levelDataHeader.maxStartLevel;
    /*
        read all the blocks into memory:
    */
    blocks = new Block[MAX_BLOCK_TYPES];
    memset ( blocks,0,sizeof( Block ) * MAX_BLOCK_TYPES );
    for( int blockNr = 0; blockNr < nrOfBlocks; blockNr++ )
    {
        BlockHeader     blockHeader;
        levelDataFile.read( (char *)(&blockHeader),sizeof(BlockHeader) );
        blocks[blockNr].width           = blockHeader.width;
        blocks[blockNr].height          = blockHeader.height;
        blocks[blockNr].hitsToDestruct  = blockHeader.hitsToDestruct;
        blocks[blockNr].color           = blockHeader.color;
        blocks[blockNr].skinFileNr      = blockHeader.skinFileNr;
        blocks[blockNr].skinXLocation   = blockHeader.skinXLocation;
        blocks[blockNr].skinYLocation   = blockHeader.skinYLocation;
        /*
            Check if there is a bmp file with the skin of the block
            and load the skin from this bmp into the blocks' bmp
        */
        std::ifstream   blockSkinBmpFile;
        char buf[MAX_SOURCE_FILENAME_LENGTH]; // strlen("blockSkinLibrary00.bmp") = 23 with zero term
        createFilename(
            buf, 
            SOURCE_FILES_PATH BLOCK_SKIN_FILENAME_BASE,
            blockHeader.skinFileNr,GFX_FILENAME_EXTENSION );
        blockSkinBmpFile.open( buf,std::ios_base::in | std::ios_base::binary );
        if ( blockSkinBmpFile ) 
        {
            blockSkinBmpFile.close();
            Rect area;
            area.x1 = blockHeader.skinXLocation;
            area.x2 = blockHeader.skinXLocation + blockHeader.width - 1;
            area.y1 = blockHeader.skinYLocation;
            area.y2 = blockHeader.skinYLocation + blockHeader.height - 1;
            int skinReadError = -1;
            if ( blockHeader.isSkinUsed )
                skinReadError = blocks[blockNr].skin.loadFromBMP( buf, area );
            if ( skinReadError != 0 )
             {               
                blocks[blockNr].skin.createEmptyBMP( 
                    blockHeader.width,blockHeader.height,blockHeader.color );
                blocks[blockNr].skin.MakeButtonEdges();
            } else { 
                blocks[blockNr].isSkinPresent = true;
                blocks[blockNr].isSkinUsed = blockHeader.isSkinUsed;
            }
        } else {
            blocks[blockNr].skin.createEmptyBMP(
                blockHeader.width,blockHeader.height,blockHeader.color );
            blocks[blockNr].isSkinPresent = false;
            blocks[blockNr].isSkinUsed = false;
            blocks[blockNr].skin.MakeButtonEdges();
        }
    }
    /*
        read all the levels into memory:
    */    
    levels = new Level[MAX_NR_OF_LEVELS];
    for( int levelNr = 0; levelNr < nrOfLevels; levelNr++ )
    {
        LevelHeader     levelHeader;
        levelDataFile.read( (char *)(&levelHeader),sizeof(LevelHeader) );
        levels[levelNr].backgroundNr   = levelHeader.backgroundNr;
        levels[levelNr].backGroundScrolltype = levelHeader.backGroundScrolltype;
        levels[levelNr].nrOfBlocks     = levelHeader.nrOfBlocks;
        levels[levelNr].gridCellWidth  = levelHeader.gridCellWidth;
        levels[levelNr].gridCellHeight = levelHeader.gridCellHeight;
        levels[levelNr].gridSpacer     = levelHeader.gridSpacer;
        levels[levelNr].blocks         = new BlockData[levelHeader.nrOfBlocks];
        for( int blockNr = 0; blockNr < levelHeader.nrOfBlocks; blockNr++ )
            levelDataFile.read( (char *)(&(levels[levelNr].blocks[blockNr])),
                sizeof(BlockData) );
    }
    levelDataFile.close();
    /*
        load the level titles from the text file LEVEL_TITLE_FILENAME:
    */
    std::string levelTitle;
    std::ifstream levelTitlesFile;
    levelTitlesFile.open ( SOURCE_FILES_PATH LEVEL_TITLE_FILENAME );
    int lineCnt = 0;
    for( lineCnt = 0; (! levelTitlesFile.eof()) && (lineCnt < MAX_NR_OF_LEVELS); lineCnt++ )
        getline( levelTitlesFile,levelTitle );
    levelTitlesFile.clear();
    levelTitlesFile.seekg(0, std::ios::beg);
    for( int lineNr = 0; lineNr < lineCnt; lineNr++ )
    {
        getline( levelTitlesFile,levelTitle );
        const char *s = levelTitle.c_str();
        char *d = levels[lineNr].title;
        for( int i = 0; i < LEVEL_TITLE_MAX_LENGTH; i++ ) *d++ = *s++;
    }
    levelTitlesFile.close();
    return 0;
}
int LevelData::generateLevelData()
{
    LevelDataHeader levelDataHeader;    
    LevelHeader     levelHeader;
    std::ofstream   levelDataFile;
    levelDataFile.open(SOURCE_FILES_PATH LEVELDATA_FILENAME,
        std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
    if ( ! levelDataFile ) return - 1;
    /*
        Create a level data file with only a few levels.
        First we write the general header:
    */
    strcpy( (char *)levelDataHeader.idString,LEVELDATA_ID_STRING );
    levelDataHeader.asciiz = '\0';   
    levelDataHeader.eofChar = END_OF_FILE_CHAR;
    levelDataHeader.nrOfBlocks = DEFAULT_NR_OF_BLOCKS;
    levelDataHeader.nrOfLevels = 1;
    levelDataHeader.blockDataOffset = BLOCK_DATA_OFFSET;
    levelDataHeader.levelDataOffset[0] = sizeof(LevelDataHeader) 
                            + sizeof(BlockHeader) * levelDataHeader.nrOfBlocks;
    levelDataHeader.maxStartLevel = 0;
    levelDataFile.write( (char *)(&levelDataHeader),sizeof(LevelDataHeader) );
    /*
        Create the blocks based on the image in DEFAULT_BLOCK_SKIN_FILENAME if 
        possible, otherwise make DEFAULT_NR_OF_BLOCKS blocks with different 
        colors:
    */
    BlockHeader     *blockHeader = new BlockHeader[DEFAULT_NR_OF_BLOCKS];
    Bitmap          skinLibrary;
    int blockNr = 0;
    if(skinLibrary.loadBMP( SOURCE_FILES_PATH DEFAULT_BLOCK_SKIN_FILENAME ) == 0 )
    {
        for( int j = 0; j < DEFAULT_NR_OF_BLOCK_ROWS; j++ )
            for( int i = 0; i < DEFAULT_NR_OF_BLOCK_COLUMNS; i++ )
            {   
                blockHeader[blockNr].isSkinUsed = true;
                blockHeader[blockNr].skinFileNr = 0;
                blockHeader[blockNr].width = BLOCK_WIDTH;
                blockHeader[blockNr].height = BLOCK_HEIGHT;
                blockHeader[blockNr].hitsToDestruct = 1;
                blockHeader[blockNr].skinXLocation = i * COLLISION_CELL_WIDTH + SPACER;
                blockHeader[blockNr].skinYLocation = j * COLLISION_CELL_HEIGHT + SPACER;
                blockHeader[blockNr].color = skinLibrary.getPixel(
                    blockHeader[blockNr].skinXLocation + BLOCK_WIDTH  / 2,
                    blockHeader[blockNr].skinYLocation + BLOCK_HEIGHT / 2 );
                blockNr++;
            }
        skinLibrary.unLoadBMP();
    } else {
        for( int j = 0; j < DEFAULT_NR_OF_BLOCK_ROWS; j++ )
            for( int i = 0; i < DEFAULT_NR_OF_BLOCK_COLUMNS; i++ )
            {                               
                blockHeader[blockNr].color = 0xFF000000 | 
                    D3DCOLOR_XRGB( 80 + j * 8,80 + i * 8,250 - i * j );
                blockHeader[blockNr].isSkinUsed = false;
                blockHeader[blockNr].skinFileNr = 0;
                blockHeader[blockNr].skinXLocation = 0;
                blockHeader[blockNr].skinYLocation = 0;
                blockHeader[blockNr].width = BLOCK_WIDTH;
                blockHeader[blockNr].height = BLOCK_HEIGHT;
                blockHeader[blockNr].hitsToDestruct = 1;
                blockNr++;
            }
    }
    for( int i = 0; i < levelDataHeader.nrOfBlocks; i++ )
        levelDataFile.write( (char *)(&(blockHeader[i])),sizeof(BlockHeader) );
    delete blockHeader;
    /*
        Now write the header of the 1st level:
    */
    memset( &levelHeader,0,sizeof(LevelHeader) );
    levelHeader.backgroundNr         = 0;
    //levelHeader.backGroundScrolltype = staticbackground;
    levelHeader.gridCellWidth        = COLLISION_CELL_WIDTH;
    levelHeader.gridCellHeight       = COLLISION_CELL_HEIGHT;
    levelHeader.gridSpacer           = SPACER;
    levelHeader.nrOfBlocks           = DEFAULT_NR_OF_BLOCKS;
    levelDataFile.write( (char *)(&levelHeader),sizeof(LevelHeader) );
    /*
        Now write the block data of the 1st level:
    */
    BlockData       blockData;
    int iBlock = 0;
    for( int j = 0; j < DEFAULT_NR_OF_BLOCK_ROWS; j++ )
        for( int i = 0; i < DEFAULT_NR_OF_BLOCK_COLUMNS; i++ )
        {
            blockData.blockNr = iBlock;
            blockData.xOrig = (i + 1) * COLLISION_CELL_WIDTH + SPACER;
            blockData.yOrig = (j + 3) * COLLISION_CELL_HEIGHT + SPACER;
            blockData.powerUp = nopowerup;
            levelDataFile.write( (char *)(&blockData),sizeof(BlockData) );
            iBlock++;
        }
    levelDataFile.close();
    return 0;
}
int LevelData::saveLevelData()
{
    LevelDataHeader levelDataHeader;    
    std::ofstream   levelDataFile;
    levelDataFile.open(SOURCE_FILES_PATH LEVELDATA_FILENAME,
        std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
    if ( ! levelDataFile ) return - 1;
    /*
        Let's count the nr of blocks:
    */
    int lastBlockNr = 0;
    for( int iBlock = 0; iBlock < MAX_BLOCK_TYPES; iBlock++ )
        if( blocks[iBlock].width > 0 ) lastBlockNr = iBlock;
    /*
        First we write the general header:
    */
    strcpy( (char *)levelDataHeader.idString,LEVELDATA_ID_STRING );
    levelDataHeader.asciiz = '\0';   
    levelDataHeader.eofChar = END_OF_FILE_CHAR;
    levelDataHeader.nrOfBlocks = lastBlockNr + 1;
    levelDataHeader.nrOfLevels = nrOfLevels;
    levelDataHeader.blockDataOffset = BLOCK_DATA_OFFSET;
    levelDataHeader.levelDataOffset[0] = sizeof(LevelDataHeader) 
        + sizeof(BlockHeader) * levelDataHeader.nrOfBlocks;
    levelDataHeader.maxStartLevel = maxStartLevel;
    levelDataFile.write( (char *)(&levelDataHeader),sizeof(LevelDataHeader) );
    /*
        write the block data to the file:
    */
    for( int iBlock = 0; iBlock <= lastBlockNr; iBlock++ )
    {
        BlockHeader blockHeader;
        blockHeader.color           = blocks[iBlock].color;
        blockHeader.width           = blocks[iBlock].width;
        blockHeader.height          = blocks[iBlock].height;
        blockHeader.hitsToDestruct  = blocks[iBlock].hitsToDestruct;
        blockHeader.isSkinUsed      = blocks[iBlock].isSkinUsed;
        blockHeader.skinFileNr      = blocks[iBlock].skinFileNr;
        blockHeader.skinXLocation   = blocks[iBlock].skinXLocation;
        blockHeader.skinYLocation   = blocks[iBlock].skinYLocation;
        levelDataFile.write( (char *)(&blockHeader),sizeof(BlockHeader) );
    }
    for( int iLevel = 0; iLevel < nrOfLevels; iLevel++ )
    {
        /*
            Now write the header of the level:
        */
        LevelHeader levelHeader;
        levelHeader.backgroundNr = levels[iLevel].backgroundNr;
        levelHeader.backGroundScrolltype = levels[iLevel].backGroundScrolltype;
        levelHeader.gridCellWidth = levels[iLevel].gridCellWidth;
        levelHeader.gridCellHeight = levels[iLevel].gridCellHeight;
        levelHeader.gridSpacer = levels[iLevel].gridSpacer;
        int blockCnt = 0;
        for( int i = 0; i < levels[iLevel].nrOfBlocks; i++ )
            if( levels[iLevel].blocks[i].xOrig > 0 ) blockCnt++;
        levelHeader.nrOfBlocks = blockCnt;
        levelDataFile.write( (char *)(&levelHeader),sizeof( LevelHeader ) );
        /*
            Now write the block data of the level:
        */
        for( int i = 0; i < levels[iLevel].nrOfBlocks; i++ )
            if( levels[iLevel].blocks[i].xOrig > 0 ) 
                levelDataFile.write( (char *)(&(levels[iLevel].blocks[i])),sizeof(BlockData) );
    }
    levelDataFile.close();
    return 0;
}
LevelDesigner::LevelDesigner()
{
    /* 
        The font is loaded after the constructor of the level designer has ran.
        Therefor the Level Designer block library bitmap can't be initialized 
        until after the font has been loaded in the constructor of the main 
        Game class. Thus we start in block designer mode here; we switch to 
        level designer mode in the Game class constructor after the font is 
        loaded there.
    */    
    font = nullptr;
    isInLevelDesignerMode = false;
    currentLevelNr = 0;
    currentBlockData = new BlockData[MAX_BLOCKS_PER_LEVEL];
    memset( currentBlockData,0,sizeof(BlockData) * MAX_BLOCKS_PER_LEVEL );
    int blockNr = 0;
    for ( ; blockNr < levelData.levels[currentLevelNr].nrOfBlocks; blockNr++ )
        currentBlockData[blockNr] = levelData.levels[currentLevelNr].blocks[blockNr];
    for ( ; blockNr < MAX_BLOCKS_PER_LEVEL; blockNr++ )
        currentBlockData[blockNr].xOrig = UNUSED_SLOT;
    currentBlockNr = 0;
    blockSkinLibXOrig = 0;
    blockSkinLibYOrig = 0;
    if ( ! levelData.blocks[currentBlockNr].isSkinUsed )
         currentBlockSkinLibNr = 0;
    else currentBlockSkinLibNr = levelData.blocks[currentBlockNr].skinFileNr;
    char filename[MAX_SOURCE_FILENAME_LENGTH];
    createFilename( filename,
        SOURCE_FILES_PATH BLOCK_SKIN_FILENAME_BASE,
        currentBlockSkinLibNr,GFX_FILENAME_EXTENSION );
    blockSkinLib.loadBMP( filename );
    blockLibraryXOrig = 0;
    blockLibraryYOrig = 0;
    setBackground( levelData.levels[currentLevelNr].backgroundNr );
}
int LevelDesigner::loadSounds( DSound& audio )
{
    inGameLevel.sounds[SOUND_PADDLE_HIT]  = audio.CreateSound( SOURCE_FILES_PATH SOUND_PADDLE_HIT_FILENAME );
    inGameLevel.sounds[SOUND_BLOCK_DIES]  = audio.CreateSound( SOURCE_FILES_PATH SOUND_BLOCK_DIES_FILENAME );
    inGameLevel.sounds[SOUND_BLOCK_HIT]   = audio.CreateSound( SOURCE_FILES_PATH SOUND_BLOCK_HIT_FILENAME );
    inGameLevel.sounds[SOUND_FIRE_BULLET] = audio.CreateSound( SOURCE_FILES_PATH SOUND_FIRE_BULLET_FILENAME );
    inGameLevel.sounds[SOUND_NEXT_LEVEL]  = audio.CreateSound( SOURCE_FILES_PATH SOUND_NEXT_LEVEL_FILENAME );
    inGameLevel.sounds[SOUND_LIVE_LOST]   = audio.CreateSound( SOURCE_FILES_PATH SOUND_LIVE_LOST_FILENAME );
    return 0;
}
LevelDesigner::~LevelDesigner()
{
    setLevelNr( currentLevelNr );
    delete [] currentBlockData;
}
int LevelDesigner::getBlockAtPosition( int x,int y )
{
    assert( x >= GAME_WINDOW_X1 );
    assert( x <= GAME_WINDOW_X2 );
    assert( y >= GAME_WINDOW_Y1 );
    assert( y <= GAME_WINDOW_Y2 );

    int iSlot;
    bool found = false;
    for ( iSlot = 0; iSlot < MAX_BLOCKS_PER_LEVEL; iSlot++ )
    {
        if( x < currentBlockData[iSlot].xOrig ) continue;
        if( y < currentBlockData[iSlot].yOrig ) continue;
        int &w = levelData.blocks[currentBlockData[iSlot].blockNr].width;
        if( x >= currentBlockData[iSlot].xOrig + w ) continue;
        int &h = levelData.blocks[currentBlockData[iSlot].blockNr].height;
        if( y >= currentBlockData[iSlot].yOrig + h ) continue;
        found = true;
        break;
    }
    if ( found ) return iSlot;
    else return -1;
}
int LevelDesigner::getFreeSlot()
{
    int iSlot;
    for ( iSlot = 0; iSlot < MAX_BLOCKS_PER_LEVEL; iSlot++ )
    {
        if( currentBlockData[iSlot].xOrig == UNUSED_SLOT ) break;
    }
    if( currentBlockData[iSlot].xOrig == UNUSED_SLOT ) return iSlot;
    return -1;
}
void LevelDesigner::putBlockAtPosition( int x,int y )
{
    removeBlockAtPosition( x,y );
    // find a free slot in the block list ( x == UNUSED_SLOT or -1 )
    int freeSlot = getFreeSlot();
    if ( freeSlot == -1 ) return; // no room left for more blocks
    // add the block to the playfield
    Rect r = grid.getCellClientRect( x,y );
    currentBlockData[freeSlot].xOrig = r.x1 
        + ((r.x2 - r.x1 + 1) - getBlock( currentBlockNr ).width) / 2;
    currentBlockData[freeSlot].yOrig = r.y1
        + ((r.y2 - r.y1 + 1) - getBlock ( currentBlockNr ).height) / 2;
    currentBlockData[freeSlot].blockNr = currentBlockNr;
    currentBlockData[freeSlot].powerUp = nopowerup;
}
inline void LevelDesigner::removeBlock( int blockNr )
{
    assert( blockNr >= 0 );
    assert( blockNr < MAX_BLOCKS_PER_LEVEL );
    getBlockList()[blockNr].xOrig = -1;
}
// a bit vague, not 100% accurate, should scan all the blocks actually
// --> to be improved!!
void LevelDesigner::removeBlockAtPosition( int x,int y )
{
    Rect r = grid.getCellClientRect( x,y );
    int pos;
    pos = getBlockAtPosition( r.x1,r.y1 ); 
    if( pos != -1 ) removeBlock( pos );
    pos = getBlockAtPosition( r.x1,r.y2 );
    if( pos != -1 ) removeBlock( pos );
    pos = getBlockAtPosition( r.x2,r.y1 );
    if( pos != -1 ) removeBlock( pos );
    pos = getBlockAtPosition( r.x2,r.y2 );
    if( pos != -1 ) removeBlock( pos );
    pos = getBlockAtPosition ( x,y );
    if ( pos != -1 ) removeBlock ( pos );
    pos = getBlockAtPosition(
        r.x1 + (r.x2 - r.x1) / 2,
        r.y1 + (r.y2 - r.y1) / 2 );
    if( pos != -1 ) removeBlock( pos );
}
int  LevelDesigner::setBackground( int backGroundNr )
{
    assert ( backGroundNr >= 0 );
    assert ( backGroundNr < MAX_NR_OF_LEVELS );
    char bgFilename[MAX_SOURCE_FILENAME_LENGTH]; 
    createFilename(bgFilename, SOURCE_FILES_PATH BACKGROUND_FILENAME_BASE, 
        backGroundNr, GFX_FILENAME_EXTENSION );
    std::ifstream   backGroundFile;
    backGroundFile.open( bgFilename,std::ios_base::in | std::ios_base::binary );
    if ( ! backGroundFile ) return -1;
    backGroundFile.close();
    currentBackground.loadBMP( bgFilename );
    levelData.levels[currentLevelNr].backgroundNr = backGroundNr;
    if ( levelData.levels[currentLevelNr].backGroundScrolltype == staticbackground )
    {
        backGroundX = (HOR_SCREEN_RES - getBackground().getWidth())  / 2;
        backGroundY = (VER_SCREEN_RES - getBackground().getHeight()) / 2;
    } else {
        backGroundX = 0;
        backGroundY = 0;
    }
    /*
    // debug:
    currentBackground.fill( 0xFF0000 );
    for( int j = 1; j < currentBackground.getHeight() - 1; j++ )
        for ( int i = 1; i < currentBackground.getWidth() - 1; i++ )
            currentBackground.putPixel( i,j,0xFFFFFF );
    // end debug 
    */
    /*
    // added:
    int w = currentBackground.getWidth();
    int h = currentBackground.getHeight();
    if ( w < HOR_SCREEN_RES || h < VER_SCREEN_RES )
    {
        Bitmap tile;
        tile.loadFromMemory(  
            currentBackground.getWidth(),
            currentBackground.getHeight(),
            currentBackground.getPixelData() );
        currentBackground.unLoadBMP();
        int xTileCnt = (HOR_SCREEN_RES / w) + 1;
        int yTileCnt = (VER_SCREEN_RES / h) + 1;
        if ( xTileCnt < 2 ) xTileCnt++;
        if ( yTileCnt < 2 ) yTileCnt++;
        currentBackground.createEmptyBMP( xTileCnt * w,yTileCnt * h );
        for ( int j = 0; j < yTileCnt; j++ )
            for ( int i = 0; i < xTileCnt; i++ )
                currentBackground.insertFromBMP( i * w,j * h,tile );
    }
    
    // game is always played full screen:
    backGroundX = (HOR_SCREEN_RES - getBackground().getWidth()) / 2;
    backGroundY = (VER_SCREEN_RES - getBackground().getHeight()) / 2;
    */      
    /*
    backGroundX = 0;
    backGroundY = 0;
    */
    return 0;
}
void LevelDesigner::incBackgroundNr()
{   
    int& currentBackgroundNr = levelData.levels[currentLevelNr].backgroundNr;
    int origBG = currentBackgroundNr;
    currentBackgroundNr++;
    if (currentBackgroundNr >= MAX_NR_OF_LEVELS)
        currentBackgroundNr = MAX_NR_OF_LEVELS - 1;
    if (currentBackgroundNr < 0 ) currentBackgroundNr = 0;
    if ( setBackground ( currentBackgroundNr ) != 0 )
    {
        currentBackgroundNr = origBG;
        setBackground ( currentBackgroundNr );
    }
}
void LevelDesigner::decBackgroundNr()
{   
    int& currentBackgroundNr = levelData.levels[currentLevelNr].backgroundNr;
    int origBG = currentBackgroundNr;
    currentBackgroundNr--;
    if ( currentBackgroundNr >= MAX_NR_OF_LEVELS )
        currentBackgroundNr = MAX_NR_OF_LEVELS - 1;
    if ( currentBackgroundNr < 0 ) currentBackgroundNr = 0;
    if (setBackground( currentBackgroundNr ) != 0 )
    {
        currentBackgroundNr = origBG;
        setBackground( currentBackgroundNr );
    }
}
void LevelDesigner::switchBackgroundScrollType()
{
    backgroundscrolltype& scrollType =
        levelData.levels[currentLevelNr].backGroundScrolltype;
    if ( scrollType == staticbackground ) scrollType = scrollup;
    else if ( scrollType == scrollup    ) scrollType = scrolldown;
    else if ( scrollType == scrolldown  ) scrollType = scrollleft;
    else if ( scrollType == scrollleft  ) scrollType = scrollright;
    else if ( scrollType == scrollright ) scrollType = randomscroll;
    else scrollType = staticbackground;
}
void LevelDesigner::setLevelNr( int levelNr )
{
    int prevLevelNr = currentLevelNr;
    if ( levelNr >= MAX_NR_OF_LEVELS ) 
        levelNr = MAX_NR_OF_LEVELS - 1;
    if ( levelNr < 0 ) levelNr = 0;
    //if ( prevLevelNr == levelNr ) return; removed for saving functionality
    currentLevelNr = levelNr;
    // save the level we were working on to the library:
    Level& level = levelData.levels[prevLevelNr];    
    if( level.blocks != nullptr ) delete [] level.blocks;
    int blockNr = 0;
    for ( int iBlock = 0; iBlock < MAX_BLOCKS_PER_LEVEL; iBlock++ )
        if ( currentBlockData[iBlock].xOrig >= 0 ) blockNr++;
    level.nrOfBlocks = blockNr;
    level.blocks = new BlockData[blockNr];
    int blockCnt = 0;
    for ( int iBlock = 0; iBlock < MAX_BLOCKS_PER_LEVEL; iBlock++ )
    {
        if ( currentBlockData[iBlock].xOrig >= 0 )
        {
            level.blocks[blockCnt] = currentBlockData[iBlock];
            blockCnt++;
            currentBlockData[iBlock].xOrig = UNUSED_SLOT;
        }
    }
    // load the current level nr:
    Level& newLevel = levelData.levels[currentLevelNr];
    for ( int iBlock = 0; iBlock < newLevel.nrOfBlocks; iBlock++ )
        currentBlockData[iBlock] = newLevel.blocks[iBlock];
    setBackground ( levelData.levels[currentLevelNr].backgroundNr );
    grid.Init( levelData.levels[currentLevelNr].gridCellWidth,
               levelData.levels[currentLevelNr].gridCellHeight,
               levelData.levels[currentLevelNr].gridSpacer );        
}
void LevelDesigner::incLevelNr()
{
    setLevelNr( currentLevelNr + 1 ); 
    if( (currentLevelNr + 1) > (levelData.nrOfLevels) )
        levelData.nrOfLevels = currentLevelNr + 1;
}   
void LevelDesigner::decLevelNr()
{
    setLevelNr( currentLevelNr - 1 );  
}   
int  LevelDesigner::setBlockSkinLibNr( int skinLibNr )        
{   
    assert( currentBlockSkinLibNr >= 0 );
    assert( currentBlockSkinLibNr < MAX_BLOCK_SKIN_FILES );
    char skinFilename[MAX_SOURCE_FILENAME_LENGTH]; // strlen("blockSkinLibrary00.bmp") = 23 with zero term
    createFilename(
        skinFilename, 
        SOURCE_FILES_PATH BLOCK_SKIN_FILENAME_BASE, 
        currentBlockSkinLibNr, 
        GFX_FILENAME_EXTENSION );
    std::ifstream   blockSkinLibFile;
    blockSkinLibFile.open( skinFilename,std::ios_base::in | std::ios_base::binary );
    if ( ! blockSkinLibFile ) return - 1;
    blockSkinLibFile.close();
    blockSkinLib.loadBMP (skinFilename );
    Block& block = levelData.blocks[currentBlockNr];
    if ( block.isSkinUsed )
    {
        Rect area;
        area.x1 = block.skinXLocation;
        area.y1 = block.skinYLocation;
        area.x2 = area.x1 + block.width  - 1;
        area.y2 = area.y1 + block.height - 1;
        if( (area.x2 >= blockSkinLib.getWidth() - 1) ||
            (area.y2 >= blockSkinLib.getHeight() - 1) )
        {
            area.x1 = 0;
            area.y1 = 0;
            area.x2 = area.x1 + block.width  - 1;
            area.y2 = area.y1 + block.height - 1;
            blockSkinLibXOrig = 0;          
            blockSkinLibYOrig = 0;          
            if( (area.x2 >= blockSkinLib.getWidth() - 1) ||
                (area.y2 >= blockSkinLib.getHeight() - 1) ) return 0;
        }
    }
    return 0;
}
void LevelDesigner::incBlockSkinLibNr()
{
    currentBlockSkinLibNr++;
    if ( (currentBlockSkinLibNr + 1) >= MAX_BLOCK_SKIN_FILES ) 
        currentBlockSkinLibNr = MAX_BLOCK_SKIN_FILES - 1;
    if ( currentBlockSkinLibNr < 0 ) currentBlockSkinLibNr = 0;
    setBlockSkinLibNr( currentBlockSkinLibNr );
}
void LevelDesigner::decBlockSkinLibNr()
{
    currentBlockSkinLibNr--;
    if ( (currentBlockSkinLibNr + 1) >= MAX_BLOCK_SKIN_FILES ) 
        currentBlockSkinLibNr = MAX_BLOCK_SKIN_FILES - 1;
    if ( currentBlockSkinLibNr < 0 ) currentBlockSkinLibNr = 0;
    setBlockSkinLibNr( currentBlockSkinLibNr );
}
void LevelDesigner::setBlockColor()
{
    Block& block = levelData.blocks[currentBlockNr];
    int& c = block.color;
    if ( (c & 0xFFFFFF) == 0 ) c++; // color can't be 100% black
    if ( ! block.isSkinUsed )
    {   
        block.skin.fill( block.color );
        block.skin.MakeButtonEdges();
    }
}
void LevelDesigner::changeRGBColor(rgbcomponent c,int relChange )
{
    int& color = levelData.blocks[currentBlockNr].color;
    switch ( c )
    {
        case rcomponent:
        {
            int t = (color >> 16) & 0xFF;
            t += relChange;
            if( t < 0 ) t = 0;
            if (t > 0xFF) t = 0xFF;
            color = (color & 0x00FFFF) + (t << 16);
            break;
        }
        case gcomponent:
        {
            int t = (color >> 8) & 0xFF;
            t += relChange;
            if (t < 0) t = 0;
            if (t > 0xFF) t = 0xFF;
            color = (color & 0xFF00FF) + (t << 8);
            break;
        }
        case bcomponent:
        {
            int t = color & 0xFF;
            t += relChange;
            if (t < 0) t = 0;
            if (t > 0xFF) t = 0xFF;
            color = (color & 0xFFFF00) + t;
            break;
        }
    }
    setBlockColor ();
}
void LevelDesigner::setBlockNr( int blockNr ) 
{
    assert( blockNr >= 0 );
    assert( blockNr < MAX_BLOCK_TYPES );
    currentBlockNr = blockNr;
    Block& block = levelData.blocks[currentBlockNr];
    currentBlockSkinLibNr = block.skinFileNr; 
    setBlockSkinLibNr( currentBlockSkinLibNr );
    setBlockCursorLocation( block.skinXLocation,block.skinYLocation );
    if ( (block.hitsToDestruct < 1) || (block.hitsToDestruct > PERSISTENT_BLOCK) )
        block.hitsToDestruct = 1;

}
void LevelDesigner::changeBlockNr( int relChange )
{
    currentBlockNr += relChange;
    if ( (currentBlockNr + 1) >= MAX_BLOCK_TYPES ) 
        currentBlockNr = MAX_BLOCK_TYPES - 1;
    if ( currentBlockNr < 0 ) currentBlockNr = 0;
    setBlockNr( currentBlockNr );
}
void LevelDesigner::setBlockCursorLocation( int relX,int relY )
{
    // set the skin block cursor the to skin location and try to center it
    Block& block = levelData.blocks[currentBlockNr];
    blockSkinLibXOrig = 0; 
    blockSkinLibYOrig = 0; 
    shiftBlockSkinLibXOrig( block.skinXLocation - 2 );
    shiftBlockSkinLibYOrig( block.skinYLocation - 2 );
}
void LevelDesigner::switchBlockColorOrSkinned()
{    
    Block& block = levelData.blocks[currentBlockNr];
    block.isSkinUsed = ! block.isSkinUsed;
    if ( ! block.isSkinUsed ) setBlockColor();
    else {
        if ( block.isSkinPresent )
        {
            Rect area;
            area.x1 = block.skinXLocation;
            area.x2 = block.skinXLocation + block.width - 1;
            area.y1 = block.skinYLocation;
            area.y2 = block.skinYLocation + block.height - 1;
            block.skin.copyFromBMP( blockSkinLib,area );
        } else {
            block.isSkinUsed = false;
            setBlockColor();
        }
    }
}
void LevelDesigner::incHitsToDestruct()
{
    int& hitsToDestruct = levelData.blocks[currentBlockNr].hitsToDestruct;
    hitsToDestruct++;
    if ( hitsToDestruct > PERSISTENT_BLOCK ) hitsToDestruct = 1;
}
void LevelDesigner::selectBlockFromSkinLib( int x,int y )
{
    if ( x < skinPreviewClientRect.x1 ) return;
    if ( x > skinPreviewClientRect.x2 ) return;
    if ( y < skinPreviewClientRect.y1 ) return;
    if ( y > skinPreviewClientRect.y2 ) return;
    Rect r = blockSkinLib.getBlockBorders(
        x - skinPreviewClientRect.x1 + blockSkinLibXOrig, 
        y - skinPreviewClientRect.y1 + blockSkinLibYOrig );
    if ( (r.x1 == r.x2) || (r.y1 == r.y2) ) return;
    Block& block = levelData.blocks[currentBlockNr];
    block.skinXLocation = r.x1;
    block.skinYLocation = r.y1;
    block.width  = r.x2 - r.x1 + 1;
    block.height = r.y2 - r.y1 + 1;
    block.isSkinPresent = true;
    block.isSkinUsed = true;
    block.skin.copyFromBMP( blockSkinLib,r );
    /*
    if ( block.isSkinUsed ) {
        block.skin.copyFromBMP( blockSkinLib,r );
    } else { 
        if ( block.skin.pixelData != nullptr ) delete block.skin.pixelData;
        const int size = block.width * block.height;
        block.skin.pixelData = new int[size];
        for ( int i = 0; i < size; i++ )
            block.skin.pixelData[i] = block.color;
        block.skin.width  = block.width;
        block.skin.height = block.height;
        //MakeButtonEdges( block.skin );
        block.skin.MakeButtonEdges();
    }
    */
    block.skinFileNr = currentBlockSkinLibNr;
}
void LevelDesigner::initBlockLibrary ()
{
    blockLibrary.unLoadBMP();
    int blockLibWidth = blockPickerClientRect.x2 - blockPickerClientRect.x1 + 1;
    blockLibrarySpacer = 2;
    // find the last block:
    int lastBlockNr = 0;
    for ( int i = 0; i < MAX_BLOCK_TYPES; i++ )
        if ( levelData.blocks[i].width > 0 ) lastBlockNr = i;
    // find the widest & highest block (can be different)
    int blockMaxWidth = 0;
    int blockMaxHeight = 0;
    for ( int i = 0; i <= lastBlockNr; i++ )
    {
        if ( levelData.blocks[i].width > blockMaxWidth )
            blockMaxWidth = levelData.blocks[i].width;
        if ( levelData.blocks[i].height > blockMaxHeight )
            blockMaxHeight = levelData.blocks[i].height;
    }
    blockLibraryCellWidth  = (blockMaxWidth  + blockLibrarySpacer);
    blockLibraryCellHeight = (blockMaxHeight + blockLibrarySpacer);
    blockLibraryColumns = (blockLibWidth - blockLibrarySpacer)
                                                    / blockLibraryCellWidth;
    blockLibraryRows = (lastBlockNr + 1) / blockLibraryColumns;
    if ( (lastBlockNr + 1) % blockLibraryColumns != 0 ) blockLibraryRows++;
    int blockLibHeight = blockLibrarySpacer +
                                blockLibraryCellHeight * blockLibraryRows;
    blockLibrary.createEmptyBMP( blockLibWidth,blockLibHeight,0x0 );
    // Draw the blocks' skin onto the block library bitmap:
    int drawY = blockLibrarySpacer;
    int blockNr = 0;
    char buf[2];
    buf[1] = '\0';
    for ( int y = 0; y < blockLibraryRows; y++ )
    {
        int drawX = blockLibrarySpacer;
        for ( int x = 0; x < blockLibraryColumns; x++ )
        {
            // opgepast voor blocks wiens PixelData niet is geinitialiseerd!!!!
            // -> niet hoger gaan dan laatste block in block designer!!!
            // should be moved to the bitmap class?
            //if( levelData.blocks[blockNr].skin.pixelData != nullptr )
            if ( levelData.blocks[blockNr].skin.isImagePresent() )
                for ( int j = 0; j < levelData.blocks[blockNr].height; j++ )
                    for ( int i = 0; i < levelData.blocks[blockNr].width; i++ )
                        blockLibrary.putPixel(
                            drawX + i,
                            drawY + j,
                            levelData.blocks[blockNr].skin.getPixel( i,j ) );
            // print the blocks hit to destruct number
            if ( levelData.blocks[blockNr].hitsToDestruct < PERSISTENT_BLOCK )
                buf[0] = '0' + levelData.blocks[blockNr].hitsToDestruct;
            else buf[0] = (signed char)INFINITE_SYMBOL;
            blockLibrary.PrintXY( drawX + 3,drawY + 3,buf,font );
            drawX += blockLibraryCellWidth;
            blockNr++;
            if ( blockNr > lastBlockNr ) return;
        }
        drawY += blockLibraryCellHeight;
    }
}
void LevelDesigner::selectBlockFromLibrary( int x,int y )
{
    if ( x < blockPickerClientRect.x1 ) return;
    if ( x > blockPickerClientRect.x2 ) return;
    if ( y < blockPickerClientRect.y1 ) return;
    if ( y > blockPickerClientRect.y2 ) return;
    Rect r = blockLibrary.getBlockBorders(
        x - blockPickerClientRect.x1 + blockLibraryXOrig,
        y - blockPickerClientRect.y1 + blockLibraryYOrig );
    if ( (r.x1 == r.x2) || (r.y1 == r.y2) ) return;
    int col = (r.x1 + (r.x2 - r.x1) / 2) / blockLibraryCellWidth;
    int row = (r.y1 + (r.y2 - r.y1) / 2) / blockLibraryCellHeight;
    currentBlockNr = row * blockLibraryColumns + col;
    setBlockNr( currentBlockNr );
}
void LevelDesigner::loadInGameLevel( int levelNr )
{
    assert( levelNr >= 0 );
    assert( levelNr < MAX_NR_OF_LEVELS );  
    setLevelNr( levelNr );
    inGameLevel.levelTitle = getLevelTitle( levelNr );
    inGameLevel.backgroundBMP = &currentBackground;
    inGameLevel.backGroundScrolltype = 
                                levelData.levels[levelNr].backGroundScrolltype;
    int blockNr = 0;
    for ( int iBlock = 0; iBlock < MAX_BLOCKS_PER_LEVEL; iBlock++ )
        if ( currentBlockData[iBlock].xOrig >= 0 ) blockNr++;
    inGameLevel.nrOfBlocks = blockNr + SPECIAL_BLOCKS; // added '5' for the walls & paddle
    if ( inGameLevel.blocks != nullptr ) delete [] inGameLevel.blocks;
    inGameLevel.blocks = new InGameBlockData[inGameLevel.nrOfBlocks];
    // add walls & paddle:
    inGameLevel.blocks[SPECIAL_BLOCK_BOTTOMWALL ] = inGameLevel.bottomWall;
    inGameLevel.blocks[SPECIAL_BLOCK_LEFTWALL   ] = inGameLevel.leftWall;
    inGameLevel.blocks[SPECIAL_BLOCK_RIGHTWALL  ] = inGameLevel.rightWall;
    inGameLevel.blocks[SPECIAL_BLOCK_TOPWALL    ] = inGameLevel.topWall;
    inGameLevel.blocks[SPECIAL_BLOCK_PADDLE     ] = inGameLevel.paddle;
    // add the blocks of the playfield:
    int blockCnt = SPECIAL_BLOCKS; // start at 5
    for ( int iBlock = 0; iBlock < MAX_BLOCKS_PER_LEVEL; iBlock++ )
    {
        if ( currentBlockData[iBlock].xOrig >= 0 )
        {
            Block& srcBlock = levelData.blocks[currentBlockData[iBlock].blockNr];
            InGameBlockData& dstBlock = inGameLevel.blocks[blockCnt];
            dstBlock.hitsToDestruct = srcBlock.hitsToDestruct;
            dstBlock.isAlive = true;
            dstBlock.powerUp = currentBlockData[iBlock].powerUp;
            dstBlock.xOrig   = currentBlockData[iBlock].xOrig;
            dstBlock.yOrig   = currentBlockData[iBlock].yOrig;
            dstBlock.x2      = dstBlock.xOrig + srcBlock.width - 1;
            dstBlock.y2      = dstBlock.yOrig + srcBlock.height - 1;
            dstBlock.skin    = &srcBlock.skin;
            blockCnt++;
        }
    }
}
/*
    Creates a filename string based on a given name, number and extension. The 
    range of the number is from 0 to 99. Numbers smaller than 10 will start 
    with a zero, so background bitmap number three will be called:
    "background03.bmp" rather than "background3.bmp".
*/
void createFilename( char *dest,const char *filenameBase, const int nr, const char *filenameExt )
{
    assert( nr >= 0 );
    assert( nr < 100 );
    assert((strlen(filenameBase) + 2 + strlen(filenameExt)) < MAX_SOURCE_FILENAME_LENGTH);
    *dest = '\0';
    strcat(dest, filenameBase);
    char buf[3];
    _itoa(nr, buf, 10);
    if (nr < 10) strcat( dest, "0");
    strcat(dest, buf);
    strcat(dest, filenameExt);
}


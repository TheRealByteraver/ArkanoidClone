/*
    The level data file contains the following information:
    - how many levels there are
    - how many different types of blocks there are in total
    For each level the following data is kept:
    - the number of the background bitmap to use in the level
    - the number of blocks there are on the level
    - a list with the block data. For each item in this list
      we keep the following data:
      - the blocks' location (x,y position of top left corner)
      - the powerup that is freed upon destruction of the block
      - whick type of block it is (an index in the block table)
      - the grid settings of the level editor for this level
    For each block we keep the following data:
    - the height & width of the block
    - the color of the block (only used if no bitmap is available)
      this 32 bit value also contains the transparancy level of the 
      block (0xFF for opaque, 0 for fully transparent, transparency
      is not currently supported however)
      (transparancy is not currently implemented)
    - the number of the skin file (the bitmap containing the graphics)
    - the (x,y) coordinates of the skin in the bitmap file
    - how many times it should be hit before the block is destroyed.
      The value of 5 will be used for indestructible blocks.

    First we store a basic header, then the blocks, and then the levels.
    We choose this layout so we can reuse blocks for more than one level.
    Maxmimum number of blocks: 1000 (0 .. 999)
    Maxmimum number of levels: 100 (0 .. 99)


    Level Design file format layout:

    Offset: Size(in bytes): Description:
    ------- --------------- -------------------------------------------

    0       19              "Arkanoid Level Data"   ID string
    19      1               ASCIIZ character (0)
    20      4               EOF character 26
    24      4               number of blocks
    28      4               number of levels
    32      4               block data offset (should be 464, not used)
    36      4               level data 0 offset (not used nor set)
    40      4               level data 1 offset (not used nor set)

    ...                     
    ...
    ...

    432     4               level data 99 offset (not used nor set)

    436     24              reserved

    464     block data
    ??      level data


    Block header layout:

    Offset: Size(in bytes): Description:
    ------- --------------- -------------------------------------------
    0       4               block width
    4       4               block height
    8       4               block color
    12      127             bitmap file name & path, max. 127 chars long
    139     1               ASCIIZ character (0)
    140     4               x location of skin in bitmap file
    144     4               y location of skin in bitmap file
    148     4               number of hits the block takes before destruction


    Level header layout:

    Offset: Size(in bytes): Description:
    ------- --------------- -------------------------------------------
    0       4               background file number
    4       4               background scroll type
    8       4               nr of blocks in this level
    12      4               cell width of the grid (level designer)
    16      4               cell height of the grid (level designer)
    20      4               cell spacer (level designer)
    
    24      4               1st block block number (which block from the 
                            block database should be put here)
    28      4               1st block x coord (top left corner)
    32      4               1st block y coord (top left corner)
    36      4               1st block powerup id (0 for no powerup)

    ...     4               nth block block number
    ...     4               nth block x coord (top left corner)
    ...     4               nth block y coord (top left corner)
    ...     4               nth block powerup id (0 for no powerup)

    ...     4               last block block number
    ...     4               last block x coord (top left corner)
    ...     4               last block y coord (top left corner)
    ...     4               last block powerup id (0 for no powerup)

    ...     ...             data of next level


    ***************************************************************************

    Level data in memory:

    For each level we need the following info in the game:
    - the title
    - the background image
    - a list with the blocks in the level
    - each block is:
        - hitstodestruct
        - x coord
        - y coord
        - x2 coord
        - y2 coord
        - powerup type
        - reference to bitmap with the skin
        - bool alive (should it be drawn / checked for collision?)

    Strategy: we load all the blocks designs in memory at once during 
    initialization. They do not take that much memory.  
*/
#pragma once

#include "Arkanoid.h"
#include "D3DGraphics.h"
#include "Sound.h"

#define BLOCK_INDESTRUCTIBLE                (-1)
#define MAX_BLOCK_TYPES                     1000
#define MAX_BLOCK_TYPES_STR                 "1000"
#define UNUSED_SLOT                         (-1)
#define MAX_NR_OF_LEVELS                    100
#define LEVEL_TITLE_MAX_LENGTH              23  // excluding zero term. char
#define LEVEL_TITLE_FILENAME                "leveltitles.txt"
#define LEVELDATA_ID_STRING                 "Arkanoid Level Data"
#define MAX_FILENAME_LENGTH                 127
#define LEVELDATA_FILENAME                  "leveldata.dat"
#define END_OF_FILE_CHAR                    26
#define BLOCK_DATA_OFFSET                   464
#define MAX_LEVEL_TITLE_LENGTH              23 
#define INFINITE_SYMBOL                     236
/*
    These constants are used whenever the game creates a default level data 
    file:
*/
#define COLLISION_CELL_WIDTH                (HOR_SCREEN_RES / 16) 
#define COLLISION_CELL_HEIGHT               (VER_SCREEN_RES / 32) 
#define COLLISION_GRID_WIDTH                (HOR_SCREEN_RES / COLLISION_CELL_WIDTH) 
#define COLLISION_GRID_HEIGHT               (VER_SCREEN_RES / COLLISION_CELL_HEIGHT)
#define SPACER                              2 // 0 means block touch each other
#define BLOCK_WIDTH                         (COLLISION_CELL_WIDTH  - SPACER * 2)
#define BLOCK_HEIGHT                        (COLLISION_CELL_HEIGHT - SPACER * 2)
#define DEFAULT_NR_OF_BLOCK_COLUMNS         13
#define DEFAULT_NR_OF_BLOCK_ROWS            11
#define DEFAULT_NR_OF_BLOCKS (DEFAULT_NR_OF_BLOCK_COLUMNS * DEFAULT_NR_OF_BLOCK_ROWS)
#define LEVEL_DESIGN_GRID_INTERVAL          6
#define LEVEL_DESIGN_GRID_COLOR             0x303030
#define DEFAULT_BLOCK_SKIN_FILENAME         "blockSkinLibrary00.bmp"
#define BACKGROUND_FILENAME_BASE            "background"
#define BLOCK_SKIN_FILENAME_BASE            "blockSkinLibrary"
#define GFX_FILENAME_EXTENSION              ".bmp"
#define DEFAULT_LEVEL_NAME                  "Into Space"
#define MAX_SOURCE_FILENAME_LENGTH          128
#define MAX_BLOCK_SKIN_FILES                100 // range from 00 .. 99
#define SOURCE_FILES_PATH                   "c:\\media\\"
//#define SOURCE_FILES_PATH                   ".\\"
/*
    These constants are needed to draw the level designer control panel:
*/
#define PANEL_X1                0
#define PANEL_X2                (HOR_SCREEN_RES - 1)
#define PANEL_Y1                (VER_SCREEN_RES - (VER_SCREEN_RES / 4) + 22)
#define PANEL_Y2                (VER_SCREEN_RES - 1)
#define GAME_WINDOW_X1          0
#define GAME_WINDOW_Y1          0
#define GAME_WINDOW_X2          (HOR_SCREEN_RES - 1)
#define GAME_WINDOW_Y2          (PANEL_Y1 - 1)
#define FRAME_F                 DEFAULT_FRAME_WIDTH
#define TEXT_SPACER             (2 + FRAME_F)
#define BUTTON_SPACER           2
#define FONT_HEIGHT             14
#define FONT_WIDTH              8
#define BUTTON_HEIGHT           (FONT_HEIGHT * 2 + FRAME_F)
#define FONT_Y_OFFSET           ((BUTTON_HEIGHT - FONT_HEIGHT) / 2)
// level manipulation section
#define LEVELBOX_X1             (PANEL_X1 + FRAME_F + 591)
#define LEVELBOX_X2             (LEVELBOX_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 8) // strlen("Level 00") = 8
#define LEVELBOX_Y1             (PANEL_Y2 - (FRAME_F + FONT_HEIGHT) * 2)
#define LEVELBOX_Y2             (LEVELBOX_Y1 + BUTTON_HEIGHT)
#define LEVELSELECTOR_X1        (LEVELBOX_X2 + BUTTON_SPACER)
#define LEVELSELECTOR_X2        (LEVELSELECTOR_X1 + FONT_WIDTH + 2 * TEXT_SPACER)
#define LEVELSELECTOR_Y1        (LEVELBOX_Y1 + FRAME_F / 2)
#define LEVELSELECTOR_Y2        (LEVELBOX_Y2 - FRAME_F / 2)
#define LEVELSELECTOR_YSPLIT    (LEVELBOX_Y1 + (LEVELBOX_Y2 - LEVELBOX_Y1) / 2 - 1)
#define LEVELTITLE_X1           (LEVELSELECTOR_X2 + BUTTON_SPACER) 
#define LEVELTITLE_X2           (LEVELTITLE_X1 + FONT_WIDTH * MAX_LEVEL_TITLE_LENGTH + 2 * TEXT_SPACER)
#define LEVELBACKGROUND_X1      (LEVELTITLE_X2 + 1)
#define LEVELBACKGROUND_X2      (LEVELBACKGROUND_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 13) // strlen("Background 00") = 13
#define BACKGROUNDSELECTOR_X1   (LEVELBACKGROUND_X2 + BUTTON_SPACER)
#define BACKGROUNDSELECTOR_X2   (BACKGROUNDSELECTOR_X1 + FONT_WIDTH + 2 * TEXT_SPACER)
// block section
#define BLOCKNR_X1              (PANEL_X1 + FRAME_F)
#define BLOCKNR_X2              (BLOCKNR_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 9) // strlen("Block 000") = 9
#define BLOCKNR_Y1              (PANEL_Y1 + FRAME_F)
#define BLOCKNR_Y2              (BLOCKNR_Y1 + BUTTON_HEIGHT)
#define BLOCKNRSELECTOR_X1      (BLOCKNR_X2 + BUTTON_SPACER)
#define BLOCKNRSELECTOR_X2      (BLOCKNRSELECTOR_X1 + FONT_WIDTH + 2 * TEXT_SPACER)
#define BLOCKNRSELECTOR_Y1      (BLOCKNR_Y1 + FRAME_F / 2)
#define BLOCKNRSELECTOR_Y2      (BLOCKNR_Y2 - FRAME_F / 2)
#define BLOCKNRSELECTORFAST_X1  (BLOCKNRSELECTOR_X2 + 1 + BUTTON_SPACER)
#define BLOCKNRSELECTORFAST_X2  (BLOCKNRSELECTORFAST_X1 + (FONT_WIDTH + TEXT_SPACER) * 2)
#define COLORSKINSWITCH_Y1      (BLOCKNR_Y2 + BUTTON_SPACER)
#define COLORSKINSWITCH_Y2      (COLORSKINSWITCH_Y1 + BUTTON_HEIGHT)
#define HITSTODESTRUCTSWITCH_Y1 (COLORSKINSWITCH_Y2 + BUTTON_SPACER + 1)
#define HITSTODESTRUCTSWITCH_Y2 (HITSTODESTRUCTSWITCH_Y1 + BUTTON_HEIGHT)
#define BLOCKPREVIEW_X1         (BLOCKNRSELECTORFAST_X2 + BUTTON_SPACER)
#define BLOCKPREVIEW_X2         (BLOCKPREVIEW_X1 + 160)
#define BLOCKPREVIEW_Y2         (HITSTODESTRUCTSWITCH_Y2 + 1)
#define COLORBLOCKR_X1          (BLOCKNR_X1)
#define COLORBLOCKR_X2          (COLORBLOCKR_X1  + TEXT_SPACER * 2 + FONT_WIDTH * 6) // strlen ("R: 255") = 6
#define COLORBLOCKR_Y1          (HITSTODESTRUCTSWITCH_Y2 + BUTTON_SPACER)
#define COLORBLOCKR_Y2          (COLORBLOCKR_Y1  + BUTTON_HEIGHT)
#define COLORBLOCKRS_X1         (COLORBLOCKR_X2  + BUTTON_SPACER)
#define COLORBLOCKRS_X2         (COLORBLOCKRS_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 1)
#define COLORBLOCKRS_Y1         (COLORBLOCKR_Y1 + FRAME_F / 2)
#define COLORBLOCKRS_Y2         (COLORBLOCKR_Y2 - FRAME_F / 2)
#define COLORBLOCKG_X1          (COLORBLOCKRS_X2 + BUTTON_SPACER)
#define COLORBLOCKG_X2          (COLORBLOCKG_X1  + TEXT_SPACER * 2 + FONT_WIDTH * 6)
#define COLORBLOCKGS_X1         (COLORBLOCKG_X2  + BUTTON_SPACER)
#define COLORBLOCKGS_X2         (COLORBLOCKGS_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 1)
#define COLORBLOCKB_X1          (COLORBLOCKGS_X2 + BUTTON_SPACER)
#define COLORBLOCKB_X2          (COLORBLOCKB_X1  + TEXT_SPACER * 2 + FONT_WIDTH * 6)
#define COLORBLOCKBS_X1         (COLORBLOCKB_X2  + BUTTON_SPACER)
#define COLORBLOCKBS_X2         (COLORBLOCKBS_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 1)
#define COLORSAMPLE_X1          (COLORBLOCKBS_X2 + BUTTON_SPACER)
#define COLORSAMPLE_X2          (BLOCKPREVIEW_X2)
#define SKINLIBRARY_X1          (BLOCKNR_X1)
#define SKINLIBRARY_X2          (SKINLIBRARY_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 15) // strlen("Skin library   ") = 12
#define SKINLIBRARY_Y1          (COLORBLOCKR_Y2 + 1)
#define SKINLIBRARY_Y2          (SKINLIBRARY_Y1 + BUTTON_HEIGHT)
#define SKINLIBRARYSELECTOR_X1  (SKINLIBRARY_X2 + BUTTON_SPACER)
#define SKINLIBRARYSELECTOR_X2  (SKINLIBRARYSELECTOR_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 1)
#define SKINLIBRARYSELECTOR_Y1  (SKINLIBRARY_Y1 + FRAME_F / 2)
#define SKINLIBRARYSELECTOR_Y2  (SKINLIBRARY_Y2 - FRAME_F / 2)
#define SKINPREVIEWWINDOW_X1    (BLOCKPREVIEW_X2 + BUTTON_SPACER - 1)
#define SKINPREVIEWWINDOW_X2    (LEVELBOX_X1 - BUTTON_SPACER + 1)
#define SKINPREVIEWWINDOW_Y1    BLOCKNR_Y1
#define SKINPREVIEWWINDOW_Y2    LEVELBOX_Y2
#define SKINPREVIEWWINDOWCL_X1  (SKINPREVIEWWINDOW_X1 + DEFAULT_FRAME_WIDTH)  
#define SKINPREVIEWWINDOWCL_X2  (SKINPREVIEWWINDOW_X2 - DEFAULT_FRAME_WIDTH)  
#define SKINPREVIEWWINDOWCL_Y1  (SKINPREVIEWWINDOW_Y1 + DEFAULT_FRAME_WIDTH)  
#define SKINPREVIEWWINDOWCL_Y2  (SKINPREVIEWWINDOW_Y2 - DEFAULT_FRAME_WIDTH)  
#define HELPWINDOW_X1           LEVELBOX_X1
#define HELPWINDOW_X2           (PANEL_X2 - FRAME_F)
#define HELPWINDOW_Y1           (PANEL_Y1 + FRAME_F)
#define HELPWINDOW_Y2           (LEVELBOX_Y1 - BUTTON_SPACER + 1)
#define LEVEL_DESIGN_SWITCH_X1  (SKINLIBRARYSELECTOR_X2 + BUTTON_SPACER + 1)
#define LEVEL_DESIGN_SWITCH_X2  (COLORSAMPLE_X2 - 1)
#define LEVEL_DESIGN_SWITCH_Y1  (SKINLIBRARY_Y1 + 1)
#define LEVEL_DESIGN_SWITCH_Y2  (SKINLIBRARY_Y2 - 1)
// coords for level designer:
#define BLOCK_DESIGN_SWITCH_X2  (LEVELBOX_X1 - BUTTON_SPACER)
#define BLOCK_DESIGN_SWITCH_X1  (BLOCK_DESIGN_SWITCH_X2 - 138)
#define BLOCK_DESIGN_SWITCH_Y1  (SKINLIBRARY_Y1 + 1)
#define BLOCK_DESIGN_SWITCH_Y2  (SKINLIBRARY_Y2 - 1)
#define BLOCK_PICKER_X1         BLOCKNR_X1
#define BLOCK_PICKER_X2         (PANEL_X2 - FRAME_F)
#define BLOCK_PICKER_Y1         (PANEL_Y1 + FRAME_F)
#define BLOCK_PICKER_Y2         (LEVELBOX_Y1 - BUTTON_SPACER + 1)
#define BLOCK_PICKERCL_X1       (BLOCK_PICKER_X1 + DEFAULT_FRAME_WIDTH)  
#define BLOCK_PICKERCL_X2       (BLOCK_PICKER_X2 - DEFAULT_FRAME_WIDTH)  
#define BLOCK_PICKERCL_Y1       (BLOCK_PICKER_Y1 + DEFAULT_FRAME_WIDTH)  
#define BLOCK_PICKERCL_Y2       (BLOCK_PICKER_Y2 - DEFAULT_FRAME_WIDTH) 
#define GRID_ON_OFF_X1          (PANEL_X1 + FRAME_F)
#define GRID_ON_OFF_X2          (GRID_ON_OFF_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 9) // strlen("Frame Off") = 9
#define GRID_ON_OFF_Y1          BLOCK_DESIGN_SWITCH_Y1
#define GRID_ON_OFF_Y2          BLOCK_DESIGN_SWITCH_Y2
#define GRID_WIDTH_X1           (GRID_ON_OFF_X2 + BUTTON_SPACER)
#define GRID_WIDTH_X2           (GRID_WIDTH_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 6) // strlen("W: 100") = 6
#define GRID_WIDTH_Y1           (GRID_ON_OFF_Y1 - 1)
#define GRID_WIDTH_Y2           (GRID_ON_OFF_Y2 + 1)
#define GRID_WIDTH_SELECTOR_X1  (GRID_WIDTH_X2 + BUTTON_SPACER)
#define GRID_WIDTH_SELECTOR_X2  (GRID_WIDTH_SELECTOR_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 1)
#define GRID_WIDTH_SELECTOR_Y1  GRID_ON_OFF_Y1  
#define GRID_WIDTH_SELECTOR_Y2  GRID_ON_OFF_Y2
#define GRID_HEIGHT_X1          (GRID_WIDTH_SELECTOR_X2 + BUTTON_SPACER)
#define GRID_HEIGHT_X2          (GRID_HEIGHT_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 6) // strlen("H: 100") = 6
#define GRID_HEIGHT_Y1          (GRID_ON_OFF_Y1 - 1)
#define GRID_HEIGHT_Y2          (GRID_ON_OFF_Y2 + 1)
#define GRID_HEIGHT_SELECTOR_X1 (GRID_HEIGHT_X2 + BUTTON_SPACER)
#define GRID_HEIGHT_SELECTOR_X2 (GRID_HEIGHT_SELECTOR_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 1)
#define GRID_HEIGHT_SELECTOR_Y1 GRID_ON_OFF_Y1
#define GRID_HEIGHT_SELECTOR_Y2 GRID_ON_OFF_Y2
#define GRID_SPACER_X1          (GRID_HEIGHT_SELECTOR_X2 + BUTTON_SPACER)
#define GRID_SPACER_X2          (GRID_SPACER_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 4) // strlen("S: 1") = 4
#define GRID_SPACER_Y1          (GRID_ON_OFF_Y1 - 1)
#define GRID_SPACER_Y2          (GRID_ON_OFF_Y2 + 1)
#define GRID_SPACER_SELECTOR_X1 (GRID_SPACER_X2 + BUTTON_SPACER)
#define GRID_SPACER_SELECTOR_X2 (GRID_SPACER_SELECTOR_X1 + TEXT_SPACER * 2 + FONT_WIDTH * 1)
#define GRID_SPACER_SELECTOR_Y1 GRID_ON_OFF_Y1
#define GRID_SPACER_SELECTOR_Y2 GRID_ON_OFF_Y2

#define SAVE_AND_EXIT_BUTTON_X1 (GRID_SPACER_SELECTOR_X2 + BUTTON_SPACER + 1)
#define SAVE_AND_EXIT_BUTTON_X2 (BLOCK_DESIGN_SWITCH_X1 - BUTTON_SPACER - 1)
#define SAVE_AND_EXIT_BUTTON_Y1 BLOCK_DESIGN_SWITCH_Y1
#define SAVE_AND_EXIT_BUTTON_Y2 BLOCK_DESIGN_SWITCH_Y2  

// block designer part:
const Rect gameWindowRect           = { GAME_WINDOW_X1,GAME_WINDOW_Y1,GAME_WINDOW_X2,GAME_WINDOW_Y2 };
const Rect panelRect                = { PANEL_X1,PANEL_Y1,PANEL_X2,PANEL_Y2 };
const Rect levelNrRect              = { LEVELBOX_X1,LEVELBOX_Y1,LEVELBOX_X2,LEVELBOX_Y2 };
const Rect levelSelectorRect        = { LEVELSELECTOR_X1,LEVELSELECTOR_Y1,LEVELSELECTOR_X2,LEVELSELECTOR_Y2 };
const Rect levelTitleRect           = { LEVELTITLE_X1,LEVELBOX_Y1,LEVELTITLE_X2,LEVELBOX_Y2 };
const Rect backgroundNrRect         = { LEVELBACKGROUND_X1,LEVELBOX_Y1,LEVELBACKGROUND_X2,LEVELBOX_Y2 };
const Rect backgroundSelectorRect   = { BACKGROUNDSELECTOR_X1,LEVELSELECTOR_Y1,BACKGROUNDSELECTOR_X2,LEVELSELECTOR_Y2 };
const Rect blockNrRect              = { BLOCKNR_X1,BLOCKNR_Y1,BLOCKNR_X2,BLOCKNR_Y2 };
const Rect blockNrSelectorRect      = { BLOCKNRSELECTOR_X1,BLOCKNRSELECTOR_Y1,BLOCKNRSELECTOR_X2,BLOCKNRSELECTOR_Y2 };
const Rect blockNrFastSelectorRect  = { BLOCKNRSELECTORFAST_X1,BLOCKNRSELECTOR_Y1,BLOCKNRSELECTORFAST_X2,BLOCKNRSELECTOR_Y2 };
const Rect blockSkinOrColorRect     = { BLOCKNR_X1 + 1,COLORSKINSWITCH_Y1,BLOCKNRSELECTORFAST_X2,COLORSKINSWITCH_Y2 };
const Rect hitsToDestructRect       = { BLOCKNR_X1 + 1,HITSTODESTRUCTSWITCH_Y1,BLOCKNRSELECTORFAST_X2,HITSTODESTRUCTSWITCH_Y2 };
const Rect blockPreviewRect         = { BLOCKPREVIEW_X1,BLOCKNR_Y1,BLOCKPREVIEW_X2,BLOCKPREVIEW_Y2 };
const Rect colorRRect               = { COLORBLOCKR_X1,COLORBLOCKR_Y1,COLORBLOCKR_X2,COLORBLOCKR_Y2 };
const Rect colorRSelectorRect       = { COLORBLOCKRS_X1,COLORBLOCKRS_Y1,COLORBLOCKRS_X2,COLORBLOCKRS_Y2 };
const Rect colorGRect               = { COLORBLOCKG_X1,COLORBLOCKR_Y1,COLORBLOCKG_X2,COLORBLOCKR_Y2 };
const Rect colorGSelectorRect       = { COLORBLOCKGS_X1,COLORBLOCKRS_Y1,COLORBLOCKGS_X2,COLORBLOCKRS_Y2 };
const Rect colorBRect               = { COLORBLOCKB_X1,COLORBLOCKR_Y1,COLORBLOCKB_X2,COLORBLOCKR_Y2 };
const Rect colorBSelectorRect       = { COLORBLOCKBS_X1,COLORBLOCKRS_Y1,COLORBLOCKBS_X2,COLORBLOCKRS_Y2 };
const Rect colorSampleRect          = { COLORSAMPLE_X1,COLORBLOCKR_Y1,COLORSAMPLE_X2,COLORBLOCKR_Y2 };
const Rect skinLibraryRect          = { SKINLIBRARY_X1,SKINLIBRARY_Y1,SKINLIBRARY_X2,SKINLIBRARY_Y2 };
const Rect skinLibrarySelectorRect  = { SKINLIBRARYSELECTOR_X1,SKINLIBRARYSELECTOR_Y1,SKINLIBRARYSELECTOR_X2,SKINLIBRARYSELECTOR_Y2 };
const Rect skinPreviewRect          = { SKINPREVIEWWINDOW_X1,SKINPREVIEWWINDOW_Y1,SKINPREVIEWWINDOW_X2,SKINPREVIEWWINDOW_Y2 };
const Rect skinPreviewClientRect    = { SKINPREVIEWWINDOWCL_X1,SKINPREVIEWWINDOWCL_Y1,SKINPREVIEWWINDOWCL_X2,SKINPREVIEWWINDOWCL_Y2 };
const Rect helpWindowRect           = { HELPWINDOW_X1,HELPWINDOW_Y1,HELPWINDOW_X2,HELPWINDOW_Y2 };
const Rect toLevelEditorSwitchRect  = { LEVEL_DESIGN_SWITCH_X1,LEVEL_DESIGN_SWITCH_Y1,LEVEL_DESIGN_SWITCH_X2,LEVEL_DESIGN_SWITCH_Y2 };
// Level designer part:
const Rect blockPickerRect          = { BLOCK_PICKER_X1,BLOCK_PICKER_Y1,BLOCK_PICKER_X2,BLOCK_PICKER_Y2 };
const Rect blockPickerClientRect    = { BLOCK_PICKERCL_X1,BLOCK_PICKERCL_Y1,BLOCK_PICKERCL_X2,BLOCK_PICKERCL_Y2 };
const Rect toBlockEditorSwitchRect  = { BLOCK_DESIGN_SWITCH_X1,BLOCK_DESIGN_SWITCH_Y1,BLOCK_DESIGN_SWITCH_X2,BLOCK_DESIGN_SWITCH_Y2 };
const Rect gridOnOffSwitchRect      = { GRID_ON_OFF_X1,GRID_ON_OFF_Y1,GRID_ON_OFF_X2,GRID_ON_OFF_Y2 };
const Rect gridWidthRect            = { GRID_WIDTH_X1,GRID_WIDTH_Y1,GRID_WIDTH_X2,GRID_WIDTH_Y2 };
const Rect gridWidthSelectorRect    = { GRID_WIDTH_SELECTOR_X1,GRID_WIDTH_SELECTOR_Y1,GRID_WIDTH_SELECTOR_X2,GRID_WIDTH_SELECTOR_Y2 };
const Rect gridHeightRect           = { GRID_HEIGHT_X1,GRID_HEIGHT_Y1,GRID_HEIGHT_X2,GRID_HEIGHT_Y2 };
const Rect gridHeightSelectorRect   = { GRID_HEIGHT_SELECTOR_X1,GRID_HEIGHT_SELECTOR_Y1,GRID_HEIGHT_SELECTOR_X2,GRID_HEIGHT_SELECTOR_Y2 };
const Rect gridSpacerRect           = { GRID_SPACER_X1,GRID_SPACER_Y1,GRID_SPACER_X2,GRID_SPACER_Y2 };
const Rect gridSpacerSelectorRect   = { GRID_SPACER_SELECTOR_X1,GRID_SPACER_SELECTOR_Y1,GRID_SPACER_SELECTOR_X2,GRID_SPACER_SELECTOR_Y2 };
const Rect saveAndExitButtonRect    = { SAVE_AND_EXIT_BUTTON_X1,SAVE_AND_EXIT_BUTTON_Y1,SAVE_AND_EXIT_BUTTON_X2,SAVE_AND_EXIT_BUTTON_Y2 };

const char *gameWindowText[];
const char *levelSelectorText[];
const char *levelTitleText[];
const char *backgroundNrText[];
const char *backgroundSelectorText[];
const char *blockNrSelectorText[];
const char *blockSkinOrColorText[];
const char *hitsToDestructText[];
const char *blockPreviewText[];
const char *skinLibrarySelectorText[];
const char *skinPreviewClientText[];
const char *colorRGBText[];
const char *toLevelEditorSwitchText[];

enum rgbcomponent { rcomponent, gcomponent, bcomponent };

/*
    These structs match the file structure exactly for easy file reading:
*/
#pragma pack(push)
#pragma pack(1)
struct LevelDataHeader 
{
    char    idString[19];
    char    asciiz;
    int     eofChar;
    int     nrOfBlocks; // how many different block types there are in the game
    int     nrOfLevels;
    int     blockDataOffset;
    int     levelDataOffset[MAX_NR_OF_LEVELS];
    int     maxStartLevel;
    int     reserved[5];
};
struct BlockHeader      // describes a unique block type
{
    int     width;
    int     height;
    int     color;
    int     skinFileNr;
    bool    isSkinUsed;
    int     skinXLocation;
    int     skinYLocation;
    int     hitsToDestruct;
};
struct LevelHeader      // describes one level
{
    int     backgroundNr;
    backgroundscrolltype
            backGroundScrolltype;
    int     nrOfBlocks;
    int     gridCellWidth;
    int     gridCellHeight;
    int     gridSpacer;
};
struct BlockData        // used to keep track of all the blocks in one level
{
    int     blockNr;
    int     xOrig;
    int     yOrig;
    powerup powerUp;
};
#pragma pack(pop)
// some little unsafe tool ;)
void createFilename(char *dest, const char *filenameBase, const int nr, const char *filenameExt);
/*
    The classes below describe how we keep the level data in memory:
*/
class Block // describes a unique block type, as stored in memory
{           
public:
    int     width;
    int     height;
    int     color;
    int     skinFileNr;
    int     hitsToDestruct;
    bool    isSkinPresent;
    bool    isSkinUsed;
    int     skinXLocation;
    int     skinYLocation;
    Bitmap  skin;
    Block()
    {
        width = 0;
        height = 0;
        color = 0xFF000000;
        skinFileNr = 0;
        hitsToDestruct = 1;
        isSkinPresent = false;
        isSkinUsed = false;
        skinXLocation = 0;
        skinYLocation = 0;
    }
    ~Block() { }
};

class Level
{
public:
    char        *title;
    int         backgroundNr;
    backgroundscrolltype
                backGroundScrolltype;
    int         nrOfBlocks;
    int         gridCellWidth;
    int         gridCellHeight;
    int         gridSpacer;
    BlockData   *blocks;  // list of locations of the blocks & their type
    Level()
    {
        backgroundNr = 0;
        nrOfBlocks = 0;
        gridCellWidth = COLLISION_CELL_WIDTH;
        gridCellHeight = COLLISION_CELL_HEIGHT;
        gridSpacer = SPACER;
        title = new char[LEVEL_TITLE_MAX_LENGTH + 1];
        memset( title,'\0',LEVEL_TITLE_MAX_LENGTH + 1 );
        blocks = nullptr;
    }
    ~Level()
    {
        if( title != nullptr ) delete title;
        if( blocks != nullptr ) delete [] blocks;
    }
};

class LevelData
{
public:
    LevelData();
    ~LevelData();
    int         generateLevelData();
    int         loadLevelData();
    int         saveLevelData();
    int         nrOfLevels;
    int         nrOfBlocks;
    int         maxStartLevel;
    Block       *blocks;
    Level       *levels;
};

#define DESIGNER_GRID_MAX_CELL_WIDTH          200
#define DESIGNER_GRID_MAX_CELL_HEIGHT         100
#define DESIGNER_GRID_MAX_CELL_SPACER         9

class Grid
{
public:
    Grid()  
    { 
        Init( COLLISION_CELL_WIDTH,COLLISION_CELL_HEIGHT,SPACER );
        isGridVisible = true; 
    }
    void    Init( int cellWidth_,int cellHeight_,int spacer_ )
    {
        if( cellWidth_ <= DESIGNER_GRID_MAX_CELL_WIDTH ) 
            cellWidth = cellWidth_;
        else cellWidth = DESIGNER_GRID_MAX_CELL_WIDTH;
        if ( cellHeight_ <= DESIGNER_GRID_MAX_CELL_HEIGHT ) 
            cellHeight = cellHeight_;
        else cellHeight = DESIGNER_GRID_MAX_CELL_HEIGHT;
        if ( spacer_ <= DESIGNER_GRID_MAX_CELL_SPACER ) spacer = spacer_;
        else spacer = DESIGNER_GRID_MAX_CELL_SPACER;
        if ( cellWidth < 1 ) cellWidth = 1;
        if ( cellHeight < 1 ) cellHeight = 1;
        if ( spacer < 0 ) spacer = 0;
        int gameWindowWidth  = (gameWindowRect.x2 - gameWindowRect.x1 + 1);
        int gameWindowHeight = (gameWindowRect.y2 - gameWindowRect.y1 + 1);
        xOffset     = (gameWindowWidth  % cellWidth ) / 2;
        yOffset     = 0;
        blockWidth  = cellWidth  - 2 * spacer;
        blockHeight = cellHeight - 2 * spacer;
        columns     = gameWindowWidth  / cellWidth;
        rows        = gameWindowHeight / cellHeight;
        maxX        = xOffset + columns * cellWidth  - 1;
        maxY        = yOffset + rows    * cellHeight - 1;
    }
    Rect    getCellClientRect( int x,int y ) const 
    {
        Rect res;
        res.x1 = xOffset + ((x - xOffset) / cellWidth ) * cellWidth  + spacer;
        res.y1 = yOffset + ((y - yOffset) / cellHeight) * cellHeight + spacer;
        res.x2 = res.x1 + blockWidth - 1;
        res.y2 = res.y1 + blockHeight - 1;
        return res;
    }
    int     getSpacer()      const { return spacer;                    }
    int     getBlockWidth()  const { return blockWidth;                }
    int     getBlockHeight() const { return blockHeight;               }
    int     getXOffset()     const { return xOffset;                   }
    int     getYOffset()     const { return yOffset;                   }
    int     getColumns()     const { return columns;                   }
    int     getRows()        const { return rows;                      }
    int     getCellWidth()   const { return cellWidth;                 }
    int     getCellHeight()  const { return cellHeight;                }
    int     getMaxX()        const { return maxX;                      }
    int     getMaxY()        const { return maxY;                      }
    bool    isVisible()      const { return isGridVisible;             }
    void    show()                 { isGridVisible = true;             }
    void    hide()                 { isGridVisible = false;            }
    void    switchVisible ()       { isGridVisible = ! isGridVisible;  }
private:
    int     maxX;
    int     maxY;
    int     xOffset;
    int     yOffset;
    int     spacer;
    int     blockWidth;
    int     blockHeight;
    int     columns;
    int     rows;
    int     cellWidth;
    int     cellHeight;
    bool    isGridVisible;
};

class LevelDesigner
{
public:
    LevelDesigner();
    ~LevelDesigner();
    void        setFont ( Font *f ) { font = f; } // must be bitwise font
    int         loadSounds( DSound& audio );
    int         getBlockAtPosition( int x,int y );
    void        putBlockAtPosition( int x,int y );
    void        removeBlockAtPosition( int x,int y );
    void        removeBlock( int blockNr );
    int         getCurrentLevelNr() { return currentLevelNr; }
    int         getCurrentBlockNr() { return currentBlockNr; }
    powerup     getCurrentPowerUp() { return currentPowerUp; }
    int         getCurrentBackgroundNr() 
        { return levelData.levels[currentLevelNr].backgroundNr; }
    int         getBackGroundX()    { return backGroundX;   }
    int         getBackGroundY()    { return backGroundY;   }
    void        incBackgroundNr();
    void        decBackgroundNr();
    void        switchBackgroundScrollType();
    backgroundscrolltype getBackgroundScrollType() 
        { return levelData.levels[currentLevelNr].backGroundScrolltype; }
    void        setLevelNr( int levelNr );
    void        incLevelNr();
    void        decLevelNr();
    int         getNrOfLevels() { return levelData.nrOfLevels; }
    void        incBlockSkinLibNr();
    void        decBlockSkinLibNr();
    void        changeRGBColor( rgbcomponent c,int relChange );
    void        changeBlockNr( int relChange );
    int         getCurrentBlockSkinLibNr() { return currentBlockSkinLibNr; }
    int         getBlockSkinLibXOrig() { return blockSkinLibXOrig; }
    int         getBlockSkinLibYOrig() { return blockSkinLibYOrig; }
    void        shiftBlockSkinLibXOrig( int x ) 
    { 
        blockSkinLibXOrig += x;
        if ( blockSkinLibXOrig < 0 ) blockSkinLibXOrig = 0;
        int xMax = blockSkinLib.getWidth() - 
            (skinPreviewClientRect.x2 - skinPreviewClientRect.x1) - 1;
        if ( blockSkinLibXOrig > xMax ) blockSkinLibXOrig = xMax;
        if ( blockSkinLibXOrig < 0 ) blockSkinLibXOrig = 0;
    }
    void        shiftBlockSkinLibYOrig( int y ) 
    { 
        blockSkinLibYOrig += y;
        if ( blockSkinLibYOrig < 0 ) blockSkinLibYOrig = 0;
        int yMax = blockSkinLib.getHeight() - 
            (skinPreviewClientRect.y2 - skinPreviewClientRect.y1) - 1;
        if ( blockSkinLibYOrig > yMax ) blockSkinLibYOrig = yMax;
        if ( blockSkinLibYOrig < 0 ) blockSkinLibYOrig = 0;
    }
    Bitmap      &getBlockSkin () { return  blockSkinLib; }
    int         getBlockLibraryXOrig () { return blockLibraryXOrig; }
    int         getBlockLibraryYOrig () { return blockLibraryYOrig; }
    void        shiftBlockLibraryXOrig( int x )
    {
        blockLibraryXOrig += x;
        if ( blockLibraryXOrig < 0 ) blockLibraryXOrig = 0;
        int xMax = blockLibrary.getWidth() -
            (blockPickerClientRect.x2 - blockPickerClientRect.x1) - 1;
        if ( blockLibraryXOrig > xMax ) blockLibraryXOrig = xMax;
        if ( blockLibraryXOrig < 0 ) blockLibraryXOrig = 0;
    }
    void        shiftBlockLibraryYOrig ( int y )
    {
        blockLibraryYOrig += y;
        if ( blockLibraryYOrig < 0 ) blockLibraryYOrig = 0;
        int yMax = blockLibrary.getHeight() -
            (blockPickerClientRect.y2 - blockPickerClientRect.y1) - 1;
        if ( blockLibraryYOrig > yMax ) blockLibraryYOrig = yMax;
        if ( blockLibraryYOrig < 0 ) blockLibraryYOrig = 0;
    }
    Bitmap      &getBlockLibrary () { return blockLibrary; }
    bool        isBlockSkinned()
        { return levelData.blocks[currentBlockNr].isSkinUsed; }
    void        switchBlockColorOrSkinned();
    void        switchLevelDesignerMode()
        { isInLevelDesignerMode = ! isInLevelDesignerMode; }
    bool        levelDesignerModeActive()
        { return isInLevelDesignerMode; }
    char        *getLevelTitle( int levelNr )
    { 
        assert( levelNr >= 0 );
        assert( levelNr < MAX_NR_OF_LEVELS );
        return  levelData.levels[levelNr].title; 
    }
    char        *getLevelTitle()
        {   return levelData.levels[currentLevelNr].title; }
    Bitmap      &getBackground() { return  currentBackground; }
    Block       &getBlock( int blockNr )
    {
        assert( blockNr >= 0 ); 
        assert( blockNr < MAX_BLOCK_TYPES );
        return levelData.blocks[blockNr];
    }
    BlockData   *getBlockList()
    { 
        return  currentBlockData;
    }
    int         getLevelBlockCount() 
        { return levelData.levels[currentLevelNr].nrOfBlocks; }
    void        incHitsToDestruct();
    int         getHitsToDestruct()  
        { return  levelData.blocks[currentBlockNr].hitsToDestruct; }
    void        selectBlockFromSkinLib( int x,int y );
    void        initBlockLibrary ();
    void        selectBlockFromLibrary( int x,int y );
    int         getBlockLibraryColumns()    { return blockLibraryColumns;     }
    int         getBlockLibraryRows()       { return blockLibraryRows;        }
    int         getBlockLibraryCellWidth()  { return blockLibraryCellWidth;   }
    int         getBlockLibraryCellHeight() { return blockLibraryCellHeight;  }
    int         getBlockLibrarySpacer()     { return blockLibrarySpacer;      }
    void        loadInGameLevel( int levelNr );
    int         getMaxStartLevel() { return levelData.maxStartLevel; }
    void        setMaxStartLevel( int startLevel ) 
                            { levelData.maxStartLevel = startLevel; }
        // for the grid:
    void        gridInit( int cellWidth,int cellHeight,int spacer )
    { 
        grid.Init( cellWidth,cellHeight,spacer );
        levelData.levels[currentLevelNr].gridCellWidth  = grid.getCellWidth();
        levelData.levels[currentLevelNr].gridCellHeight = grid.getCellHeight();
        levelData.levels[currentLevelNr].gridSpacer     = grid.getSpacer();
    }
    int         getGridCellWidth() 
        { return levelData.levels[currentLevelNr].gridCellWidth; }
    int         getGridCellHeight() 
        { return levelData.levels[currentLevelNr].gridCellHeight; }
    int         getGridSpacer() 
        { return levelData.levels[currentLevelNr].gridSpacer; }
    void        gridSwitchVisible() { grid.switchVisible();     }
    Rect        getGridCellClientRect( int x,int y )
        { return grid.getCellClientRect( x,y ); }
    int         getGridXOffset()    { return grid.getXOffset(); }
    int         getGridMaxX()       { return grid.getMaxX();    }
    int         getGridYOffset()    { return grid.getYOffset(); }
    int         getGridMaxY()       { return grid.getMaxY();    }
    int         getGridColumns()    { return grid.getColumns(); }
    int         getGridRows()       { return grid.getRows();    }
    bool        isGridVisible()     { return grid.isVisible();  }
    // public variables:
    Bitmap      blockLibrary;
    InGameLevelData
                inGameLevel;
private:
    // functions
    int         getFreeSlot();
    void        setBlockColor();
    int         setBackground( int backGroundNr );
    void        setBlockNr( int blockNr );
    int         setBlockSkinLibNr( int skinLibNr );         
    void        setBlockCursorLocation( int relX,int relY );
    // variables
    Grid        grid;
    Font        *font;
    bool        isInLevelDesignerMode;
    int         backGroundX;
    int         backGroundY;    
    BlockData   *currentBlockData;
    LevelData   levelData;
    Bitmap      blockSkinLib;
    Bitmap      currentBackground;
    int         currentBlockSkinLibNr;
    int         currentBlockNr;
    int         blockSkinLibXOrig;
    int         blockSkinLibYOrig;
    int         blockLibraryXOrig;
    int         blockLibraryYOrig;
    int         blockLibraryColumns;
    int         blockLibraryRows;
    int         blockLibraryCellWidth;
    int         blockLibraryCellHeight;
    int         blockLibrarySpacer;
    int         currentLevelNr;
    powerup     currentPowerUp;
};
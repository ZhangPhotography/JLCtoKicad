/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 CERN
 * Copyright (C) 2021 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <bitmaps.h>
#include <common.h>
#include <eda_units.h>
#include <frame_type.h>
#include <tool/actions.h>
#include <tool/tool_action.h>
#include <tool/tool_event.h>

// Actions, being statically-defined, require specialized I18N handling.  We continue to
// use the _() macro so that string harvesting by the I18N framework doesn't have to be
// specialized, but we don't translate on initialization and instead do it in the getters.

#undef _
#define _(s) s

TOOL_ACTION ACTIONS::doNew( "common.Control.new",
        AS_GLOBAL,
        MD_CTRL + 'N', LEGACY_HK_NAME( "New" ),
        _( "New..." ), _( "Create a new document in the editor" ),
        BITMAPS::new_generic );

TOOL_ACTION ACTIONS::newLibrary( "common.Control.newLibrary",
        AS_GLOBAL,
        0, "",
        _( "New Library..." ), _( "Create a new library folder" ),
        BITMAPS::new_library );

TOOL_ACTION ACTIONS::addLibrary( "common.Control.addLibrary",
        AS_GLOBAL,
        0, "",
        _( "Add Library..." ), _( "Add an existing library folder" ),
        BITMAPS::add_library );

TOOL_ACTION ACTIONS::open( "common.Control.open",
        AS_GLOBAL,
        MD_CTRL + 'O', LEGACY_HK_NAME( "Open" ),
        _( "Open..." ), _( "Open existing document" ),
        BITMAPS::directory_open );

TOOL_ACTION ACTIONS::addsymbol( "common.Control.addsymbol", 
        AS_GLOBAL,  
        0, "",
        _( "addsymbol..." ), 
        _( "addsymbol existing document" ),
        BITMAPS::directory_open );


TOOL_ACTION ACTIONS::save( "common.Control.save",
        AS_GLOBAL,
        MD_CTRL + 'S', LEGACY_HK_NAME( "Save" ),
        _( "Save" ), _( "Save changes" ),
        BITMAPS::save );

TOOL_ACTION ACTIONS::saveAs( "common.Control.saveAs",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'S', LEGACY_HK_NAME( "Save As" ),
        _( "Save As..." ), _( "Save current document to another location" ),
        BITMAPS::save_as );

TOOL_ACTION ACTIONS::saveCopy( "common.Control.saveCopy",
        AS_GLOBAL,
        0, "",
        _( "Save a Copy..." ), _( "Save a copy of the current document to another location" ),
        BITMAPS::save_as );

TOOL_ACTION ACTIONS::saveAll( "common.Control.saveAll",
        AS_GLOBAL,
        0, "",
        _( "Save All" ), _( "Save all changes" ),
        BITMAPS::save );

TOOL_ACTION ACTIONS::revert( "common.Control.revert",
        AS_GLOBAL,
        0, "",
        _( "Revert" ), _( "Throw away changes" ) );

TOOL_ACTION ACTIONS::pageSettings( "common.Control.pageSettings",
        AS_GLOBAL,
        0, "",
        _( "Page Settings..." ), _( "Settings for paper size and title block info" ),
        BITMAPS::sheetset );

TOOL_ACTION ACTIONS::print( "common.Control.print",
        AS_GLOBAL,
        MD_CTRL + 'P', LEGACY_HK_NAME( "Print" ),
        _( "Print..." ), _( "Print" ),
        BITMAPS::print_button );

TOOL_ACTION ACTIONS::plot( "common.Control.plot",
        AS_GLOBAL,
        0, "",
        _( "Plot..." ), _( "Plot" ),
        BITMAPS::plot );

TOOL_ACTION ACTIONS::quit( "common.Control.quit",
        AS_GLOBAL,
        0, "",   // Not currently in use due to wxWidgets crankiness
        _( "Quit" ), _( "Close the current editor" ),
        BITMAPS::exit );

// Generic Edit Actions
TOOL_ACTION ACTIONS::cancelInteractive( "common.Interactive.cancel",
        AS_GLOBAL,
        0, "",   // ESC key is handled in the dispatcher
        _( "Cancel" ), _( "Cancel current tool" ),
        BITMAPS::cancel, AF_NONE );

TOOL_ACTION ACTIONS::showContextMenu( TOOL_ACTION_ARGS()
        .Name( "common.Control.showContextMenu" )
        .Scope( AS_GLOBAL )
        .MenuText( _( "Show Context Menu" ) )
        .Tooltip( _( "Perform the right-mouse-button action" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_RIGHT_CLICK ) );

TOOL_ACTION ACTIONS::updateMenu( "common.Interactive.updateMenu",
        AS_GLOBAL );

TOOL_ACTION ACTIONS::undo( "common.Interactive.undo",
        AS_GLOBAL,
        MD_CTRL + 'Z', LEGACY_HK_NAME( "Undo" ),
        _( "Undo" ), _( "Undo last edit" ),
        BITMAPS::undo );

TOOL_ACTION ACTIONS::redo( "common.Interactive.redo",
        AS_GLOBAL,
#if defined( __WXMAC__ )
        MD_SHIFT + MD_CTRL + 'Z',
#else
        MD_CTRL + 'Y',
#endif
        LEGACY_HK_NAME( "Redo" ),
        _( "Redo" ), _( "Redo last edit" ),
        BITMAPS::redo );

// The following actions need to have a hard-coded UI ID using a wx-specific ID
// to fix things like search controls in standard file dialogs. If wxWidgets
// doesn't find these specific IDs somewhere in the menus then it won't enable
// cut/copy/paste.
TOOL_ACTION ACTIONS::cut( TOOL_ACTION_ARGS()
        .Name( "common.Interactive.cut" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( MD_CTRL + 'X' )
        .LegacyHotkeyName( "Cut" )
        .MenuText( _( "Cut" ) )
        .Tooltip( _( "Cut selected item(s) to clipboard" ) )
        .Icon( BITMAPS::cut )
        .Flags( AF_NONE )
        .UIId( wxID_CUT ) );

TOOL_ACTION ACTIONS::copy( TOOL_ACTION_ARGS()
        .Name( "common.Interactive.copy" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( MD_CTRL + 'C' )
        .LegacyHotkeyName( "Copy" )
        .MenuText( _( "Copy" ) )
        .Tooltip( _( "Copy selected item(s) to clipboard" ) )
        .Icon( BITMAPS::copy )
        .Flags( AF_NONE )
        .UIId( wxID_COPY ) );

TOOL_ACTION ACTIONS::paste( TOOL_ACTION_ARGS()
        .Name( "common.Interactive.paste" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( MD_CTRL + 'V' )
        .LegacyHotkeyName( "Paste" )
        .MenuText( _( "Paste" ) )
        .Tooltip( _( "Paste item(s) from clipboard" ) )
        .Icon( BITMAPS::paste )
        .Flags( AF_NONE )
        .UIId( wxID_PASTE ) );

TOOL_ACTION ACTIONS::selectAll( "common.Interactive.selectAll",
        AS_GLOBAL,
        MD_CTRL + 'A', "",
        _( "Select All" ), _( "Select all items on screen" ) );

TOOL_ACTION ACTIONS::pasteSpecial( "common.Interactive.pasteSpecial",
        AS_GLOBAL, 0, "",
        _( "Paste Special..." ), _( "Paste item(s) from clipboard with annotation options" ),
        BITMAPS::paste_special );

TOOL_ACTION ACTIONS::duplicate( "common.Interactive.duplicate",
        AS_GLOBAL,
        MD_CTRL + 'D', LEGACY_HK_NAME( "Duplicate" ),
        _( "Duplicate" ), _( "Duplicates the selected item(s)" ),
        BITMAPS::duplicate );

TOOL_ACTION ACTIONS::doDelete( TOOL_ACTION_ARGS()
        .Name( "common.Interactive.delete" )
        .Scope( AS_GLOBAL )
#if defined( __WXMAC__ )
        .DefaultHotkey( WXK_BACK )
#else
        .DefaultHotkey( WXK_DELETE )
#endif
        .LegacyHotkeyName( "Delete Item" )
        .MenuText( _( "Delete" ) )
        .Tooltip( _( "Deletes selected item(s)" ) )
        .Icon( BITMAPS::trash )
        .Parameter( ACTIONS::REMOVE_FLAGS::NORMAL ) );

TOOL_ACTION ACTIONS::deleteTool( "common.Interactive.deleteTool",
        AS_GLOBAL, 0, "",
        _( "Interactive Delete Tool" ), _( "Delete clicked items" ),
        BITMAPS::delete_cursor, AF_ACTIVATE );

TOOL_ACTION ACTIONS::activatePointEditor( "common.Control.activatePointEditor",
        AS_GLOBAL );

TOOL_ACTION ACTIONS::cycleArcEditMode( "common.Interactive.cycleArcEditMode", AS_GLOBAL,
        MD_CTRL + ' ', "", _( "Cycle arc editing mode" ),
        _( "Switch to a different method of editing arcs" ) );

TOOL_ACTION ACTIONS::showSearch( "common.Interactive.search",
        AS_GLOBAL,
        MD_CTRL + 'G', LEGACY_HK_NAME( "Search" ),
        _( "Show Search Panel" ), _( "Show/hide the search panel" ),
        BITMAPS::find );

TOOL_ACTION ACTIONS::find( "common.Interactive.find",
        AS_GLOBAL,
        MD_CTRL + 'F', LEGACY_HK_NAME( "Find" ),
        _( "Find" ), _( "Find text" ),
        BITMAPS::find );

TOOL_ACTION ACTIONS::findAndReplace( "common.Interactive.findAndReplace",
        AS_GLOBAL,
        MD_CTRL + MD_ALT + 'F', LEGACY_HK_NAME( "Find and Replace" ),
        _( "Find and Replace" ), _( "Find and replace text" ),
        BITMAPS::find_replace );

TOOL_ACTION ACTIONS::findNext( "common.Interactive.findNext",
        AS_GLOBAL,
        WXK_F3, LEGACY_HK_NAME( "Find Next" ),
        _( "Find Next" ), _( "Find next match" ),
        BITMAPS::find );

TOOL_ACTION ACTIONS::findPrevious( "common.Interactive.findPrevious",
        AS_GLOBAL,
        MD_SHIFT + WXK_F3, LEGACY_HK_NAME( "Find Previous" ),
        _( "Find Previous" ), _( "Find previous match" ),
        BITMAPS::find );

TOOL_ACTION ACTIONS::findNextMarker( "common.Interactive.findNextMarker",
        AS_GLOBAL,
        MD_CTRL + MD_SHIFT + WXK_F3, LEGACY_HK_NAME( "Find Next Marker" ),
        _( "Find Next Marker" ), "",
        BITMAPS::find );

TOOL_ACTION ACTIONS::replaceAndFindNext( "common.Interactive.replaceAndFindNext",
        AS_GLOBAL,
        0, "",
        _( "Replace and Find Next" ), _( "Replace current match and find next" ),
        BITMAPS::find_replace );

TOOL_ACTION ACTIONS::replaceAll( "common.Interactive.replaceAll",
        AS_GLOBAL,
        0, "",
        _( "Replace All" ), _( "Replace all matches" ),
        BITMAPS::find_replace );

TOOL_ACTION ACTIONS::updateFind( "common.Control.updateFind",
        AS_GLOBAL );


// Marker Controls
TOOL_ACTION ACTIONS::prevMarker( "common.Checker.prevMarker",
        AS_GLOBAL,
        0, "",
        _( "Previous Marker" ), _( "Go to previous marker in Checker window" ),
        BITMAPS::marker_previous );

TOOL_ACTION ACTIONS::nextMarker( "common.Checker.nextMarker",
        AS_GLOBAL,
        0, "",
        _( "Next Marker" ), _( "Go to next marker in Checker window" ),
        BITMAPS::marker_next );

TOOL_ACTION ACTIONS::excludeMarker( "common.Checker.excludeMarker",
        AS_GLOBAL,
        0, "",
        _( "Exclude Marker" ), _( "Mark current violation in Checker window as an exclusion" ),
        BITMAPS::marker_exclude );

// View Controls
TOOL_ACTION ACTIONS::zoomRedraw( "common.Control.zoomRedraw",
        AS_GLOBAL,
#if defined( __WXMAC__ )
        MD_CTRL + 'R',
#else
        WXK_F5,
#endif
        LEGACY_HK_NAME( "Zoom Redraw" ),
        _( "Refresh" ), _( "Refresh" ),
        BITMAPS::refresh );

TOOL_ACTION ACTIONS::zoomFitScreen( "common.Control.zoomFitScreen",
        AS_GLOBAL,
#if defined( __WXMAC__ )
        MD_CTRL + '0',
#else
        WXK_HOME,
#endif
        LEGACY_HK_NAME( "Zoom Auto" ),
        _( "Zoom to Fit" ), _( "Zoom to Fit" ),
        BITMAPS::zoom_fit_in_page );

TOOL_ACTION ACTIONS::zoomFitObjects( "common.Control.zoomFitObjects",
        AS_GLOBAL, MD_CTRL + WXK_HOME, "",
        _( "Zoom to Objects" ), _( "Zoom to Objects" ),
        BITMAPS::zoom_fit_to_objects );

TOOL_ACTION ACTIONS::zoomIn( "common.Control.zoomIn",
        AS_GLOBAL,
#if defined( __WXMAC__ )
        MD_CTRL + '+',
#else
        WXK_F1,
#endif
        LEGACY_HK_NAME( "Zoom In" ),
        _( "Zoom In at Cursor" ), _( "Zoom In at Cursor" ),
        BITMAPS::zoom_in );

TOOL_ACTION ACTIONS::zoomOut( "common.Control.zoomOut",
        AS_GLOBAL,
#if defined( __WXMAC__ )
        MD_CTRL + '-',
#else
        WXK_F2,
#endif
        LEGACY_HK_NAME( "Zoom Out" ),
        _( "Zoom Out at Cursor" ), _( "Zoom Out at Cursor" ),
        BITMAPS::zoom_out );

TOOL_ACTION ACTIONS::zoomInCenter( "common.Control.zoomInCenter",
        AS_GLOBAL,
        0, "",
        _( "Zoom In" ), _( "Zoom In" ),
        BITMAPS::zoom_in );

TOOL_ACTION ACTIONS::zoomOutCenter( "common.Control.zoomOutCenter",
        AS_GLOBAL,
        0, "",
        _( "Zoom Out" ), _( "Zoom Out" ),
        BITMAPS::zoom_out );

TOOL_ACTION ACTIONS::zoomCenter( "common.Control.zoomCenter",
        AS_GLOBAL,
        WXK_F4, LEGACY_HK_NAME( "Zoom Center" ),
        _( "Center on Cursor" ), _( "Center on Cursor" ),
        BITMAPS::zoom_center_on_screen );

TOOL_ACTION ACTIONS::zoomTool( "common.Control.zoomTool",
        AS_GLOBAL,
        MD_CTRL + WXK_F5, LEGACY_HK_NAME( "Zoom to Selection" ),
        _( "Zoom to Selection" ), _( "Zoom to Selection" ),
        BITMAPS::zoom_area, AF_ACTIVATE );

TOOL_ACTION ACTIONS::zoomPreset( TOOL_ACTION_ARGS()
        .Name( "common.Control.zoomPreset" )
        .Scope( AS_GLOBAL )
        .Parameter<int>( 0 ) );      // Default parameter is the 0th item in the list

TOOL_ACTION ACTIONS::centerContents( "common.Control.centerContents",
        AS_GLOBAL );

// Cursor control
TOOL_ACTION ACTIONS::cursorUp( TOOL_ACTION_ARGS()
        .Name( "common.Control.cursorUp" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( WXK_UP )
        .MenuText( _( "Cursor Up" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_UP ) );

TOOL_ACTION ACTIONS::cursorDown( TOOL_ACTION_ARGS()
        .Name( "common.Control.cursorDown" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( WXK_DOWN )
        .MenuText( _( "Cursor Down" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_DOWN ) );

TOOL_ACTION ACTIONS::cursorLeft( TOOL_ACTION_ARGS()
        .Name( "common.Control.cursorLeft" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( WXK_LEFT )
        .MenuText( _( "Cursor Left" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_LEFT ) );

TOOL_ACTION ACTIONS::cursorRight( TOOL_ACTION_ARGS()
        .Name( "common.Control.cursorRight" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( WXK_RIGHT )
        .MenuText( _( "Cursor Right" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_RIGHT ) );


TOOL_ACTION ACTIONS::cursorUpFast( TOOL_ACTION_ARGS()
        .Name( "common.Control.cursorUpFast" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( MD_CTRL + WXK_UP )
        .MenuText( _( "Cursor Up Fast" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_UP | CURSOR_FAST_MOVE ) );

TOOL_ACTION ACTIONS::cursorDownFast( TOOL_ACTION_ARGS()
        .Name( "common.Control.cursorDownFast" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( MD_CTRL + WXK_DOWN )
        .MenuText( _( "Cursor Down Fast" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_DOWN | CURSOR_FAST_MOVE ) );

TOOL_ACTION ACTIONS::cursorLeftFast( TOOL_ACTION_ARGS()
        .Name( "common.Control.cursorLeftFast" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( MD_CTRL + WXK_LEFT )
        .MenuText( _( "Cursor Left Fast" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_LEFT | CURSOR_FAST_MOVE ) );

TOOL_ACTION ACTIONS::cursorRightFast( TOOL_ACTION_ARGS()
        .Name( "common.Control.cursorRightFast" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( MD_CTRL + WXK_RIGHT )
        .MenuText( _( "Cursor Right Fast" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_RIGHT | CURSOR_FAST_MOVE ) );

TOOL_ACTION ACTIONS::cursorClick( TOOL_ACTION_ARGS()
        .Name( "common.Control.cursorClick" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( WXK_RETURN )
        .LegacyHotkeyName( "Mouse Left Click" )
        .MenuText( _( "Click" ) )
        .Tooltip( _( "Performs left mouse button click" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_CLICK ) );

TOOL_ACTION ACTIONS::cursorDblClick( TOOL_ACTION_ARGS()
        .Name( "common.Control.cursorDblClick" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( WXK_END )
        .LegacyHotkeyName( "Mouse Left Double Click" )
        .MenuText( _( "Double-click" ) )
        .Tooltip( _( "Performs left mouse button double-click" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_DBL_CLICK ) );

TOOL_ACTION ACTIONS::refreshPreview( "common.Control.refreshPreview",
        AS_GLOBAL );

TOOL_ACTION ACTIONS::pinLibrary( "common.Control.pinLibrary",
        AS_GLOBAL, 0, "",
        _( "Pin Library" ),
        _( "Keep the library at the top of the list" ) );

TOOL_ACTION ACTIONS::unpinLibrary( "common.Control.unpinLibrary",
        AS_GLOBAL, 0, "",
        _( "Unpin Library" ),
        _( "No longer keep the library at the top of the list" ) );

TOOL_ACTION ACTIONS::panUp( TOOL_ACTION_ARGS()
        .Name( "common.Control.panUp" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( MD_SHIFT + WXK_UP )
        .MenuText( _( "Pan Up" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_UP ) );

TOOL_ACTION ACTIONS::panDown( TOOL_ACTION_ARGS()
        .Name( "common.Control.panDown" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( MD_SHIFT + WXK_DOWN )
        .MenuText( _( "Pan Down" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_DOWN ) );

TOOL_ACTION ACTIONS::panLeft( TOOL_ACTION_ARGS()
        .Name( "common.Control.panLeft" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( MD_SHIFT + WXK_LEFT )
        .MenuText( _( "Pan Left" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_LEFT ) );

TOOL_ACTION ACTIONS::panRight( TOOL_ACTION_ARGS()
        .Name( "common.Control.panRight" )
        .Scope( AS_GLOBAL )
        .DefaultHotkey( MD_SHIFT + WXK_RIGHT )
        .MenuText( _( "Pan Right" ) )
        .Flags( AF_NONE )
        .Parameter( CURSOR_RIGHT ) );

// Grid control
TOOL_ACTION ACTIONS::gridFast1( "common.Control.gridFast1",
        AS_GLOBAL,
        MD_ALT + '1', LEGACY_HK_NAME( "Switch Grid To Fast Grid1" ),
        _( "Switch to Fast Grid 1" ), "" );

TOOL_ACTION ACTIONS::gridFast2( "common.Control.gridFast2",
        AS_GLOBAL,
        MD_ALT + '2', LEGACY_HK_NAME( "Switch Grid To Fast Grid2" ),
        _( "Switch to Fast Grid 2" ), "" );

TOOL_ACTION ACTIONS::gridNext( "common.Control.gridNext",
        AS_GLOBAL,
        'N', LEGACY_HK_NAME( "Switch Grid To Next" ),
        _("Switch to Next Grid" ), "" );

TOOL_ACTION ACTIONS::gridPrev( "common.Control.gridPrev",
        AS_GLOBAL, MD_SHIFT + 'N', LEGACY_HK_NAME( "Switch Grid To Previous" ),
        _( "Switch to Previous Grid" ), "" );

TOOL_ACTION ACTIONS::gridSetOrigin( TOOL_ACTION_ARGS()
        .Name( "common.Control.gridSetOrigin" )
        .Scope( AS_GLOBAL )
        .LegacyHotkeyName( "Set Grid Origin" )
        .MenuText( _( "Grid Origin" ) )
        .Tooltip( _( "Set the grid origin point" ) )
        .Icon( BITMAPS::grid_select_axis )
        .Parameter<VECTOR2D*>( nullptr ) );

TOOL_ACTION ACTIONS::gridResetOrigin( "common.Control.gridResetOrigin",
        AS_GLOBAL,
        0, LEGACY_HK_NAME( "Reset Grid Origin" ),
        _( "Reset Grid Origin" ), "" );

TOOL_ACTION ACTIONS::gridPreset( TOOL_ACTION_ARGS()
        .Name( "common.Control.gridPreset" )
        .Scope( AS_GLOBAL )
        .Parameter<int>( 0 ) );          // Default to the 1st element of the list

TOOL_ACTION ACTIONS::toggleGrid( TOOL_ACTION_ARGS()
        .Name("common.Control.toggleGrid")
        .Scope( AS_GLOBAL)
        .MenuText( _( "Show Grid" ) )
        .Tooltip( _( "Display background grid in the edit window" ) )
        .Icon( BITMAPS::grid ) );

TOOL_ACTION ACTIONS::toggleGridOverrides( TOOL_ACTION_ARGS()
        .Name("common.Control.toggleGridOverrides")
        .DefaultHotkey( MD_CTRL + MD_SHIFT + 'G' )
        .Scope( AS_GLOBAL)
        .MenuText( _( "Grid Overrides" ) )
        .Tooltip( _( "Enables item-specific grids that override the current grid" ) )
        .Icon( BITMAPS::grid_override ) );

TOOL_ACTION ACTIONS::gridProperties( "common.Control.gridProperties",
        AS_GLOBAL, 0, "",
        _( "Grid Properties..." ), _( "Set grid dimensions" ),
        BITMAPS::grid_select );

TOOL_ACTION ACTIONS::inchesUnits( TOOL_ACTION_ARGS()
        .Name( "common.Control.imperialUnits" )
        .Scope( AS_GLOBAL )
        .MenuText( _( "Inches" ) )
        .Tooltip( _( "Use inches" ) )
        .Icon( BITMAPS::unit_inch )
        .Flags( AF_NONE )
        .Parameter( EDA_UNITS::INCHES ) );

TOOL_ACTION ACTIONS::milsUnits( TOOL_ACTION_ARGS()
        .Name( "common.Control.mils" )
        .Scope( AS_GLOBAL )
        .MenuText( _( "Mils" ) )
        .Tooltip( _( "Use mils" ) )
        .Icon( BITMAPS::unit_mil )
        .Flags( AF_NONE )
        .Parameter( EDA_UNITS::MILS ) );

TOOL_ACTION ACTIONS::millimetersUnits( TOOL_ACTION_ARGS()
        .Name( "common.Control.metricUnits" )
        .Scope( AS_GLOBAL )
        .MenuText( _( "Millimeters" ) )
        .Tooltip( _( "Use millimeters" ) )
        .Icon( BITMAPS::unit_mm )
        .Flags( AF_NONE )
        .Parameter( EDA_UNITS::MILLIMETRES ) );

TOOL_ACTION ACTIONS::updateUnits( "common.Control.updateUnits",
        AS_GLOBAL );

TOOL_ACTION ACTIONS::updatePreferences( "common.Control.updatePreferences",
        AS_GLOBAL );

TOOL_ACTION ACTIONS::selectColumns( "common.Control.selectColumns",
                                    AS_GLOBAL, 0, "",
                                    _( "Select Columns" ) );

TOOL_ACTION ACTIONS::toggleUnits( "common.Control.toggleUnits",
        AS_GLOBAL,
        MD_CTRL + 'U', LEGACY_HK_NAME( "Switch Units" ),
        _( "Switch units" ), _( "Switch between imperial and metric units" ),
        BITMAPS::unit_mm );

TOOL_ACTION ACTIONS::togglePolarCoords( "common.Control.togglePolarCoords",
        AS_GLOBAL, 0, "",
        _( "Polar Coordinates" ), _( "Switch between polar and cartesian coordinate systems" ),
        BITMAPS::polar_coord );

TOOL_ACTION ACTIONS::resetLocalCoords( "common.Control.resetLocalCoords",
        AS_GLOBAL,
        ' ', LEGACY_HK_NAME( "Reset Local Coordinates" ),
        _( "Reset Local Coordinates" ), "" );

TOOL_ACTION ACTIONS::toggleCursor( "common.Control.toggleCursor",
        AS_GLOBAL,
        // Don't be tempted to remove "Modern Toolset only".  It's in the legacy property name.
        MD_CTRL + MD_SHIFT + 'X', LEGACY_HK_NAME( "Toggle Cursor Display (Modern Toolset only)" ),
        _( "Always Show Cursor" ), _( "Display crosshairs even in selection tool" ),
        BITMAPS::cursor );

TOOL_ACTION ACTIONS::toggleCursorStyle( "common.Control.toggleCursorStyle",
        AS_GLOBAL, 0, "",
        _( "Full-Window Crosshairs" ), _( "Switch display of full-window crosshairs" ),
        BITMAPS::cursor_shape );

TOOL_ACTION ACTIONS::highContrastMode( "common.Control.highContrastMode",
        AS_GLOBAL,
        0, LEGACY_HK_NAME( "Toggle High Contrast Mode" ),
        _( "Inactive Layer View Mode" ),
        _( "Toggle inactive layers between normal and dimmed" ),
        BITMAPS::contrast_mode );

TOOL_ACTION ACTIONS::highContrastModeCycle( "common.Control.highContrastModeCycle",
        AS_GLOBAL,
        'H', "",
        _( "Inactive Layer View Mode (3-state)" ),
        _( "Cycle inactive layers between normal, dimmed, and hidden" ),
        BITMAPS::contrast_mode );

TOOL_ACTION ACTIONS::toggleBoundingBoxes( "common.Control.toggleBoundingBoxes",
        AS_GLOBAL, 0, "",
        _( "Draw Bounding Boxes" ), _( "Draw Bounding Boxes" ),
        BITMAPS::gerbview_show_negative_objects );

TOOL_ACTION ACTIONS::selectionTool( "common.InteractiveSelection.selectionTool",
        AS_GLOBAL, 0, "",
        _( "Select item(s)" ), _( "Select item(s)" ),
        BITMAPS::cursor, AF_ACTIVATE );

TOOL_ACTION ACTIONS::measureTool( "common.InteractiveEdit.measureTool",
        AS_GLOBAL,
        // Don't be tempted to remove "Modern Toolset only".  It's in the legacy property name.
        MD_CTRL + MD_SHIFT + 'M', LEGACY_HK_NAME( "Measure Distance (Modern Toolset only)" ),
        _( "Measure Tool" ), _( "Interactively measure distance between points" ),
        BITMAPS::measurement, AF_ACTIVATE );

TOOL_ACTION ACTIONS::pickerTool( "common.InteractivePicker.pickerTool",
        AS_GLOBAL, 0, "",
        "", "",
        BITMAPS::INVALID_BITMAP, AF_ACTIVATE );

TOOL_ACTION ACTIONS::pickerSubTool( "common.InteractivePicker.pickerSubTool",
        AS_GLOBAL );

TOOL_ACTION ACTIONS::show3DViewer( "common.Control.show3DViewer",
        AS_GLOBAL,
        MD_ALT + '3', LEGACY_HK_NAME( "3D Viewer" ),
        _( "3D Viewer" ), _( "Show 3D viewer window" ),
        BITMAPS::three_d );

TOOL_ACTION ACTIONS::showSymbolBrowser( TOOL_ACTION_ARGS()
        .Name( "common.Control.showSymbolBrowser" )
        .Scope( AS_GLOBAL )
        .MenuText( _( "Symbol Library Browser" ) )
        .Tooltip( _( "Browse symbol libraries" ) )
        .Icon( BITMAPS::library_browser )
        .Flags( AF_NONE)
        .Parameter( FRAME_SCH_VIEWER ) );

TOOL_ACTION ACTIONS::showSymbolEditor( TOOL_ACTION_ARGS()
        .Name( "common.Control.showSymbolEditor" )
        .Scope( AS_GLOBAL )
        .MenuText( _( "Symbol Editor" ) )
        .Tooltip( _( "Create, delete and edit symbols" ) )
        .Icon( BITMAPS::libedit )
        .Flags( AF_NONE )
        .Parameter( FRAME_SCH_SYMBOL_EDITOR ) );

TOOL_ACTION ACTIONS::showFootprintBrowser( TOOL_ACTION_ARGS()
        .Name( "common.Control.showFootprintBrowser" )
        .Scope( AS_GLOBAL )
        .MenuText( _( "Footprint Library Browser" ) )
        .Tooltip( _( "Browse footprint libraries" ) )
        .Icon( BITMAPS::library_browser )
        .Flags( AF_NONE )
        .Parameter( FRAME_FOOTPRINT_VIEWER ) );

TOOL_ACTION ACTIONS::showFootprintEditor( TOOL_ACTION_ARGS()
        .Name( "common.Control.showFootprintEditor" )
        .Scope( AS_GLOBAL )
        .MenuText( _( "Footprint Editor" ) )
        .Tooltip( _( "Create, delete and edit footprints" ) )
        .Icon( BITMAPS::module_editor )
        .Flags( AF_NONE )
        .Parameter( FRAME_FOOTPRINT_EDITOR ) );

TOOL_ACTION ACTIONS::showProperties( "common.Control.showProperties",
        AS_GLOBAL, 0, "",
        _( "Show Properties Manager" ), _( "Show/hide the properties manager" ),
        BITMAPS::tools );

TOOL_ACTION ACTIONS::updatePcbFromSchematic( "common.Control.updatePcbFromSchematic",
        AS_GLOBAL,
        WXK_F8, LEGACY_HK_NAME( "Update PCB from Schematic" ),
        _( "Update PCB from Schematic..." ), _( "Update PCB with changes made to schematic" ),
        BITMAPS::update_pcb_from_sch );

TOOL_ACTION ACTIONS::updateSchematicFromPcb( "common.Control.updateSchematicFromPCB",
        AS_GLOBAL, 0, "",
        _( "Update Schematic from PCB..." ), _( "Update schematic with changes made to PCB" ),
        BITMAPS::update_sch_from_pcb );

TOOL_ACTION ACTIONS::openPreferences( "common.SuiteControl.openPreferences",
        AS_GLOBAL, MD_CTRL + ',', "",
        _( "Preferences..." ), _( "Show preferences for all open tools" ),
        BITMAPS::preference );

TOOL_ACTION ACTIONS::configurePaths( "common.SuiteControl.configurePaths",
        AS_GLOBAL, 0, "",
        _( "Configure Paths..." ), _( "Edit path configuration environment variables" ),
        BITMAPS::path );

TOOL_ACTION ACTIONS::showSymbolLibTable( "common.SuiteControl.showSymbolLibTable",
        AS_GLOBAL, 0, "",
        _( "Manage Symbol Libraries..." ),
        _( "Edit the global and project symbol library lists" ),
        BITMAPS::library_table );

TOOL_ACTION ACTIONS::showFootprintLibTable( "common.SuiteControl.showFootprintLibTable",
        AS_GLOBAL, 0, "",
        _( "Manage Footprint Libraries..." ),
        _( "Edit the global and project footprint library lists" ),
        BITMAPS::library_table );

TOOL_ACTION ACTIONS::gettingStarted( "common.SuiteControl.gettingStarted",
        AS_GLOBAL, 0, "",
        _( "Getting Started with KiCad" ),
        _( "Open \"Getting Started in KiCad\" guide for beginners" ),
        BITMAPS::help );

TOOL_ACTION ACTIONS::help( "common.SuiteControl.help",
        AS_GLOBAL, 0, "",
        _( "Help" ),
        _( "Open product documentation in a web browser" ),
        BITMAPS::help_online );

TOOL_ACTION ACTIONS::listHotKeys( "common.SuiteControl.listHotKeys",
        AS_GLOBAL,
        MD_CTRL + WXK_F1, LEGACY_HK_NAME( "List Hotkeys" ),
        _( "List Hotkeys..." ),
        _( "Displays current hotkeys table and corresponding commands" ),
        BITMAPS::hotkeys );

TOOL_ACTION ACTIONS::getInvolved( "common.SuiteControl.getInvolved",
        AS_GLOBAL, 0, "",
        _( "Get Involved" ),
        _( "Open \"Contribute to KiCad\" in a web browser" ),
        BITMAPS::info );

TOOL_ACTION ACTIONS::donate( "common.SuiteControl.donate",
        AS_GLOBAL, 0, "",
        _( "Donate" ),
        _( "Open \"Donate to KiCad\" in a web browser" ) );

TOOL_ACTION ACTIONS::reportBug( "common.SuiteControl.reportBug",
        AS_GLOBAL, 0, "",
        _( "Report Bug" ),
        _( "Report a problem with KiCad" ),
        BITMAPS::bug );

TOOL_ACTION ACTIONS::ddAddLibrary( "common.Control.ddaddLibrary",
        AS_GLOBAL );

// System-wide selection Events

const TOOL_EVENT EVENTS::PointSelectedEvent( TC_MESSAGE, TA_ACTION, "common.Interactive.pointSelected" );
const TOOL_EVENT EVENTS::SelectedEvent( TC_MESSAGE, TA_ACTION, "common.Interactive.selected" );
const TOOL_EVENT EVENTS::UnselectedEvent( TC_MESSAGE, TA_ACTION, "common.Interactive.unselected" );
const TOOL_EVENT EVENTS::ClearedEvent( TC_MESSAGE, TA_ACTION, "common.Interactive.cleared" );

const TOOL_EVENT EVENTS::ConnectivityChangedEvent( TC_MESSAGE, TA_ACTION, "common.Interactive.connectivityChanged" );

const TOOL_EVENT EVENTS::SelectedItemsModified( TC_MESSAGE, TA_ACTION, "common.Interactive.modified" );
const TOOL_EVENT EVENTS::SelectedItemsMoved( TC_MESSAGE, TA_ACTION, "common.Interactive.moved" );
const TOOL_EVENT EVENTS::InhibitSelectionEditing( TC_MESSAGE, TA_ACTION, "common.Interactive.inhibit" );
const TOOL_EVENT EVENTS::UninhibitSelectionEditing( TC_MESSAGE, TA_ACTION, "common.Interactive.uninhibit" );

const TOOL_EVENT EVENTS::DisambiguatePoint( TC_MESSAGE, TA_ACTION, "common.Interactive.disambiguate" );

const TOOL_EVENT EVENTS::GridChangedByKeyEvent( TC_MESSAGE, TA_ACTION,
                                                "common.Interactive.gridChangedByKey" );

const TOOL_EVENT EVENTS::ContrastModeChangedByKeyEvent( TC_MESSAGE, TA_ACTION,
                                                        "common.Interactive.contrastModeChangedByKeyEvent" );
/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008 Wayne Stambaugh <stambaughw@gmail.com>
 * Copyright (C) 2004-2023 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <confirm.h>
#include <common.h>
#include <symbol_lib_table.h>
#include <symbol_edit_frame.h>
#include <symbol_library.h>
#include <wildcards_and_files_ext.h>
#include <lib_symbol_library_manager.h>
#include <wx/filename.h>
#include <wx/filedlg.h>
#include <string_utils.h>
//#include <wx/msw/dirdlg.h>
#include <wx/dirdlg.h>
#include <ee_tool_base.h>


void SYMBOL_EDIT_FRAME::ImportSymbol()
{
    wxString msg;
    wxString libName = getTargetLib();

    if( !m_libMgr->LibraryExists( libName ) )
    {
        libName = SelectLibraryFromList();

        if( !m_libMgr->LibraryExists( libName ) )
            return;
    }

    wxString wildcards = AllSymbolLibFilesWildcard()
                         + "|" + KiCadSymbolLibFileWildcard()
                         + "|" + LegacySymbolLibFileWildcard();

    wxFileDialog dlg( this, _( "Import Symbol" ), m_mruPath, wxEmptyString,
                      wildcards, wxFD_OPEN | wxFD_FILE_MUST_EXIST );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    wxFileName fn;

    if( dlg.GetFilterIndex() == 0 )
        fn = EnsureFileExtension( dlg.GetPath(), KiCadSymbolLibFileExtension );
    else
        fn = EnsureFileExtension( dlg.GetPath(), LegacySymbolLibFileExtension );

    m_mruPath = fn.GetPath();

    wxArrayString symbols;
    SCH_IO_MGR::SCH_FILE_T piType = SCH_IO_MGR::GuessPluginTypeFromLibPath( fn.GetFullPath() );
    SCH_PLUGIN::SCH_PLUGIN_RELEASER pi( SCH_IO_MGR::FindPlugin( piType ) );

    // TODO dialog to select the symbol to be imported if there is more than one
    try
    {
        pi->EnumerateSymbolLib( symbols, fn.GetFullPath() );
    }
    catch( const IO_ERROR& ioe )
    {
        msg.Printf( _( "Cannot import symbol library '%s'." ), fn.GetFullPath() );
        DisplayErrorMessage( this, msg, ioe.What() );
        return;
    }

    if( symbols.empty() )
    {
        msg.Printf( _( "Symbol library file '%s' is empty." ), fn.GetFullPath() );
        DisplayError( this,  msg );
        return;
    }

    wxString symbolName = symbols[0];
    LIB_SYMBOL* entry = pi->LoadSymbol( fn.GetFullPath(), symbolName );

    entry->SetName( EscapeString( entry->GetName(), CTX_LIBID ) );

    if( m_libMgr->SymbolExists( entry->GetName(), libName ) )
    {
        msg.Printf( _( "Symbol %s already exists in library '%s'." ), symbolName, libName );

        KIDIALOG errorDlg( this, msg, _( "Confirmation" ), wxOK | wxCANCEL | wxICON_WARNING );
        errorDlg.SetOKLabel( _( "Overwrite" ) );
        errorDlg.DoNotShowCheckbox( __FILE__, __LINE__ );

        if( errorDlg.ShowModal() == wxID_CANCEL )
            return;
    }

    m_libMgr->UpdateSymbol( entry, libName );
    SyncLibraries( false );
    LoadSymbol( entry->GetName(), libName, 1 );
}


void SYMBOL_EDIT_FRAME::ExportSymbol()
{
    wxString msg;
    LIB_SYMBOL* symbol = getTargetSymbol();

    if( !symbol )
    {
        ShowInfoBarError( _( "There is no symbol selected to save." ) );
        return;
    }

    wxFileName fn;

    fn.SetName( symbol->GetName().Lower() );
    fn.SetExt( KiCadSymbolLibFileExtension );

    wxFileDialog dlg( this, _( "Export Symbol" ), m_mruPath, fn.GetFullName(),
                      KiCadSymbolLibFileWildcard(), wxFD_SAVE );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    fn = dlg.GetPath();
    fn.MakeAbsolute();

    LIB_SYMBOL* old_symbol = nullptr;
    SCH_IO_MGR::SCH_FILE_T pluginType = SCH_IO_MGR::GuessPluginTypeFromLibPath( fn.GetFullPath() );
    SCH_PLUGIN::SCH_PLUGIN_RELEASER pi( SCH_IO_MGR::FindPlugin( pluginType ) );

    if( fn.FileExists() )
    {
        try
        {
            old_symbol = pi->LoadSymbol( fn.GetFullPath(), symbol->GetName() );
        }
        catch( const IO_ERROR& ioe )
        {
            msg.Printf( _( "Error occurred attempting to load symbol library file '%s'." ),
                        fn.GetFullPath() );
            DisplayErrorMessage( this, msg, ioe.What() );
            return;
        }

        if( old_symbol )
        {
            msg.Printf( _( "Symbol %s already exists in library '%s'." ),
                        UnescapeString( symbol->GetName() ),
                        fn.GetFullName() );

            KIDIALOG errorDlg( this, msg, _( "Confirmation" ), wxOK | wxCANCEL | wxICON_WARNING );
            errorDlg.SetOKLabel( _( "Overwrite" ) );
            errorDlg.DoNotShowCheckbox( __FILE__, __LINE__ );

            if( errorDlg.ShowModal() == wxID_CANCEL )
                return;
        }
    }

    if( fn.Exists() && !fn.IsDirWritable() )
    {
        msg.Printf( _( "Insufficient permissions to save library '%s'." ),
                    fn.GetFullPath() );
        DisplayError( this, msg );
        return;
    }

    try
    {
        if( !fn.FileExists() )
            pi->CreateSymbolLib( fn.GetFullPath() );

        // The flattened symbol is most likely what the user would want.  As some point in
        // the future as more of the symbol library inheritance is implemented, this may have
        // to be changes to save symbols of inherited symbols.
        pi->SaveSymbol( fn.GetFullPath(), symbol->Flatten().release() );
    }
    catch( const IO_ERROR& ioe )
    {
        msg.Printf( _( "Failed to create symbol library file '%s'." ), fn.GetFullPath() );
        DisplayErrorMessage( this, msg, ioe.What() );
        msg.Printf( _( "Error creating symbol library '%s'." ), fn.GetFullName() );
        SetStatusText( msg );
        return;
    }

    m_mruPath = fn.GetPath();

    msg.Printf( _( "Symbol %s saved to library '%s'." ),
                UnescapeString( symbol->GetName() ),
                fn.GetFullPath() );
    SetStatusText( msg );

    // See if the user wants it added to a library table (global or project)
    SYMBOL_LIB_TABLE* libTable = SelectSymLibTable( true );

    if( libTable )
    {
        if( !m_libMgr->AddLibrary( fn.GetFullPath(), libTable ) )
        {
            DisplayError( this, _( "Could not open the library file." ) );
            return;
        }

        bool globalTable = ( libTable == &SYMBOL_LIB_TABLE::GetGlobalLibTable() );
        saveSymbolLibTables( globalTable, !globalTable );
    }
}


//Ixport single SYM( -sxl)
void SYMBOL_EDIT_FRAME::ImportSymbol( wxFileName& fn, wxString& fileFullPath)
{
    wxString msg;
    wxString libName = getTargetLib();
    if( !m_libMgr->LibraryExists( libName ) )
    {
        libName = SelectLibraryFromList();

        if( !m_libMgr->LibraryExists( libName ) )
            return;
    }

    wxString wildcards = AllSymbolLibFilesWildcard() + "|" + KiCadSymbolLibFileWildcard() + "|"
                         + LegacySymbolLibFileWildcard();

    fn = EnsureFileExtension( fileFullPath, KiCadSymbolLibFileExtension );

    m_mruPath = fn.GetPath();


    wxArrayString          symbols;
    SCH_IO_MGR::SCH_FILE_T piType = SCH_IO_MGR::GuessPluginTypeFromLibPath( fn.GetFullPath() );
    SCH_PLUGIN::SCH_PLUGIN_RELEASER pi( SCH_IO_MGR::FindPlugin( piType ) );

    // TODO dialog to select the symbol to be imported if there is more than one
    try
    {
        pi->EnumerateSymbolLib( symbols, fn.GetFullPath() );
    }
    catch( const IO_ERROR& ioe )
    {
        msg.Printf( _( "Cannot import symbol library '%s'." ), fn.GetFullPath() );
        DisplayErrorMessage( this, msg, ioe.What() );
        return;
    }

    if( symbols.empty() )
    {
        msg.Printf( _( "Symbol library file '%s' is empty." ), fn.GetFullPath() );
        DisplayError( this, msg );
        return;
    }

    wxString    symbolName = symbols[0];
    LIB_SYMBOL* entry = pi->LoadSymbol( fn.GetFullPath(), symbolName );

    entry->SetName( EscapeString( entry->GetName(), CTX_LIBID ) );

    if( m_libMgr->SymbolExists( entry->GetName(), libName ) )
    {
        msg.Printf( _( "Symbol %s already exists in library '%s'." ), symbolName, libName );

        KIDIALOG errorDlg( this, msg, _( "Confirmation" ), wxOK | wxCANCEL | wxICON_WARNING );
        errorDlg.SetOKLabel( _( "Overwrite" ) );
        errorDlg.DoNotShowCheckbox( __FILE__, __LINE__ );

        if( errorDlg.ShowModal() == wxID_CANCEL )
            return;
    }

    m_libMgr->UpdateSymbol( entry, libName );
    SyncLibraries( false );
    LoadSymbol( entry->GetName(), libName, 1 );

}


//Ixport multi SYM( -sxl)
void SYMBOL_EDIT_FRAME::ImportSymbol( wxFileName& fn, wxString& fileFullPath, wxString filenameLC )
{
    wxString msg;
    wxString libName = getTargetLib();

    if( !m_libMgr->LibraryExists( libName ) )
    {
        libName = SelectLibraryFromList();

        if( !m_libMgr->LibraryExists( libName ) )
            return;
    }

    wxString wildcards = AllSymbolLibFilesWildcard() + "|" + KiCadSymbolLibFileWildcard() + "|"
                         + LegacySymbolLibFileWildcard();


    fn = EnsureFileExtension( fileFullPath, KiCadSymbolLibFileExtension );

    m_mruPath = fn.GetPath();


    wxArrayString          symbols;
    SCH_IO_MGR::SCH_FILE_T piType = SCH_IO_MGR::GuessPluginTypeFromLibPath( fn.GetFullPath() );
    SCH_PLUGIN::SCH_PLUGIN_RELEASER pi( SCH_IO_MGR::FindPlugin( piType ) );

    // TODO dialog to select the symbol to be imported if there is more than one
    try
    {
        pi->EnumerateSymbolLib( symbols, fn.GetFullPath() );
    }
    catch( const IO_ERROR& ioe )
    {
        msg.Printf( _( "'%s' cannot import symbol library '%s'." ), filenameLC, fn.GetFullPath() );
        DisplayErrorMessage( this, msg, ioe.What() );
        return;
    }

    if( symbols.empty() )
    {
        msg.Printf( _( "flie '%s' Symbol library file '%s' is empty." ), filenameLC, fn.GetFullPath() );
        msg.Printf( _( "Symbol library file '%s' is empty." ), filenameLC );
        DisplayError( this, msg );
        return;
    }

    wxString    symbolName = symbols[0];
    LIB_SYMBOL* entry = pi->LoadSymbol( fn.GetFullPath(), symbolName );

    entry->SetName( EscapeString( entry->GetName(), CTX_LIBID ) );

    if( m_libMgr->SymbolExists( entry->GetName(), libName ) )
    {
        msg.Printf( _( "Symbol %s already exists in library '%s'." ), symbolName, libName );

        KIDIALOG errorDlg( this, msg, _( "Confirmation" ), wxOK | wxCANCEL | wxICON_WARNING );
        errorDlg.SetOKLabel( _( "Overwrite" ) );
        errorDlg.DoNotShowCheckbox( __FILE__, __LINE__ );

        if( errorDlg.ShowModal() == wxID_CANCEL )
            return;
    }

    m_libMgr->UpdateSymbol( entry, libName );
    SyncLibraries( false );
    LoadSymbol( entry->GetName(), libName, 1 );
}

//Ixport single SYM LibraryFile( -sxl,not used)
wxString SYMBOL_EDIT_FRAME::AddLibraryFile( bool aCreateNew,  wxFileName& fn )
{

     wxString      msg;
    wxArrayString          symbols;
    SCH_IO_MGR::SCH_FILE_T piType = SCH_IO_MGR::GuessPluginTypeFromLibPath( fn.GetFullPath() );
    SCH_PLUGIN::SCH_PLUGIN_RELEASER pi( SCH_IO_MGR::FindPlugin( piType ) );
    // Select the target library table (global/project)
    SYMBOL_LIB_TABLE* libTable = SelectSymLibTable();

    if( !libTable )
        return wxEmptyString;

    //wxFileName fn = m_libMgr->GetUniqueLibraryName();


    wxString libName = fn.GetName();

    if( libName.IsEmpty() )
        return wxEmptyString;


    if( aCreateNew )
    {
        if( !m_libMgr->CreateLibrary( fn.GetFullPath(), libTable ) )
        {
            DisplayError( this, wxString::Format( _( "Could not create the library file '%s'.\n"
                                                     "Make sure you have write permissions and "
                                                     "try again." ),
                                                  fn.GetFullPath() ) );
            return wxEmptyString;
        }
    }
    else
    {
        if( !m_libMgr->AddLibrary( fn.GetFullPath(), libTable ) )
        {
            DisplayError( this, _( "Could not open the library file." ) );
            return wxEmptyString;
        }
    }

    bool globalTable = ( libTable == &SYMBOL_LIB_TABLE::GetGlobalLibTable() );
    saveSymbolLibTables( globalTable, !globalTable );

    std::string packet = fn.GetFullPath().ToStdString();
    //this->Kiway().ExpressMail( FRAME_SCH_SYMBOL_EDITOR, MAIL_LIB_EDIT, packet );



    // TODO dialog to select the symbol to be imported if there is more than one
    try
    {
        pi->EnumerateSymbolLib( symbols, fn.GetFullPath() );
    }
    catch( const IO_ERROR& ioe )
    {
        msg.Printf( _( "Cannot import symbol library '%s'." ), fn.GetFullPath() );
        DisplayErrorMessage( this, msg, ioe.What() );
        return wxEmptyString;
    }

    wxString    symbolName = symbols[0];
    LIB_SYMBOL* entry = pi->LoadSymbol( fn.GetFullPath(), symbolName );

    entry->SetName( EscapeString( entry->GetName(), CTX_LIBID ) );

    if( m_libMgr->SymbolExists( entry->GetName(), libName ) )
    {
        msg.Printf( _( "Symbol %s already exists in library '%s'." ), symbolName, libName );

        KIDIALOG errorDlg( this, msg, _( "Confirmation" ), wxOK | wxCANCEL | wxICON_WARNING );
        errorDlg.SetOKLabel( _( "Overwrite" ) );
        errorDlg.DoNotShowCheckbox( __FILE__, __LINE__ );

        if( errorDlg.ShowModal() == wxID_CANCEL )
            return wxEmptyString;
    }

    m_libMgr->UpdateSymbol( entry, libName );
    SyncLibraries( false );
    LoadSymbol( entry->GetName(), libName, 1 );

    return fn.GetFullPath();
}


//Export single SYM( -sxl)
void SYMBOL_EDIT_FRAME::ExportSymbol( LIB_SYMBOL* aSymbol, wxString& fileSaveFullPath )
{
    wxString    msg;
    LIB_SYMBOL* symbol = aSymbol;

    if( !symbol )
    {
        ShowInfoBarError( _( "There is no symbol selected to save." ) );
        return;
    }

    wxFileName fn;

    fn.SetName( symbol->GetName().Lower() );
    fn.SetExt( KiCadSymbolLibFileExtension );

    wxFileDialog dlg( this, _( "Export Symbol" ), m_mruPath, fn.GetFullName(),
                      KiCadSymbolLibFileWildcard(), wxFD_SAVE );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    fn = dlg.GetPath();
    fn.MakeAbsolute();

    LIB_SYMBOL*            old_symbol = nullptr;
    SCH_IO_MGR::SCH_FILE_T pluginType = SCH_IO_MGR::GuessPluginTypeFromLibPath( fn.GetFullPath() );
    SCH_PLUGIN::SCH_PLUGIN_RELEASER pi( SCH_IO_MGR::FindPlugin( pluginType ) );

    if( fn.FileExists() )
    {
        try
        {
            old_symbol = pi->LoadSymbol( fn.GetFullPath(), symbol->GetName() );
        }
        catch( const IO_ERROR& ioe )
        {
            msg.Printf( _( "Error occurred attempting to load symbol library file '%s'." ),
                        fn.GetFullPath() );
            DisplayErrorMessage( this, msg, ioe.What() );
            return;
        }

        if( old_symbol )
        {
            msg.Printf( _( "Symbol %s already exists in library '%s'." ),
                        UnescapeString( symbol->GetName() ), fn.GetFullName() );

            KIDIALOG errorDlg( this, msg, _( "Confirmation" ), wxOK | wxCANCEL | wxICON_WARNING );
            errorDlg.SetOKLabel( _( "Overwrite" ) );
            errorDlg.DoNotShowCheckbox( __FILE__, __LINE__ );

            if( errorDlg.ShowModal() == wxID_CANCEL )
                return;
        }
    }

    if( fn.Exists() && !fn.IsDirWritable() )
    {
        msg.Printf( _( "Insufficient permissions to save library '%s'." ), fn.GetFullPath() );
        DisplayError( this, msg );
        return;
    }

    try
    {
        //if( !fn.FileExists() )
        //    pi->CreateSymbolLib( fn.GetFullPath() );
        if( fn.FileExists() )
        {
            // delete file
            if( !wxRemoveFile( fn.GetFullPath() ) )
            {
                // delete failure
                wxString msg = wxString::Format( _( "Error deleting file '%s'" ), fn.GetFullPath() );
                DisplayError( nullptr, msg );
            }
        }
        pi->CreateSymbolLib( fn.GetFullPath() );
        // The flattened symbol is most likely what the user would want.  As some point in
        // the future as more of the symbol library inheritance is implemented, this may have
        // to be changes to save symbols of inherited symbols.
        pi->SaveSymbol( fn.GetFullPath(), symbol->Flatten().release() );
        fileSaveFullPath = fn.GetFullPath(); // 添加这里

    }
    catch( const IO_ERROR& ioe )
    {
        msg.Printf( _( "Failed to create symbol library file '%s'." ), fn.GetFullPath() );
        DisplayErrorMessage( this, msg, ioe.What() );
        msg.Printf( _( "Error creating symbol library '%s'." ), fn.GetFullName() );
        SetStatusText( msg );
        return;
    }

    //ImportSymbol( fn, fileSaveFullPath );
    AddLibraryFile( false, fn );

}

//Export multi SYM. NO pop Dialog( -sxl)
void SYMBOL_EDIT_FRAME::ExportSymbolNoPopDialog( LIB_SYMBOL* aSymbol, wxString outDir,
                                                 wxString filenameLC )
{
    wxString    msg;
    LIB_SYMBOL* symbol = aSymbol;
    wxString    fileFullPath;

    if( !symbol )
    {
        ShowInfoBarError( _( "There is no symbol selected to save." ) );
        return;
    }

    wxFileName fn;

    fn.SetName( symbol->GetName().Lower() );
    fn.SetExt( KiCadSymbolLibFileExtension );
    //outDir 导出目录
    fn.SetPath( outDir );

    fn.MakeAbsolute();


    LIB_SYMBOL*            old_symbol = nullptr;
    SCH_IO_MGR::SCH_FILE_T pluginType = SCH_IO_MGR::GuessPluginTypeFromLibPath( fn.GetFullPath() );
    SCH_PLUGIN::SCH_PLUGIN_RELEASER pi( SCH_IO_MGR::FindPlugin( pluginType ) );

    if( fn.FileExists() )
    {
        try
        {
            old_symbol = pi->LoadSymbol( fn.GetFullPath(), symbol->GetName() );
        }
        catch( const IO_ERROR& ioe )
        {
            msg.Printf( _( "Error occurred attempting to load symbol library file '%s'." ),
                        fn.GetFullPath() );
            DisplayErrorMessage( this, msg, ioe.What() );
            return;
        }

        if( old_symbol )
        {
            msg.Printf( _( "Symbol %s already exists in library '%s'." ),
                        UnescapeString( symbol->GetName() ), fn.GetFullName() );

            KIDIALOG errorDlg( this, msg, _( "Confirmation" ), wxOK | wxCANCEL | wxICON_WARNING );
            errorDlg.SetOKLabel( _( "Overwrite" ) );
            errorDlg.DoNotShowCheckbox( __FILE__, __LINE__ );

            if( errorDlg.ShowModal() == wxID_CANCEL )
                return;
        }
    }

    if( fn.Exists() && !fn.IsDirWritable() )
    {
        msg.Printf( _( "Insufficient permissions to save library '%s'." ), fn.GetFullPath() );
        DisplayError( this, msg );
        return;
    }

    try
    {
        if( !fn.FileExists() )
            pi->CreateSymbolLib( fn.GetFullPath() );

        // The flattened symbol is most likely what the user would want.  As some point in
        // the future as more of the symbol library inheritance is implemented, this may have
        // to be changes to save symbols of inherited symbols.
        pi->SaveSymbol( fn.GetFullPath(), symbol->Flatten().release() );
        fileFullPath = outDir + "\\" + symbol->GetName().Lower();   //添加在这里
        fileFullPath = fileFullPath + "." + KiCadSymbolLibFileExtension;
    }
    catch( const IO_ERROR& ioe )
    {
        msg.Printf( _( "'%s' failed to create symbol library file '%s'." ), filenameLC, fn.GetFullPath() );
        DisplayErrorMessage( this, msg, ioe.What() );
        msg.Printf( _( "'%s' error creating symbol library '%s'." ), filenameLC, fn.GetFullName() );
        SetStatusText( msg );
        return;
    }

    m_mruPath = fn.GetPath();
    ImportSymbol( fn, fileFullPath, filenameLC );

    msg.Printf( _( "Symbol %s saved to library '%s'." ), UnescapeString( symbol->GetName() ),
                fn.GetFullPath() );
    SetStatusText( msg );
}


//wx Open Selected File dialog( -sxl)
wxFileName SYMBOL_EDIT_FRAME::OpenFileDialog( const wxString& aLastPath )
{
    static int lastFilterIndex = 0; // To store the last choice during a session.
    wxString   wildCard;

    wildCard << "LCSYM(*.esym)|*.esym" << wxChar( '|' ) << "LCSYMJson(*.json)|*.json"
             << wxChar( '|' ) << " AllFiles( *.*)|*.*";

    wxFileDialog dlg( this, _( "Import LCEDASYM" ), aLastPath, wxEmptyString, wildCard,
                      wxFD_OPEN | wxFD_FILE_MUST_EXIST );

    dlg.SetFilterIndex( lastFilterIndex );

    if( dlg.ShowModal() == wxID_CANCEL )
        return wxFileName();

    lastFilterIndex = dlg.GetFilterIndex();

    return wxFileName( dlg.GetPath() );
}

//wx Open Selected Dir dialog( -sxl)
wxString SYMBOL_EDIT_FRAME::OpenDirDialog( wxString strTip )
{
    wxDirDialog dlg( nullptr, strTip, "F:", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dlg.ShowModal() == wxID_OK )
    {
        return dlg.GetPath();
    }

    return "";
}

//wx Get All File in Dir( -sxl)
wxArrayString SYMBOL_EDIT_FRAME::GetAllFilesInDir( wxString strDir )
{
    wxDir         dir;
    wxArrayString fileLists;
    wxString      fileSpec = wxT( "*.esym" );
    wxString      fileSpecJson = wxT( "*.json" );
    int           numFilesFound;
    if( dir.Open( strDir ) )
    {
        numFilesFound = dir.GetAllFiles( strDir, &fileLists, fileSpec );
        numFilesFound = dir.GetAllFiles( strDir, &fileLists, fileSpecJson );
    }

    return fileLists;
}



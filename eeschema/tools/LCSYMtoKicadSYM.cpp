#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "LCSYMtoKicadSYM.h"
#include "nlohmann_json/nlohmann/json.hpp"
#include <wx/arrstr.h>
//#include <symbol_library.h>
#include <lib_symbol.h>
#include <wx/string.h>

#include <pin_type.h>
#include<lib_pin.h>
#include <lib_shape.h>
#include <default_values.h>
#include<lib_field.h>
#include <template_fieldnames.h>
#include <lib_text.h>


#include <map>
#include <wx/log.h>
#include <dsnlexer.h>
#include <eda_shape.h>
#include <memory>
//#include <symbol_editor_edit_tool.cpp>
//#include <dialog_field_properties.cpp>


//class LIB_FIELD;
//class LIB_PIN;
//class LIB_SHAPE;

using namespace std;
using nlohmann::json;

//立创专业版EDAsymbol文件数据扩大倍数
#define LCSYMExpendNum 100000 * 0.0254
//#define LCSYMLengthExpend 10 * 0.2

/**
 * Simple container to manage fill parameters.
 */
class FILL_PARAMS
{
public:
    FILL_T  m_FillType;
    COLOR4D m_Color;
};


LCSYMtoKicadSYM::LCSYMtoKicadSYM()
{
    m_symbolObj = nullptr;
    
}
LCSYMtoKicadSYM ::~LCSYMtoKicadSYM()
{

}

//解析立创EDA symbol文件
int LCSYMtoKicadSYM::importLCSYM( std::string strInFileFullPath, int fileType )
{
    int resultID = 0;
    if( 1 > strInFileFullPath.size() )
    {
        return resultID;
    }

    //分割symbol文件路径，获取文件名
    std::vector<std::string> vecStrTemp = Split( strInFileFullPath, "\\" );
    std::string              strSYMFileName = vecStrTemp.back();
    
    // 获取SYMBOL名，并新建symbol
    wxString strSYMName = "";
    size_t lastDotPos = strSYMFileName.find_last_of( '.' );
    // 使用 substr 函数截取子字符串
    strSYMName = wxString( strSYMFileName.substr( 0, lastDotPos ).c_str(), wxConvUTF8 );
    m_symbolObj = new LIB_SYMBOL( strSYMName );
    //m_symbolObj = new LIB_SYMBOL( wxName );

    //存储读取的每行数据
    vector<std::string> vecLCSYMLine;

    if( 0 == fileType )
    {
        resultID = readLCSYMFileByLine( strInFileFullPath, vecLCSYMLine );
    }
    else if( 1 == fileType )
    {
        resultID = readLCSYMFileByLineFromJson( strInFileFullPath, vecLCSYMLine );
    }
    resultID = parseLines( vecLCSYMLine );
   // m_symbolObj = new LIB_SYMBOL( wxName );
 
    //判断标识执行是否成功
    return resultID;
}



//按行读取立创封装文件
int LCSYMtoKicadSYM::readLCSYMFileByLine( std::string               strInFileFullPath,
                                          std::vector<std::string>& vecLCSYMLines )
{
    //函数执行结果
    int      resultID = 1;
    ifstream readFile;
    readFile.open( strInFileFullPath, ios::in );

    if( readFile.is_open() )
    {
        string str;
        int    i=0;
        //将每一行数据存入str
        while( getline( readFile, str ) )
        {
            vecLCSYMLines.push_back( str );
            std::cout << "Debug information: vecLCSYMLines = " << vecLCSYMLines[i] << std::endl;
            i++;
        }
    }
    else
    {
        //文件打开失败
        resultID = 2;
    }

    readFile.close();
    return resultID;
}

//按行读取立创封装文件
int LCSYMtoKicadSYM::readLCSYMFileByLineFromJson( std::string strInFileFullPath,
    std::vector<std::string>& vecLCSYMLines)
{
    int           result = 0;
    std::ifstream t( strInFileFullPath );
    std::string   strJson( ( std::istreambuf_iterator<char>( t ) ),
                           std::istreambuf_iterator<char>() );
    json          jsonRootObj = json::parse( strJson );
    if( jsonRootObj.contains( "code" ) )
    {
        string strCode = jsonRootObj["code"].get<string>();
    }
    if( jsonRootObj.contains( "device_info" ) )
    {
        json jsonDevice_info = jsonRootObj["device_info"].get<json>();

        if( jsonDevice_info.contains( "symbol_info" ) )
        {
            json jsonSYMInfor = jsonDevice_info["symbol_info"].get<json>();

            if( jsonSYMInfor.contains( "dataStr" ) )
            {
                string strSYMData = jsonSYMInfor["dataStr"].get<string>();
                cout << "strSYMData: " << strSYMData << endl;

                wxArrayString strList = wxSplit( strSYMData, '\n' );
                for( int i = 0; i < strList.size(); i++ )
                {
                    std::string stdStr = strList[i].ToStdString();
                    size_t      len = stdStr.length();
                    if( len > 0 && stdStr[len - 1] == '\r' )
                    {
                        // 删除末尾的 "\r"
                        stdStr.erase( len - 1 );
                    }
                    vecLCSYMLines.push_back( stdStr );
                }
            }
        }
    }

    return result;
}



//解析每一行数据
int LCSYMtoKicadSYM::parseLines( std::vector<std::string> vecLCSYMLines )
{
    int resultID = 0;
    //std::vector<LIB_PIN*> pins;
    LIB_PIN* pin;
    VECTOR2I symbolvalue_xy( LCSYMExpendNum * stof( "22" ),
                             LCSYMExpendNum * stof( "22" ) ); // 右上角坐标

    int  cout = 0;
    int  m_unit = 0;
    int m_convert = 1;

    std::vector<std::string>              vecLCSYMUnit; // 用于存储每个 "PART" 之间的数据
    std::vector<std::vector<std::string>> allUnits;     // 用于存储所有 "PART" 之间的数据
    bool storingData = false;                           // 用于标记是否正在存储数据
    for( int i = 0; i < vecLCSYMLines.size(); i++ )
    {
        //const std::string& line = vecLCSYMLines[i];
        if( vecLCSYMLines[i].find( "PART" ) != std::string::npos )
        {
            cout++;
            // 找到一个 "PART"，开始存储后面的数据
            storingData = true;
            vecLCSYMUnit.clear(); // 清空之前的数据,存储PART行
            vecLCSYMUnit.push_back( vecLCSYMLines[i] );
        }
        else if( storingData )
        {
            // 存储 "PART" 后面的每一行数据
            vecLCSYMUnit.push_back( vecLCSYMLines[i] );

            // 检查是否遇到下一个 "PART" 或到达末尾
            if( i + 1 < vecLCSYMLines.size()   && vecLCSYMLines[i + 1].find( "PART" ) != std::string::npos )
            {
                // 下一个 "PART" 出现，停止存储数据
                storingData = false;
                allUnits.push_back( vecLCSYMUnit );
            }
            else if( i == vecLCSYMLines.size() - 1 && storingData )
            {
                // 处理最后一个 "PART" 后面的数据
                allUnits.push_back( vecLCSYMUnit );
            }
        }
    }
    // 输出每个 "PART" 后面的数据
    // for( const std::vector<std::string>& unit : allUnits )
    for( int j = 0; j < allUnits.size() ; j++)
    {
        m_unit++;
        if( m_unit > 1 )
        {
            m_symbolObj->SetUnitCount( m_unit, false );
        }
        std::vector<std::string> unit = allUnits[j];
        for( int i = 0; i < unit.size(); i++ )
        {
            string strLine = unit[i];
            //去除首尾的 "[]" 和 "\""
            strLine = spp( strLine, "[", "" );
            strLine = spp( strLine, "]", "" );
            strLine = spp( strLine, "\"", "" );

            vector<string> vecSplit = Split( strLine, "," );

            //去除双引号
            vecStrAnalyse( vecSplit );

            //筛除无效数据
            if( 1 > vecSplit.size() )
            {
                continue;
            }

            if( strLine.size() > vecSplit[0].find( "PART" ) )
            {
                partOfvecSplits.push_back( vecSplit );
            }
            else if( strLine.size() > vecSplit[0].find( "ATTR" )
                     && ( strLine.size() > vecSplit[3].find( "Symbol" )
                          || strLine.size() > vecSplit[3].find( "Designator" ) ) )
            {
                partOfvecSplits.push_back( vecSplit );
            }
            else if( strLine.size() > vecSplit[0].find( "PIN" ) )
            {
                pinOfvecSplits.push_back( vecSplit );
            }
            else if( strLine.find( "ATTR" ) != std::string::npos
                     && ( vecSplit[3].find( "NAME" ) != std::string::npos
                          || vecSplit[3].find( "NUMBER" ) != std::string::npos
                          || vecSplit[3].find( "Pin Type" ) != std::string::npos ) )
            {
                pinOfvecSplits.push_back( vecSplit );
            }
            else if( strLine.size() > vecSplit[0].find( "ARC" ) )
            {
                parseARC( vecSplit, m_unit, m_convert );
            }
            else if( strLine.size() > vecSplit[0].find( "RECT" ) )
            {
                symbolvalue_xy= parseRECT( vecSplit, m_unit, m_convert );
            }
            else if( strLine.size() > vecSplit[0].find( "CIRCLE" ) )
            {
                parseCIRCLE( vecSplit, m_unit, m_convert );
            }
            else if( strLine.size() > vecSplit[0].find( "POLY" ) )
            {
                parsePOLY( vecSplit, m_unit, m_convert );
            }
            else if( strLine.size() > vecSplit[0].find( "TEXT" ) )
            {
                parseTEXT( vecSplit, m_unit, m_convert );
            }

            //存储PART数据
            if( ( i == unit.size() - 1 ) && partOfvecSplits.size() == 3 )
            {
                parsePART( partOfvecSplits, symbolvalue_xy );
                partOfvecSplits.clear();
            }
            else if( ( i == unit.size() - 1 ) && partOfvecSplits.size() == 1 )
            {
                parsePART( partOfvecSplits, symbolvalue_xy );
                partOfvecSplits.clear();
            }


            //存储引脚数据
            if( pinOfvecSplits.size() == 4 )
            {
                pin = parsePIN( pinOfvecSplits, m_unit, m_convert );
                m_symbolObj->AddDrawItem( pin, false );
                //pins.push_back( parsePIN( pinOfvecSplits,m_unit ) );
                pinOfvecSplits.clear();
            }
        }
    }
    return 0;
}


//解析PART
int LCSYMtoKicadSYM::parsePART( std::vector<std::vector<std::string>> vecOfVecStr,
                                           VECTOR2I&                             f_xy )
{
    int    resultID = 0;
    std::vector<std::string> vecStr;
    //wxString  wxReference;
    //wxString  wxName;
    //wxString  wxType = "";
    //SymbolProperty symbolProp;
    //wxString       wxName = symbolProp.wxName;
    //wxString       wxType = symbolProp.wxType;


    for( int i = 0; i < vecOfVecStr.size(); i++ )
    {
        vecStr = vecOfVecStr[i];
        if( vecStr[0] == "PART" )
        {
            wxName = wxString( vecStr[1].c_str(), wxConvUTF8 );
            //去字符串“.”前面的名称
            int dotPos = wxName.Find( '.', true );
            if( dotPos != wxNOT_FOUND )
                wxName.resize( dotPos );
            //如果有"/"替换为"_"
            if( wxName.Contains( "/" ) )
            {
                wxName.Replace( "/", "_" );
            }
            else if (wxName.Contains(":"))
            {
                wxName.Replace( ":", "_" );
            }
            else if( wxName.Contains( "*" ) )
            {
                wxName.Replace( "*", "_" );
            }
        }
        //else if( vecStr[3] == "Symbol" )
        //{
        //    wxReference = wxString( vecStr[4].c_str(), wxConvUTF8 );
        //}
        else if( vecStr[3] == "Designator" )
        {
            wxType = wxString( vecStr[4].c_str(), wxConvUTF8 );
        }
    }

    m_fieldIDsRead.clear();
    // Make sure the mandatory field IDs are reserved as already read,
    // the field parser will set the field IDs to the correct value if
    // the field name matches a mandatory field name
    for( int i = 0; i < MANDATORY_FIELDS; i++ )
        m_fieldIDsRead.insert( i );
    

    LIB_ID id( wxName, wxName );
    m_symbolObj->SetName( wxName );
    m_symbolObj->SetLibId( id );

    m_symbolObj->SetPinNameOffset( LCSYMExpendNum * 3.993 );
    m_symbolObj->SetShowPinNames( true );
    m_symbolObj->SetShowPinNumbers( true );
    m_symbolObj->SetExcludedFromBOM( false );
    m_symbolObj->SetExcludedFromBoard( false );

    std::map<wxString, wxString> myProperty;
    myProperty.insert( std::make_pair( "Reference", wxType ) );
    myProperty.insert( std::make_pair( "Value", wxName ) );
    myProperty.insert( std::make_pair( "Footprint", "" ) );
    myProperty.insert( std::make_pair( "Datasheet", "" ) );
    myProperty.insert( std::make_pair( "Description", "" ) );

    for( std::map<wxString, wxString>::iterator it = myProperty.begin(); it != myProperty.end();
         ++it )
    {
        wxString name = it->first;
        wxString value = it->second;

        setPROPERTY( name, value, f_xy );
    }

    return resultID;
}


//设置kicad的property
LIB_FIELD* LCSYMtoKicadSYM::setPROPERTY( const wxString& name, const wxString& value,
                                         VECTOR2I& f_xy )
{
    //设置属性
    LIB_FIELD* field = new LIB_FIELD( m_symbolObj, MANDATORY_FIELDS );
    fp_xy = f_xy;

    if( name == "Reference" )
    {
        //field->SetName( name );
        for( int ii = 0; ii < MANDATORY_FIELDS; ++ii )
        {
            if( !name.CmpNoCase( TEMPLATE_FIELDNAME::GetDefaultFieldName( ii ) ) )
            {
                field->SetId( ii );
                break;
            }
        }
        field->SetText( value );
        fp_xy.y = ( fp_xy.y + LCSYMExpendNum * stof( "12.7" ) );
    }
    else if( name == "Value" )
    {
        //field->SetName( name );
        for( int ii = 0; ii < MANDATORY_FIELDS; ++ii )
        {
            if( !name.CmpNoCase( TEMPLATE_FIELDNAME::GetDefaultFieldName( ii ) ) )
            {
                field->SetId( ii );
                break;
            }
        }
        field->SetText( value );
        fp_xy.y = ( fp_xy.y + LCSYMExpendNum * stof( "5.7" ) );
    }
    else
    {
        if( !field->IsMandatory() )
        {
            // field->SetName( name );
            // Correctly set the ID based on canonical (untranslated) field name
            // If ID is stored in the file (old versions), it will overwrite this
            for( int ii = 0; ii < MANDATORY_FIELDS; ++ii )
            {
                if( !name.CmpNoCase( TEMPLATE_FIELDNAME::GetDefaultFieldName( ii ) ) )
                {
                    field->SetId( ii );
                    break;
                }
            }
            field->SetText( value );
        }
    }
    //parsePROPERTY_TEXT( static_cast<EDA_TEXT*>( field.get() ) );
    
    field->SetPosition( fp_xy );
    field->SetTextAngle( EDA_ANGLE( 0.0, DEGREES_T ) );
    field->SetNameShown( false );
    field->SetCanAutoplace( true );

    LIB_FIELD* existingField;
    existingField = m_symbolObj->GetFieldById( field->GetId() );
    *existingField = *field;
    m_fieldIDsRead.insert( field->GetId() );
    return existingField;

    if( !existingField )
    {
        m_symbolObj->AddDrawItem( field, false );
        m_fieldIDsRead.insert( field->GetId() );
    }
    else
    {
        // We cannot handle 2 fields with the same name, so skip this one
        return nullptr;
    }
    //return field;
}


//修改kicad的property
void LCSYMtoKicadSYM::changePROPERTY( LIB_SYMBOL* curSymbol )
{
    LIB_FIELD* field_val = curSymbol->GetFieldById( 1 );
    field_val->SetText( wxName );
    //fp_xy.y = ( fp_xy.y + LCSYMExpendNum * stof( "5.7" ) );
    field_val->SetPosition( fp_xy );
    LIB_FIELD* field_ref = curSymbol->GetFieldById( 0 );
    field_ref->SetText( wxType );
    fp_xy.y = ( fp_xy.y + LCSYMExpendNum * stof( "7" ) );
    field_ref->SetPosition( fp_xy );
}


//判断LC引脚类型
ELECTRICAL_PINTYPE parsePinType( const wxString& pinType )
{
    if( pinType.Cmp( "IN" ) == 0 )
    {
        return ELECTRICAL_PINTYPE::PT_INPUT;
    }
    else if( pinType.Cmp( "OUT" ) == 0 )
    {
        return ELECTRICAL_PINTYPE::PT_OUTPUT;
    }
    else if( pinType.Cmp( "BI" ) == 0 )
    {
        return ELECTRICAL_PINTYPE::PT_BIDI;
    }
    else
    {
        //default:
        return ELECTRICAL_PINTYPE::PT_UNSPECIFIED;
    }
}


//解析PIN
LIB_PIN* LCSYMtoKicadSYM::parsePIN( std::vector<std::vector<std::string>> vecOfVecStr, int m_unit,
                                    int m_convert )
{
    int resultID = 0;
    std::vector<std::string> vecStr;
    LIB_PIN*  pin = new LIB_PIN( m_symbolObj );
    COLOR4D  color = COLOR4D::UNSPECIFIED;
    wxString  wxPinName;
    wxString  wxPinNumber;

    for( int i = 0; i < vecOfVecStr.size(); i++ )
    {
        vecStr = vecOfVecStr[i];

        if( vecStr[0] == "PIN" )
        {
            pin->SetUnit( m_unit );
            pin->SetConvert( m_convert );
            
            // Pin shape 没有对应，设置默认
            pin->LIB_PIN::SetShape( GRAPHIC_PINSHAPE::LINE );
            pin->SetVisible(true );

            //定义、设置引脚坐标
            VECTOR2I p_xy( 0, 0 );
            p_xy.x = LCSYMExpendNum * stof( vecStr[4] );
            p_xy.y = LCSYMExpendNum * stof( vecStr[5] );
            pin->SetPosition( p_xy );

            //Pin length
            int p_length = LCSYMExpendNum * stof( vecStr[6] );
            pin->SetLength( p_length );


            //Pin orientation of rotation
            int p_orientation = stoi( vecStr[7] );
            if( p_orientation == 0 )
            {
                pin->SetOrientation( PIN_ORIENTATION::PIN_RIGHT );
            }
            else if( p_orientation == 90 )
            {
                pin->SetOrientation( PIN_ORIENTATION::PIN_UP );
            }
            else if( p_orientation == 180 )
            {
                pin->SetOrientation( PIN_ORIENTATION::PIN_LEFT );
            }
            else if( p_orientation == 270 )
            {
                pin->SetOrientation( PIN_ORIENTATION::PIN_DOWN );
            }
            else
            {
                resultID = 2;
            }

        }
        else if( vecStr[3] == "NAME" )
        {
            wxPinName = wxString( vecStr[4].c_str(), wxConvUTF8 );
            //pin->SetName( wxPinName );
        }
        else if( vecStr[3] == "NUMBER" )
        {
            wxPinNumber = wxString( vecStr[4].c_str(), wxConvUTF8 );
            //pin->SetNumber( wxPinNumber );
        }
        else if( vecStr[3] == "Pin Type" )
        {
            m_pinType = vecStr[4];
            ELECTRICAL_PINTYPE pinType = parsePinType( m_pinType );
            pin->SetType( pinType );
        }
    }
    if (wxPinName == wxPinNumber)
    {
        pin->SetName( "~" );
        pin->SetNumber( wxPinNumber );
    }
    else
    {
        pin->SetName( wxPinName );
        pin->SetNumber( wxPinNumber );
    }

    return pin;
}



//解析ARC(弧线)
int LCSYMtoKicadSYM::parseARC( std::vector<std::string> vecStr, int m_unit, int m_convert )
{
    int resultID = 0;

    LIB_SHAPE* arc = new LIB_SHAPE( m_symbolObj, SHAPE_T::ARC );
    VECTOR2I   startPoint( 1, 0 ); // Initialize to a  arc just for safety
    VECTOR2I   midPoint( 1, 1 );
    VECTOR2I   endPoint( 0, 1 );
    

    //设置默认stroke
    STROKE_PARAMS stroke( schIUScale.MilsToIU( DEFAULT_LINE_WIDTH_MILS ), PLOT_DASH_TYPE::DEFAULT );
    //设置默认fill;
    FILL_PARAMS fill;
    fill.m_FillType = FILL_T::NO_FILL;
    fill.m_Color = COLOR4D::UNSPECIFIED;

    arc->SetUnit( m_unit );
    arc->SetConvert( m_convert );
    //private单元仅用于组成一个复杂符号,不会单独使用。
    arc->SetPrivate( false );

    startPoint.x = LCSYMExpendNum * stof( vecStr[2] );
    startPoint.y = LCSYMExpendNum * stof( vecStr[3] );
    midPoint.x = LCSYMExpendNum * stof( vecStr[4] );
    midPoint.y = LCSYMExpendNum * stof( vecStr[5] );
    endPoint.x = LCSYMExpendNum * stof( vecStr[6] );
    endPoint.y = LCSYMExpendNum * stof( vecStr[7] );

    arc->SetArcGeometry( startPoint, midPoint, endPoint );
    arc->SetStroke( stroke );
    arc->SetFillMode( fill.m_FillType );
    arc->SetFillColor( fill.m_Color );

    m_symbolObj->AddDrawItem( arc, false );
    return resultID;
}


    //解析RECT(矩形)
VECTOR2I LCSYMtoKicadSYM::parseRECT( std::vector<std::string> vecStr, int m_unit, int m_convert )
{
    int resultID = 0;
    LIB_SHAPE* rectangle = new LIB_SHAPE(m_symbolObj, SHAPE_T::RECTANGLE);
    VECTOR2I startPoint( 0, 0 ); // 左下角坐标
    VECTOR2I endPoint( 1, 1 );   // 右上角坐标
    STROKE_PARAMS stroke( schIUScale.MilsToIU(  DEFAULT_LINE_WIDTH_MILS/0.6 ),PLOT_DASH_TYPE::DEFAULT );
    FILL_PARAMS   fill;


    rectangle->SetUnit( m_unit );
    rectangle->SetConvert( m_convert );
    rectangle->SetPrivate( false );

    startPoint.x = LCSYMExpendNum * stof( vecStr[2] );
    startPoint.y = LCSYMExpendNum * stof( vecStr[3] );
    endPoint.x = LCSYMExpendNum * stof( vecStr[4] );
    endPoint.y = LCSYMExpendNum * stof( vecStr[5] );

    rectangle->SetPosition( startPoint );
    rectangle->SetEnd( endPoint );
    rectangle->SetStroke( stroke );
    rectangle->SetFillMode( fill.m_FillType );
    rectangle->SetFillColor( fill.m_Color );

    m_symbolObj->AddDrawItem( rectangle, false );
    return endPoint;
}

//解析CIRCLE(圆形)
int LCSYMtoKicadSYM::parseCIRCLE( std::vector<std::string> vecStr, int m_unit, int m_convert )
{
    int resultID = 0;
    LIB_SHAPE* circle = new LIB_SHAPE( m_symbolObj, SHAPE_T::CIRCLE );
    VECTOR2I   center( 0, 0 );
    int radius = 1; // defaulting to 0 could result in troublesome math....
    STROKE_PARAMS stroke( schIUScale.MilsToIU( DEFAULT_LINE_WIDTH_MILS ), PLOT_DASH_TYPE::DEFAULT );
    FILL_PARAMS   fill;

    circle->SetUnit( m_unit );
    circle->SetConvert( m_convert );
    circle->SetPrivate( false );

    center.x = LCSYMExpendNum * stof( vecStr[2] );
    center.y = LCSYMExpendNum * stof( vecStr[3] );
    radius = LCSYMExpendNum * stof( vecStr[4] );

    circle->SetStroke( stroke );
    circle->SetFillMode( fill.m_FillType );
    circle->SetFillColor( fill.m_Color );
    circle->SetCenter( center );
    circle->SetEnd( VECTOR2I( center.x + radius, center.y ) );

    m_symbolObj->AddDrawItem( circle, false );
    return 0;
}


    //解析POLY(多边形)
int LCSYMtoKicadSYM::parsePOLY( std::vector<std::string> vecStr, int m_unit, int m_convert )
{
    int resultID = 0;
    int numPointsToAdd = 0;
    LIB_SHAPE*    poly = new LIB_SHAPE( m_symbolObj, SHAPE_T::POLY );
    STROKE_PARAMS stroke( schIUScale.MilsToIU( DEFAULT_LINE_WIDTH_MILS ), PLOT_DASH_TYPE::DEFAULT );
    FILL_PARAMS   fill;


    poly->SetUnit( m_unit );
    poly->SetConvert( m_convert );
    poly->SetPrivate( false );


    for( int i = 0; i < vecStr.size(); i++ )
    {
        if( vecStr[i].find( "st" ) != std::string::npos )
        {
            numPointsToAdd = ( i - 7 ) / 2 + 2;

            for( int j = 0; j < numPointsToAdd; j++ )
            {
                VECTOR2I point;
                point.x = LCSYMExpendNum * stof( vecStr[2 + j * 2] );
                point.y = LCSYMExpendNum * stof( vecStr[3 + j * 2] );
                poly->AddPoint( point );
            }
        }
    }
    poly->SetStroke( stroke );
    poly->SetFillMode( fill.m_FillType );
    poly->SetFillColor( fill.m_Color );

    m_symbolObj->AddDrawItem( poly, false );
    return resultID;
}


    //解析TEXT
int LCSYMtoKicadSYM::parseTEXT( std::vector<std::string> vecStr, int m_unit, int m_convert )
{
    int       resultID = 0;

    LIB_TEXT* text = new LIB_TEXT( m_symbolObj );
    VECTOR2I  pointText( 0, 0 );
    double  angle = 0.0;

    text->SetUnit( m_unit );
    text->SetConvert( m_convert );
    text->SetPrivate( false );
    
    text->SetText( wxString( vecStr[5].c_str(), wxConvUTF8 ) );
    pointText.x = LCSYMExpendNum * stof( vecStr[2] );
    pointText.y = LCSYMExpendNum * stof( vecStr[3] );
    text->SetPosition( pointText );
    //angle
    //angle = stof( vecStr[4] );
    //text->SetTextAngle( EDA_ANGLE( angle, DEGREES_T ) );
    text->SetTextAngle( EDA_ANGLE( 0.0, DEGREES_T ) );
    
    m_symbolObj->AddDrawItem( text, false );
    return resultID;
}



//字符串分割
std::vector<std::string> LCSYMtoKicadSYM::Split( std::string strContext,
                                                     std::string StrDelimiter )
{
    vector<string> vecResult;
    if( strContext.empty() )
    {
        return vecResult;
    }
    if( StrDelimiter.empty() )
    {
        vecResult.push_back( strContext );
        return vecResult;
    }

    strContext += StrDelimiter;
    int iSize = strContext.size();
    for( int i = 0; i < iSize; i++ )
    {
        int iPos = strContext.find( StrDelimiter, i );
        if( iPos < iSize )
        {
            string strElement = strContext.substr( i, iPos - i );
            vecResult.push_back( strElement );
            i = iPos + StrDelimiter.size() - 1;
        }
    }
    return vecResult;
}


//去除vector容器内单个元素的前后双引号
void LCSYMtoKicadSYM::vecStrAnalyse( std::vector<std::string> vecStr )
{
    for( int i = 0; i < vecStr.size(); i++ )
    {
        if( vecStr[i].size() > vecStr[i].find( "\"" ) )
        {
            string str = vecStr[i].substr( 1, vecStr[i].size() - 2 );
            vecStr[i] = vecStr[i].substr( 1, vecStr[i].size() - 2 );
        }
    }
}

/// <summary>
/// 字符串批量替换
/// <param name="str">输入的文本</param>
/// <param name="a">目标文本</param>
/// <param name="b">替换内容</param>
/// <returns>替换好的文本</returns>
std::string LCSYMtoKicadSYM::spp( std::string str, std::string a, std::string b )
{
    int oldPos = 0;
    while( str.find( a, oldPos ) != -1 ) //在未被替换的文本中寻找目标文本
    {
        int start = str.find( a, oldPos ); //找到目标文本的起始下标
        str.replace( start, a.size(), b );
        oldPos = start + b.size(); //记录未替换文本的起始下标
    }
    return str;
}



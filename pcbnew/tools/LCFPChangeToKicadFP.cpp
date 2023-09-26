#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "LCFPChangeToKicadFP.h"
#include "footprint.h"
#include "pad.h"
#include "plugins/kicad/pcb_plugin.h"
#include "pcb_shape.h"
#include "zone.h"
#include "footprint_edit_frame.h"
#include "../plugins/kicad/pcb_parser.h"
#include <../../../thirdparty/nlohmann_json/nlohmann/json.hpp>
using namespace std;
using nlohmann::json;

//立创专业版EDA封装文件数据扩大倍数
#define LCFPExpendNum 1000000 * 0.0254

LCFPChangeToKicadFP::LCFPChangeToKicadFP( BOARD* board )
{
    m_LCFPSingleFullPath = "";
    m_board = board;
    m_KicadFPObj = nullptr;
}

LCFPChangeToKicadFP::~LCFPChangeToKicadFP()
{
}

//解析立创EDA专业版封装文件，并存储数据至Kicad数据结构
int LCFPChangeToKicadFP::importLCFP( string strInFileFullPath, int fileType )
{
    int resultID = 0;
    if( 1 > strInFileFullPath.size() )
    {
        return resultID;
    }

    m_KicadFPObj = new FOOTPRINT( m_board );
    std::vector<std::string> vecStrTemp = Split( strInFileFullPath, "\\" );
    std::string              strFPName = vecStrTemp.back();
    strFPName = strFPName.substr( 0, strFPName.size() - 5 );

    //设置Kicad封装所在文件夹名称和封装名称
    m_KicadFPObj->SetFPID( LIB_ID( "LCPROFP", strFPName ) );

    vector<std::string> vecLCFPLine;

    if( 0 == fileType )
    {
        resultID = readLCFPFileByLine( strInFileFullPath, vecLCFPLine );
    }
    else if( 1 == fileType )
    {
        resultID = readLCFPFileByLineFromJson( strInFileFullPath, vecLCFPLine );
    }

    resultID = parseLines( vecLCFPLine );

    return resultID;
}

//导出Kicad封装文件
int LCFPChangeToKicadFP::exportKicadFP( string strOutFileFullPath )
{
    return 1;
}

//按行读取立创封装文件
int LCFPChangeToKicadFP::readLCFPFileByLine( string               strInFileFullPath,
                                             vector<std::string>& vecLCFPLines )
{
    int      resultID = 1;
    ifstream readFile;
    readFile.open( strInFileFullPath, ios::in );

    if( readFile.is_open() )
    {
        string str;
        while( getline( readFile, str ) )
        {
            vecLCFPLines.push_back( str );
        }
    }
    else
    {
        resultID = 2;
    }

    readFile.close();

    return resultID;
}

//按行读取立创封装文件
int LCFPChangeToKicadFP::readLCFPFileByLineFromJson( std::string               strInFileFullPath,
                                                     std::vector<std::string>& vecLCFPLines )
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

        if( jsonDevice_info.contains( "footprint_info" ) )
        {
            json jsonFPInfor = jsonDevice_info["footprint_info"].get<json>();

            if( jsonFPInfor.contains( "dataStr" ) )
            {
                string strFPData = jsonFPInfor["dataStr"].get<string>();

                wxArrayString strList = wxSplit( strFPData, '\n' );
                for( int i = 0; i < strList.size(); i++ )
                {
                    wxString str = strList[i];
                    vecLCFPLines.push_back( strList[i].ToStdString() );
                }
            }
        }
    }

    return result;
}

//解析行数据集合
int LCFPChangeToKicadFP::parseLines( std::vector<std::string> vecLCFPLines )
{
    int resultID = 0;
    for( int i = 0; i < vecLCFPLines.size(); i++ )
    {
        string strLine = vecLCFPLines[i];

        //去除首尾的[]
        strLine = spp( strLine, "[", "" );
        strLine = spp( strLine, "]", "" );
        strLine = spp( strLine, "\"", "" );

        vector<string> vecSplit = Split( strLine, "," );

        ////去除双引号
        //vecStrAnalyse( vecSplit );

        //筛除无效数据
        if( 1 > vecSplit.size() )
        {
            continue;
        }
        else
        {
            if( strLine.size() <= vecSplit[1].find( "e" ) )
            {
                continue;
            }
        }

        //解析数据
        if( strLine.size() > vecSplit[0].find( "PAD" ) )
        {
            //解析PAD
            resultID = parsePad( vecSplit );
        }
        else if( strLine.size() > vecSplit[0].find( "POLY" ) )
        {
            //解析POLY
            resultID = parsePOLY( vecSplit );
        }
        else if( strLine.size() > vecSplit[0].find( "FILL" ) )
        {
            //解析FILL
            resultID = parseFILL( vecSplit );
        }
        else if( strLine.size() > vecSplit[0].find( "STRING" ) )
        {
            //解析STRING
            resultID = parseSTRING( vecSplit );
        }
        else if( strLine.size() > vecSplit[0].find( "REGION" ) )
        {
            //解析REGION
            resultID = parseREGION( vecSplit );
        }
        else if( strLine.size() > vecSplit[0].find( "ATTR" ) )
        {
            //解析REGION
            resultID = parseATTR( vecSplit );
        }
    }

    return resultID;
}

//解析PAD
int LCFPChangeToKicadFP::parsePad( std::vector<std::string> vecStr )
{
    int resultID = 0;
    if( 13 > vecStr.size() )
    {
        return resultID;
    }

    PAD* pad = new PAD( m_KicadFPObj );

    //设置pad编号
    pad->SetNumber( vecStr[5] );

    //获取焊盘是通孔还是贴片
    string padMakeType = vecStr[4];
    if( "12" == padMakeType )
    {
        pad->SetAttribute( PAD_ATTRIB::PTH );

        pad->SetLayerSet( LSET::AllCuMask() );
    }
    else
    {
        pad->SetAttribute( PAD_ATTRIB::SMD );

        if( 0 == LCLayerIDToKicadLayerID( padMakeType ) )
        {
            LSET copperT( 3, F_Cu, B_Mask, F_Mask );
            pad->SetLayerSet( copperT );
        }
        else if( 31 == LCLayerIDToKicadLayerID( padMakeType ) )
        {
            LSET copperT( 3, B_Cu, B_Mask, F_Mask );
            pad->SetLayerSet( copperT );
        }
    }

    //获取焊盘图元的类型
    VECTOR2I sz;
    VECTOR2I pt;
    string   strPadType = vecStr[10];

    //解析焊盘（无内孔）
    if( strPadType.size() > strPadType.find( "ELLIPSE" ) )
    {
        //圆形(外框)
        pad->SetShape( PAD_SHAPE::CIRCLE );
        sz.x = LCFPExpendNum * stof( vecStr[11] );
        sz.y = LCFPExpendNum * stof( vecStr[12] );
        pad->SetSize( sz );
        pt.x = LCFPExpendNum * stof( vecStr[6] );
        pt.y = -LCFPExpendNum * stof( vecStr[7] );
        pad->SetFPRelativePosition( pt );
        pad->SetOrientation( EDA_ANGLE( stof( vecStr[8] ), DEGREES_T ) );

        if( 28 < vecStr.size() )
        {
            if( CheckStringIsNumbers( vecStr[19] ) )
            {
                //阻焊扩展
                pad->SetLocalSolderMaskMargin( LCFPExpendNum * stof( vecStr[19] ) );
            }

            if( CheckStringIsNumbers( vecStr[20] ) )
            {
                //助焊扩展
                pad->SetLocalSolderPasteMargin( LCFPExpendNum * stof( vecStr[20] ) );
            }

            if( CheckStringIsNumbers( vecStr[25] ) && CheckStringIsNumbers( vecStr[26] ) )
            {
                switch( stoi( vecStr[25] ) )
                {
                case 0: pad->SetZoneConnection( ZONE_CONNECTION::THERMAL ); break;
                case 1: pad->SetZoneConnection( ZONE_CONNECTION::FULL ); break;
                case 2: pad->SetZoneConnection( ZONE_CONNECTION::NONE ); break;
                default: break;
                }
            }

            //发散间距
            if( CheckStringIsNumbers( vecStr[26] ) )
            {
                pad->SetThermalGap( LCFPExpendNum * stof( vecStr[26] ) );
            }

            //发散线宽
            if( CheckStringIsNumbers( vecStr[27] ) )
            {
                pad->SetThermalSpokeWidth( LCFPExpendNum * stof( vecStr[27] ) );
            }

            //发散角度
            if( CheckStringIsNumbers( vecStr[28] ) )
            {
                pad->SetThermalSpokeAngleDegrees( stof( vecStr[28] ) );
            }
        }


        m_KicadFPObj->Add( pad, ADD_MODE::APPEND );
    }
    else if( strPadType.size() > strPadType.find( "RECT" ) )
    {
        double FilletRadius = LCFPExpendNum * stof( vecStr[13] );
        if( 0 < FilletRadius )
        {
            //矩形(圆角矩形)
            pad->SetShape( PAD_SHAPE::ROUNDRECT );
            pad->SetRoundRectRadiusRatio( 0.01 * stof( vecStr[13] ) );
        }
        else
        {
            //矩形
            pad->SetShape( PAD_SHAPE::RECT );
        }


        sz.x = LCFPExpendNum * stof( vecStr[11] );
        sz.y = LCFPExpendNum * stof( vecStr[12] );
        pad->SetSize( sz );
        pt.x = LCFPExpendNum * stof( vecStr[6] );
        pt.y = -LCFPExpendNum * stof( vecStr[7] );
        pad->SetOrientation( EDA_ANGLE( stof( vecStr[8] ), DEGREES_T ) );
        pad->SetFPRelativePosition( pt );

        if( 28 < vecStr.size() )
        {
            if( CheckStringIsNumbers( vecStr[20] ) )
            {
                //阻焊扩展
                pad->SetLocalSolderMaskMargin( LCFPExpendNum * stof( vecStr[20] ) );
            }

            if( CheckStringIsNumbers( vecStr[21] ) )
            {
                //助焊扩展
                pad->SetLocalSolderPasteMargin( LCFPExpendNum * stof( vecStr[21] ) );
            }

            if( CheckStringIsNumbers( vecStr[25] ) && CheckStringIsNumbers( vecStr[26] ) )
            {
                switch( stoi( vecStr[25] ) )
                {
                case 0: pad->SetZoneConnection( ZONE_CONNECTION::THERMAL ); break;
                case 1: pad->SetZoneConnection( ZONE_CONNECTION::FULL ); break;
                case 2: pad->SetZoneConnection( ZONE_CONNECTION::NONE ); break;
                default: break;
                }
            }

            //发散间距
            if( CheckStringIsNumbers( vecStr[26] ) )
            {
                pad->SetThermalGap( LCFPExpendNum * stof( vecStr[26] ) );
            }

            //发散线宽
            if( CheckStringIsNumbers( vecStr[27] ) )
            {
                pad->SetThermalSpokeWidth( LCFPExpendNum * stof( vecStr[27] ) );
            }

            //发散角度
            if( CheckStringIsNumbers( vecStr[28] ) )
            {
                pad->SetThermalSpokeAngleDegrees( stof( vecStr[28] ) );
            }
        }

        m_KicadFPObj->Add( pad, ADD_MODE::APPEND );
    }
    else if( strPadType.size() > strPadType.find( "OVAL" ) )
    {
        //椭圆
        pad->SetShape( PAD_SHAPE::OVAL );
        sz.x = LCFPExpendNum * stof( vecStr[11] );
        sz.y = LCFPExpendNum * stof( vecStr[12] );
        pad->SetSize( sz );
        pt.x = LCFPExpendNum * stof( vecStr[6] );
        pt.y = -LCFPExpendNum * stof( vecStr[7] );
        pad->SetOrientation( EDA_ANGLE( stof( vecStr[8] ), DEGREES_T ) );
        pad->SetFPRelativePosition( pt );

        if( 28 < vecStr.size() )
        {
            if( CheckStringIsNumbers( vecStr[19] ) )
            {
                //阻焊扩展
                pad->SetLocalSolderMaskMargin( LCFPExpendNum * stof( vecStr[19] ) );
            }

            if( CheckStringIsNumbers( vecStr[20] ) )
            {
                //助焊扩展
                pad->SetLocalSolderPasteMargin( LCFPExpendNum * stof( vecStr[20] ) );
            }

            if( CheckStringIsNumbers( vecStr[25] ) && CheckStringIsNumbers( vecStr[26] ) )
            {
                switch( stoi( vecStr[25] ) )
                {
                case 0: pad->SetZoneConnection( ZONE_CONNECTION::THERMAL ); break;
                case 1: pad->SetZoneConnection( ZONE_CONNECTION::FULL ); break;
                case 2: pad->SetZoneConnection( ZONE_CONNECTION::NONE ); break;
                default: break;
                }
            }

            //发散间距
            if( CheckStringIsNumbers( vecStr[26] ) )
            {
                pad->SetThermalGap( LCFPExpendNum * stof( vecStr[26] ) );
            }

            //发散线宽
            if( CheckStringIsNumbers( vecStr[27] ) )
            {
                pad->SetThermalSpokeWidth( LCFPExpendNum * stof( vecStr[27] ) );
            }

            //发散角度
            if( CheckStringIsNumbers( vecStr[28] ) )
            {
                pad->SetThermalSpokeAngleDegrees( stof( vecStr[28] ) );
            }
        }


        m_KicadFPObj->Add( pad, ADD_MODE::APPEND );
    }


    //解析焊盘（有内孔）
    string strPadDriType = vecStr[9];
    string strPadOutType = vecStr[12];
    if( strPadDriType.size() > strPadDriType.find( "ROUND" ) && 19 < vecStr.size() )
    {
        //解析pad内孔（ROUND）
        pad->SetDrillShape( PAD_DRILL_SHAPE_CIRCLE );
        sz.x = LCFPExpendNum * stof( vecStr[10] );
        sz.y = LCFPExpendNum * stof( vecStr[11] );
        pad->SetDrillSize( sz );
        int MetalType = stof( vecStr[19] );
        if( 0 == MetalType )
        {
            //非金属化
            pad->SetAttribute( PAD_ATTRIB::NPTH );
        }

        if( 30 < vecStr.size() )
        {
            if( CheckStringIsNumbers( vecStr[22] ) )
            {
                //阻焊扩展
                pad->SetLocalSolderMaskMargin( LCFPExpendNum * stof( vecStr[22] ) );
            }

            if( CheckStringIsNumbers( vecStr[23] ) )
            {
                //助焊扩展
                pad->SetLocalSolderPasteMargin( LCFPExpendNum * stof( vecStr[23] ) );
            }

            if( CheckStringIsNumbers( vecStr[27] ) && CheckStringIsNumbers( vecStr[28] ) )
            {
                switch( stoi( vecStr[27] ) )
                {
                case 0: pad->SetZoneConnection( ZONE_CONNECTION::THERMAL ); break;
                case 1: pad->SetZoneConnection( ZONE_CONNECTION::FULL ); break;
                case 2: pad->SetZoneConnection( ZONE_CONNECTION::NONE ); break;
                default: break;
                }
            }


            //发散间距
            if( CheckStringIsNumbers( vecStr[28] ) )
            {
                pad->SetThermalGap( LCFPExpendNum * stof( vecStr[28] ) );
            }

            //发散线宽
            if( CheckStringIsNumbers( vecStr[29] ) )
            {
                pad->SetThermalSpokeWidth( LCFPExpendNum * stof( vecStr[29] ) );
            }

            //发散角度
            if( CheckStringIsNumbers( vecStr[30] ) )
            {
                pad->SetThermalSpokeAngleDegrees( stof( vecStr[30] ) );
            }
        }


        //解析pad外框
        if( strPadOutType.size() > strPadOutType.find( "ELLIPSE" ) )
        {
            pad->SetShape( PAD_SHAPE::CIRCLE );
            sz.x = LCFPExpendNum * stof( vecStr[13] );
            sz.y = LCFPExpendNum * stof( vecStr[14] );
            pad->SetSize( sz );
            pt.x = LCFPExpendNum * stof( vecStr[6] );
            pt.y = -LCFPExpendNum * stof( vecStr[7] );
            pad->SetFPRelativePosition( pt );
            pad->SetOrientation( EDA_ANGLE( stof( vecStr[8] ), DEGREES_T ) );
        }
        else if( strPadOutType.size() > strPadOutType.find( "RECT" ))
        {
            //矩形(圆角矩形)
            pad->SetShape( PAD_SHAPE::ROUNDRECT );
            sz.x = LCFPExpendNum * stof( vecStr[13] );
            sz.y = LCFPExpendNum * stof( vecStr[14] );
            pad->SetSize( sz );
            pt.x = LCFPExpendNum * stof( vecStr[6] );
            pt.y = -LCFPExpendNum * stof( vecStr[7] );
            pad->SetOrientation( EDA_ANGLE( stof( vecStr[8] ), DEGREES_T ) );
            pad->SetFPRelativePosition( pt );
            pad->SetRoundRectRadiusRatio( 0.01 * stof( vecStr[15] ) );
        }
        else if( strPadOutType.size() > strPadOutType.find( "OVAL" ))
        {
            //椭圆
            pad->SetShape( PAD_SHAPE::OVAL );
            sz.x = LCFPExpendNum * stof( vecStr[13] );
            sz.y = LCFPExpendNum * stof( vecStr[14] );
            pad->SetSize( sz );
            pt.x = LCFPExpendNum * stof( vecStr[6] );
            pt.y = -LCFPExpendNum * stof( vecStr[7] );
            pad->SetOrientation( EDA_ANGLE( stof( vecStr[8] ), DEGREES_T ) );
            pad->SetFPRelativePosition( pt );
        }

        m_KicadFPObj->Add( pad, ADD_MODE::APPEND );
    }
    else if( strPadDriType.size() > strPadDriType.find( "SLOT" ) && 19 < vecStr.size() )
    {
        //解析pad内孔（SLOT）
        pad->SetDrillShape( PAD_DRILL_SHAPE_OBLONG );

        int drillAngle = 0;
        if( CheckStringIsNumbers( vecStr[18] ) )
        {
            drillAngle = stoi( vecStr[18] );
        }

        switch( drillAngle )
        {
        case 90:
        case -90:
        case 270:
        case -270:
            sz.x = LCFPExpendNum * stof( vecStr[11] );
            sz.y = LCFPExpendNum * stof( vecStr[10] );
            break;
        default:
            sz.x = LCFPExpendNum * stof( vecStr[10] );
            sz.y = LCFPExpendNum * stof( vecStr[11] );
            break;
        }
        pad->SetDrillSize( sz );

        int MetalType = stof( vecStr[19] );
        if( 0 == MetalType )
        {
            //非金属化
            pad->SetAttribute( PAD_ATTRIB::NPTH );
        }

        if( 28 < vecStr.size() )
        {
            if( CheckStringIsNumbers( vecStr[22] ) )
            {
                //阻焊扩展
                pad->SetLocalSolderMaskMargin( LCFPExpendNum * stof( vecStr[22] ) );
            }

            if( CheckStringIsNumbers( vecStr[23] ) )
            {
                //助焊扩展
                pad->SetLocalSolderPasteMargin( LCFPExpendNum * stof( vecStr[23] ) );
            }

            if( CheckStringIsNumbers( vecStr[27] ) && CheckStringIsNumbers( vecStr[28] ) )
            {
                switch( stoi( vecStr[27] ) )
                {
                case 0: pad->SetZoneConnection( ZONE_CONNECTION::THERMAL ); break;
                case 1: pad->SetZoneConnection( ZONE_CONNECTION::FULL ); break;
                case 2: pad->SetZoneConnection( ZONE_CONNECTION::NONE ); break;
                default: break;
                }
            }


            //发散间距
            if( CheckStringIsNumbers( vecStr[27] ) )
            {
                pad->SetThermalGap( LCFPExpendNum * stof( vecStr[27] ) );
            }

            //发散线宽
            if( CheckStringIsNumbers( vecStr[28] ) )
            {
                pad->SetThermalSpokeWidth( LCFPExpendNum * stof( vecStr[28] ) );
            }

            //发散角度
            if( CheckStringIsNumbers( vecStr[29] ) )
            {
                pad->SetThermalSpokeAngleDegrees( stof( vecStr[29] ) );
            }
        }


        //解析pad外框
        if( strPadOutType.size() > strPadOutType.find( "ELLIPSE" ) )
        {
            pad->SetShape( PAD_SHAPE::CIRCLE );
            sz.x = LCFPExpendNum * stof( vecStr[13] );
            sz.y = LCFPExpendNum * stof( vecStr[14] );
            pad->SetSize( sz );
            pt.x = LCFPExpendNum * stof( vecStr[6] );
            pt.y = -LCFPExpendNum * stof( vecStr[7] );
            pad->SetOrientation( EDA_ANGLE( stof( vecStr[8] ), DEGREES_T ) );
            pad->SetFPRelativePosition( pt );
        }
        else if( strPadOutType.size() > strPadOutType.find( "RECT" ) )
        {
            //矩形(圆角矩形)
            pad->SetShape( PAD_SHAPE::ROUNDRECT );
            sz.x = LCFPExpendNum * stof( vecStr[13] );
            sz.y = LCFPExpendNum * stof( vecStr[14] );
            pad->SetSize( sz );
            pt.x = LCFPExpendNum * stof( vecStr[6] );
            pt.y = -LCFPExpendNum * stof( vecStr[7] );
            pad->SetOrientation( EDA_ANGLE( stof( vecStr[8] ), DEGREES_T ) );
            pad->SetFPRelativePosition( pt );
            pad->SetRoundRectRadiusRatio( 0.01 * stof( vecStr[15] ) );
        }
        else if( strPadOutType.size() > strPadOutType.find( "OVAL" ) )
        {
            //椭圆
            pad->SetShape( PAD_SHAPE::OVAL );
            sz.x = LCFPExpendNum * stof( vecStr[13] );
            sz.y = LCFPExpendNum * stof( vecStr[14] );
            pad->SetSize( sz );
            pt.x = LCFPExpendNum * stof( vecStr[6] );
            pt.y = -LCFPExpendNum * stof( vecStr[7] );
            pad->SetOrientation( EDA_ANGLE( stof( vecStr[8] ), DEGREES_T ) );
            pad->SetFPRelativePosition( pt );
        }

        m_KicadFPObj->Add( pad, ADD_MODE::APPEND );
    }

    return resultID;
}

//解析POLY
int LCFPChangeToKicadFP::parsePOLY( std::vector<std::string> vecStr )
{
    int resultID = 0;
    if( 7 > vecStr.size() )
    {
        return resultID;
    }

    PCB_SHAPE* shape = new PCB_SHAPE( m_KicadFPObj );
    VECTOR2I   cp;
    VECTOR2I   sp;
    VECTOR2I   ep;
    VECTOR2I   mp;

    string shapeLayerType = vecStr[4];
    if( 1000 != LCLayerIDToKicadLayerID( shapeLayerType ) )
    {
        shape->SetLayer( (PCB_LAYER_ID) LCLayerIDToKicadLayerID( shapeLayerType ) );
    }
    else
    {
        return 0;
    }

    //获取POLY类型1
    string POLYType = vecStr[6];
    if( "CIRCLE" == POLYType && 9 < vecStr.size())
    {
        //线条圆
        shape->SetShape( SHAPE_T::CIRCLE );
        cp.x = LCFPExpendNum * stof( vecStr[7] );
        cp.y = -LCFPExpendNum * stof( vecStr[8] );
        shape->SetStart( cp );
        ep.x = LCFPExpendNum * ( stof( vecStr[7] ) + stof( vecStr[9] ) );
        ep.y = -LCFPExpendNum * stof( vecStr[8] );
        shape->SetEnd( ep );
        shape->SetWidth( LCFPExpendNum * stof( vecStr[5] ) );
        m_KicadFPObj->Add( shape, ADD_MODE::APPEND, true );
    }
    else if( "R" == POLYType && 9 < vecStr.size() )
    {
        //线条矩形
        shape->SetShape( SHAPE_T::RECT );
        cp.x = LCFPExpendNum * stof( vecStr[7] );
        cp.y = -LCFPExpendNum * stof( vecStr[8] );
        shape->SetStart( cp );
        ep.x = LCFPExpendNum * ( stof( vecStr[7] ) + stof( vecStr[9] ) );
        ep.y = -LCFPExpendNum * ( stof( vecStr[8] ) + stof( vecStr[9] ) );
        shape->SetEnd( ep );
        shape->SetWidth( LCFPExpendNum * stof( vecStr[5] ) );
        m_KicadFPObj->Add( shape, ADD_MODE::APPEND, true );
    }

    //获取POLY类型2
    string POLYTypeEx = vecStr[8];
    if( "L" == POLYTypeEx && 10 < vecStr.size() )
    {
        //线条多边形
        std::vector<VECTOR2I> aPoints;

        if( 13 > vecStr.size() )
        {
            //单条线段起点
            shape->SetShape( SHAPE_T::SEGMENT );
            sp.x = LCFPExpendNum * stof( vecStr[6] );
            sp.y = -LCFPExpendNum * stof( vecStr[7] );
            ep.x = LCFPExpendNum * stof( vecStr[9] );
            ep.y = -LCFPExpendNum * stof( vecStr[10] );
            shape->SetStart( sp );
            shape->SetEnd( ep );
        }
        else
        {
            shape->SetShape( SHAPE_T::POLY );
            VECTOR2I p;
            for( int ip = 9; ip < vecStr.size() - 2; ip += 2 )
            {
                p.x = LCFPExpendNum * ( stof( vecStr[ip] ) );
                p.y = -LCFPExpendNum * ( stof( vecStr[ip + 1] ) );
                aPoints.push_back( p );
            }
            shape->SetPolyPoints( aPoints );
        }


        shape->SetWidth( LCFPExpendNum * stof( vecStr[5] ) );

        m_KicadFPObj->Add( shape, ADD_MODE::APPEND, true );
    }
    else if( "ARC" == POLYTypeEx && 11 < vecStr.size() )
    {
        //线条两点圆弧
        shape->SetShape( SHAPE_T::ARC );
        sp.x = LCFPExpendNum * stof( vecStr[6] );
        sp.y = LCFPExpendNum * stof( vecStr[7] );

        ep.x = LCFPExpendNum * stof( vecStr[10] );
        ep.y = LCFPExpendNum * stof( vecStr[11] );

        shape->SetWidth( LCFPExpendNum * stof( vecStr[5] ) );
        double mx, my;
        GeometryUVExt::GetExt().MidPointOfArcEx( sp.x, sp.y, ep.x, ep.y, abs( stof( vecStr[9] ) ),
                                                 mx, my );
        mp.x = mx;
        mp.y = my;

        shape->SetCenter( mp );
        if( 0 < stof( vecStr[9] ) )
        {
            shape->SetStart( sp );
            shape->SetEnd( ep );
        }
        else
        {
            shape->SetStart( ep );
            shape->SetEnd( sp );
        }

        m_KicadFPObj->Add( shape, ADD_MODE::APPEND, true );
    }
    else if( "CARC" == POLYTypeEx && 11 < vecStr.size() )
    {
        //线条中心圆弧
        shape->SetShape( SHAPE_T::ARC );
        ep.x = LCFPExpendNum * stof( vecStr[6] );
        ep.y = LCFPExpendNum * stof( vecStr[7] );

        sp.x = LCFPExpendNum * stof( vecStr[10] );
        sp.y = LCFPExpendNum * stof( vecStr[11] );
        shape->SetWidth( LCFPExpendNum * stof( vecStr[5] ) );

        double mx, my;
        GeometryUVExt::GetExt().MidPointOfArcEx( sp.x, sp.y, ep.x, ep.y, abs( stof( vecStr[9] ) ),
                                                 mx, my );
        mp.x = mx;
        mp.y = my;

        shape->SetCenter( mp );
        if( 0 < stof( vecStr[9] ) )
        {
            shape->SetStart( ep );
            shape->SetEnd( sp );
        }
        else
        {
            shape->SetStart( sp );
            shape->SetEnd( ep );
        }

        m_KicadFPObj->Add( shape, ADD_MODE::APPEND, true );
    }

    return resultID;
}

//解析FILL
int LCFPChangeToKicadFP::parseFILL( std::vector<std::string> vecStr )
{
    int resultID = 0;
    if( 10 > vecStr.size() )
    {
        return resultID;
    }

    //立创专业版EDA封装中 填充区域和挖槽区域同属于FILL，区别标识就是按层ID区分，
    //填充区域 放在顶层（层id：1），挖槽区域放在多层（层id：12），在封装中填充区域和
    //禁止区域常出现在PCB板内，几乎不会在封装中出现，因此禁止区域和填充区域在此不
    //做转换处理，对于立创封装FILL标识的，目前只转换挖槽区域（FILL+层id（12）确认）
    PCB_SHAPE* shape = new PCB_SHAPE( m_KicadFPObj );
    VECTOR2I   cp;
    VECTOR2I   sp;
    VECTOR2I   ep;

    string shapeLayerType = vecStr[4];
    if( "12" == shapeLayerType )
    {
        shape->SetLayerSet( Edge_Cuts );
    }
    else
    {
        return 0;
    }

    //获取FILL类型1
    string FilType = vecStr[7];
    if( "CIRCLE" == FilType && 10 < vecStr.size() )
    {
        //填充圆
        shape->SetShape( SHAPE_T::CIRCLE );
        cp.x = LCFPExpendNum * stof( vecStr[8] );
        cp.y = -LCFPExpendNum * stof( vecStr[9] );
        shape->SetCenter( cp );
        ep.x = LCFPExpendNum * ( stof( vecStr[8] ) + stof( vecStr[10] ) );
        ep.y = -LCFPExpendNum * stof( vecStr[9] );
        shape->SetEnd( ep );
        shape->SetFilled( true );

        m_KicadFPObj->Add( shape, ADD_MODE::APPEND, true );
    }
    else if( "R" == FilType && 11 < vecStr.size() )
    {
        //填充矩形
        shape->SetShape( SHAPE_T::RECT );
        cp.x = LCFPExpendNum * stof( vecStr[8] );
        cp.y = -LCFPExpendNum * stof( vecStr[9] );
        shape->SetStart( cp );
        ep.x = LCFPExpendNum * ( stof( vecStr[8] ) + stof( vecStr[10] ) );
        ep.y = -LCFPExpendNum * ( stof( vecStr[9] ) + stof( vecStr[11] ) );
        shape->SetEnd( ep );

        shape->SetFilled( true );

        m_KicadFPObj->Add( shape, ADD_MODE::APPEND, true );
    }

    //获取FILL类型2
    string FilTypeEx = vecStr[9];
    if( "L" == FilTypeEx)
    {
        //填充多边形
        std::vector<VECTOR2I> aPoints;
        shape->SetShape( SHAPE_T::POLY );
        VECTOR2I p;
        for( int ip = 10; ip < vecStr.size() - 2; ip += 2 )
        {
            if( !CheckStringIsNumbers( vecStr[ip] ) )
            {
                return 0;
            }
            p.x = LCFPExpendNum * ( stof( vecStr[ip] ) );
            p.y = -LCFPExpendNum * ( stof( vecStr[ip + 1] ) );
            aPoints.push_back( p );
        }

        shape->SetPolyPoints( aPoints );
        shape->SetFilled( true );
        m_KicadFPObj->Add( shape, ADD_MODE::APPEND, true );
    }

    return resultID;
}

//解析STRING
int LCFPChangeToKicadFP::parseSTRING( std::vector<std::string> vecStr )
{
    int resultID = 0;
    if( 9 > vecStr.size() )
    {
        return resultID;
    }
    VECTOR2I  cp, size;
    PCB_TEXT* text = new PCB_TEXT( m_KicadFPObj, PCB_TEXT::TEXT_is_DIVERS );

    string textLayerType = vecStr[3];
    if( 1000 != LCLayerIDToKicadLayerID( textLayerType ) )
    {
        text->SetLayer( (PCB_LAYER_ID) LCLayerIDToKicadLayerID( textLayerType ) );
    }
    else
    {
        return 0;
    }

    cp.x = LCFPExpendNum * ( stof( vecStr[4] ) );
    cp.y = -LCFPExpendNum * ( stof( vecStr[5] ) );
    text->SetTextPos( cp );
    text->SetText( vecStr[6] );
    size.x = LCFPExpendNum * ( stof( vecStr[8] ) );
    size.y = LCFPExpendNum * ( stof( vecStr[8] ) );
    text->SetTextSize( size );
    text->SetTextThickness( 0.3 );
    m_KicadFPObj->Add( text, ADD_MODE::APPEND, true );
    return resultID;
}

//解析REGION
int LCFPChangeToKicadFP::parseREGION( std::vector<std::string> vecStr )
{
    //针对立创专业版疯转中的禁止区域不做转换处理
    return 0;

    int resultID = 0;
    if( 4 > vecStr.size() )
    {
        return resultID;
    }

    ZONE*    zone = new ZONE( m_KicadFPObj );
    VECTOR2I cp;
    VECTOR2I sp;
    VECTOR2I ep;

    string zoneLayerType = vecStr[3];
    if( 1000 != LCLayerIDToKicadLayerID( zoneLayerType ) )
    {
        zone->SetLayer( (PCB_LAYER_ID) LCLayerIDToKicadLayerID( zoneLayerType ) );
    }
    else
    {
        return 0;
    }

    //获取FILL类型1
    string RegionType = "";
    int    RegionTypeIndex = 0;
    for( int num = 0; num < vecStr.size(); num++ )
    {
        if( "CIRCLE" == vecStr[num] )
        {
            RegionType = "CIRCLE";
            RegionTypeIndex = num;
        }
        else if( "R" == vecStr[num] )
        {
            RegionType = "R";
            RegionTypeIndex = num;
        }
        else if( "L" == vecStr[num] )
        {
            RegionType = "L";
            RegionTypeIndex = num;
        }
    }


    if( "CIRCLE" == RegionType )
    {
        //填充圆
        PCB_SHAPE* shape = new PCB_SHAPE( m_KicadFPObj );
        shape->SetShape( SHAPE_T::CIRCLE );
        cp.x = LCFPExpendNum * stof( vecStr[RegionTypeIndex + 1] );
        cp.y = LCFPExpendNum * stof( vecStr[RegionTypeIndex + 2] );
        shape->SetCenter( cp );
        ep.x = LCFPExpendNum
               * ( stof( vecStr[RegionTypeIndex + 1] ) + stof( vecStr[RegionTypeIndex + 3] ) );
        ep.y = LCFPExpendNum * stof( vecStr[RegionTypeIndex + 2] );
        shape->SetEnd( ep );
        shape->SetFilled( true );

        m_KicadFPObj->Add( shape, ADD_MODE::APPEND, true );
    }
    else if( "R" == RegionType )
    {
        //填充矩形
        PCB_SHAPE* shape = new PCB_SHAPE( m_KicadFPObj );
        shape->SetShape( SHAPE_T::RECT );
        cp.x = LCFPExpendNum * stof( vecStr[RegionTypeIndex + 1] );
        cp.y = LCFPExpendNum * stof( vecStr[RegionTypeIndex + 2] );
        shape->SetStart( cp );
        ep.x = LCFPExpendNum
               * ( stof( vecStr[RegionTypeIndex + 1] ) + stof( vecStr[RegionTypeIndex + 3] ) );
        ep.y = LCFPExpendNum
               * ( stof( vecStr[RegionTypeIndex + 2] ) + stof( vecStr[RegionTypeIndex + 4] ) );
        shape->SetEnd( ep );
        shape->SetFilled( true );

        m_KicadFPObj->Add( shape, ADD_MODE::APPEND, true );
    }

    //获取FILL类型2
    if( "L" == RegionType )
    {
        std::vector<VECTOR2I> aPoints;
        zone->SetZoneName( vecStr[1] );
        zone->SetFillMode( ZONE_FILL_MODE::HATCH_PATTERN );
        zone->SetThermalReliefGap( 0.5 );
        zone->SetIsRuleArea( true );

        zone->SetDoNotAllowCopperPour( false );
        zone->SetDoNotAllowVias( false );
        zone->SetDoNotAllowTracks( false );
        zone->SetDoNotAllowPads( false );
        zone->SetDoNotAllowFootprints( false );

        int typeNum = RegionTypeIndex - 7;
        switch( typeNum )
        {
        case 1:
            if( CheckStringIsNumbers( vecStr[5] ) )
            {
                switch( stoi( vecStr[5] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            break;
        case 2:
            if( CheckStringIsNumbers( vecStr[5] ) )
            {
                switch( stoi( vecStr[5] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            if( CheckStringIsNumbers( vecStr[6] ) )
            {
                switch( stoi( vecStr[6] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            break;
        case 3:
            break;
            if( CheckStringIsNumbers( vecStr[5] ) )
            {
                switch( stoi( vecStr[5] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            if( CheckStringIsNumbers( vecStr[6] ) )
            {
                switch( stoi( vecStr[6] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            if( CheckStringIsNumbers( vecStr[7] ) )
            {
                switch( stoi( vecStr[7] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
        case 4:
            break;
            if( CheckStringIsNumbers( vecStr[5] ) )
            {
                switch( stoi( vecStr[5] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            if( CheckStringIsNumbers( vecStr[6] ) )
            {
                switch( stoi( vecStr[6] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            if( CheckStringIsNumbers( vecStr[7] ) )
            {
                switch( stoi( vecStr[7] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            if( CheckStringIsNumbers( vecStr[8] ) )
            {
                switch( stoi( vecStr[8] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
        case 5:
            break;
            if( CheckStringIsNumbers( vecStr[5] ) )
            {
                switch( stoi( vecStr[5] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            if( CheckStringIsNumbers( vecStr[6] ) )
            {
                switch( stoi( vecStr[6] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            if( CheckStringIsNumbers( vecStr[7] ) )
            {
                switch( stoi( vecStr[7] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            if( CheckStringIsNumbers( vecStr[8] ) )
            {
                switch( stoi( vecStr[8] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
            if( CheckStringIsNumbers( vecStr[9] ) )
            {
                switch( stoi( vecStr[9] ) )
                {
                case 2: zone->SetDoNotAllowFootprints( true ); break;
                case 5: zone->SetDoNotAllowTracks( true ); break;
                case 6: break;
                case 7: zone->SetDoNotAllowCopperPour( true ); break;
                case 8: break;

                default: break;
                }
            }
        default: break;
        }

        VECTOR2I p;
        for( int ip = RegionTypeIndex + 1; ip < vecStr.size() - 2; ip += 2 )
        {
            p.x = LCFPExpendNum * ( stof( vecStr[ip] ) );
            p.y = LCFPExpendNum * ( stof( vecStr[ip + 1] ) );
            aPoints.push_back( p );
        }

        zone->AddPolygon( aPoints );

        m_KicadFPObj->Add( zone, ADD_MODE::APPEND, true );
    }


    return resultID;
}

//解析ATTR
int LCFPChangeToKicadFP::parseATTR( std::vector<std::string> vecStr )
{
    int resultID = 0;
    if( 9 > vecStr.size() )
    {
        return resultID;
    }

   if( "Footprint" == vecStr[7] )
    {
       VECTOR2I  posUser, posVALUE, posREFERENCE, size;
       BOX2I     box = m_KicadFPObj->GetBoundingBox();
       PCB_TEXT* textUser = new PCB_TEXT( m_KicadFPObj, PCB_TEXT::TEXT_is_DIVERS );
       size.x = LCFPExpendNum * 30;
       size.y = LCFPExpendNum * 30;

       textUser->SetText( "${REFERENCE}" );
       posUser.x = box.GetCenter().x;
       posUser.y = box.GetCenter().y;
       textUser->SetTextPos( posUser );
       textUser->SetLayer( F_Fab );
       textUser->SetTextSize( size );
       m_KicadFPObj->Add( textUser, ADD_MODE::APPEND, true );

       m_KicadFPObj->SetReference( "REF**" );
       m_KicadFPObj->SetValue( vecStr[8] );

       posVALUE.x = box.GetCenter().x;
       posVALUE.y = box.GetCenter().y + 0.7 * box.GetHeight();
       posREFERENCE.x = box.GetCenter().x;
       posREFERENCE.y = box.GetCenter().y - 0.7 * box.GetHeight();

       //
       m_KicadFPObj->Value().SetTextPos( posVALUE );
       m_KicadFPObj->Reference().SetTextPos( posREFERENCE );

       m_KicadFPObj->Value().SetLayer(F_Fab);
       m_KicadFPObj->Reference().SetLayer( F_SilkS );

       m_KicadFPObj->Value().SetTextSize( size );
       m_KicadFPObj->Reference().SetTextSize( size );
    }

    return resultID;
}

//判断字符串是否全为数字
bool LCFPChangeToKicadFP::CheckStringIsNumbers( std::string str )
{
    bool result = false;
    result = ( "" == str ) ? false : true;

    for( int i = 0; i < str.size(); i++ )
    {
        //_与.  ？
        if( ( str[i] >= '0' && str[i] <= '9' ) || '-' == str[i] || '.' == str[i] )
        {
            continue;
        }
        else
        {
            return false;
        }
    }

    return result;
}

//字符串分割
std::vector<std::string> LCFPChangeToKicadFP::Split( std::string strContext,
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
void LCFPChangeToKicadFP::vecStrAnalyse( std::vector<std::string> vecStr )
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
/// </summary>
/// <param name="str">输入的文本</param>
/// <param name="a">目标文本</param>
/// <param name="b">替换内容</param>
/// <returns>替换好的文本</returns>
std::string LCFPChangeToKicadFP::spp( std::string str, std::string a, std::string b )
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

//立创专业版EDA封装层ID转换为KiCad的封装层ID
int LCFPChangeToKicadFP::LCLayerIDToKicadLayerID( std::string layerID )
{
    int resultID = 1000;
    if( CheckStringIsNumbers( layerID ) )
    {
        int layID = stoi( layerID );
        switch( layID )
        {
        case 1: resultID = 0; break;//顶层铜
        case 2: resultID = 31; break;//底层铜
        case 3: resultID = 37; break;//顶层丝印层
        case 4: resultID = 36; break;//底层丝印层
        case 5: resultID = 39; break;//顶层阻焊层
        case 6: resultID = 38; break;//底层阻焊层
        case 7: resultID = 35; break;//顶层锡膏层（顶层助焊层）
        case 8: resultID = 34; break;//底层锡膏层（底层助焊层）
        case 12: resultID = 0; break;//立创（多层）
        default: break;
        }
    }

    return resultID;
}


////////////辅助类///////////////////////
// GeometryUVExt.cpp
// by sxl 2019/12/18
/////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// EpsilonState
//////////////////////////////////////////////////////////////////////////
EpsilonState::EpsilonState( double setEpsilon ) : m_Ext( GeometryUVExt::GetExt() )
{
    m_LastEpsilon = m_Ext.GetEpsilon();
    m_Ext.SetEpsilon( setEpsilon );
}
EpsilonState::~EpsilonState()
{
    m_Ext.SetEpsilon( m_LastEpsilon );
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
GeometryUVExt::GeometryUVExt()
{
    m_Epsilon = Epsilon;
    SinTable = new double[TrigTableSize];
    CosTable = new double[TrigTableSize];
    TanTable = new double[TrigTableSize];
    for( unsigned int i = 0; i < TrigTableSize; ++i )
    {
        SinTable[i] = sin( ( 1.0 * i ) * _PIDiv180 );
        CosTable[i] = cos( ( 1.0 * i ) * _PIDiv180 );
        TanTable[i] = tan( ( 1.0 * i ) * _PIDiv180 );
    }
}

GeometryUVExt& GeometryUVExt::GetExt()
{
    static GeometryUVExt theExt;
    return theExt;
}

GeometryUVExt::~GeometryUVExt()
{
}

bool GeometryUVExt::IsCollinear( double x1, double y1, double x2, double y2, double x3, double y3 )
{
    return IsEqual( ( x2 - x1 ) * ( y3 - y1 ), ( x3 - x1 ) * ( y2 - y1 ) );
}

bool GeometryUVExt::IsEqual( double v1, double v2 )
{
    return fabs( v1 - v2 ) < m_Epsilon ? TRUE : FALSE;
}

bool GeometryUVExt::IsGreaterEqual( double v1, double v2 )
{
    return v1 > v2 || IsEqual( v1, v2 );
}

bool GeometryUVExt::IsLessEqual( double v1, double v2 )
{
    return v1 < v2 || IsEqual( v1, v2 );
}

bool GeometryUVExt::IsGreater( double v1, double v2 )
{
    return v1 > v2 && !IsEqual( v1, v2 );
}

bool GeometryUVExt::IsLess( double v1, double v2 )
{
    return v1 < v2 && !IsEqual( v1, v2 );
}

void GeometryUVExt::double5( double& v )
{
    const long rate = 100000;
    double     c = floor( v * rate + 0.5 ) / rate;
    v = c;
}
bool GeometryUVExt::Is90Degree( double angle )
{
    return IsTimeOfDegree( angle, 90 ) && !IsTimeOfDegree( angle, 180 );
}

bool GeometryUVExt::Is180Degree( double angle )
{
    return IsTimeOfDegree( angle, 180 );
}

bool GeometryUVExt::IsTimeOfDegree( double angle, int intDegree )
{
    if( intDegree == 0 )
    {
        int intValue = ceil( angle );
        return intValue % 360 == 0;
    }
    if( IsEqual( ceil( angle ), angle ) )
    {
        int intValue = ceil( angle );
        return intValue % intDegree == 0;
    }
    else if( IsEqual( floor( angle ), angle ) )
    {
        int intValue = floor( angle );
        return intValue % intDegree == 0;
    }
    else
    {
        return FALSE;
    }
}

double GeometryUVExt::RegulareDegree( double angle )
{
    if( GetExt().IsGreaterEqual( angle, -360 ) && GetExt().IsLessEqual( angle, 360 ) )
    {
        return angle;
    }
    double intPart = 0;
    double dotPart = modf( angle, &intPart );

    int intValue = intPart;
    intValue %= 360;
    return intValue + dotPart;
}

double GeometryUVExt::GetIncludeAngle( double xs, double ys, double xb, double yb, double xe,
                                       double ye )
{
    double startAngle = GetDegreeAngle( xb, yb, xs, ys );
    double endAngle = GetDegreeAngle( xb, yb, xe, ye );
    return RoundDegreeAngle2( endAngle - startAngle );
}

bool GeometryUVExt::NotEqual( double v1, double v2 )
{
    double dDiff = fabs( v1 - v2 );
    return dDiff > GetExt().m_Epsilon;
}

bool GeometryUVExt::IsEqualAngle( double angle1, double angle2 )
{
    double check1 = angle1;
    double check2 = angle2;
    RoundDegreeAngle( check1 );
    RoundDegreeAngle( check2 );
    return IsEqual( check1, check2 ) || IsEqual( check1 + 360, check2 )
           || IsEqual( check1, check2 + 360 );
}

bool GeometryUVExt::IsAngleInRange( double angle, double sAngle, double eAngle, int dir )
{
    double refA = angle;
    double checkSA = sAngle;
    double checkEA = eAngle;
    RoundDegreeAngle( refA );
    RoundDegreeAngle( checkSA );
    RoundDegreeAngle( checkEA );
    if( IsEqual( refA, checkSA ) || IsEqual( refA, checkEA ) )
    {
        return TRUE;
    }
    if( dir == Clockwise )
    {
        while( IsLessEqual( checkSA, checkEA ) )
        {
            checkSA += 360;
        }
    }
    else
    {
        while( IsLessEqual( checkEA, checkSA ) )
        {
            checkEA += 360;
        }
    }

    return ( ( refA - checkSA ) * ( refA - checkEA ) < 0 )
           || ( ( refA + 360 - checkSA ) * ( refA + 360 - checkEA ) < 0 );
}

double GeometryUVExt::GetAnglePercent( double angle, double sAngle, double eAngle, int dir )
{
    double refA = angle;
    double checkSA = sAngle;
    double checkEA = eAngle;
    RoundDegreeAngle( refA );
    RoundDegreeAngle( checkSA );
    if( IsEqual( refA, checkSA ) )
    {
        return 0;
    }

    RoundDegreeAngle( checkEA );
    if( IsEqual( refA, checkEA ) )
    {
        return 100;
    }

    if( dir == Clockwise )
    {
        while( IsLessEqual( checkSA, checkEA ) )
        {
            checkSA += 360;
        }
    }
    else
    {
        while( IsLessEqual( checkEA, checkSA ) )
        {
            checkEA += 360;
        }
    }

    if( ( refA - checkSA ) * ( refA - checkEA ) < 0 )
    {
        return 100 * ( refA - checkSA ) / ( checkEA - checkSA );
    }
    else
    {
        refA += 360;
        if( ( refA - checkSA ) * ( refA - checkEA ) < 0 )
        {
            return 100 * ( refA - checkSA ) / ( checkEA - checkSA );
        }
    }
    return InvalidePercentValue;
}

void GeometryUVExt::MidPointOfArc( double cx, double cy, double r, double startAng, double endAng,
                                   double& mx, double& my )
{
    double eAngle = endAng;
    while( IsLessEqual( eAngle, startAng ) )
    {
        eAngle += 360;
    }
    double angle = ( startAng + eAngle ) / 2 * _PIDiv180;
    mx = cx + r * cos( angle );
    my = cy + r * sin( angle );
}

void GeometryUVExt::MidPointOfArc( int arcDir, double cx, double cy, double r, double startAng,
                                   double endAng, double& mx, double& my )
{
    double sA = startAng;
    double eA = endAng;
    RoundDegreeAngle( sA );
    RoundDegreeAngle( eA );

    if( arcDir == Clockwise )
    {
        while( IsLessEqual( sA, eA ) )
        {
            sA += 360;
        }
    }
    else
    {
        while( IsLessEqual( eA, sA ) )
        {
            eA += 360;
        }
    }

    double angle = ( sA + eA ) / 2 * _PIDiv180;
    mx = cx + r * cos( angle );
    my = cy + r * sin( angle );
}

void GeometryUVExt::MidPointOfArcEx( double sx, double sy, double ex, double ey, double ArcAngle,
                                     double& mx, double& my )
{
    double cx[2];
    double cy[2];
    GetArcCentersBySweepAngle( sx, sy, ex, ey, abs( ArcAngle ), cx, cy );
    double r = Distance( cx[0], cy[0], sx, sy );
    double midEx = ( sx + ex ) * 0.5;
    double midEy = ( sy + ey ) * 0.5;

    if( IsGreater( abs( ArcAngle ), 180 ) )
    {
        mx = cx[1];
        my = cy[1];
        //优弧
        /* if( IsGreaterEqual( ArcAngle, 0 ) )
        {
            MidPointOfArc( 0, cx[1], cy[1], r, GetDegreeAngle( cx[1], cy[1], sx, sy ),
                           GetDegreeAngle( cx[1], cy[1], ex, ey ), mx, my );
        }
        else
        {
            MidPointOfArc( 1, cx[1], cy[1], r, GetDegreeAngle( cx[1], cy[1], sx, sy ),
                           GetDegreeAngle( cx[1], cy[1], ex, ey ), mx, my );
        }*/
    }
    else
    {
        //劣弧
        mx = cx[0];
        my = cy[0];
        /*if( IsGreaterEqual( ArcAngle, 0 ) )
        {
            MidPointOfArc( 0, cx[0], cy[0], r, GetDegreeAngle( cx[0], cy[0], sx, sy ),
                           GetDegreeAngle( cx[0], cy[0], ex, ey ), mx, my );
        }
        else
        {
            MidPointOfArc( 1, cx[0], cy[0], r, GetDegreeAngle( cx[0], cy[0], sx, sy ),
                           GetDegreeAngle( cx[0], cy[0], ex, ey ), mx, my );
        }*/
    }
}

double GeometryUVExt::GetDegreeAngle( double x, double y )
{
    if( ( x > 0 ) && ( y > 0 ) )
        return ( atan( y / x ) * _180DivPI );
    else if( ( x < 0 ) && ( y > 0 ) )
        return ( atan( -x / y ) * _180DivPI ) + 90.0;
    else if( ( x < 0 ) && ( y < 0 ) )
        return ( atan( y / x ) * _180DivPI ) + 180;
    else if( ( x > 0 ) && ( y < 0 ) )
        return ( atan( -x / y ) * _180DivPI ) + 270;
    else if( IsEqual( x, 0 ) && ( y > 0 ) )
        return 90;
    else if( ( x < 0 ) && IsEqual( y, 0 ) )
        return 180;
    else if( IsEqual( x, 0 ) && ( y < 0 ) )
        return 270;
    else
        return 0;
}

double GeometryUVExt::GetDegreeAngle( double sx, double sy, double ex, double ey )
{
    return GetDegreeAngle( ex - sx, ey - sy );
}

double GeometryUVExt::GetABCLineDegreeAngle( double a, double b, double c )
{
    bool   isAEqualZero = IsEqual( a, 0 );
    bool   isBEqualZero = IsEqual( b, 0 );
    double sX = 0, sY = 0;
    double eX = 0, eY = 0;
    if( !isAEqualZero && !isBEqualZero )
    {
        sX = 1;
        sY = -( a / b * sX + c / b );
        eX = -1;
        eY = -( a / b * eX + c / b );
    }
    else if( isAEqualZero && !isBEqualZero )
    {
        sX = 1;
        sY = -c / b;
        eX = -1;
        eY = -c / b;
    }
    else if( !isAEqualZero && isBEqualZero )
    {
        sX = -c / a;
        sY = 1;
        eX = -c / a;
        eY = -1;
    }
    return GetDegreeAngle( sX, sY, eX, eY );
}

double GeometryUVExt::GetRadianAngle( double x, double y )
{
    if( ( x > 0 ) && ( y > 0 ) )
        return ( atan( y / x ) );
    else if( ( x < 0 ) && ( y > 0 ) )
        return ( atan( -x / y ) ) + _HALFPI;
    else if( ( x < 0 ) && ( y < 0 ) )
        return ( atan( y / x ) ) + _PI;
    else if( ( x > 0 ) && ( y < 0 ) )
        return ( atan( -x / y ) ) + _ONEHALFPI;
    else if( IsEqual( x, 0 ) && ( y > 0 ) )
        return _HALFPI;
    else if( ( x < 0 ) && IsEqual( y, 0 ) )
        return _PI;
    else if( IsEqual( x, 0 ) && ( y < 0 ) )
        return _ONEHALFPI;
    else
        return 0;
}

double GeometryUVExt::GetRadianAngle( double sx, double sy, double ex, double ey )
{
    return GetRadianAngle( ex - sx, ey - sy );
}


void GeometryUVExt::GetVector( double length, double degreeAngle, double& deltaX, double& deltaY )
{
    double radiansAngle = Degree2Radians( degreeAngle );
    deltaX = length * cos( radiansAngle );
    deltaY = length * sin( radiansAngle );
}

void GeometryUVExt::GetVector( int plane, double length, double degreeAngle, double& deltaX,
                               double& deltaY, double& deltaZ )
{
    double deltaU = 0, deltaV = 0;
    GetVector( length, degreeAngle, deltaU, deltaV );
    deltaX = deltaY = deltaZ = 0;
    if( plane == PlaneType_ZX )
    {
        deltaZ = deltaU;
        deltaX = deltaV;
    }
    else if( plane == PlaneType_YZ )
    {
        deltaY = deltaU;
        deltaZ = deltaV;
    }
    else
    {
        deltaX = deltaU;
        deltaY = deltaV;
    }
}

int GeometryUVExt::GetLineEndValueByLength( bool isXOK, double sx, double sy, double refValue,
                                            double length, double* pResult )
{
    if( isXOK )
    {
        double deltaX = fabs( refValue - sx );
        if( deltaX > length )
        {
            return 0;
        }
        else if( IsEqual( deltaX, length ) )
        {
            if( pResult )
            {
                pResult[0] = sy;
            }
            return 1;
        }
        else
        {
            double deltaY = sqrt( length * length - deltaX * deltaX );
            if( pResult )
            {
                pResult[0] = sy + deltaY;
                pResult[1] = sy - deltaY;
            }
            return 2;
        }
    }
    else
    {
        double deltaY = fabs( refValue - sy );
        if( deltaY > length )
        {
            return 0;
        }
        else if( IsEqual( deltaY, length ) )
        {
            if( pResult )
            {
                pResult[0] = sx;
            }
            return 1;
        }
        else
        {
            double deltaX = sqrt( length * length - deltaY * deltaY );
            if( pResult )
            {
                pResult[0] = sx + deltaX;
                pResult[1] = sx - deltaX;
            }
            return 2;
        }
    }
}

bool GeometryUVExt::GetLineEndValueByAngle( bool isXOK, double sx, double sy, double refValue,
                                            double angle, double& result )
{
    if( isXOK )
    {
        if( Is90Degree( angle ) )
        {
            return FALSE;
        }
        result = sy + ( refValue - sx ) * tan( Degree2Radians( angle ) );
    }
    else
    {
        if( Is180Degree( angle ) )
        {
            return FALSE;
        }
        result = sx + ( refValue - sy ) / tan( Degree2Radians( angle ) );
    }
    return TRUE;
}

bool GeometryUVExt::GetLineEndValueByEndPos( bool isXOK, double sx, double sy, double refValue,
                                             double ex, double ey, double& result )
{
    double degreeAngle = GetDegreeAngle( sx, sy, ex, ey );
    return GetLineEndValueByAngle( isXOK, sx, sy, refValue, degreeAngle, result );
}

void GeometryUVExt::GetLineEndValues( double sx, double sy, double length, double angle, double& ex,
                                      double& ey )
{
    double radiansAngle = Degree2Radians( angle );
    ex = sx + length * cos( radiansAngle );
    ey = sy + length * sin( radiansAngle );
}

void GeometryUVExt::GetLineEndingValues( bool isX1Y2, double refX, double refY, double length,
                                         double angle, double& xResult, double& yResult )
{
    double radiansAngle = Degree2Radians( angle );
    if( isX1Y2 )
    {
        xResult = refX + length * cos( radiansAngle );
        yResult = refY - length * sin( radiansAngle );
    }
    else
    {
        xResult = refX - length * cos( radiansAngle );
        yResult = refY + length * sin( radiansAngle );
    }
}

void GeometryUVExt::Rotate( double rotationAngle, double x, double y, double& nx, double& ny )
{
    double sin_val = sin( rotationAngle * _PIDiv180 );
    double cos_val = cos( rotationAngle * _PIDiv180 );
    nx = ( x * cos_val ) - ( y * sin_val );
    ny = ( y * cos_val ) + ( x * sin_val );
}

void GeometryUVExt::Rotate( double rotationAngle, double x, double y, double ox, double oy,
                            double& nx, double& ny )
{
    Rotate( rotationAngle, x - ox, y - oy, nx, ny );
    nx += ox;
    ny += oy;
}

void GeometryUVExt::FastRotate( const int rotationAngle, double x, double y, double& nx,
                                double& ny )
{
    int rot_ang = rotationAngle % TrigTableSize;
    if( rot_ang < 0 )
    {
        rot_ang = TrigTableSize + rot_ang;
    }

    double sin_val = SinTable[rot_ang];
    double cos_val = CosTable[rot_ang];
    nx = ( x * cos_val ) - ( y * sin_val );
    ny = ( y * cos_val ) + ( x * sin_val );
}

void GeometryUVExt::FastRotate( const int rotationAngle, double x, double y, double ox, double oy,
                                double& nx, double& ny )
{
    FastRotate( rotationAngle, x - ox, y - oy, nx, ny );
    nx += ox;
    ny += oy;
}

double GeometryUVExt::GetSin( int index )
{
    int rot_ang = index % TrigTableSize;
    if( rot_ang < 0 )
    {
        rot_ang = TrigTableSize + rot_ang;
    }
    return SinTable[rot_ang];
}

double GeometryUVExt::GetCos( int index )
{
    int rot_ang = index % TrigTableSize;
    if( rot_ang < 0 )
    {
        rot_ang = TrigTableSize + rot_ang;
    }
    return CosTable[rot_ang];
}

double GeometryUVExt::GetTan( int index )
{
    int rot_ang = index % TrigTableSize;
    if( rot_ang < 0 )
    {
        rot_ang = TrigTableSize + rot_ang;
    }
    return TanTable[rot_ang];
}

bool GeometryUVExt::TwoRayIntersect( double x1, double y1, double angle1, double x2, double y2,
                                     double angle2, double& ix, double& iy )
{
    if( IsEqual( angle1, angle2 ) || ( IsEqual( x1, x2 ) && IsEqual( y1, y2 ) ) )
    {
        return FALSE;
    }
    double tanAngle1 = tan( Degree2Radians( angle1 ) );
    double tanAngle2 = tan( Degree2Radians( angle2 ) );
    if( IsEqual( tanAngle1, tanAngle2 ) )
    {
        return FALSE;
    }

    double ex1 = 0, ey1 = 0;
    OffsetPointByAngleLength( x1, y1, angle1, 10, ex1, ey1 );
    double ex2 = 0, ey2 = 0;
    OffsetPointByAngleLength( x2, y2, angle2, 10, ex2, ey2 );
    Line2LineIntersectionPoint( x1, y1, ex1, ey1, x2, y2, ex2, ey2, ix, iy );
    return TRUE;
}

void GeometryUVExt::RayToLongLine( double x0, double y0, double lineAngle, double& sx, double& sy,
                                   double& ex, double& ey )
{
    double tanLineAngle = tan( Degree2Radians( lineAngle ) );
    if( Is90Degree( lineAngle ) )
    {
        sy = y0 - DefaultOffset;
        ey = y0 + DefaultOffset;
        sx = ex = x0;
    }
    else if( Is180Degree( lineAngle ) )
    {
        sx = x0 - DefaultOffset;
        ex = x0 + DefaultOffset;
        sy = ey = y0;
    }
    else
    {
        sy = y0 - DefaultOffset;
        sx = x0 - DefaultOffset * tanLineAngle;
        ey = y0 + DefaultOffset;
        ex = x0 + DefaultOffset * tanLineAngle;
    }
}

void GeometryUVExt::RayToABCLine( double x0, double y0, double lineAngle, double& a, double& b,
                                  double& c )
{
    double ox = 0, oy = 0;
    OffsetPointByAngleLength( x0, y0, lineAngle, 10, ox, oy );
    LineToABCLine( x0, y0, ox, oy, a, b, c );
}

void GeometryUVExt::PointRayPerpendicular( double x, double y, double x0, double y0,
                                           double rayAngle, double& nx, double& ny )
{
    double a = 0, b = 0, c = 0;
    RayToABCLine( x0, y0, rayAngle, a, b, c );
    PointABCLinePerpendicular( x, y, a, b, c, nx, ny );
}

void GeometryUVExt::LineToABCLine( double sx, double sy, double ex, double ey, double& a, double& b,
                                   double& c )
{
    a = sy - ey;
    b = ex - sx;
    c = sx * ey - ex * sy;
}

void GeometryUVExt::OffsetRay( double x0, double y0, double rayAngle, double distance, bool isLeft,
                               double& newX, double& newY )
{
    double perpendicularAngle = isLeft ? rayAngle + 90 : rayAngle - 90;
    GetLineEndValues( x0, y0, distance, perpendicularAngle, newX, newY );
}

int GeometryUVExt::RayCircleIntersect( double x0, double y0, double lineAngle, double cx, double cy,
                                       double radius, double* xIntersects, double* yIntersects )
{
    double a = 0, b = 0, c = 0;
    RayToABCLine( x0, y0, lineAngle, a, b, c );
    int itCount = ABCLineCircleIntersect( a, b, c, cx, cy, radius, xIntersects, yIntersects );
    if( itCount < 1 )
    {
        return itCount;
    }
    double curAngle1 = GetDegreeAngle( x0, y0, xIntersects[0], yIntersects[0] );
    int    realFalg = 0;
    if( IsEqualAngle( curAngle1, lineAngle ) )
    {
        realFalg = 1;
    }
    if( itCount > 1 )
    {
        double curAngle2 = GetDegreeAngle( x0, y0, xIntersects[1], yIntersects[1] );
        if( IsEqualAngle( curAngle2, lineAngle ) )
        {
            realFalg |= 2;
        }
    }
    if( realFalg == 3 || realFalg == 0 )
    {
        return ( realFalg == 3 ) ? 2 : 0;
    }
    if( realFalg == 2 )
    {
        xIntersects[0] = xIntersects[1];
        yIntersects[0] = yIntersects[1];
    }
    return 1;
}

int GeometryUVExt::LineCircleIntersect( double sx, double sy, double ex, double ey, double cx,
                                        double cy, double radius, double* xIntersects,
                                        double* yIntersects )
{
    double lineAngle = GetDegreeAngle( sx, sy, ex, ey );

    int insectionCount =
            RayCircleIntersect( sx, sy, lineAngle, cx, cy, radius, xIntersects, yIntersects );
    if( insectionCount == 0 )
    {
        insectionCount = RayCircleIntersect( ex, ey, lineAngle + 180, cx, cy, radius, xIntersects,
                                             yIntersects );
    }
    if( insectionCount == 2 )
    {
        if( !IsPointOnLine( xIntersects[0], yIntersects[0], sx, sy, ex, ey ) )
        {
            xIntersects[0] = xIntersects[1];
            yIntersects[0] = yIntersects[1];
            --insectionCount;
        }
        if( !IsPointOnLine( xIntersects[1], yIntersects[1], sx, sy, ex, ey ) )
        {
            --insectionCount;
        }
        return insectionCount;
    }
    else if( insectionCount == 1 )
    {
        if( !IsPointOnLine( xIntersects[0], yIntersects[0], sx, sy, ex, ey ) )
        {
            --insectionCount;
        }
    }

    return insectionCount;
}

int GeometryUVExt::LineCircleIntersect2( double sx, double sy, double ex, double ey, double cx,
                                         double cy, double radius, double* xIntersects,
                                         double* yIntersects )
{
    double lineAngle = GetDegreeAngle( sx, sy, ex, ey );
    double a = 0, b = 0, c = 0;
    RayToABCLine( sx, sy, lineAngle, a, b, c );
    return ABCLineCircleIntersect( a, b, c, cx, cy, radius, xIntersects, yIntersects );
}

int GeometryUVExt::ABCLineCircleIntersect( double a, double b, double c, double cx, double cy,
                                           double radius, double* xIntersects, double* yIntersects )
{
    double d = PointABCLineDistance( cx, cy, a, b, c );
    if( IsEqual( d, 0 ) )
    { //圆心在直线上
        double lineAngle = GetABCLineDegreeAngle( a, b, c );
        OffsetPointByAngleLength( cx, cy, lineAngle, radius, xIntersects[0], yIntersects[0] );
        OffsetPointByAngleLength( cx, cy, lineAngle + 180, radius, xIntersects[1], yIntersects[1] );
        return 2;
    }

    //圆心不再直线上
    bool isTangent = IsEqual( d, radius );
    if( !isTangent && d > radius )
    {
        return 0;
    }

    double px = 0, py = 0;
    PointABCLinePerpendicular( cx, cy, a, b, c, px, py );
    if( isTangent )
    {
        xIntersects[0] = px;
        yIntersects[0] = py;
        return 1;
    }

    double centerAngle = GetDegreeAngle( cx, cy, px, py );
    //intercross
    double distance = sqrt( radius * radius - d * d );
    OffsetPointByAngleLength( px, py, centerAngle + 90, distance, xIntersects[0], yIntersects[0] );
    OffsetPointByAngleLength( px, py, centerAngle - 90, distance, xIntersects[1], yIntersects[1] );
    return 2;
}

int GeometryUVExt::CircleCircleIntersect( double cx1, double cy1, double radius1, double cx2,
                                          double cy2, double radius2, double* xIntersects,
                                          double* yIntersects )
{
    double distanceCenter = Distance( cx1, cy1, cx2, cy2 );
    if( IsEqual( distanceCenter, 0 ) && IsEqual( radius1, radius2 ) )
    {
        return FALSE; // No intersection or Same circle.
    }

    double deltaRadius = fabs( radius1 - radius2 );
    if( IsEqual( distanceCenter, deltaRadius ) || IsEqual( distanceCenter, ( radius1 + radius2 ) ) )
    {
        OffsetPointByRegPtLength( cx1, cy1, cx2, cy2, radius1, xIntersects[0], yIntersects[0] );
        return 1;
    }
    else if( distanceCenter > deltaRadius && distanceCenter < ( radius1 + radius2 ) )
    {
        double dstsqr = distanceCenter * distanceCenter;
        double r1sqr = radius1 * radius1;
        double r2sqr = radius2 * radius2;
        double a = ( dstsqr - r2sqr + r1sqr ) / ( 2 * distanceCenter );
        double h = sqrt( r1sqr - ( a * a ) );

        double ratio_a = a / distanceCenter;
        double ratio_h = h / distanceCenter;

        double dx = cx2 - cx1;
        double dy = cy2 - cy1;

        double phix = cx1 + ( ratio_a * dx );
        double phiy = cy1 + ( ratio_a * dy );

        dx = dx * ratio_h;
        dy = dy * ratio_h;

        xIntersects[0] = phix + dy;
        yIntersects[0] = phiy - dx;

        xIntersects[1] = phix - dy;
        yIntersects[1] = phiy + dx;
        return 2;
    }
    return 0;
}

double GeometryUVExt::PointToLineDistance( double x, double y, double sx, double sy, double ex,
                                           double ey )
{
    double a = 0, b = 0, c = 0;
    LineToABCLine( sx, sy, ex, ey, a, b, c );
    return ( fabs( a * x + b * y + c ) ) / sqrt( a * a + b * b );
}

void GeometryUVExt::PointLinePerpendicular( double x, double y, double sx, double sy, double ex,
                                            double ey, double& nx, double& ny )
{
    double a = 0, b = 0, c = 0;
    LineToABCLine( sx, sy, ex, ey, a, b, c );
    PointABCLinePerpendicular( x, y, a, b, c, nx, ny );
}

void GeometryUVExt::OffsetLine( double& sx, double& sy, double& ex, double& ey, double offsetLength,
                                bool isLeftOffset )
{
    static const double Epsilon = 0.005;
    static const double _PI = double( 3.141592653589793238462643383279500 );
    static const double _HALFPI = _PI * 0.5;
    static const double _ONEHALFPI = _PI * 1.5;
    double              dx = ex - sx;
    double              dy = ey - sy;
    bool                isSameX = fabs( dx ) <= Epsilon;
    bool                isSameY = fabs( dy ) <= Epsilon;
    double              lineAngle = 0;
    if( !isSameX && !isSameY )
    {
        if( ( dx > 0 ) && ( dy > 0 ) )
        {
            lineAngle = atan( dy / dx );
        }
        else if( ( dx < 0 ) && ( dy > 0 ) )
        {
            lineAngle = ( atan( -dx / dy ) ) + _HALFPI;
        }
        else if( ( dx < 0 ) && ( dy < 0 ) )
        {
            lineAngle = ( atan( dy / dx ) ) + _PI;
        }
        else if( ( dx > 0 ) && ( dy < 0 ) )
        {
            lineAngle = ( atan( -dx / dy ) ) + _ONEHALFPI;
        }
    }
    else if( isSameX && !isSameY )
    {
        lineAngle = dy > 0 ? _HALFPI : _ONEHALFPI;
    }
    else if( !isSameX && isSameY )
    {
        lineAngle = dx > 0 ? 0 : _PI;
    }

    double offsetAngle = lineAngle;
    offsetAngle += isLeftOffset ? _HALFPI : -_HALFPI;
    double sinOffset = offsetLength * sin( offsetAngle );
    double cosOffset = offsetLength * cos( offsetAngle );

    sx = sx + cosOffset;
    sy = sy + sinOffset;
    ex = ex + cosOffset;
    ey = ey + sinOffset;
}

int GeometryUVExt::GetArcCenters( double sx, double sy, double ex, double ey, double radius,
                                  double* xCenters, double* yCenters )
{
    double distance = Distance( sx, sy, ex, ey );
    if( ( distance > radius * 2 && !IsEqual( distance, radius * 2 ) ) || IsEqual( distance, 0 ) )
    {
        return 0;
    }
    double midX = ( sx + ex ) * 0.5;
    double midY = ( sy + ey ) * 0.5;
    if( IsEqual( distance, radius * 2 ) )
    {
        xCenters[0] = midX;
        yCenters[0] = midY;
        return 1;
    }
    double lx = ( ex - sx ) / distance;
    double ly = ( ey - sy ) / distance;

    double mCenter = sqrt( radius * radius - distance * distance * 0.25 );
    xCenters[0] = midX + mCenter * ly;
    yCenters[0] = midY - mCenter * lx;

    xCenters[1] = midX - mCenter * ly;
    yCenters[1] = midY + mCenter * lx;
    return 2;
}

int GeometryUVExt::GetArcCentersBySweepAngle( double sx, double sy, double ex, double ey,
                                              double sweepAngle, double* xCenters,
                                              double* yCenters )
{
    double realAngle = RegulareDegree( sweepAngle );
    if( IsEqualAngle( realAngle, 0 ) || IsEqualAngle( realAngle, 360 ) )
    {
        return 0;
    }
    double midX = ( sx + ex ) * 0.5;
    double midY = ( sy + ey ) * 0.5;
    if( IsEqualAngle( realAngle, 180 ) )
    {
        xCenters[0] = midX;
        yCenters[0] = midY;
        return 1;
    }

    double randAngle = Degree2Radians( realAngle * 0.5 );
    double arcRadius = Distance( sx, sy, midX, midY ) / sin( randAngle );

    return GetArcCenters( sx, sy, ex, ey, arcRadius, xCenters, yCenters );
}

int GeometryUVExt::GetArcCentersByStartAngle( double sx, double sy, double ex, double ey,
                                              double startAngleWithXAxis, double* xCenters,
                                              double* yCenters, double& sweepAngle )
{
    double chordLength = Distance( sx, sy, ex, ey );
    double lineAngle = GetDegreeAngle( sx, sy, ex, ey );
    startAngleWithXAxis = RegulareDegree( startAngleWithXAxis );
    double angleOfOsculation = lineAngle - startAngleWithXAxis;
    sweepAngle = angleOfOsculation * 2;
    //切角定理:弦切角的度数等于它所夹的弧的圆心角度数的一半。
    return GetArcCentersBySweepAngle( sx, sy, ex, ey, sweepAngle, xCenters, yCenters );
};

int GeometryUVExt::Get2LineTagArc( double sx, double sy, double ex, double ey, double sx2,
                                   double sy2, double ex2, double ey2, double radius, double& acx,
                                   double& acy, double& asx, double& asy, double& aex, double& aey )
{
    //a1 = y1 - y2: b1 = x2 - x1: c1 = x1 * y2 - x2 * y1
    double a1 = sy - ey;
    double b1 = ex - sx;
    double c1 = sx * ey - ex * sy;
    //a3 = a1: b3 = b1: c3 = c1 + r * Sqr(a1 * a1 + b1 * b1)
    double a3 = a1, b3 = b1;
    double c3 = c1 + radius * sqrt( a1 * a1 + b1 * b1 );

    //	a2 = y3 - y4: b2 = x4 - x3: c2 = x3 * y4 - x4 * y3
    double a2 = sy2 - ey2;
    double b2 = ex2 - sx2;
    double c2 = sx2 * ey2 - ex2 * ey2;

    //	a4 = a2: b4 = b2: c4 = c2 + r * Sqr(a2 * a2 + b2 * b2)
    double a4 = a2, b4 = b2;
    double c4 = c2 + radius * sqrt( a2 * a2 + b2 * b2 );

    acx = ( b3 * c4 - b4 * c3 ) / ( a3 * b4 - a4 * b3 );
    acy = ( a3 * c4 - a4 * c3 ) / ( a4 * b3 - a3 * b4 );
    // a5 = -b1: b5 = a1: c5 = b1 * xc - a1 * yc
    double a5 = -b1, b5 = a1;
    double c5 = b1 * acx - a1 * acy;

    asx = ( b1 * c5 - b5 * c1 ) / ( a1 * b5 - a5 * b1 );
    asy = ( a1 * c5 - a5 * c1 ) / ( a5 * b1 - a1 * b5 );
    // a6 = -b2: b6 = a2: c6 = b2 * xc - a2 * yc
    double a6 = -b2, b6 = a2;
    double c6 = b2 * acx - a2 * acy;

    aex = ( b2 * c6 - b6 * c2 ) / ( a2 * b6 - a6 * b2 );
    aey = ( a2 * c6 - a6 * c2 ) / ( a6 * b2 - a2 * b6 );
    return true;
}


int GeometryUVExt::LineCircleTangentPoints( double lx, double ly, double cx, double cy,
                                            double radius, double* xIntersects,
                                            double* yIntersects )
{
    double deltaX = lx - cx;
    double deltaY = ly - cy;
    double sqrLength = HalSqr( deltaX ) + HalSqr( deltaY );
    double sqrRadius = HalSqr( radius );
    if( sqrLength > sqrRadius )
    {
        double ratio = 1.0 / sqrLength;
        double deltaDist = sqrt( abs( sqrLength - sqrRadius ) );
        xIntersects[0] = cx + radius * ( radius * deltaX - deltaY * deltaDist ) * ratio;
        yIntersects[0] = cy + radius * ( radius * deltaY + deltaX * deltaDist ) * ratio;

        xIntersects[1] = cx + radius * ( radius * deltaX + deltaY * deltaDist ) * ratio;
        yIntersects[1] = cy + radius * ( radius * deltaY - deltaX * deltaDist ) * ratio;
        return 2;
    }
    return 0;
}

void GeometryUVExt::OffsetPointByRegPtLength( double x0, double y0, double refX, double refY,
                                              double length, double& newX, double& newY )
{
    double degreeAngle = GetDegreeAngle( x0, y0, refX, refY );
    OffsetPointByAngleLength( x0, y0, degreeAngle, length, newX, newY );
}

void GeometryUVExt::OffsetPointByAngleLength( double x0, double y0, double angle, double length,
                                              double& newX, double& newY )
{
    double radiansAngle = Degree2Radians( angle );
    newX = x0 + length * cos( radiansAngle );
    newY = y0 + length * sin( radiansAngle );
}

int GeometryUVExt::Circle2CircleType( double cx1, double cy1, double radius1, double cx2,
                                      double cy2, double radius2 )
{
    double distanceCenter = Distance( cx1, cy1, cx2, cy2 );
    double deltaRadius = fabs( radius1 - radius2 );
    double sumRadius = fabs( radius1 + radius2 );
    if( IsEqual( distanceCenter, sumRadius ) )
    {
        return LoopLoop_OutSideTangent;
    }
    else if( distanceCenter > sumRadius )
    {
        return LoopLoop_OutSide;
    }
    else if( IsEqual( distanceCenter, deltaRadius ) )
    {
        return LoopLoop_InsideTangent;
    }
    else if( distanceCenter > deltaRadius && distanceCenter < sumRadius )
    {
        return LoopLoop_Intersect;
    }
    else
    {
        return LoopLoop_Inside;
    }
}

bool GeometryUVExt::Line2LineIntersectionPoint( double sx1, double sy1, double ex1, double ey1,
                                                double sx2, double sy2, double ex2, double ey2,
                                                double& ix, double& iy )
{
    if( IsParallel( sx1, sy1, ex1, ey1, sx2, sy2, ex2, ey2 ) )
    {
        return false;
    }

    double dx1 = ex1 - sx1;
    double dx2 = ex2 - sx2;
    double dx3 = sx1 - sx2;

    double dy1 = ey1 - sy1;
    double dy2 = sy1 - sy2;
    double dy3 = ey2 - sy2;

    double ratio = dx1 * dy3 - dy1 * dx2;
    if( NotEqual( ratio, 0 ) )
    {
        ratio = ( dy2 * dx2 - dx3 * dy3 ) / ratio;
        ix = sx1 + ratio * dx1;
        iy = sy1 + ratio * dy1;
    }
    else
    {
        if( IsEqual( ( dx1 * -dy2 ), ( -dx3 * dy1 ) ) )
        {
            ix = sx2;
            iy = sy2;
        }
        else
        {
            ix = ex2;
            iy = ey2;
        }
    }
    return true;
}

bool GeometryUVExt::Segment2SegmentIntersect( double x1, double y1, double x2, double y2, double x3,
                                              double y3, double x4, double y4, double& ix,
                                              double& iy )
{
    double ax = x2 - x1;

    double lowerx = ( ax < 0.0 ) ? x2 : x1;
    double upperx = ( ax < 0.0 ) ? x1 : x2;
    double bx = x3 - x4;
    if( bx > 0.0 )
    {
        if( ( upperx < x4 ) || ( x3 < lowerx ) )
        {
            return FALSE;
        }
    }
    else if( ( upperx < x3 ) || ( x4 < lowerx ) )
    {
        return FALSE;
    }

    double ay = y2 - y1;
    double uppery = ( ay < 0.0 ) ? y1 : y2;
    double lowery = ( ay < 0.0 ) ? y2 : y1;

    double by = y3 - y4;
    if( by > 0.0 )
    {
        if( ( uppery < y4 ) || ( y3 < lowery ) )
        {
            return FALSE;
        }
    }
    else if( ( uppery < y3 ) || ( y4 < lowery ) )
    {
        return FALSE;
    }

    // 	if (IsEqual(ax, 0) && IsEqual(bx, 0)) {
    // 		ix = x1;
    // 		if (IsEqual(y3, y1) || IsEqual(y3, y2)) {
    // 			iy = y3;
    // 		}
    // 		else if (IsEqual(y4, y1) || IsEqual(y4, y2)) {
    // 			iy = y4;
    // 		}
    // 		return TRUE;
    // 	}
    // 	else if (IsEqual(ay, 0) && IsEqual(by, 0)) {
    // 		iy = y1;
    // 		if (IsEqual(x3, x1) || IsEqual(x3, x2)) {
    // 			ix = x3;
    // 		}
    // 		else if (IsEqual(x4, x1) || IsEqual(x4, x2)) {
    // 			ix = x4;
    // 		}
    // 		return TRUE;
    // 	}

    double cx = x1 - x3;
    double cy = y1 - y3;
    double d = ( by * cx ) - ( bx * cy );
    double f = ( ay * bx ) - ( ax * by );

    if( f > 0.0 )
    {
        if( ( d < 0.0 ) || ( d > f ) )
        {
            return FALSE;
        }
    }
    else if( ( d > 0.0 ) || ( d < f ) )
    {
        return FALSE;
    }

    double e = ( ax * cy ) - ( ay * cx );
    if( f > 0.0 )
    {
        if( ( e < 0.0 ) || ( e > f ) )
        {
            return FALSE;
        }
    }
    else if( ( e > 0.0 ) || ( e < f ) )
    {
        return FALSE;
    }

    double ratio = ( ax * -by ) - ( ay * -bx );
    if( NotEqual( ratio, 0.0 ) )
    {
        ratio = ( ( cy * -bx ) - ( cx * -by ) ) / ratio;
        ix = x1 + ( ratio * ax );
        iy = y1 + ( ratio * ay );
    }
    else
    {
        if( IsEqual( ( ax * -cy ), ( -cx * ay ) ) )
        {
            ix = x3;
            iy = y3;
        }
        else
        {
            ix = x4;
            iy = y4;
        }
    }
    return TRUE;
}


bool GeometryUVExt::IsParallel( double x1, double y1, double x2, double y2, double x3, double y3,
                                double x4, double y4 )
{
    return IsEqual( ( ( y1 - y2 ) * ( x3 - x4 ) ), ( ( y3 - y4 ) * ( x1 - x2 ) ) );
}

bool GeometryUVExt::IsPerpendicular( double x1, double y1, double x2, double y2, double x3,
                                     double y3, double x4, double y4 )
{
    return IsEqual( -1.0 * ( y2 - y1 ) * ( y4 - y3 ), ( x4 - x3 ) * ( x2 - x1 ) );
}

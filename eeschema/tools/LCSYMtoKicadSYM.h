#ifndef LCSYMtoKicadSYM_H_
#define LCSYMtoKicadSYM_H_

#include "wx/stringimpl.h"   
#include <symbol_library_common.h>
#include <set>
#include <lib_shape.h>


#include <lib_symbol.h>
#include <wx/string.h>

#include <pin_type.h>
#include<lib_pin.h>
#include <lib_shape.h>
#include <default_values.h>
#include<lib_field.h>
#include <template_fieldnames.h>
#include <lib_text.h>


class LIB_SYMBOL;
class LIB_PIN;
class LIB_FIELD;




class LCSYMtoKicadSYM
{

public:
    LCSYMtoKicadSYM();

    ~LCSYMtoKicadSYM();



public:
    //解析立创EDA symbol文件
    int importLCSYM( std::string strInFileFullPath, int fileType = 0 );


    //按行读取立创封装文件，返回每一行数据。
    int readLCSYMFileByLine( std::string               strInFileFullPath,
                                     std::vector<std::string>& vecLCSYMLines );

    //按行读取立创封装文件
    int readLCSYMFileByLineFromJson( std::string               strInFileFullPath,
                                     std::vector<std::string>& vecLCSYMLines );

    //解析每一行数据
    int parseLines( std::vector<std::string> vecLCSYMLines );


    //解析PART
    int parsePART( std::vector<std::vector<std::string>> vecOfVecStr, VECTOR2I& f_xy );



    //设置kicad的property
    LIB_FIELD* setPROPERTY( const wxString& name, const wxString& value, VECTOR2I& f_xy );
    
    //修改kicad的property
    void changePROPERTY( LIB_SYMBOL* curSymbol );


    //解析PIN
    LIB_PIN* parsePIN( std::vector<std::vector<std::string>> vecOfVecStr, int m_unit,int m_convert );


    //解析ARC(弧线)
    int parseARC( std::vector<std::string> vecStr, int m_unit, int m_convert );

    //解析RECT(矩形)
    VECTOR2I parseRECT( std::vector<std::string> vecStr, int m_unit, int m_convert );

    //解析CIRCLE(圆形)
    int parseCIRCLE( std::vector<std::string> vecStr, int m_unit, int m_convert );

    //解析POLY(多边形)
    int parsePOLY( std::vector<std::string> vecStr, int m_unit, int m_convert );

    //解析TEXT
    int parseTEXT( std::vector<std::string> vecStr, int m_unit, int m_convert );



    //字符串分割
    std::vector<std::string> Split( std::string strContext, std::string StrDelimiter );

    //去除vector容器内单个元素的前后双引号
    void vecStrAnalyse( std::vector<std::string> vecStr );

    /// <summary>
    /// 字符串批量替换
    /// </summary>
    /// <param name="str">输入的文本</param>
    /// <param name="a">目标文本</param>
    /// <param name="b">替换内容</param>
    /// <returns>替换好的文本</returns>
    std::string spp( std::string str, std::string a, std::string b );



public:
    LIB_SYMBOL*  m_symbolObj;
    //std::unique_ptr<LIB_SYMBOL> symbolobj = std::make_unique<LIB_SYMBOL>( wxEmptyString );
    //std::unique_ptr<LIB_PIN>    pin = std::make_unique<LIB_PIN>( nullptr );

    int    m_unit = 0;     ///< The current unit being parsed.
    int    m_convert = 0; ///< The current body style being parsed.
    
    std::string m_pinType;  //引脚类型
    wxString wxPinName;
    wxString wxPinNumber;
    std::vector<std::vector<std::string>> pinOfvecSplits;
    std::vector<std::vector<std::string>> partOfvecSplits;

    /// Field IDs that have been read so far for the current symbol.
    std::set<int> m_fieldIDsRead;
    std::vector<LIB_FIELD> fields;

    wxString wxName = "";
    wxString wxType = "";
    VECTOR2I fp_xy;
    //struct SymbolProperty
    //{
    //    wxString wxName = "";
    //    wxString wxType = "";
    //};


};

#endif LCSYMtoKicadSYM_H_ // !LCSYMtoKicadSYM_H

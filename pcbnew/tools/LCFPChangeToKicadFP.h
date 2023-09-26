
#ifndef LCFPChangeToKicadFP_H
#define LCFPChangeToKicadFP_H

class BOARD;
class FOOTPRINT;
class LCFPChangeToKicadFP
{
    //立创专业版EDA封装转Kicad封装
public:
    LCFPChangeToKicadFP(BOARD *board);
    ~LCFPChangeToKicadFP();

public:
    //解析立创EDA专业版封装文件，并存储数据至Kicad数据结构
    int importLCFP( std::string strInFileFullPath ,int fileType=0);

   //导出Kicad封装文件
    int exportKicadFP( std::string strOutFileFullPath );

private:
    //按行读取立创封装文件
    int readLCFPFileByLine( std::string strInFileFullPath, std::vector<std::string>& vecLCFPLines );

    //按行读取立创封装文件
    int readLCFPFileByLineFromJson( std::string strInFileFullPath,std::vector<std::string>& vecLCFPLines );

    //解析行数据几何
    int parseLines( std::vector<std::string> vecLCFPLines );

    //解析PAD
    int parsePad( std::vector<std::string> vecStr );

    //解析POLY
    int parsePOLY( std::vector<std::string> vecStr );

    //解析FILL
    int parseFILL( std::vector<std::string> vecStr );

    //解析STRING
    int parseSTRING( std::vector<std::string> vecStr );

    //解析REGION
    int parseREGION( std::vector<std::string> vecStr );

    //解析ATTR
    int parseATTR( std::vector<std::string> vecStr );

    //字符串分割
    std::vector<std::string> Split( std::string strContext, std::string StrDelimiter );
    
    //判断字符串是否全为数字
    bool CheckStringIsNumbers( std::string str );
    
    //去除vector容器内单个元素的前后双引号
    void vecStrAnalyse( std::vector<std::string> vecStr);

    /// <summary>
    /// 字符串批量替换
    /// </summary>
    /// <param name="str">输入的文本</param>
    /// <param name="a">目标文本</param>
    /// <param name="b">替换内容</param>
    /// <returns>替换好的文本</returns>
    std::string spp( std::string str, std::string a, std::string b );

    //立创专业版EDA封装层ID转换为KiCad的封装层ID
    int LCLayerIDToKicadLayerID( std::string layerID );

public:
    //单个立创EDA专业版封装文件路径
    std::string m_LCFPSingleFullPath;

    //Kicad封装结构指针
    FOOTPRINT* m_KicadFPObj;

    //Board
    BOARD* m_board;

private:

};



// GeometryUVExt.h: 二维几何算法封装类.
// by sxl 2019/12/18
//////////////////////////////////////////////////////////////////////

#pragma once

//精度相关常量
static const double Epsilon_High = 1E-5;
static const double Epsilon_Medium = 5E-4;
static const double Epsilon_Low = 1E-2;
static const double Epsilon = Epsilon_Low;
static const double Epsilon_5Percent = 0.05;

//圆周率相关常量
static const double       _PI = double( 3.141592653589793238462643383279500 );
static const double       _PI2 = double( 6.283185307179586476925286766559000 );
static const double       _PIDiv180 = double( 0.017453292519943295769236907684886 );
static const double       _180DivPI = double( 57.295779513082320876798154814105000 );
static const double       _HALFPI = _PI * 0.5;
static const double       _ONEHALFPI = _PI * 1.5;
static const long         Degree360 = 360;
static const double       Invalid_NCValue = -1000000;
static const unsigned int TrigTableSize = 360;
static const double       DefaultOffset = 99888.55;
static const double       INF = 1E200;
static const double       EP = 1E-10;
static const int          MAXV = 300;

//缺省尺寸信息
static const double InvalidePercentValue = -1;

//标准平面类型
enum PlaneType
{
    PlaneType_Unknown = -1,
    PlaneType_XY = 0,
    PlaneType_YZ,
    PlaneType_ZX,
};


//环的顺逆时针
enum CircularDirection
{
    Clockwise = 0,
    Counterclockwise,
    UnknownDirection,
};

//角度弧度转换
inline static double Degree2Radians( double degree )
{
    double nInteger = 0;
    double y = modf( degree, &nInteger );
    long   nDegree = (long) nInteger;
    nDegree %= Degree360;
    return ( y + nDegree ) * _PIDiv180;
}
inline static double Radians2Degree( double radians )
{
    return radians * _180DivPI;
}

inline static void RoundDegreeAngle( double& angleDegree )
{
    while( angleDegree < 0 )
    {
        angleDegree += 360;
    }
    double nInteger = 0;
    double y = modf( angleDegree, &nInteger );
    long   nDegree = (long) nInteger;
    nDegree %= Degree360;
    angleDegree = ( y + nDegree );
}

inline static double RoundDegreeAngle2( double angle )
{
    double angleDegree = angle;
    while( angleDegree > 360 )
    {
        angleDegree -= 360;
    }
    while( angleDegree < 0 )
    {
        angleDegree += 360;
    }
    return angleDegree;
}

//环之间关系类型
enum LoopLoopType
{
    LoopLoop_OutSide = 0,
    LoopLoop_OutSideTangent,
    LoopLoop_Intersect,
    LoopLoop_InsideTangent,
    LoopLoop_Inside,
    LoopLoop_OuterInclude, //反包围
    LoopLoop_Unknown = 100,
};

class GeometryUVExt
{
public:
    static GeometryUVExt& GetExt();
    ~GeometryUVExt();
    //点在直线上判断
    bool IsCollinear( double x1, double y1, double x2, double y2, double x3, double y3 );
    //计算精度设置
    double GetEpsilon() { return m_Epsilon; }
    void   SetEpsilon( double value ) { m_Epsilon = value; }

    //两点距离
    inline double Distance( double x1, double y1, double x2, double y2 )
    {
        return sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) );
    }
    inline double LazyDistance( double x1, double y1, double x2, double y2 )
    {
        return ( x2 - x1 ) * ( x2 - x1 ) + ( y2 - y1 ) * ( y2 - y1 );
    }
    bool IsPointInCircle( double px, double py, double cx, double cy, double radius )
    {
        return IsEqual( Distance( px, py, cx, cy ), radius );
    }
    //点p 在以线段l 为对角线的矩形内
    bool IsPointInLineRange( double checkU, double checkV, double su, double sv, double eu,
                             double ev )
    {
        return ( ( checkU - su ) * ( checkU - eu ) <= 0 )
               && ( ( checkV - sv ) * ( checkV - ev ) <= 0 );
    }

    //叉乘
    double CrossProduct( double su, double sv, double eu, double ev, double ou, double ov )
    {
        return ( ( su - ou ) * ( ev - ov ) - ( eu - ou ) * ( sv - ov ) );
    }
    //点乘
    double DotProduct( double su, double sv, double eu, double ev, double ou, double ov )
    {
        return ( ( su - ou ) * ( eu - ou ) + ( sv - ov ) * ( ev - ov ) );
    }

    //点在线段上判断
    bool IsPointOnLine( double checkU, double checkV, double sU, double sV, double eU, double eV )
    {
        if( fabs( CrossProduct( eU, eV, checkU, checkV, sU, sV ) ) > m_Epsilon )
        {
            return false;
        }
        bool hasEqualU = fabs( checkU - sU ) <= m_Epsilon || fabs( checkU - eU ) <= m_Epsilon;
        bool hasEqualV = fabs( checkV - sV ) <= m_Epsilon || fabs( checkV - eV ) <= m_Epsilon;
        bool inURange = ( checkU - sU ) * ( checkU - eU ) < 0;
        bool inVRange = ( checkV - sV ) * ( checkV - eV ) < 0;
        return ( hasEqualU || inURange ) && ( hasEqualV || inVRange ) ? true : false;
    }

    //点与直线的位置关系
    enum PointLineRelationship
    {
        PointLineRelationship_OnLine = 0,
        PointLineRelationship_Left,
        PointLineRelationship_Right,
    };
    int GetPointLineRelationship( double checkU, double checkV, double sU, double sV, double eU,
                                  double eV )
    {
        double result = ( eU - sU ) * ( checkV - sV ) - ( eV - sV ) * ( checkU - sU );
        if( IsEqual( result, 0 ) )
        {
            return PointLineRelationship_OnLine;
        }
        return result > 0 ? PointLineRelationship_Left : PointLineRelationship_Right;
    }

    //点与直线关系
    int GetPointLineRelation( double checkU, double checkV, double sU, double sV, double eU,
                              double eV )
    {
        double A = eV - sV;
        double B = sU - eU;
        double C = eU * sV - sU * eV;
        double D = A * checkU + B * checkV + C;
        if( IsEqual( D, 0 ) )
        {
            return 0;
        }
        else
        {
            return D > 0 ? 1 : -1;
        }
    }


    //将浮点数取指定位小数
    void double5( double& v );
    //角度判断
    bool          Is90Degree( double angle );
    bool          Is180Degree( double angle );
    bool          IsTimeOfDegree( double angle, int intDegree );
    static double RegulareDegree( double angle );
    double        GetDegreeAngle( double deltaX, double deltaY );
    double        GetDegreeAngle( double sx, double sy, double ex, double ey );
    double        GetABCLineDegreeAngle( double a, double b, double c );
    double        GetRadianAngle( double deltaX, double deltaY );
    double        GetRadianAngle( double sx, double sy, double ex, double ey );

    //获取夹角
    double GetIncludeAngle( double xs, double ys, double xb, double yb, double xe, double ye );
    //向量操作
    void GetVector( double length, double degreeAngle, double& deltaX, double& deltaY );
    void GetVector( int plane, double length, double degreeAngle, double& deltaX, double& deltaY,
                    double& deltaZ );

    //圆弧中点
    void MidPointOfArc( double cx, double cy, double r, double startAng, double endAng, double& mx,
                        double& my );
    void MidPointOfArc( int arcDir, double cx, double cy, double r, double startAng, double endAng,
                        double& mx, double& my );
    void MidPointOfArcEx( double sx, double sy, double ex, double ey, double ArcAngle, double& mx,
                        double& my );

    //直线扩展操作
    int  GetLineEndValueByLength( bool isXOK, double sx, double sy, double refValue, double length,
                                  double* pResult );
    bool GetLineEndValueByAngle( bool isXOK, double sx, double sy, double refValue, double angle,
                                 double& result );
    bool GetLineEndValueByEndPos( bool isXOK, double sx, double sy, double refValue, double ex,
                                  double ey, double& result );
    void GetLineEndValues( double sx, double sy, double length, double angle, double& ex,
                           double& ey );
    void GetLineEndingValues( bool isX1Y2, double refX, double refY, double length, double angle,
                              double& xResult, double& yResult );

    inline double HalSqr( double val ) { return val * val; }

    //几何变换
    void Rotate( double rotationAngle, double x, double y, double& nx, double& ny );
    void Rotate( double rotationAngle, double x, double y, double ox, double oy, double& nx,
                 double& ny );
    void FastRotate( const int rotationAngle, double x, double y, double& nx, double& ny );
    void FastRotate( const int rotationAngle, double x, double y, double ox, double oy, double& nx,
                     double& ny );
    void Translate( double x, double y, double dx, double dy, double& nx, double& ny )
    {
        nx = x + dx;
        ny = y + dy;
    }

    void Translate( double x, double y, double delta, double& nx, double& ny )
    {
        nx = x + delta;
        ny = y + delta;
    }

    void Scale( double x, double y, double dx, double dy, double& nx, double& ny )
    {
        nx = x * dx;
        ny = y * dy;
    }

    void Scale( double x, double y, double scale, double& nx, double& ny )
    {
        nx = x * scale;
        ny = y * scale;
    }
    void Mirror( double old, double& newV ) { newV = -old; }

    //相等判断
    bool   IsEqual( double v1, double v2 );
    bool   IsGreaterEqual( double v1, double v2 );
    bool   IsLessEqual( double v1, double v2 );
    bool   IsGreater( double v1, double v2 );
    bool   IsLess( double v1, double v2 );
    bool   NotEqual( double v1, double v2 );
    bool   IsEqualAngle( double angle1, double angle2 );
    bool   IsAngleInRange( double angle, double sAngle, double eAngle, int dir );
    double GetAnglePercent( double angle, double sAngle, double eAngle, int dir );
    bool   IsValueInRange( double boundeary1, double boundeary2, double checkV,
                           bool isIncludeBoundary )
    {
        if( ( checkV - boundeary1 ) * ( checkV - boundeary2 ) < 0 )
        {
            return true;
        }
        bool hasEqual = fabs( checkV - boundeary1 ) <= m_Epsilon
                        || fabs( checkV - boundeary2 ) <= m_Epsilon;
        return ( isIncludeBoundary && hasEqual );
    }

    double GetSin( int index );
    double GetCos( int index );
    double GetTan( int index );

    //射线相交判断
    bool TwoRayIntersect( double x1, double y1, double angle1, double x2, double y2, double angle2,
                          double& ix, double& iy );
    void RayToLongLine( double x0, double y0, double lineAngle, double& sx, double& sy, double& ex,
                        double& ey );
    void RayToABCLine( double x0, double y0, double lineAngle, double& a, double& b, double& c );
    void LineToABCLine( double sx, double sy, double ex, double ey, double& a, double& b,
                        double& c );
    void OffsetRay( double x0, double y0, double RayAngle, double distance, bool isLeft,
                    double& newX, double& newY );
    //直线圆弧关系
    int RayCircleIntersect( double x0, double y0, double lineAngle, double cx, double cy,
                            double radius, double* xIntersects, double* yIntersects );
    int LineCircleIntersect( double sx, double sy, double ex, double ey, double cx, double cy,
                             double radius, double* xIntersects, double* yIntersects );
    int LineCircleIntersect2( double sx, double sy, double ex, double ey, double cx, double cy,
                              double radius, double* xIntersects, double* yIntersects );
    int ABCLineCircleIntersect( double a, double b, double c, double cx, double cy, double radius,
                                double* xIntersects, double* yIntersects );
    int CircleCircleIntersect( double cx1, double cy1, double radius1, double cx2, double cy2,
                               double radius2, double* xIntersects, double* yIntersects );
    int LineCircleTangentPoints( double lx, double ly, double cx, double cy, double radius,
                                 double* xIntersects, double* yIntersects );
    //直线偏移
    void OffsetPointByRegPtLength( double x0, double y0, double refX, double refY, double length,
                                   double& newX, double& newY );
    void OffsetPointByAngleLength( double x0, double y0, double angle, double length, double& newX,
                                   double& newY );
    //圆与圆关系
    int Circle2CircleType( double cx1, double cy1, double radius1, double cx2, double cy2,
                           double radius2 );
    //两条直线关系
    bool Line2LineIntersectionPoint( double sx1, double sy1, double ex1, double ey1, double sx2,
                                     double sy2, double ex2, double ey2, double& ix, double& iy );
    //获取两条线段关系
    bool Segment2SegmentIntersect( double x1, double y1, double x2, double y2, double x3, double y3,
                                   double x4, double y4, double& ix, double& iy );
    //通过起始点终点半径获取圆心
    int GetArcCenters( double sx, double sy, double ex, double ey, double radius, double* xCenters,
                       double* yCenters );
    //获取与两条直线都相切的圆弧
    int Get2LineTagArc( double sx, double sy, double ex, double ey, double sx2, double sy2,
                        double ex2, double ey2, double radius, double& acx, double& acy,
                        double& asx, double& asy, double& aex, double& aey );
    //通过起始点终点夹角获取圆心
    int GetArcCentersBySweepAngle( double sx, double sy, double ex, double ey, double sweepAngle,
                                   double* xCenters, double* yCenters );
    //通过起始点终点起始角获取圆心
    int GetArcCentersByStartAngle( double sx, double sy, double ex, double ey,
                                   double startAngleWithXAxis, double* xCenters, double* yCenters,
                                   double& sweepAngle );

    /* > 0 is Contour Clockwise, < 0 is clockwise, =0 is on line*/
    inline int CheckPointPosOfLine( double sx, double sy, double ex, double ey, double mx,
                                    double my )
    {
        double ss = ( sx - mx ) * ( ey - my ) - ( sy - my ) * ( ex - mx );
        if( IsEqual( ss, 0 ) )
        {
            return 0;
        }
        else
        {
            return ss > 0 ? 1 : -1;
        }
    }
    //获取直线中点
    inline void LineMiddlePoint( double sx, double sy, double ex, double ey, double& mx,
                                 double& my )
    {
        mx = ( sx + ex ) * 0.5;
        my = ( sy + ey ) * 0.5;
    }
    //点到直线的距离
    double PointToLineDistance( double x, double y, double sx, double sy, double ex, double ey );
    inline double PointABCLineDistance( double x, double y, double a, double b, double c )
    {
        return ( fabs( a * x + b * y + c ) ) / sqrt( a * a + b * b );
    }
    void PointLinePerpendicular( double x, double y, double sx, double sy, double ex, double ey,
                                 double& nx, double& ny );

    //直线平移
    void OffsetLine( double& sx, double& sy, double& ex, double& ey, double dv, bool isLeftOffset );
    //点到直线的垂点
    inline void PointABCLinePerpendicular( double x, double y, double a, double b, double c,
                                           double& nx, double& ny )
    {
        nx = ( b * b * x - a * b * y - a * c ) / ( a * a + b * b );
        ny = ( a * a * y - a * b * x - b * c ) / ( a * a + b * b );
    }
    //点到射线的垂点
    void PointRayPerpendicular( double x, double y, double x0, double y0, double rayAngle,
                                double& nx, double& ny );
    //两直线是否平息
    bool IsParallel( double x1, double y1, double x2, double y2, double x3, double y3, double x4,
                     double y4 );
    //两直线是否垂直
    bool IsPerpendicular( double x1, double y1, double x2, double y2, double x3, double y3,
                          double x4, double y4 );
    //点到直线的投影
    inline void ProjectPointDelta( double srcx, double srcy, double destx, double desty, double t,
                                   double& nx, double& ny )
    {
        nx = srcx + t * ( destx - srcx );
        ny = srcy + t * ( desty - srcy );
    }

    inline void ProjectPoint( double srcx, double srcy, double destx, double desty, double dist,
                              double& nx, double& ny )
    {
        ProjectPointDelta( srcx, srcy, destx, desty, dist / Distance( srcx, srcy, destx, desty ),
                           nx, ny );
    }

    //点到线段的最近点
    inline void ClosestPointOnSegmentFromPoint( double x1, double y1, double x2, double y2,
                                                double px, double py, double& nx, double& ny )
    {
        double vx = x2 - x1;
        double vy = y2 - y1;
        double wx = px - x1;
        double wy = py - y1;

        double c1 = vx * wx + vy * wy;

        if( c1 <= double( 0.0 ) )
        {
            nx = x1;
            ny = y1;
            return;
        }

        double c2 = vx * vx + vy * vy;

        if( c2 <= c1 )
        {
            nx = x2;
            ny = y2;
            return;
        }

        double ratio = c1 / c2;

        nx = x1 + ratio * vx;
        ny = y1 + ratio * vy;
    }

    inline void ClosestPointOnLineFromPoint( double x1, double y1, double x2, double y2, double px,
                                             double py, double& nx, double& ny )
    {
        double vx = x2 - x1;
        double vy = y2 - y1;
        double wx = px - x1;
        double wy = py - y1;

        double c1 = vx * wx + vy * wy;
        double c2 = vx * vx + vy * vy;

        double ratio = c1 / c2;

        nx = x1 + ratio * vx;
        ny = y1 + ratio * vy;
    }

    // Factorial数列
    static double Factorial( int n )
    {
        double nResult = 1;
        while( n > 1 )
        {
            nResult *= n;
            --n;
        }
        return nResult;
    }

    //Combination数列
    static double Combination( int n, int i )
    {
        double n1 = Factorial( n );
        double n2 = Factorial( i );
        double n3 = Factorial( n - i );
        double nn = ( 1.0 * n1 ) / ( n2 * n3 );
        return ( (double) Factorial( n ) ) / ( (double) ( Factorial( i ) * Factorial( n - i ) ) );
    }

    //阶乘
    static void GetCnk( int n, int* c )
    {
        int i = 0, k = 0;
        for( k = 0; k <= n; ++k )
        {
            c[k] = 1;
            for( i = n; i >= k + 1; --i )
            {
                c[k] = c[k] * i;
            }
            for( i = n - k; i >= 2; --i )
            {
                c[k] = c[k] / i;
            }
        }
    }
    //N次方
    static double NSquare( double u, int n )
    {
        double sum = 1.0;
        int    nn = n;
        while( nn > 0 )
        {
            sum *= u;
            --nn;
        }
        return sum;
    }

private:
    GeometryUVExt();
    double  m_Epsilon;
    double* SinTable;
    double* CosTable;
    double* TanTable;
};

class EpsilonState
{
public:
    EpsilonState( double setEpsilon );
    ~EpsilonState();

private:
    double         m_LastEpsilon;
    GeometryUVExt& m_Ext;
};



#endif LCFPChangeToKicadFP_H

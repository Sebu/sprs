#ifndef MATH_H
#define MATH_H

#include <iostream>
#include <vector>
#include <opencv/cv.h>
#include <math.h>

#define PI 3.1415926f


template<int dimension, typename T>
struct Vector{

    T m_v[dimension];

    Vector();

    Vector(T x, T y){
        m_v[0]=x;
        m_v[1]=y;
    }

    Vector(T x, T y, T z, T w){
        m_v[0]=x;
        m_v[1]=y;
        m_v[2]=z;
        m_v[3]=w;
    }

    operator cv::Point() { return cv::Point(m_v[0], m_v[1]); }

    void normalize();
    T length();


};


template<int dimension, typename T>
Vector<dimension,T>::Vector(){
    for (int i=0; i<dimension; i++)
        m_v[i] = 0.0f;
}


template<int dimension, typename T>
inline void Vector<dimension, T>::normalize()
{
    float length = this->length();
    for (int i=0; i<dimension; i++)
        m_v[i] /= length;
}

template<int dimension, typename T>
inline T Vector<dimension,T>::length()
{
    T result = 0;
    for (int i=0; i<dimension; i++)
        result += m_v[i]*m_v[i];
    return sqrt( result );
}

template<int dimension, typename T>
inline T operator*(const Vector<dimension,T>& lhs, const Vector<dimension,T>& rhs)
{
    T result = 0;
    for (int i=0; i<dimension; i++)
        result += lhs.m_v[i]*rhs.m_v[i];
    return result;
}

template<int dimension, typename T>
inline Vector<dimension,T> operator*(const Vector<dimension,T>& lhs, const T rhs)
{
    Vector<dimension,T> result;
    for (int i=0; i<dimension; i++)
        result.m_v[i] = lhs.m_v[i]*rhs;
    return result;
}

template<int dimension, typename T>
inline Vector<dimension,T> operator+(const Vector<dimension,T>& lhs, const Vector<dimension,T>& rhs)
{
    Vector<dimension,T> result;
    for (int i=0; i<dimension; i++)
        result.m_v[i] = lhs.m_v[i]+rhs.m_v[i];
    return result;
}

template<int dimension, typename T>
inline Vector<dimension,T> operator-(const Vector<dimension,T>& lhs, const Vector<dimension,T>& rhs)
{
    Vector<dimension,T> result;
    for (int i=0; i<dimension; i++)
        result.m_v[i] = lhs.m_v[i]-rhs.m_v[i];
    return result;
}


struct Matrix4 {

    union {
        float m[16];
        struct {
            float	m11, m12, m13, m14,
            m21, m22, m23, m24,
            m31, m32, m33, m34,
            m41, m42, m43, m44;
        };
        float mxm[4][4];
    };

    Matrix4 inverse();

    void scaleUniform(float s){
        m11 *= s;
        m22 *= s;
        m33 *= s;
    }

    void rotateZ(float a){
        m11 = m22 = cos(a);
        m21 = -sin(a);
        m12 = -m21;
        m33 = 1.0f;

    }

    void translate(float x, float y, float z){
        m41 += x; m42 += y; m43 += z;
    }

    static const Matrix4 Identity;

    Matrix4(float _m11, float _m12, float _m13, float _m14,
            float _m21,	float _m22, float _m23, float _m24,
            float _m31, float _m32, float _m33, float _m34,
            float _m41, float _m42, float _m43, float _m44) {
        m11 = _m11; m12 = _m12; m13 = _m13; m14 = _m14;
        m21 = _m21; m22 = _m22; m23 = _m23; m24 = _m24;
        m31 = _m31; m32 = _m32; m33 = _m33; m34 = _m34;
        m41 = _m41; m42 = _m42; m43 = _m43; m44 = _m44;
    };


    void identity() {
        m11 = m22 = m33 = m44 = 1.0f;
        m12 = m13 = m14 = m21 = m23 = m24 = m31 = m32 = m34 = m41 = m42 = m43 = 0.0f;
    };
    Matrix4() {
        m11 = m22 = m33 = m44 = m12 = m13 = m14 = m21 = m23 = m24 = m31 = m32 = m34 = m41 = m42 = m43 = 0.0f;
    };

};

typedef Vector<2,float> Vector2f;
typedef Vector<4,float> Vector4f;



inline Vector4f operator*(const Matrix4& lhs, const Vector4f& rhs)
{
    Vector4f m;
    for (int r=0;r<4;r++) {
        m.m_v[r] = 0.0f;
        for (int c=0;c<4;c++) {
            m.m_v[r]+=lhs.mxm[c][r]*rhs.m_v[c];
        }
    }
    return m;
}



inline Matrix4 operator*(const Matrix4& lhs, const Matrix4& rhs)
{
    Matrix4 m;
    for (int r=0;r<4;r++) {
        for (int c=0;c<4;c++) {
            for (int i=0;i<4;i++) {
                m.mxm[r][c]+=lhs.mxm[r][i]*rhs.mxm[i][c];
            }
        }
    }
    return m;
};


class AABB {

public:
    Vector2f min;
    Vector2f max;

    AABB(): min(Vector2f(FLT_MAX,FLT_MAX)), max(Vector2f(0,0)) {

    }

    inline float width() {
       return max.m_v[0]-min.m_v[0];
    }

    inline float height() {
        return max.m_v[1]-min.m_v[1];
    }

    inline float area() {
        return width()*height();
    }

    inline bool intersect(AABB& box) {
        if (box.min.m_v[0]>max.m_v[0] &&  box.min.m_v[1]>max.m_v[1]) return false;
        if (box.max.m_v[0]<min.m_v[0] &&  box.max.m_v[1]<min.m_v[1]) return false;
        return true;
    }
};

class Polygon {
public:
    std::vector<Vector2f> verts;

    static Polygon Square(float x, float y, float w, float h) {
        Polygon p;
        p.verts.push_back(Vector2f(x,y));
        p.verts.push_back(Vector2f(x+w-1,y));
        p.verts.push_back(Vector2f(x+w-1,y+h-1));
        p.verts.push_back(Vector2f(x,y+h-1));
        return p;
    }

    inline bool isInFrontOf(const Vector2f& point, const Vector2f& dir) {
        bool front = false, back = false;
        for(uint i=0; i<verts.size(); i++) {
            float t = dir*(verts[i]-point);
            if(t > 0) front=true;
            else if(t < 0) back=true;
            if(front && back) return false;
        }
        return front;
    }

    inline bool isInFrontOf(Polygon& rhs) {
        for(uint i=0,j=verts.size()-1; i<verts.size(); j=i, i++) {
            Vector2f tmp = verts[i] - verts[j];
            Vector2f perp(tmp.m_v[1], -tmp.m_v[0]);
            if( rhs.isInFrontOf(verts[j], perp ) ) return true;
        }
        return false;
    }


    inline bool intersects(Polygon& rhs) {
        if (rhs.isInFrontOf(*this) || isInFrontOf(rhs)) return false;
        return true;
    }

    AABB getBox();

};



#endif // MATH_H

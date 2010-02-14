#include "matrix.h"

const Matrix4 Matrix4::Identity(1.0f, 0.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f);


Matrix4 Matrix4::inverse()
{

    float t;
    int i, j, k, swap;
    Matrix4 tmp;

    Matrix4 id;
    id.identity();

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            tmp.mxm[i][j] = this->m[i*4+j];
        }
    }


    for (i = 0; i < 4; i++) {
        /* look for largest element in column. */
        swap = i;
        for (j = i + 1; j < 4; j++) {
            if (fabs(tmp.mxm[j][i]) > fabs(tmp.mxm[i][i])) {
                swap = j;
            }
        }

        if (swap != i) {
            /* swap rows. */
            for (k = 0; k < 4; k++) {
                t = tmp.mxm[i][k];
                tmp.mxm[i][k] = tmp.mxm[swap][k];
                tmp.mxm[swap][k] = t;

                t = id.m[i*4+k];
                id.m[i*4+k] = id.m[swap*4+k];
                id.m[swap*4+k] = t;
            }
        }

        if (tmp.mxm[i][i] == 0) {
            /* no non-zero pivot.  the matrix is singular, which
            shouldn't happen.  This means the user gave us a bad
            matrix. */
            return id;
        }

        t = tmp.mxm[i][i];
        for (k = 0; k < 4; k++) {
            tmp.mxm[i][k] /= t;
            id.m[i*4+k] /= t;
        }
        for (j = 0; j < 4; j++) {
            if (j != i) {
                t = tmp.mxm[j][i];
                for (k = 0; k < 4; k++) {
                    tmp.mxm[j][k] -= tmp.mxm[i][k]*t;
                    id.m[j*4+k] -= id.m[i*4+k]*t;
                }
            }
        }
    }
    return id;
}

bool Polygon::isInFrontOf(const Vector2f& point, const Vector2f& dir) {
    bool front = false, back = false;
    for(uint i=0; i<verts.size(); i++) {
        float t = dir*(verts[i]-point);
        if(t > 0) front=true;
        else if(t < 0) back=true;
        if(front && back) return false;
    }
    return front;
}

bool Polygon::isInFrontOf(Polygon& rhs) {
    for(uint i=0,j=verts.size()-1; i<verts.size(); j=i, i++) {
        Vector2f tmp = verts[i] - verts[j];
        Vector2f perp(tmp.m_v[1], -tmp.m_v[0]);
        if( rhs.isInFrontOf(verts[j], perp ) ) return true;
    }
    return false;
}


bool Polygon::intersects(Polygon& rhs) {
    if (rhs.isInFrontOf(*this) || isInFrontOf(rhs)) return false;
    return true;
}
AABB Polygon::getBox() {
    AABB box;
    for(uint i=0; i<verts.size(); i++) {
        Vector2f vert = verts[i];
        if(vert.m_v[0]<box.min.m_v[0]) box.min.m_v[0] = vert.m_v[0];
        if(vert.m_v[1]<box.min.m_v[1]) box.min.m_v[1] = vert.m_v[1];
        if(vert.m_v[0]>box.max.m_v[0]) box.max.m_v[0] = vert.m_v[0];
        if(vert.m_v[1]>box.max.m_v[1]) box.max.m_v[1] = vert.m_v[1];
    }
    return box;
}

